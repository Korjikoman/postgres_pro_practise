#ifndef UNTITLED_TESTS_H
#define UNTITLED_TESTS_H

#include "hash_multiset.h"
#include "hash_set.h"
#include "lock_free_hash_table.h"

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

void test_lock_free_hash_table_create(void);
void test_lock_free_hash_table_set_get_update(void);
void test_lock_free_hash_table_update_when_full(void);
void test_lock_free_hash_table_delete_and_reactivate(void);
void test_lock_free_hash_table_deleted_slot_keeps_key(void);

void run_tests(void);

#endif //UNTITLED_TESTS_H
