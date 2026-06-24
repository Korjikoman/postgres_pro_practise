#include "headers/tests.h"


// тесты для hash table
int* make_int(int value) {
    int* p = malloc(sizeof(int));
    assert(p != NULL);

    *p = value;
    return p;
}

void free_hash_table_values(hash_table* table) {
    hti it = ht_iterator(table);

    // Базовая таблица не владеет value, поэтому тесты освобождают их вручную.
    while (hash_table_next(&it)) {
        free(it.value);
    }
}

void test_create_empty_table(void) {
    hash_table* table = hash_table_create();

    assert(table != NULL);
    assert(ht_length(table) == 0);
    assert(hash_table_get(table, "missing") == NULL);

    hash_table_destroy(table);
}

void test_set_and_get(void) {
    hash_table* table = hash_table_create();

    int* value = make_int(10);

    assert(hash_table_set(table, "one", value) != NULL);
    assert(ht_length(table) == 1);

    int* found = hash_table_get(table, "one");
    assert(found != NULL);
    assert(*found == 10);

    assert(hash_table_get(table, "missing") == NULL);

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_update_existing_key(void) {
    hash_table* table = hash_table_create();

    int* old_value = make_int(10);
    int* new_value = make_int(20);

    assert(hash_table_set(table, "key", old_value) != NULL);
    assert(ht_length(table) == 1);

    assert(hash_table_set(table, "key", new_value) != NULL);
    assert(ht_length(table) == 1);

    free(old_value);

    int* found = hash_table_get(table, "key");
    assert(found != NULL);
    assert(*found == 20);

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_delete_existing_key(void) {
    hash_table* table = hash_table_create();

    int* a = make_int(1);
    int* b = make_int(2);

    assert(hash_table_set(table, "a", a) != NULL);
    assert(hash_table_set(table, "b", b) != NULL);
    assert(ht_length(table) == 2);

    int* deleted = hash_table_delete(table, "a");

    assert(deleted == a);
    assert(*deleted == 1);
    free(deleted);

    assert(ht_length(table) == 1);
    assert(hash_table_get(table, "a") == NULL);

    int* found_b = hash_table_get(table, "b");
    assert(found_b != NULL);
    assert(*found_b == 2);

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_delete_missing_key(void) {
    hash_table* table = hash_table_create();

    int* value = make_int(100);

    assert(hash_table_set(table, "exists", value) != NULL);
    assert(ht_length(table) == 1);

    void* deleted = hash_table_delete(table, "missing");

    assert(deleted == NULL);
    assert(ht_length(table) == 1);

    int* found = hash_table_get(table, "exists");
    assert(found != NULL);
    assert(*found == 100);

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_delete_and_insert_again(void) {
    hash_table* table = hash_table_create();

    int* old_value = make_int(5);

    assert(hash_table_set(table, "x", old_value) != NULL);
    assert(ht_length(table) == 1);

    int* deleted = hash_table_delete(table, "x");
    assert(deleted == old_value);
    free(deleted);

    assert(ht_length(table) == 0);
    assert(hash_table_get(table, "x") == NULL);

    int* new_value = make_int(50);

    assert(hash_table_set(table, "x", new_value) != NULL);
    assert(ht_length(table) == 1);

    int* found = hash_table_get(table, "x");
    assert(found != NULL);
    assert(*found == 50);

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_expand_table(void) {
    hash_table* table = hash_table_create();

    // 100 элементов заставляют таблицу несколько раз расшириться.
    for (int i = 0; i < 100; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);

        int* value = make_int(i);

        assert(hash_table_set(table, key, value) != NULL);
    }

    assert(ht_length(table) == 100);

    for (int i = 0; i < 100; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);

        int* found = hash_table_get(table, key);

        assert(found != NULL);
        assert(*found == i);
    }

    free_hash_table_values(table);
    hash_table_destroy(table);
}

void test_iterator(void) {
    hash_table* table = hash_table_create();

    assert(hash_table_set(table, "a", make_int(1)) != NULL);
    assert(hash_table_set(table, "b", make_int(2)) != NULL);
    assert(hash_table_set(table, "c", make_int(3)) != NULL);

    size_t count = 0;

    hti it = ht_iterator(table);
    while (hash_table_next(&it)) {
        assert(it.key != NULL);
        assert(it.value != NULL);
        count++;
    }

    assert(count == 3);

    free_hash_table_values(table);
    hash_table_destroy(table);
}



// тесты для hash_set
void test_hash_set_add_and_contains(void) {
    hash_set* set = hash_set_create();

    assert(set != NULL);

    assert(hash_set_size(set) == 0);

    assert(hash_set_add(set, "apple") == true);
    assert(hash_set_contains(set, "apple") == true);
    assert(hash_set_size(set) == 1);

    assert(hash_set_contains(set, "banana") == false);

    hash_set_destroy(set);
}

void test_hash_set_no_duplicates(void) {
    hash_set* set = hash_set_create();

    assert(set != NULL);

    // Set должен оставить один ключ, даже если add вызвать несколько раз.
    assert(hash_set_add(set, "cat") == true);
    assert(hash_set_add(set, "cat") == false);
    assert(hash_set_add(set, "cat") == false);

    assert(hash_set_size(set) == 1);
    assert(hash_set_contains(set, "cat") == true);

    hash_set_destroy(set);
}
void test_hash_set_remove(void) {
    hash_set* set = hash_set_create();

    assert(set != NULL);

    assert(hash_set_add(set, "one") == true);
    assert(hash_set_add(set, "two") == true);

    assert(hash_set_size(set) == 2);

    assert(hash_set_remove(set, "one") == true);
    assert(hash_set_contains(set, "one") == false);
    assert(hash_set_contains(set, "two") == true);
    assert(hash_set_size(set) == 1);

    assert(hash_set_remove(set, "missing") == false);

    hash_set_destroy(set);
}

void test_hash_set_iterator(void) {
    hash_set* set = hash_set_create();

    assert(set != NULL);

    assert(hash_set_add(set, "a") == true);
    assert(hash_set_add(set, "b") == true);
    assert(hash_set_add(set, "c") == true);

    size_t count = 0;

    hti it = hash_set_iterator(set);
    while (hash_set_next(&it)) {
        assert(it.key != NULL);
        count++;
    }

    assert(count == 3);

    hash_set_destroy(set);
}

// multiset tests

void test_hash_multiset_add_and_count(void) {
    hash_multiset* multiset = hash_multiset_create();

    assert(multiset != NULL);

    assert(hash_multiset_total_size(multiset) == 0);
    assert(hash_multiset_unique_size(multiset) == 0);

    assert(hash_multiset_add(multiset, "apple") == true);
    assert(hash_multiset_add(multiset, "apple") == true);
    assert(hash_multiset_add(multiset, "banana") == true);

    assert(hash_multiset_count(multiset, "apple") == 2);
    assert(hash_multiset_count(multiset, "banana") == 1);
    assert(hash_multiset_count(multiset, "orange") == 0);

    assert(hash_multiset_unique_size(multiset) == 2);
    assert(hash_multiset_total_size(multiset) == 3);

    hash_multiset_destroy(multiset);
}
void test_hash_multiset_remove_one(void) {
    hash_multiset* multiset = hash_multiset_create();

    assert(multiset != NULL);

    assert(hash_multiset_add(multiset, "cat") == true);
    assert(hash_multiset_add(multiset, "cat") == true);
    assert(hash_multiset_add(multiset, "cat") == true);

    assert(hash_multiset_count(multiset, "cat") == 3);
    assert(hash_multiset_total_size(multiset) == 3);
    assert(hash_multiset_unique_size(multiset) == 1);

    assert(hash_multiset_remove_one(multiset, "cat") == true);

    assert(hash_multiset_count(multiset, "cat") == 2);
    assert(hash_multiset_total_size(multiset) == 2);
    assert(hash_multiset_unique_size(multiset) == 1);

    hash_multiset_destroy(multiset);
}

void test_hash_multiset_remove_all(void) {
    hash_multiset* multiset = hash_multiset_create();

    assert(multiset != NULL);

    assert(hash_multiset_add(multiset, "cat") == true);
    assert(hash_multiset_add(multiset, "cat") == true);
    assert(hash_multiset_add(multiset, "dog") == true);

    assert(hash_multiset_count(multiset, "cat") == 2);
    assert(hash_multiset_count(multiset, "dog") == 1);

    assert(hash_multiset_remove_all(multiset, "cat") == true);

    assert(hash_multiset_count(multiset, "cat") == 0);
    assert(hash_multiset_count(multiset, "dog") == 1);

    assert(hash_multiset_unique_size(multiset) == 1);
    assert(hash_multiset_total_size(multiset) == 1);

    hash_multiset_destroy(multiset);
}

void test_hash_multiset_remove_missing(void) {
    hash_multiset* multiset = hash_multiset_create();

    assert(multiset != NULL);

    assert(hash_multiset_add(multiset, "one") == true);

    assert(hash_multiset_remove_one(multiset, "missing") == false);
    assert(hash_multiset_remove_all(multiset, "missing") == false);

    assert(hash_multiset_count(multiset, "one") == 1);
    assert(hash_multiset_total_size(multiset) == 1);
    assert(hash_multiset_unique_size(multiset) == 1);

    hash_multiset_destroy(multiset);
}
void test_hash_multiset_iterator(void) {
    hash_multiset* multiset = hash_multiset_create();

    assert(multiset != NULL);

    assert(hash_multiset_add(multiset, "a") == true);
    assert(hash_multiset_add(multiset, "a") == true);
    assert(hash_multiset_add(multiset, "b") == true);
    assert(hash_multiset_add(multiset, "c") == true);
    assert(hash_multiset_add(multiset, "c") == true);
    assert(hash_multiset_add(multiset, "c") == true);

    size_t unique_count = 0;
    size_t total_count = 0;

    // Итератор идет по уникальным ключам, а total_count собираем из счетчиков.
    hti it = hash_multiset_iterator(multiset);

    while (hash_multiset_next(&it)) {
        assert(it.key != NULL);
        assert(it.value != NULL);

        size_t* count = (size_t*)it.value;

        unique_count++;
        total_count += *count;
    }

    assert(unique_count == 3);
    assert(total_count == 6);

    hash_multiset_destroy(multiset);
}

// lock-free hash table tests

void test_lock_free_hash_table_create(void) {
    lock_free_hash_table* table = lf_hash_table_create(4);

    assert(table != NULL);
    assert(lf_hash_table_capacity(table) == 4);
    assert(lf_hash_table_length(table) == 0);
    assert(lf_hash_table_get(table, "missing") == NULL);

    lf_hash_table_destroy(table);

    assert(lf_hash_table_create(0) == NULL);
}

void test_lock_free_hash_table_set_get_update(void) {
    lock_free_hash_table* table = lf_hash_table_create(8);

    assert(table != NULL);

    int first = 10;
    int second = 20;

    assert(lf_hash_table_set(table, "key", &first) == true);
    assert(lf_hash_table_length(table) == 1);
    assert(lf_hash_table_get(table, "key") == &first);

    assert(lf_hash_table_set(table, "key", &second) == true);
    assert(lf_hash_table_length(table) == 1);
    assert(lf_hash_table_get(table, "key") == &second);

    assert(lf_hash_table_set(table, "null_value", NULL) == false);
    assert(lf_hash_table_length(table) == 1);

    lf_hash_table_destroy(table);
}

void test_lock_free_hash_table_update_when_full(void) {
    lock_free_hash_table* table = lf_hash_table_create(2);

    assert(table != NULL);

    int one = 1;
    int two = 2;
    int three = 3;
    int updated_two = 200;

    assert(lf_hash_table_set(table, "one", &one) == true);
    assert(lf_hash_table_set(table, "two", &two) == true);
    assert(lf_hash_table_length(table) == 2);

    assert(lf_hash_table_set(table, "three", &three) == false);
    assert(lf_hash_table_length(table) == 2);

    // Главная проверка: полная таблица не мешает обновить уже существующий ключ.
    assert(lf_hash_table_set(table, "two", &updated_two) == true);
    assert(lf_hash_table_length(table) == 2);
    assert(lf_hash_table_get(table, "two") == &updated_two);

    lf_hash_table_destroy(table);
}

void test_lock_free_hash_table_delete_and_reactivate(void) {
    lock_free_hash_table* table = lf_hash_table_create(4);

    assert(table != NULL);

    int old_value = 5;
    int new_value = 50;

    assert(lf_hash_table_set(table, "x", &old_value) == true);
    assert(lf_hash_table_length(table) == 1);

    assert(lf_hash_table_delete(table, "x") == &old_value);
    assert(lf_hash_table_length(table) == 0);
    assert(lf_hash_table_get(table, "x") == NULL);

    assert(lf_hash_table_delete(table, "x") == NULL);
    assert(lf_hash_table_length(table) == 0);

    assert(lf_hash_table_set(table, "x", &new_value) == true);
    assert(lf_hash_table_length(table) == 1);
    assert(lf_hash_table_get(table, "x") == &new_value);

    lf_hash_table_destroy(table);
}

void test_lock_free_hash_table_deleted_slot_keeps_key(void) {
    lock_free_hash_table* table = lf_hash_table_create(1);

    assert(table != NULL);

    int old_value = 1;
    int other_value = 2;
    int revived_value = 3;

    assert(lf_hash_table_set(table, "same", &old_value) == true);
    assert(lf_hash_table_delete(table, "same") == &old_value);
    assert(lf_hash_table_length(table) == 0);

    // Удаленный key остается в слоте, поэтому другой key сюда не вставится.
    assert(lf_hash_table_set(table, "other", &other_value) == false);
    assert(lf_hash_table_length(table) == 0);

    // Но тот же ключ можно снова сделать активным.
    assert(lf_hash_table_set(table, "same", &revived_value) == true);
    assert(lf_hash_table_get(table, "same") == &revived_value);
    assert(lf_hash_table_length(table) == 1);

    lf_hash_table_destroy(table);
}


// запускаем все тесты
void run_tests(void) {
    test_create_empty_table();
    test_set_and_get();
    test_update_existing_key();
    test_delete_existing_key();
    test_delete_missing_key();
    test_delete_and_insert_again();
    test_expand_table();
    test_iterator();

    test_hash_set_add_and_contains();
    test_hash_set_no_duplicates();
    test_hash_set_remove();
    test_hash_set_iterator();

    test_hash_multiset_add_and_count();
    test_hash_multiset_remove_one();
    test_hash_multiset_remove_all();
    test_hash_multiset_remove_missing();
    test_hash_multiset_iterator();

    test_lock_free_hash_table_create();
    test_lock_free_hash_table_set_get_update();
    test_lock_free_hash_table_update_when_full();
    test_lock_free_hash_table_delete_and_reactivate();
    test_lock_free_hash_table_deleted_slot_keeps_key();
}
