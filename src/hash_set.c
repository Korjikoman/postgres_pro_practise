#include "headers/hash_set.h"

static char HASH_SET_PRESENT_VALUE = 1;
#define HASH_SET_PRESENT (&HASH_SET_PRESENT_VALUE)





hash_set* hash_set_create(void) {
    hash_set* set = malloc(sizeof(hash_set));
    if (set == NULL) {
        return NULL;
    }

    set->table = hash_table_create();
    if (set->table == NULL) {
        free(set);
        return NULL;
    }

    return set;
}

void hash_set_destroy(hash_set* set) {
    if (set == NULL) {
        return;
    }

    hash_table_destroy(set->table);
    free(set);
}

bool hash_set_add(hash_set* set, const char* key) {
    if (set == NULL || key == NULL) {
        return false;
    }
    if (hash_set_contains(set, key)) {
        return false;
    }

    return hash_table_set(set->table, key, HASH_SET_PRESENT) != NULL;
}
bool hash_set_contains(hash_set* set, const char* key) {
    if (set == NULL || key == NULL) {
        return false;
    }

    return hash_table_get(set->table, key) != NULL;
}

bool hash_set_remove(hash_set* set, const char* key) {
    if (set == NULL || key == NULL) {
        return false;
    }

    void* removed = hash_table_delete(set->table, key);

    return removed != NULL;
}

size_t hash_set_size(hash_set* set) {
    if (set == NULL) {
        return 0;
    }

    return ht_length(set->table);
}

hti hash_set_iterator(hash_set* set) {
    return ht_iterator(set->table);
}

bool hash_set_next(hti* it) {
    return hash_table_next(it);
}