#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static Node *allocate_node(const char *type) {
    Node *node = (Node *) calloc(1, sizeof(Node));
    node->type = type;
    return node;
}

Node *new_node(const char *type) {
    return allocate_node(type);
}
Node *new_int_node(int value) {
    Node *node = allocate_node("VALOR");
    node->ivalue = value;
    return node;
}
Node *new_float_node(float value) {
    Node *node = allocate_node("float");
    node->fvalue = value;
    return node;
}
Node *new_bool_node(bool value) {
    Node *node = allocate_node("bool");
    node->bvalue = value;
    return node;
}
Node *new_value_node(const char *value) {
    Node *node = allocate_node("value");
    node->value = strdup(value);
    return node;
}
Node *new_binary_op_node(const char *type, Node *left, Node *right) {
    Node *node = allocate_node(type);
    node->left = left;
    node->right = right;
    return node;
}
Node *new_unary_op_node(const char *type, Node *operand) {
    Node *node = allocate_node(type);
    node->left = operand;
    return node;
}
Node *new_function_call_node(const char *name, List *args) {
    Node *node = allocate_node("function_call");
    node->value = strdup(name);
    node->list = args;
    return node;
}
Node *new_assignment_node(const char *name, Node *value) {
    Node *node = allocate_node("ASIGNACION");
    node->value = strdup(name);
    node->right = value;
    return node;
}