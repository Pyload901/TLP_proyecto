#pragma once
#include <stddef.h>
#include "tokens.h"
typedef struct Node Node;
typedef struct List List;

struct List {
    Node** items;
    int size, capacity;
};
struct Node {
    const char* node_type;
    union {
        char *value; // for literals and identifiers
        long ivalue; // for integer literals
        double fvalue; // for float literals
        bool bvalue; // for boolean literals
        char cvalue; // for character literals
    };
    Node *left;
    Node *right;
    Node *extra;

    List *list; // for complex items
};
/* listas */
List *L_new(void);
void  L_push(List*, Node*);

/* constructores */
Node *N(const char *type);
Node *N_id(char *name);
Node *N_char(char c);
Node *N_int(long value);
Node *N_float(double value);
Node *N_bool(bool value);
Node *N_bin(const char *op, Node *left, Node *right);
Node *N_unary(const char *op, Node *X);
Node *N_assign(Node *left, Node *right);
Node *N_block(List *stmts);
Node *N_program(List *stmts);
Node *N_decla(char *typename, char* varname, Node* initial_value);
Node *N_decla_array(char *typename, char* varname, long array_size);
Node *N_for(Node *init, Node *cond, Node *update, Node *body);
Node *N_while(Node *cond, Node *body);
Node *N_if(Node *cond, Node *then_branch, Node *else_branch);
Node *N_exec_fun(char *func_name, List *args);
Node *N_return(Node *expr);
Node *N_decla_fun(char *func_name, List *params, char *return_type, Node *body);
Node *N_arr_vals(List *elements);
/* util */
void  ast_print(Node *n, int indent);