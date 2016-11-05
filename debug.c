#include "debug.h"
#include "globals.h"
#include "token.h"
#include <string.h>


#ifdef DEBUG
// variables for each function:
static int prog;
static int body;
static int class;
static int cbody;
static int cbody2;
static int func;
static int fbody;
static int par;
static int par2;
static int par3;
static int st_list;
static int stat;
static int init;
static int ret;
static int st_else;
static int st_else2;
static int type;

void enter(const char *fi) {

    int var = 0;
    if (!strcmp(fi, "prog"))
        var = prog++;
    else if (!strcmp(fi, "body"))
        var = body++;
    else if (!strcmp(fi, "class"))
        var = class++;
    else if (!strcmp(fi, "cbody"))
        var = cbody++;
    else if (!strcmp(fi, "cbody2"))
        var = cbody2++;
    else if (!strcmp(fi, "func"))
        var = func++;
    else if (!strcmp(fi, "fbody"))
        var = fbody++;
    else if (!strcmp(fi, "par"))
        var = par++;
    else if (!strcmp(fi, "par2"))
        var = par2++;
    else if (!strcmp(fi, "par3"))
        var = par3++;
    else if (!strcmp(fi, "st_list"))
        var = st_list++;
    else if (!strcmp(fi, "stat"))
        var = stat++;
    else if (!strcmp(fi, "init"))
        var = init++;
    else if (!strcmp(fi, "ret"))
        var = ret++;
    else if (!strcmp(fi, "st_else"))
        var = st_else++;
    else if (!strcmp(fi, "st_else2"))
        var = st_else2++;
    else if (!strcmp(fi, "type"))
        var = type++;

    fprintf(stderr, "Entering to a function: %s() %d\n", fi, var);
}

int leave(const char *fi, int rc) {

    int var = 0;
    if (!strcmp(fi, "prog"))
        var = --prog;
    else if (!strcmp(fi, "body"))
        var = --body;
    else if (!strcmp(fi, "class"))
        var = --class;
    else if (!strcmp(fi, "cbody"))
        var = --cbody;
    else if (!strcmp(fi, "cbody2"))
        var = --cbody2;
    else if (!strcmp(fi, "func"))
        var = --func;
    else if (!strcmp(fi, "fbody"))
        var = --fbody;
    else if (!strcmp(fi, "par"))
        var = --par;
    else if (!strcmp(fi, "par2"))
        var = --par2;
    else if (!strcmp(fi, "par3"))
        var = --par3;
    else if (!strcmp(fi, "st_list"))
        var = --st_list;
    else if (!strcmp(fi, "stat"))
        var = --stat;
    else if (!strcmp(fi, "init"))
        var = --init;
    else if (!strcmp(fi, "ret"))
        var = --ret;
    else if (!strcmp(fi, "st_else"))
        var = --st_else;
    else if (!strcmp(fi, "st_else2"))
        var = --st_else2;
    else if (!strcmp(fi, "type"))
        var = --type;

    fprintf(stderr, "Leaving a function: %s() %d\n", fi, var);
    return rc;
}

int show_token(int rc) {
    printf("Getting token: ");
    if (token->type != TT_empty) {
        switch (token->type) {
            case TT_plus:
                printf("+\n");
                break;
            case TT_minus:
                printf("-\n");
                break;

            case TT_mul:
                printf("*\n");
                break;

            case TT_div:
                printf("/\n");
                break;

            case TT_greater:
                printf(">\n");
                break;

            case TT_lesser:
                printf("<\n");
                break;

            case TT_greatEq:
                printf(">=\n");
                break;

            case TT_lessEq:
                printf("<=\n");
                break;

            case TT_equal:
                printf("==\n");
                break;

            case TT_notEq:
                printf("!=\n");
                break;

            case TT_semicolon:
                printf(";\n");
                break;

            case TT_assign:
                printf("=\n");
                break;

            case TT_comma:
                printf(",\n");
                break;

            case TT_dot:
                printf(".\n");
                break;

            case TT_lBracket:
                printf("(\n");
                break;

            case TT_rBracket:
                printf(")\n");
                break;

            case TT_lCurlBracket:
                printf("{\n");
                break;

            case TT_rCurlBracket:
                printf("}\n");
                break;

            case TT_or:
                printf("||\n");
                break;

            case TT_and:
                printf("&&\n");
                break;

            case TT_not:
                printf("~\n");
                break;

            case TT_id:
                printf("identifier\n");
                break;

            case TT_keyword:
                switch (token->attr.keyword) {
                    case TK_boolean:
                        printf("bool\n");
                        break;
                    case TK_break:
                        printf("break\n");
                        break;
                    case TK_class:
                        printf("class\n");
                        break;
                    case TK_continue:
                        printf("continue\n");
                        break;
                    case TK_do:
                        printf("do\n");
                        break;
                    case TK_double:
                        printf("double\n");
                        break;
                    case TK_else:
                        printf("else\n");
                        break;
                    case TK_false:
                        printf("false\n");
                        break;
                    case TK_for:
                        printf("for\n");
                        break;
                    case TK_if:
                        printf("if\n");
                        break;
                    case TK_int:
                        printf("int\n");
                        break;
                    case TK_return:
                        printf("return\n");
                        break;
                    case TK_String:
                        printf("String\n");
                        break;
                    case TK_static:
                        printf("static\n");
                        break;
                    case TK_true:
                        printf("true\n");
                        break;
                    case TK_void:
                        printf("void\n");
                        break;
                    case TK_while:
                        printf("while\n");
                        break;
                    default:
                        printf("BAD\n");
                        break;
                }
                break;
            case TT_int:
                printf("var_int\n");
                break;

            case TT_string:
                printf("var_string\n");
                break;

            case TT_double:
                printf("var_double\n");
                break;

            case TT_eof:
                printf("eof\n");
                break;

            default:
                printf("BAD_default\n");
                break;
        }
    }
    return rc;
}
#else
void enter(const char *fi) {
    (void)fi;
}

int leave(const char *fi, int rc) {
    (void)fi;
    return rc;
}
int show_token() {
    return 0;
}
#endif