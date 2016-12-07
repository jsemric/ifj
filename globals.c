/*
 * IFJ 2016
 * FIT VUT Brno
 * IFJ16 Interpret Project
 *
 * Authors:
 * Jakub   Semric     - xsemri00
 * Peter   Rusinak    - xrusin03
 * Krystof Rykala     - xrykal00
 * Martin  Mikan      - xmikan00
 * Martin  Polakovic  - xpolak33
 *
 * Unless otherwise stated, all code is licenced under a
 * GNU General Public License v2.0
 *
 */

#include "globals.h"
#include "token.h"
#include "token_vector.h"
#include <stdio.h>
#include "ial.h"
#include "instruction.h"
#include "symbol.h"
#include "ilist.h"
#include "stack.h"
#include <stdlib.h>
#include <stdbool.h>
#include "debug.h"
#include "precedence_analyser_stack.h"

T_symbol_table *symbol_tab;
FILE *source;
T_token *token;
ilist *glist;
token_vector global_token_vector;
T_stack *frame_stack;
T_stack *main_stack;
char *char_vector;
T_instr *div_point;

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
    return NULL;
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
    if (global_token_vector)
        token_vec_delete(global_token_vector);
    prec_stack_free();
    stack_remove(&frame_stack, true);
    stack_remove(&main_stack, false);
    // dividing lists
    if (glist) {
        if (div_point) {
            div_point->next = NULL;
            glist->last = div_point;
        }
        else {
            glist->first = NULL;
            glist->last = NULL;
        }
    }

    list_free(&glist);
    fclose(source);
    table_remove(&symbol_tab);
    free(char_vector);

    exit(err_code);
}

