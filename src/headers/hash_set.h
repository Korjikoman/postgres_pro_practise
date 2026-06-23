#ifndef UNTITLED_HASH_SET_H
#define UNTITLED_HASH_SET_H
#include "hash_table.h"


typedef struct hash_set {
    hash_table* table;
} hash_set;



hash_set* hash_set_create(void);
void hash_set_destroy(hash_set* set);

bool hash_set_add(hash_set* set, const char* key);
bool hash_set_contains(hash_set* set, const char* key);
bool hash_set_remove(hash_set* set, const char* key);
size_t hash_set_size(hash_set* set);

hti hash_set_iterator(hash_set* set);
bool hash_set_next(hti* it);


#endif //UNTITLED_HASH_SET_H
