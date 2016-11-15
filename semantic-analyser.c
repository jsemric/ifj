/*
 *  parser.c
 *  Jakub Semric
 *
 */

#include "token.h"
#include <stdio.h>
#include "string.h"
#include <string.h>
#include "globals.h"
#include "scanner.h"
#include "token_vector.h"
#include <stdbool.h>
#include <stdlib.h>
#include "debug.h"
#include "instruction.h"
#include "ial.h"
#include "symbol.h"

// unget token
static bool get_token_flag = false;

#ifdef DEBUG
#define return return leave(__func__, 0) +
#endif

#define get_token()  (get_token_flag ? (get_token_flag = false) : get_token())
#define unget_token() get_token_flag = true

// global variables
T_symbol_table *symbol_tab;
T_token *token;
// pointer to actual class
static T_symbol *actual_class;
static T_symbol *actual_func;

// each function represents a nonterminal symbol in LL(1) table
static int prog();
static int body();
static int class();
static int cbody();
static int cbody2(T_symbol_table *local_tab);
static int func(T_symbol_table *local_tab);
static int fbody(T_symbol_table *local_tab);
static int par();
static int st_list(T_symbol_table *local_tab);
static int stat(T_symbol_table *local_tab);
static int st_else(T_symbol_table *local_tab);
static int st_else2(T_symbol_table *local_tab);

/*******************************************************
 * function handling helper functions
 *
 */

#define BI_COUNT 9


// built-ins
typedef enum {
    b_readI,
    b_readD,
    b_readS,
    b_print,
    b_length,
    b_substr,
    b_compare,
    b_find,
    b_sort
} T_builtins;


// built-ins
static char *arr_ifj16[] = {
    "readInt", "readDouble", "readString", "print",
    "length",  "substr", "compare", "find", "sort"
};

/**
 *
 * Proceed a build-in functions.
 *
 * @param func_id function name
 * @param tv vector of tokens
 * @param L instruction list
 * @param dest pointer to a symbol in local table on stack, or in global table
 * @return 0 on success, TYPE_ERROR or DEFINITION_ERROR
 *
 */
int handle_builtins(const char *func_id, token_vector tv, ilist *L, T_symbol *dest,
                    T_symbol_table *local_tab)
{{{
    int i;
    // loop over all ifj.*
    for (i = 0; i < BI_COUNT && strcmp(func_id, arr_ifj16[i]);i++);
    T_token *it = tv->arr;     // token vector iterator
    switch (i) {
        case b_readI:
        case b_readD:
        case b_readS:
            {{{
                // no parameters
                if (it->type != TT_rBracket) {
                    return TYPE_ERROR;
                }
                // a = readInt();
                T_data_type dtype = dest->attr.var->data_type;
                // XXX double a = readInt
                if (i == b_readI && (dtype == is_int || dtype == is_double )) {
                    create_instr(L, TI_readInt, 0, dest->id, NULL, NULL);
                }
                else if (i == b_readD && dtype == is_double) {
                    create_instr(L, TI_readDouble, 0, dest->id, NULL, NULL);
                }
                else if (i == b_readD && dtype == is_str) {
                    create_instr(L, TI_readString, 0, dest->id, NULL, NULL);
                }
                else
                    return TYPE_ERROR;
                break;
            }}}
        case b_print:
            // TODO push push push ...
            break;


        case b_length:
        case b_sort:
            // str (str) | int (str)
            {{{
                T_instr_type ins = i == b_sort ? TI_sort : TI_length;
                // checking destination data type
                T_data_type dtype = dest->attr.var->data_type;
                if ((i == b_sort && dtype != is_str ) || (i == b_length &&
                            (dtype == is_int || dtype == is_double) ))
                {
                    return TYPE_ERROR;
                }
                // checking string variable
                if (it->type == TT_id || it->type == TT_fullid) {
                    // checking type and if it was defined
                    is_defined(it->attr.str, local_tab,
                               actual_class, is_str);
                    create_instr(L, ins, TM_const_all, dest->id,
                                 it->attr.str, NULL);
                }
                else if (it->type == TT_string) {
                    create_instr(L, ins, TM_const_all, dest->id,
                                 it->attr.str, NULL);
                }
                else
                    return TYPE_ERROR;
                break;
            }}}
        case b_find:
        case b_compare:
            // int (str, str)
            {{{
                // checking data type
                T_data_type dtype = dest->attr.var->data_type;
                if (dtype == is_int || dtype == is_double)
                {
                    return TYPE_ERROR;
                }

                // four tokens required
                if (tv->last != 3) {
                    return TYPE_ERROR;
                }
                // second argument
                T_token *it2 = it + 2;
                // setting instruction
                T_instr_type ins = (i == b_find ? b_find : b_compare);
                T_instr_mode mode = TM_noconst;
                // determining constant or identifier
                if (it->type == TT_id || it->type == TT_fullid) {
                    is_defined(it->attr.str, local_tab, actual_class ,is_str);
                }
                else if (it->type == TT_string) {
                    // first is constant
                    mode = TM_const1;
                }
                if (it2->type == TT_id || it2->type == TT_fullid) {
                    is_defined(it2->attr.str, local_tab, actual_class ,is_str);
                }
                else if (it2->type == TT_string) {
                    // second or both are constant
                    mode = mode == TM_const1 ? TM_const_all : TM_const2;
                }

                // creating instruction
                create_instr(L, ins, mode, dest->id, it->attr.str, it2->attr.str);
                it = it2; // last argument
                break;
            }}}
        case b_substr:
            // TODO I need stack
            {{{


                break;
            }}}
        default:
            return DEFINITION_ERROR;
    }
    // too many parameters TODO check id next is id or constant
    it++; // moving to ')'
    if (it->type == TT_comma)
        return TYPE_ERROR;
    // no bracket found - syntax errors
    return it->type == TT_rBracket ? 0: SYNTAX_ERROR;
}}}
/**
 *
 * Proceed a function.
 *
 * @param func_id function name
 * @param tv vector of tokens
 * @param L instruction list
 * @return 0 on success, TYPE_ERROR or DEFINITION_ERROR
 *
 */
int handle_function(T_symbol *func, token_vector tv, ilist *L, T_symbol_table
                    *local_tab)
{{{
    T_token *it = tv->arr;
    T_symbol **pars = (T_symbol**)func->attr.func->arguments;
    // handling parameters
    for (unsigned j = 0; j < func->attr.func->par_count;j++) {
        // data type of each parameter
        T_data_type dtype = pars[j]->attr.var->data_type;

        if (token->type == TT_id || token->type == TT_fullid) {
            // check the identifier
            is_defined(it->attr.str, local_tab, actual_class, dtype);
            // push identifier on stack
        }
        else if (token->type == TT_string && dtype == is_str) {
            // push just value or variable-const
        }
        else if (token->type == TT_int && dtype == is_int) {
            // push value or variable-const
        }
        else if ((token->type == TT_double || token->type == TT_double)
                  && dtype == is_double)
        {
            // push value or variable-const
        }
        else {
            return SYNTAX_ERROR; // XXX or another error code ?
        }
        // going for comma
        it++;
        if (it->type != TT_comma) {
            return SYNTAX_ERROR;
        }
        // next parameter or right bracket
        it++;
    }
    // function call must end with ')'
    if (it->type != TT_rBracket)
        return SYNTAX_ERROR;
    // insert instruction CALL
    return 0;
}}}

bool is_function(token_vector tv)
{{{
    if (tv->last > 2) {
        T_token *it = tv->arr;
        // id (
        // id.id (
        return ((it->type == TT_id || it->type == TT_fullid) &&
                (it+1)->type == TT_rBracket);
    }
    return false;
}}}

/******************************************************
 * semantic analysis
 *
 */
// PROG -> BODY eof
static int prog()
{{{
    enter(__func__);

    return  body();
}}}

// BODY -> CLASS BODY
// BODY -> e
static int body()
{{{
    enter(__func__);
    unsigned res = 0;   // return value
    get_token();

    // `class` or `eof` expected
    if (token->type == TT_keyword && token->attr.keyword == TK_class ) {
        res = class();
        if (res)
            return res;

        return body();
    }
    else if (token->type == TT_eof) {
        return  0;
    }

    return SYNTAX_ERROR;
}}}

/* CLASS -> id lb CBODY rb
*/
static int class()
{{{
    enter(__func__);
    // going for id
    get_token();

    // already tested in by first throughpass
    actual_class = table_find(symbol_tab, token->attr.str, NULL);

    // going for '{'
    get_token();

    return cbody();
}}}

/*
   CBODY -> static TYPE id CBODY2 CBODY
*/
static int cbody()
{{{
    enter(__func__);
    int res;
    // static or '}'
    get_token();

    // 'static' expected
    if (token->type == TT_keyword && token->attr.keyword == TK_static) {
        // reading type
        get_token();

        // 'id' expected
        get_token();


        // in case that symbol is function local symbol table is set
        T_symbol *s = table_find(symbol_tab,token->attr.str, actual_class);
        if (s->symbol_type == is_func) {
            // updating actual function
            actual_func = s;
            // local table
            T_symbol_table *local_tab = s->attr.func->local_table;
            res = cbody2(local_tab);
        }
        else {
            res = cbody2(NULL); // if variable - no local table
        }

        if (res)
            return res;

        return cbody();
    }
    else
        return 0;
}}}

/*
   CBODY2 -> '=' ';' '('
*/
static int cbody2(T_symbol_table *local_tab)
{{{
    enter(__func__);
    // '=' or ';' or '(' expected
    get_token();

    // variable
    if (token->type == TT_assign || token->type == TT_semicolon) {

        // cannot be called function XXX
        // just reading everything till `;`
        while (token->type != TT_semicolon) {
            // TODO fill token vector, handle expression
            get_token();
        }

        return 0;
    }
    else {
        return func(local_tab);
    }
}}}

/* FUNC -> ( PAR ) FBODY
 '(' has been read
*/
static int func(T_symbol_table *local_tab)
{{{
    enter(__func__);
    // just read parameters
    par();
    // process body
    int res = fbody(local_tab);
    // just for debug
    print_function(actual_func);
    return res;
}}}

/*
   just read till ')'
*/
static int par()
{{{
    enter(__func__);
    while (token->type != TT_rBracket)
        get_token();
    return 0;
}}}

// FBODY -> { ST_LIST }
// FBODY -> ;
static int fbody(T_symbol_table *local_tab)
{{{
    enter(__func__);
    get_token();

    // XXX maybe semicolon makes syntax error ?
    if (token->type == TT_semicolon) {
        return 0;
    }
    else {
        return st_list(local_tab);
    }

}}}

// ST_LIST -> ε
// ST_LIST -> {STLIST} STLIST
// ST_LIST -> STAT STLIST
static int st_list(T_symbol_table *local_tab)
{{{
    enter(__func__);
    // read only if '{' or ';'
    if (token->type == TT_lCurlBracket || token->type == TT_rCurlBracket ||
        token->type == TT_semicolon || token->type == TT_keyword) {
        // expecting whatever what is statement
        get_token();
    }
    // keyword
    if (token->type == TT_keyword || token->type == TT_id) {

        int res = stat(local_tab);
        if (res)
            return res;

        return st_list(local_tab);
    }
    else if (token->type == TT_rCurlBracket) {
        return 0;
    }
    else if (token->type == TT_lCurlBracket) {
        int res = st_list(local_tab);        // { ST-LIST } ST-LIST
        if (res)
            return res;
        return st_list(local_tab);
    }
    return SYNTAX_ERROR;
}}}

// STAT -> many...
static int stat(T_symbol_table *local_tab) {
    enter(__func__);
    int bc = 0;
    // while|for|if|return|continue|break|types
    if (token->type == TT_keyword) {
        switch(token->attr.keyword) {
            case TK_int:
            case TK_double:
            case TK_String:
                {{{
                    // rule: STAT -> TYPE id ;| = EXPR ;

                    unsigned dtype = token->attr.keyword;   // data type
                    // increasing local variable count
                    actual_func->attr.func->local_count++;

                    // id in token
                    get_token();
                    if (table_find(local_tab, token->attr.str, NULL)) {
                        // redefinition
                        return DEFINITION_ERROR;
                    }

                    // creating a new symbol
                    T_symbol *symbol = create_symbol(token->attr.str, 0);
                    token->attr.str = NULL;        // discredit free call

                    // creating variable
                    symbol->attr.var = create_var(dtype);

                    // inserting to table
                    table_insert(local_tab, symbol);

                    // ';' or '='
                    get_token();

                    if (token->type == TT_semicolon) {
                        return 0;
                    }
                    else {
                        // assign '='

                        // creating token vector
                        token_vector tvect = token_vec_init();

                        while (token->type != TT_semicolon) {
                            get_token();
                            // pushing token
                            token_push_back(tvect, token);
                        }
                        // TODO send tvect to precedence analyser
                        // XXX last tvect item is ';'
                        // TODO check if it is function
                        return 0;
                    }
                }}}
            case TK_while:
            case TK_if:
                {{{
                    T_keyword keyword = token->attr.keyword;
                    // ST -> if ( EXPR ) { ST_LIST } ELSE
                    get_token();

                    // reading till `)`
                    bc = 0;             // bracket counter
                    token_vector tvect = token_vec_init();

                    do {
                        get_token();
                        // pushing token
                        token_push_back(tvect, token);
                        if (token->type == TT_lBracket)
                            bc++;
                        if (token->type == TT_rBracket)
                            bc--;
                    } while ( bc != -1);
                    // TODO send tvect to precedence analyser
                    // XXX last token in tvect is ')'

                    get_token();
                    // if (..) { ....
                    // beginning new statement list
                    int res;
                    res = st_list(local_tab);
                    if (res)
                        return res;
                    // TODO insert different instruction while/if
                    if (keyword == TK_while) {
                        return 0;
                    }
                    // if next word is else do call st_else()
                    get_token();

                    if (token->type == TT_keyword && token->attr.keyword == TK_else)
                        return st_else(local_tab);
                    // no else read, must unget
                    unget_token();
                    return 0;
                }}}
            case TK_return:
                {{{
                    // RET return EXPR;
                    // RET return;
                    // reading whole expression
                    token_vector tvect = token_vec_init();

                    // reading till ';' or 'eof' read
                    while ( token->type != TT_semicolon) {
                        get_token();
                        // pushing token
                        token_push_back(tvect, token);
                    }
                    // TODO call expression handeler
                    // XXX ';' in tvect
                    return 0;
                }}}
            default:
                return SYNTAX_ERROR;
        }
    }
    // id = exp
    else if (token->type == TT_id)
    {{{
        char *iden = token->attr.str;           // identifier name
        T_symbol *loc_sym, *glob_sym;    // local and global symbol

        // checking whether it was defined
        loc_sym = table_find(local_tab, iden, NULL);    // local var
        glob_sym = table_find(symbol_tab, iden, actual_class); // static

        if (!loc_sym && !glob_sym ) {
           return DEFINITION_ERROR;
        }

        get_token();

        if (token->type == TT_assign)
        {{{
            // id = ....;
            T_symbol *sym = loc_sym ? loc_sym : glob_sym;
            // assignment goes to function or class
            if (!sym || sym->symbol_type != is_var) {
                return DEFINITION_ERROR;
            }

            // reading till ';'
            token_vector tvect = token_vec_init();  // creating vector

            while ( token->type != TT_semicolon) {
                get_token();
                token_push_back(tvect, token);
            }
            // TODO send vector
            return 0;
        }}}
        else
        {{{
            // calling goes to variable or function does not exist
            if (!glob_sym || glob_sym->symbol_type != is_func) {
                return DEFINITION_ERROR;
            }
            // id();
            // reading till ';'
            token_vector tvect = token_vec_init();  // creating vector

            while ( token->type != TT_semicolon) {
                get_token();
                token_push_back(tvect, token);
            }
            // TODO handle function
            // ;
            return 0;
        }}}
    }}}
    else
    {{{

        return 0;
    }}}
}

// ELSE -> .
// ELSE -> else ELSE2
// ELSE2 -> { ST_LIST }
static int st_else(T_symbol_table *local_tab)
{{{
    enter(__func__);

    get_token();
    // `else {`
    if (token->type == TT_lCurlBracket) {
        int res = st_list(local_tab);
        return res;
    }
    // `else if`
    else {
        return st_else2(local_tab);
    }

}}}

// ELSE2 -> if ( EXPR ) { ST_LIST } ELSE
static int st_else2(T_symbol_table *local_tab)
{{{
    enter(__func__);

    get_token();
    // only '('
    // reading till ')' or 'eof' read
    int bc = 0; // bracket counter
    token_vector tvect = token_vec_init();

    do {
        get_token();
        // pushing token
        token_push_back(tvect, token);
        if (token->type == TT_lBracket)
            bc++;
        if (token->type == TT_rBracket)
            bc--;
    } while ( bc != -1);
    // TODO send tvect to precedence analyser
    // XXX last token in tvect is ')'

    // {
    get_token();
    // beginning of statement list
    int res;
    res = st_list(local_tab);
    if (res)
        return res;
    // if next word is else do call st_else()
    get_token();

    if (token->type == TT_keyword && token->attr.keyword == TK_else)
        return st_else(local_tab);
    // no else, token must be returned
    unget_token();
    return 0;
}}}

int second_throughpass() {
    return prog();
}

