#include "headers/hash_table.h"

/*
 * Базовая хеш-таблица проекта.
 *
 * Коллизии решаются открытой адресацией и linear probing:
 * если слот занят чужим ключом, идем в следующий слот массива.
 *
 * Владение памятью:
 * - таблица копирует и освобождает key;
 * - value принадлежит вызывающему коду, поэтому destroy его не освобождает.
 */

static const char HT_DELETED_MARKER = '\0';
// Tombstone: слот удален, но probing через него должен продолжаться.
#define HT_DELETED (&HT_DELETED_MARKER)
#define INITIAL_CAPACITY 16
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL


hash_table * hash_table_create(void) {
    hash_table* table = malloc(sizeof(hash_table));
    if (table == NULL) return NULL;

    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    table->entries = calloc(table->capacity, sizeof(entry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }
    return table;
}

void hash_table_destroy(hash_table* table) {
    // Освобождаем только ключи: значения могут быть любыми внешними объектами.
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL &&
            table->entries[i].key != HT_DELETED) {

            free((void*)table->entries[i].key);
            }
    }

    free(table->entries);
    free(table);
}


bool hash_table_next(hti* it) {
    hash_table* table = it->hash_table;

    // Итератор пропускает пустые и удаленные слоты.
    while (it->index < table->capacity) {
        size_t i = it->index;
        it->index++;

        if (table->entries[i].key != NULL &&
            table->entries[i].key != HT_DELETED) {

            entry e = table->entries[i];
            it->key = e.key;
            it->value = e.value;
            return true;
            }
    }

    return false;
}

void* hash_table_get(hash_table* table, const char* key) {
    uint64_t hash = hash_func(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));
    size_t start_index = index;

    // NULL заканчивает цепочку поиска, HT_DELETED просто пропускаем.
    while (table->entries[index].key != NULL) {
        if (table->entries[index].key != HT_DELETED &&
            strcmp(key, table->entries[index].key) == 0) {

            return table->entries[index].value;
            }

        index++;
        if (index >= table->capacity) {
            index = 0;
        }

        if (index == start_index) {
            return NULL;
        }
    }

    return NULL;
}

void* hash_table_delete(hash_table* table, const char* key) {
    uint64_t hash = hash_func(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));
    size_t start_index = index;

    // Удаление оставляет tombstone, иначе сломается цепочка probing.
    while (table->entries[index].key != NULL) {
        if (table->entries[index].key != HT_DELETED &&
            strcmp(key, table->entries[index].key) == 0) {

            void* value = table->entries[index].value;

            free((void*)table->entries[index].key);

            table->entries[index].key = HT_DELETED;
            table->entries[index].value = NULL;

            table->length--;

            return value;
            }

        index++;
        if (index >= table->capacity) {
            index = 0;
        }

        if (index == start_index) {
            return NULL;
        }
    }

    return NULL;
}


void* hash_table_set(hash_table* table, const char * key, void * value) {
    assert( value != NULL);
    if (value == NULL) {
        return NULL;
    }


    if (table->length >= table->capacity / 2) {
        if (!ht_expand(table)) {
            return NULL;
        }
    }
    return ht_set_entry(table->entries, table->capacity, key, value, &table->length);
}

// FNV-1a: простой стабильный хеш для строковых ключей.
static  uint64_t hash_func(const char * key) {
    uint64_t hash = FNV_OFFSET;
    for (const char * ptr = key; *ptr; ptr++) {
        hash ^= (uint64_t)(unsigned char)(*ptr);
        hash *= FNV_PRIME;
    }
    return hash;
}

/*
 * Общая вставка для обычного set и для переноса при expand.
 * plength != NULL: новый ключ копируется через strdup, length растет.
 * plength == NULL: переносим уже скопированный key без изменения length.
 */
static const char* ht_set_entry(entry* entries, size_t capacity,
        const char* key, void* value, size_t* plength) {

    uint64_t hash = hash_func(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));
    size_t start_index = index;

    size_t first_deleted = SIZE_MAX;

    // Tombstone можно переиспользовать, но сначала надо убедиться, что key нет дальше.
    while (entries[index].key != NULL) {
        if (entries[index].key == HT_DELETED) {
            if (first_deleted == SIZE_MAX) {
                first_deleted = index;
            }
        } else if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }

        index++;
        if (index >= capacity) {
            index = 0;
        }

        if (index == start_index) {
            break;
        }
    }

    if (first_deleted != SIZE_MAX) {
        index = first_deleted;
    }

    // При обычной вставке копируем ключ и увеличиваем длину таблицы
    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }

    entries[index].key = key;
    entries[index].value = value;

    return key;
}

bool ht_expand(hash_table* table) {
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;
    }

    entry* new_entries = calloc(new_capacity, sizeof(entry));
    if (new_entries == NULL) {
        return false;
    }

    for (size_t i = 0; i < table->capacity; i++) {
        entry e = table->entries[i];

        if (e.key != NULL && e.key != HT_DELETED) {
            ht_set_entry(new_entries, new_capacity, e.key, e.value, NULL);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

size_t ht_length(hash_table* hash_table) {
    return hash_table->length;
}

hti ht_iterator(hash_table* table) {
    hti it;
    it.hash_table = table;
    it.index = 0;
    return it;
}
