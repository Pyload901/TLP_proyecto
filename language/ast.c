#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"

static Node *allocate_node(const char *node_type) {
    Node *node = (Node *) calloc(1, sizeof(Node));
    node->node_type = node_type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->list = NULL;
    return node;
}

Node *N(const char *node_type) {
    return allocate_node(node_type);
}
Node *N_id(char *name) {
    Node *node = allocate_node("ID");
    node->value = strdup(name);
    return node;
}
Node *N_id_array(char *name, long index) {
    Node *node = allocate_node("ID_ARRAY");
    node->value = strdup(name);
    node->left = N_int(index);
    return node;
}
Node *N_char(char c) {
    Node *node = allocate_node("CHAR");
    node->cvalue = c;
    return node;
}
Node *N_int(long value) {
    Node *node = allocate_node("INT");
    node->ivalue = value;
    return node;
}
Node *N_float(double value) {
    Node *node = allocate_node("DOUBLE");
    node->fvalue = value;
    return node;
}
Node *N_bool(bool value) {
    Node *node = allocate_node("BOOL");
    node->bvalue = value;
    return node;
}
Node *N_bin(const char *node_type, Node *left, Node *right) {
    Node *node = allocate_node(node_type);
    node->left = left;
    node->right = right;
    return node;
}
Node *N_unary(const char *op, Node *X) {
    Node *node = allocate_node(op);
    node->left = X;
    return node;
}
Node *N_assign(Node *left, Node *right) {
    Node *node = allocate_node("ASSIGN");
    node->left = left;
    node->right = right;
    return node;
}
Node *N_decla(char *typename, char* varname, Node* initial_value) {
    Node *node = allocate_node("DECLARACION");
    node->value = strdup(varname);
    Node* type_node = allocate_node("TIPO");
    type_node->value = strdup(typename);
    node->left = type_node;
    node->right = initial_value;
    return node;
}
Node *N_decla_array(char *typename, char* varname, long array_size) {
    Node *node = allocate_node("DECLARACION_ARRAY");
    node->value = strdup(varname);
    Node* type_node = allocate_node("TIPO");
    type_node->value = strdup(typename);
    node->left = type_node;
    Node *size_node = N_int(array_size);
    node->right = size_node;
    return node;
}
Node *N_block(List *stmts) {
    Node *node = allocate_node("BLOCK");
    node->list = stmts;
    return node;
}
Node *N_program(List *stmts) {
    Node *node = allocate_node("PROGRAM");
    node->list = stmts;
    return node;
}
Node* N_for(Node *init, Node *cond, Node *update, Node *body) {
    Node *node = allocate_node("FOR");
    node->left = init;
    node->right = cond;
    node->extra = update;
    node->list = L_new();
    L_push(node->list, body);
    return node;
}
Node* N_while(Node *cond, Node *body) {
    Node *node = allocate_node("WHILE");
    node->left = cond;
    node->list = L_new();
    L_push(node->list, body);
    return node;
}
Node* N_if(Node *cond, Node *then_branch, Node *else_branch) {
    Node *node = allocate_node("IF");
    node->left = cond;
    node->right = then_branch;
    node->extra = else_branch;
    return node;
}
Node *N_exec_fun(char *func_name, List *args) {
    Node *node = allocate_node("EXEC");
    node->value = strdup(func_name);
    node->list = args;
    return node;
}
Node *N_return(Node *expr) {
    Node *node = allocate_node("RETURN");
    node->left = expr;
    return node;
}
Node *N_decla_fun(char *func_name, List *params, char *return_type, Node *body) {
    Node *node = allocate_node("FUNCTION");
    node->value = strdup(func_name);
    node->list = params;
    Node* ret_type_node = allocate_node("RETURN_TYPE");
    ret_type_node->value = strdup(return_type);
    node->left = ret_type_node;
    node->right = body;
    return node;
}
Node *N_arr_vals(List *elements) {
    Node *node = allocate_node("ARRAY_VALUES");
    node->list = elements;
    return node;
}
List *L_new(void) {
    List *list = (List *) malloc(sizeof(List));
    list->size = 0;
    list->capacity = 4;
    list->items = (Node **) malloc(sizeof(Node *) * list->capacity);
    return list;
}
void L_push(List *list, Node *item) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->items = (Node **) realloc(list->items, sizeof(Node *) * list->capacity);
    }
    list->items[list->size++] = item;
}

static void pindent(int k){ while(k--) putchar(' '); }
static void printVal(Node *n){
  if (!n) return;
  if (strcmp(n->node_type,"INTVAL")==0) printf("=%ld", n->ivalue);
  else if (strcmp(n->node_type,"DOUBLEVAL")==0) printf("=%f", n->fvalue);
  else if (strcmp(n->node_type,"BOOLEAN")==0) printf("=%s", n->bvalue ? "true" : "false");
  else if (strcmp(n->node_type,"ID")==0) printf("=%s", n->value);
  else if (strcmp(n->node_type,"ID_ARRAY")==0) printf("=%s", n->value);
  else if (strcmp(n->node_type,"DECLARACION")==0) {
    printf("=%s", n->value);
    if (n->right) {
        putchar(',');
        printVal(n->right);
    }
  };
}
void ast_print(Node *n, int indent){
  if (!n){ pindent(indent); puts("(null)"); return; }
  pindent(indent);
  printf("%s", n->node_type);
  printVal(n);
//   if (n->value) printf("(%s)", n->value);
//   if (n->type && strcmp(n->type,"INTVAL")==0) printf("=%ld", n->value);
  putchar('\n');
  if (n->left) ast_print(n->left, indent+2);
  if (n->right) ast_print(n->right, indent+2);
  if (n->extra) ast_print(n->extra, indent+2);
  if (n->list){
    for (int i=0;i<n->list->size;i++) ast_print(n->list->items[i], indent+2);
  }
}