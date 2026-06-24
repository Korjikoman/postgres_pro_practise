#ifndef UNTITLED_LOCK_FREE_HASH_TABLE_H
#define UNTITLED_LOCK_FREE_HASH_TABLE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Неблокирующая версия хеш-таблицы.
 *
 * Главное:
 * - коллизии решаются linear probing, как в обычной hash_table;
 * - resize нет, capacity фиксируется при создании;
 * - операции get/set/delete не используют mutex;
 * - value == NULL запрещен, потому что NULL означает "запись удалена";
 * - key освобождается только в destroy, чтобы другие потоки не читали freed memory.
 */

typedef struct lock_free_hash_table lock_free_hash_table;


// capacity задается заранее: при переполнении новый key вставить нельзя.
lock_free_hash_table* lf_hash_table_create(size_t capacity);

// освобождает ключи, но не value.
void lf_hash_table_destroy(lock_free_hash_table* table);

/*
 * Вставляет новый ключ или обновляет существующий.
 * Сначала ищет key, поэтому update работает даже в полной таблице.
 */
bool lf_hash_table_set(lock_free_hash_table* table, const char* key, void* value);


void* lf_hash_table_get(lock_free_hash_table* table, const char* key);

/*
 * Логически удаляет key: value становится NULL, key живет до destroy.
 * Старое value возвращается вызывающему коду.
 */
void* lf_hash_table_delete(lock_free_hash_table* table, const char* key);

// Моментальный снимок числа активных записей.
size_t lf_hash_table_length(lock_free_hash_table* table);

// Фиксированное число слотов.
size_t lf_hash_table_capacity(lock_free_hash_table* table);

#endif // UNTITLED_LOCK_FREE_HASH_TABLE_H
