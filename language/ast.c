#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"

static Node *allocate_node(const char *type) {
    Node *node = (Node *) calloc(1, sizeof(Node));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->list = NULL;
    return node;
}

Node *N(const char *type) {
    return allocate_node(type);
}
Node *N_id(char *name) {
    Node *node = allocate_node("ID");
    node->value = strdup(name);
    return node;
}
Node *N_int(long value) {
    Node *node = allocate_node("INTVAL");
    node->ivalue = value;
    return node;
}
Node *N_float(double value) {
    Node *node = allocate_node("DOUBLEVAL");
    node->fvalue = value;
    return node;
}
Node *N_bool(bool value) {
    Node *node = allocate_node("BOOLEAN");
    node->bvalue = value;
    return node;
}
Node *N_bin(const char *type, Node *left, Node *right) {
    Node *node = allocate_node(type);
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
Node *N_block(List *stmts) {
    Node *node = allocate_node("BLOCK");
    node->list = stmts;
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
  if (strcmp(n->type,"INTVAL")==0) printf("=%ld", n->ivalue);
  else if (strcmp(n->type,"DOUBLEVAL")==0) printf("=%f", n->fvalue);
  else if (strcmp(n->type,"BOOLEAN")==0) printf("=%s", n->bvalue ? "true" : "false");
  else if (strcmp(n->type,"ID")==0) printf("=%s", n->value);
}
void ast_print(Node *n, int indent){
  if (!n){ pindent(indent); puts("(null)"); return; }
  pindent(indent);
  printf("%s", n->type);
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