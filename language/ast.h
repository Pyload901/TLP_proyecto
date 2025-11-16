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
Node *N_int(long value);
Node *N_float(double value);
Node *N_bool(bool value);
Node *N_bin(const char *op, Node *left, Node *right);
Node *N_unary(const char *op, Node *X);
Node *N_assign(Node *left, Node *right);
Node *N_block(List *stmts);
Node *N_decla(char *typename, char* varname, Node* initial_value);
Node *N_for(Node *init, Node *cond, Node *update, Node *body);
Node *N_while(Node *cond, Node *body);
Node *N_if(Node *cond, Node *then_branch, Node *else_branch);
/* util */
void  ast_print(Node *n, int indent);