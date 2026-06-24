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
#include "headers/lock_free_hash_table.h"
#include "headers/tests.h"

void exit_nomem(void) {
    fprintf(stderr, "out of memory\n");
    exit(1);
}

int main(void) {
    run_tests();

    printf("DEMO 1: HASH TABLE\n");

    // Обычная таблица копирует ключи, но int* значения освобождаем сами.
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

    // Set хранит только уникальные строки; повторные add не меняют размер.
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

    // unique_size и total_size могут отличаться.
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

    printf("\nDEMO 4: LOCK-FREE HASH TABLE\n");

    // Lock-free таблица фиксированного размера, новый key может не поместиться.
    lock_free_hash_table* lf_table = lf_hash_table_create(2);
    if (lf_table == NULL) {
        exit_nomem();
    }

    int* lf_workers = make_int(4);
    int* lf_jobs = make_int(128);
    int* lf_extra = make_int(999);

    if (!lf_hash_table_set(lf_table, "workers", lf_workers)) {
        exit_nomem();
    }

    if (!lf_hash_table_set(lf_table, "jobs", lf_jobs)) {
        exit_nomem();
    }

    printf("After insert: length=%zu, capacity=%zu\n",
           lf_hash_table_length(lf_table),
           lf_hash_table_capacity(lf_table));

    if (!lf_hash_table_set(lf_table, "extra", lf_extra)) {
        printf("Cannot insert extra: fixed-capacity table is full\n");
        free(lf_extra);
    }

    int* new_workers = make_int(8);

    // Update существующего key должен работать даже когда таблица уже полная.
    if (!lf_hash_table_set(lf_table, "workers", new_workers)) {
        free(new_workers);
        exit_nomem();
    }

    free(lf_workers);
    lf_workers = new_workers;

    printf("Updated workers while table is full: %d\n",
           *(int*)lf_hash_table_get(lf_table, "workers"));

    int* deleted_jobs = lf_hash_table_delete(lf_table, "jobs");
    if (deleted_jobs != NULL) {
        printf("Deleted jobs: %d\n", *deleted_jobs);
        free(deleted_jobs);
        lf_jobs = NULL;
    }

    printf("After delete: length=%zu, jobs=%p\n",
           lf_hash_table_length(lf_table),
           lf_hash_table_get(lf_table, "jobs"));

    lf_jobs = make_int(256);

    // Повторная вставка того же key оживляет логически удаленную запись.
    if (!lf_hash_table_set(lf_table, "jobs", lf_jobs)) {
        free(lf_jobs);
        exit_nomem();
    }

    printf("Reactivated jobs: %d\n",
           *(int*)lf_hash_table_get(lf_table, "jobs"));
    printf("Final lock-free table length: %zu\n",
           lf_hash_table_length(lf_table));

    lf_hash_table_destroy(lf_table);
    free(lf_workers);
    free(lf_jobs);

    return 0;
}
