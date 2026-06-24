cmake_minimum_required(VERSION 4.2)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled src/main.c
        src/hash_multiset.c
        src/headers/hash_multiset.h
        src/lock_free_hash_table.c
        src/headers/lock_free_hash_table.h
        src/hash_table.c
        src/headers/hash_table.h
        src/hash_set.c
        src/headers/hash_set.h
        src/tests.c
        src/tests.c
        src/headers/tests.h)
