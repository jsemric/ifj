#include "globals.h"
#include "token.h"
#include <stdio.h>
#include "ial.h"
#include "instruction.h"
#include "symbol.h"
#include "ilist.h"
#include <stdlib.h>

ilist *instr_list;
T_symbol_table *symbol_tab;
FILE *source;
T_token *token;

struct T_pool pool = {NULL, NULL, 0};
#define POOL_SIZE 100

void *alloc(size_t size) {
    if (pool.first == NULL) {
        pool.first = calloc(POOL_SIZE + 1, sizeof(void*));
        pool.last = pool.first;
        if (pool.first == NULL)
            terminate(99);
    } else if (pool.size == POOL_SIZE) {
        pool.last[POOL_SIZE] = calloc(POOL_SIZE + 1, sizeof(void*));
        if (pool.last == NULL)
            terminate(99);
        pool.last = &pool.last[POOL_SIZE];
        pool.size = 0;
    }
    void *new = calloc(1, size);
    if (new == NULL)
        terminate(99);
    printf("Init %d\n", pool.size);
    pool.last[pool.size] = new;
    pool.size++;
    return new;
}

void *ralloc(void *ptr, size_t size) {
    void *new = realloc(ptr, size);
    if (new == ptr)
        return new;
    if (new == NULL)
        terminate(99);
    for (void **cur = pool.first; cur != NULL; cur = cur[POOL_SIZE]) {
        for (int i = 0; i < POOL_SIZE; i++) {
            if (cur[i] == ptr) {
                cur[i] = new;
                return new;
            }
        }
    }
    // The code should never get here, something muse be terribly wrong:
    terminate(99);
}

void terminate(int err_code) {
    void **cur = pool.first;
    while (cur != NULL) {
        for (int i = 0; i < POOL_SIZE; i++) {
            if (cur[i])
                free(cur[i]);
        }
        cur = cur[POOL_SIZE];
        free(cur);
    }

    token_free(&token);
    fclose(source);
    list_free(&instr_list);
    remove_ifj16();
    table_remove(&symbol_tab);

    exit(err_code);
}

