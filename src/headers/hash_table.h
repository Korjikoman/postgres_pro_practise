#ifndef UNTITLED_HASH_TABLE_H
#define UNTITLED_HASH_TABLE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef  struct entry {
    // key == NULL: пустой слот. Специальный HT_DELETED означает tombstone.
    const char * key;
    void * value;
} entry ;


typedef struct hash_table {
    // capacity всегда степень двойки: индекс считается через hash & (capacity - 1).
    entry * entries;
    size_t capacity;
    size_t length;

} hash_table;

typedef struct hti {
    const char * key;
    void * value;

    // Итератор хранит позицию обхода прямо между вызовами hash_table_next().
    hash_table* hash_table;
    size_t index;
}hti;


hash_table * hash_table_create(void);
void hash_table_destroy(hash_table* table);

// Возвращает value или NULL, если ключа нет.
void* hash_table_get(hash_table* hash_table, const char * key);

// Удаляет key и возвращает старое value, чтобы вызывающий код сам освободил его.
void* hash_table_delete(hash_table* table, const char* key);

// NULL value запрещен: NULL используется как "ключ не найден".
void* hash_table_set(hash_table* table, const char * key, void * value);

bool hash_table_next(hti* it);
size_t ht_length(hash_table* hash_table);
hti ht_iterator(hash_table* hash_table);


static  uint64_t hash_func(const char * key);
static const char* ht_set_entry(entry* entries, size_t capacity,
        const char* key, void* value, size_t* plength);
bool ht_expand(hash_table* table);



#endif //UNTITLED_HASH_TABLE_H
