#include "headers/lock_free_hash_table.h"

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FNV_OFFSET 14695981039346656037ULL
#define FNV_PRIME 1099511628211ULL

/*
 * Важная идея реализации:
 * слот таблицы хранит атомарный указатель на готовую cell.
 * Поток сначала полностью создает cell, потом публикует ее одним CAS.
 */

typedef struct lf_hash_cell {
    // key не меняется после публикации cell.
    char* key;

    // NULL означает логически удаленную запись.
    _Atomic(void*) value;
} lf_hash_cell;

typedef struct lf_hash_entry {
    // NULL: слот никогда не занимали. Не-NULL: слот навсегда занят этим key.
    _Atomic(lf_hash_cell*) cell;
} lf_hash_entry;

struct lock_free_hash_table {
    lf_hash_entry* entries;
    size_t capacity;

    // Активные записи: value != NULL.
    _Atomic(size_t) length;
};


// strdup не ISO C11, поэтому здесь ручная копия строки.
static char* lf_strdup(const char* key) {
    size_t length = strlen(key) + 1;
    char* copy = malloc(length);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, key, length);
    return copy;
}


static uint64_t lf_hash_func(const char* key) {
    uint64_t hash = FNV_OFFSET;

    for (const char* ptr = key; *ptr != '\0'; ptr++) {
        hash ^= (uint64_t)(unsigned char)(*ptr);
        hash *= FNV_PRIME;
    }

    return hash;
}

static size_t lf_next_index(size_t index, size_t capacity) {
    index++;

    if (index == capacity) {
        return 0;
    }

    return index;
}

static lf_hash_cell* lf_hash_cell_create(const char* key, void* value) {
    lf_hash_cell* cell = malloc(sizeof(lf_hash_cell));

    if (cell == NULL) {
        return NULL;
    }

    cell->key = lf_strdup(key);

    if (cell->key == NULL) {
        free(cell);
        return NULL;
    }

    // cell еще не видна другим потокам, но atomic-поле инициализируем явно.
    atomic_init(&cell->value, value);

    return cell;
}

static void lf_hash_cell_destroy(lf_hash_cell* cell) {
    if (cell == NULL) {
        return;
    }

    free(cell->key);
    free(cell);
}

lock_free_hash_table* lf_hash_table_create(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }

    lock_free_hash_table* table = malloc(sizeof(lock_free_hash_table));

    if (table == NULL) {
        return NULL;
    }

    table->entries = calloc(capacity, sizeof(lf_hash_entry));

    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    table->capacity = capacity;
    atomic_init(&table->length, 0);

    // Не полагаемся на calloc для atomic-объектов: инициализируем явно.
    for (size_t i = 0; i < capacity; i++) {
        atomic_init(&table->entries[i].cell, NULL);
    }

    return table;
}

void lf_hash_table_destroy(lock_free_hash_table* table) {
    if (table == NULL) {
        return;
    }

    // destroy вызывается только после остановки всех потоков-пользователей.
    for (size_t i = 0; i < table->capacity; i++) {
        lf_hash_cell* cell = atomic_load_explicit(
            &table->entries[i].cell,
            memory_order_relaxed
        );

        lf_hash_cell_destroy(cell);
    }

    free(table->entries);
    free(table);
}

bool lf_hash_table_set(lock_free_hash_table* table, const char* key, void* value) {
    if (table == NULL || key == NULL || value == NULL) {
        return false;
    }

    size_t index = (size_t)(lf_hash_func(key) % table->capacity);
    size_t checked_slots = 0;

    // Сначала ищем key: update должен работать даже при полной таблице.
    while (checked_slots < table->capacity) {
        lf_hash_entry* entry = &table->entries[index];

        // acquire синхронизируется с release-CAS, публикующим cell.
        lf_hash_cell* cell = atomic_load_explicit(
            &entry->cell,
            memory_order_acquire
        );

        if (cell == NULL) {
            // Пустой слот можно попытаться занять новым ключом.
            lf_hash_cell* new_cell = lf_hash_cell_create(key, value);

            if (new_cell == NULL) {
                return false;
            }

            lf_hash_cell* expected = NULL;

            // CAS занимает слот только если другой поток не занял его первым.
            if (atomic_compare_exchange_strong_explicit(
                    &entry->cell,
                    &expected,
                    new_cell,
                    memory_order_release,
                    memory_order_acquire
                )) {

                // length здесь только счетчик, не синхронизация памяти.
                atomic_fetch_add_explicit(&table->length, 1, memory_order_relaxed);
                return true;
            }

            // CAS не сработал: new_cell не опубликована, ее можно удалить.
            lf_hash_cell_destroy(new_cell);

            // Перечитываем этот же слот: туда мог попасть тот же key.
            continue;
        }

        if (strcmp(cell->key, key) == 0) {
            // Update или повторная активация после delete.
            void* old_value = atomic_exchange_explicit(
                &cell->value,
                value,
                memory_order_acq_rel
            );

            if (old_value == NULL) {
                // Был мертвым, стал активной записью.
                atomic_fetch_add_explicit(&table->length, 1, memory_order_relaxed);
            }

            return true;
        }

        // Коллизия: идем дальше по linear probing.
        index = lf_next_index(index, table->capacity);
        checked_slots++;
    }

    // Все слоты просмотрены: key нет и места для нового key тоже нет.
    return false;
}

void* lf_hash_table_get(lock_free_hash_table* table, const char* key) {
    if (table == NULL || key == NULL) {
        return NULL;
    }

    size_t index = (size_t)(lf_hash_func(key) % table->capacity);
    size_t checked_slots = 0;

    while (checked_slots < table->capacity) {
        lf_hash_entry* entry = &table->entries[index];
        lf_hash_cell* cell = atomic_load_explicit(
            &entry->cell,
            memory_order_acquire
        );

        if (cell == NULL) {
            // В linear probing первый пустой слот завершает поиск.
            return NULL;
        }

        if (strcmp(cell->key, key) == 0) {
            // value может параллельно измениться через set/delete.
            return atomic_load_explicit(&cell->value, memory_order_acquire);
        }

        index = lf_next_index(index, table->capacity);
        checked_slots++;
    }

    return NULL;
}

void* lf_hash_table_delete(lock_free_hash_table* table, const char* key) {
    if (table == NULL || key == NULL) {
        return NULL;
    }

    size_t index = (size_t)(lf_hash_func(key) % table->capacity);
    size_t checked_slots = 0;

    while (checked_slots < table->capacity) {
        lf_hash_entry* entry = &table->entries[index];
        lf_hash_cell* cell = atomic_load_explicit(
            &entry->cell,
            memory_order_acquire
        );

        if (cell == NULL) {
            // Пустой слот означает, что такого key в этой probing-цепочке нет.
            return NULL;
        }

        if (strcmp(cell->key, key) == 0) {
            // Логическое удаление: key оставляем, value меняем на NULL.
            void* old_value = atomic_exchange_explicit(
                &cell->value,
                NULL,
                memory_order_acq_rel
            );

            if (old_value != NULL) {
                atomic_fetch_sub_explicit(&table->length, 1, memory_order_relaxed);
            }

            return old_value;
        }

        index = lf_next_index(index, table->capacity);
        checked_slots++;
    }

    return NULL;
}

size_t lf_hash_table_length(lock_free_hash_table* table) {
    if (table == NULL) {
        return 0;
    }

    return atomic_load_explicit(&table->length, memory_order_relaxed);
}

size_t lf_hash_table_capacity(lock_free_hash_table* table) {
    if (table == NULL) {
        return 0;
    }

    return table->capacity;
}
