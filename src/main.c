#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "headers/hash_multiset.h"
#include "headers/hash_set.h"
#include "headers/hash_table.h"
#include "headers/tests.h"


void exit_nomem(void) {
    fprintf(stderr, "out of memory\n");
    exit(1);
}

int main(void) {
    run_tests();

    printf("DEMO 1: HASH TABLE\n");

    hash_table* table = hash_table_create();
    if (table == NULL) {
        exit_nomem();
    }

    int* age = make_int(18);
    int* score = make_int(95);
    int* level = make_int(3);

    if (hash_table_set(table, "age", age) == NULL) {
        exit_nomem();
    }

    if (hash_table_set(table, "score", score) == NULL) {
        exit_nomem();
    }

    if (hash_table_set(table, "level", level) == NULL) {
        exit_nomem();
    }

    printf("After insert:\n");

    hti table_it = ht_iterator(table);
    while (hash_table_next(&table_it)) {
        printf("%s -> %d\n", table_it.key, *(int*)table_it.value);
    }

    printf("Hash table length: %d\n", (int)ht_length(table));

    int* found_score = hash_table_get(table, "score");
    if (found_score != NULL) {
        printf("Found score: %d\n", *found_score);
    }

    int* old_score = hash_table_get(table, "score");
    int* new_score = make_int(100);

    if (hash_table_set(table, "score", new_score) == NULL) {
        free(new_score);
        exit_nomem();
    }

    free(old_score);

    printf("After updating score:\n");

    table_it = ht_iterator(table);
    while (hash_table_next(&table_it)) {
        printf("%s -> %d\n", table_it.key, *(int*)table_it.value);
    }

    int* deleted_level = hash_table_delete(table, "level");
    if (deleted_level != NULL) {
        printf("Deleted level: %d\n", *deleted_level);
        free(deleted_level);
    }

    printf("After deleting level:\n");

    table_it = ht_iterator(table);
    while (hash_table_next(&table_it)) {
        printf("%s -> %d\n", table_it.key, *(int*)table_it.value);
    }

    printf("Hash table length: %d\n", (int)ht_length(table));

    free_hash_table_values(table);
    hash_table_destroy(table);


    printf("\nDEMO 2: HASH SET\n");

    hash_set* set = hash_set_create();
    if (set == NULL) {
        exit_nomem();
    }

    hash_set_add(set, "cat");
    hash_set_add(set, "dog");
    hash_set_add(set, "cat");
    hash_set_add(set, "bird");
    hash_set_add(set, "dog");

    printf("Set elements:\n");

    hti set_it = hash_set_iterator(set);
    while (hash_set_next(&set_it)) {
        printf("%s\n", set_it.key);
    }

    printf("Set size: %d\n", (int)hash_set_size(set));

    if (hash_set_contains(set, "cat")) {
        printf("cat is in set\n");
    }

    if (!hash_set_contains(set, "fish")) {
        printf("fish is not in set\n");
    }

    hash_set_remove(set, "dog");

    printf("After removing dog:\n");

    set_it = hash_set_iterator(set);
    while (hash_set_next(&set_it)) {
        printf("%s\n", set_it.key);
    }

    printf("Set size: %d\n", (int)hash_set_size(set));

    hash_set_destroy(set);


    printf("\nDEMO 3: HASH MULTISET\n");

    hash_multiset* multiset = hash_multiset_create();
    if (multiset == NULL) {
        exit_nomem();
    }

    hash_multiset_add(multiset, "cat");
    hash_multiset_add(multiset, "dog");
    hash_multiset_add(multiset, "cat");
    hash_multiset_add(multiset, "bird");
    hash_multiset_add(multiset, "dog");
    hash_multiset_add(multiset, "cat");

    printf("Multiset elements:\n");

    hti multiset_it = hash_multiset_iterator(multiset);
    while (hash_multiset_next(&multiset_it)) {
        size_t* count = (size_t*)multiset_it.value;
        printf("%s -> %zu\n", multiset_it.key, *count);
    }

    printf("cat count: %zu\n", hash_multiset_count(multiset, "cat"));
    printf("dog count: %zu\n", hash_multiset_count(multiset, "dog"));
    printf("bird count: %zu\n", hash_multiset_count(multiset, "bird"));
    printf("fish count: %zu\n", hash_multiset_count(multiset, "fish"));

    printf("Unique elements: %zu\n", hash_multiset_unique_size(multiset));
    printf("Total elements: %zu\n", hash_multiset_total_size(multiset));

    hash_multiset_remove_one(multiset, "cat");

    printf("After removing one cat:\n");

    multiset_it = hash_multiset_iterator(multiset);
    while (hash_multiset_next(&multiset_it)) {
        size_t* count = (size_t*)multiset_it.value;
        printf("%s -> %zu\n", multiset_it.key, *count);
    }

    printf("cat count: %zu\n", hash_multiset_count(multiset, "cat"));
    printf("Unique elements: %zu\n", hash_multiset_unique_size(multiset));
    printf("Total elements: %zu\n", hash_multiset_total_size(multiset));

    hash_multiset_remove_all(multiset, "dog");

    printf("After removing all dog:\n");

    multiset_it = hash_multiset_iterator(multiset);
    while (hash_multiset_next(&multiset_it)) {
        size_t* count = (size_t*)multiset_it.value;
        printf("%s -> %zu\n", multiset_it.key, *count);
    }

    printf("dog count: %zu\n", hash_multiset_count(multiset, "dog"));
    printf("Unique elements: %zu\n", hash_multiset_unique_size(multiset));
    printf("Total elements: %zu\n", hash_multiset_total_size(multiset));

    hash_multiset_destroy(multiset);

    return 0;
}