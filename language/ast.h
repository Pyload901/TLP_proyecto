#pragma once
#include <stddef.h>
typedef struct Node Node;
typedef struct List List;
struct List {
    Node** items;
    int size, capacity;
};
struct Node {
    const char* type;
    union {
        char *value; // for literals and identifiers
        long ivalue; // for integer literals
        float fvalue; // for float literals
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

/* util */
void  ast_print(Node *n, int indent);