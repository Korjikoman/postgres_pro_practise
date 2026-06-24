#ifndef UNTITLED_HASH_MULTISET_H
#define UNTITLED_HASH_MULTISET_H
#include <stddef.h>

#include "hash_table.h"

/*
 * Multiset: один уникальный key может встречаться много раз.
 * unique_size берется из hash_table, total_size хранится отдельно.
 */

typedef struct hash_multiset {
    // table хранит key -> size_t*, total_count хранит сумму всех счетчиков.
    hash_table* table;
    size_t total_count;
} hash_multiset;



hash_multiset* hash_multiset_create(void);
void hash_multiset_destroy(hash_multiset* multiset);

bool hash_multiset_add(hash_multiset* multiset, const char* key);
bool hash_multiset_remove_one(hash_multiset* multiset, const char* key);
bool hash_multiset_remove_all(hash_multiset* multiset, const char* key);

size_t hash_multiset_count(hash_multiset* multiset, const char* key);
size_t hash_multiset_unique_size(hash_multiset* multiset);
size_t hash_multiset_total_size(hash_multiset* multiset);

hti hash_multiset_iterator(hash_multiset* multiset);
bool hash_multiset_next(hti* it);

#endif //UNTITLED_HASH_MULTISET_H
