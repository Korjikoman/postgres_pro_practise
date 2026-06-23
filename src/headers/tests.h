#ifndef UNTITLED_TESTS_H
#define UNTITLED_TESTS_H

#include "hash_multiset.h"
#include "hash_set.h"

int* make_int(int value);
void free_hash_table_values(hash_table* table);
void test_create_empty_table(void);
void test_set_and_get(void);
void test_update_existing_key(void);
void test_delete_existing_key(void);
void test_delete_missing_key(void);
void test_delete_and_insert_again(void);
void test_expand_table(void);
void test_iterator(void);

void test_hash_set_add_and_contains(void);
void test_hash_set_no_duplicates(void);
void test_hash_set_remove(void);
void test_hash_set_iterator(void);

void test_hash_multiset_add_and_count(void);
void test_hash_multiset_remove_one(void);
void test_hash_multiset_remove_all(void);
void test_hash_multiset_remove_missing(void);
void test_hash_multiset_iterator(void);

void run_tests(void);

#endif //UNTITLED_TESTS_H
