#include "headers/hash_multiset.h"

/*
 * Multiset хранит количество повторений каждого ключа.
 * Базовая hash_table хранит key -> size_t*, а total_count хранит сумму всех count.
 */

hash_multiset* hash_multiset_create(void) {
    hash_multiset* multiset = malloc(sizeof(hash_multiset));
    if (multiset == NULL) {
        return NULL;
    }

    multiset->table = hash_table_create();
    if (multiset->table == NULL) {
        free(multiset);
        return NULL;
    }

    multiset->total_count = 0;

    return multiset;
}

void hash_multiset_destroy(hash_multiset* multiset) {
    if (multiset == NULL) {
        return;
    }

    hti it = ht_iterator(multiset->table);

    // Значения таблицы здесь являются malloc-счетчиками.
    while (hash_table_next(&it)) {
        free(it.value);
    }

    hash_table_destroy(multiset->table);
    free(multiset);
}

bool hash_multiset_add(hash_multiset* multiset, const char* key) {
    if (multiset == NULL || key == NULL) {
        return false;
    }

    size_t* count = hash_table_get(multiset->table, key);

    if (count != NULL) {
        (*count)++;
        multiset->total_count++;
        return true;
    }

    // Для нового ключа счетчик хранится как value в базовой таблице.
    count = malloc(sizeof(size_t));
    if (count == NULL) {
        return false;
    }

    *count = 1;

    if (hash_table_set(multiset->table, key, count) == NULL) {
        free(count);
        return false;
    }

    multiset->total_count++;

    return true;
}

bool hash_multiset_remove_one(hash_multiset* multiset, const char* key) {
    if (multiset == NULL || key == NULL) {
        return false;
    }

    size_t* count = hash_table_get(multiset->table, key);

    if (count == NULL) {
        return false;
    }

    if (*count > 1) {
        (*count)--;
        multiset->total_count--;
        return true;
    }

    // Последнее повторение удаляет ключ из базовой таблицы.
    void* removed = hash_table_delete(multiset->table, key);

    if (removed != NULL) {
        free(removed);
        multiset->total_count--;
        return true;
    }

    return false;
}
bool hash_multiset_remove_all(hash_multiset* multiset, const char* key) {
    if (multiset == NULL || key == NULL) {
        return false;
    }

    size_t* count = hash_table_delete(multiset->table, key);

    if (count == NULL) {
        return false;
    }

    // total_count хранит все повторы, поэтому вычитаем удаленный счетчик целиком.
    multiset->total_count -= *count;

    free(count);

    return true;
}


size_t hash_multiset_count(hash_multiset* multiset, const char* key) {
    if (multiset == NULL || key == NULL) {
        return 0;
    }

    size_t* count = hash_table_get(multiset->table, key);

    if (count == NULL) {
        return 0;
    }

    return *count;
}

size_t hash_multiset_unique_size(hash_multiset* multiset) {
    if (multiset == NULL) {
        return 0;
    }

    return ht_length(multiset->table);
}
size_t hash_multiset_total_size(hash_multiset* multiset) {
    if (multiset == NULL) {
        return 0;
    }

    return multiset->total_count;
}
hti hash_multiset_iterator(hash_multiset* multiset) {
    return ht_iterator(multiset->table);
}

bool hash_multiset_next(hti* it) {
    return hash_table_next(it);
}
