#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symtab.h"
#include "ast.h"
#include "semantic.h"
static char *builtin_functions[] = {
    "digitalRead",
    "digitalWrite",
    "analogRead",
    "pwmWrite",
    "pinMode",
    "print",
    "forward_ms",
    "back_ms",
    "turnLeft_ms",
    "turnRight_ms",
    "readLeftSensor",
    "readRightSensor",
    "stopMotors",
};
static char *builtin_constants[] = {
    "INPUT",
    "OUTPUT",
    "HIGH",
    "LOW",
};
void analyze_symbols(Node *node);
void register_builtin_functions(void);
void register_builtin_constants(void);
void register_functions(Node *node);
void check_type_compatible(Type expected, Type actual, char *var_name);
Type parse_type(const char *type_str)
{
    Type type;
    if (strcmp(type_str, "INT") == 0)
    {
        type.base = INT;
    }
    else if (strcmp(type_str, "DOUBLE") == 0)
    {
        type.base = DOUBLE;
    }
    else if (strcmp(type_str, "BOOL") == 0)
    {
        type.base = BOOL;
    }
    else if (strcmp(type_str, "CHAR") == 0)
    {
        type.base = CHAR;
    }
    type.is_array = false;
    type.array_size = 0;
    return type;
}

void analyze_program(Node *root)
{
    symtab_init();
    register_builtin_constants();
    register_builtin_functions();
    register_functions(root);
    analyze_symbols(root);
}

void register_builtin_functions(void)
{
    for (size_t i = 0; i < sizeof(builtin_functions) / sizeof(builtin_functions[0]); i++)
    {
        char *func_name = builtin_functions[i];
        sym_insert(func_name, SYMBOL_FUNCTION, make_type(INT))->is_builtin = true;
    }
}
void register_builtin_constants(void)
{
    for (size_t i = 0; i < sizeof(builtin_constants) / sizeof(builtin_constants[0]); i++)
    {
        char *const_name = builtin_constants[i];
        sym_insert(const_name, SYMBOL_VARIABLE, make_type(INT))->is_builtin = true;
    }
}

void register_functions(Node *node)
{
    if (!node)
        return;
    Node **nodes = node->list->items;
    for (int i = 0; i < node->list->size; i++)
    {
        if (strcmp(nodes[i]->node_type, "FUNCTION") == 0)
        {
            Node *func_node = (Node *)node->list->items[i];
            char *func_name = func_node->value;
            Type return_type = parse_type(func_node->left->value);
            if (sym_lookup_current(func_name))
            {
                fprintf(stderr, "Error: Function %s already declared in this scope.\n", func_name);
                exit(1);
            }
            sym_insert(func_name, SYMBOL_FUNCTION, return_type);
        }
    }
}
void analyze_symbols(Node *node)
{
    if (!node) 
        return;

    if (strcmp(node->node_type, "DECLARACION") == 0)
    {
        char *var_name = node->value;
        Type var_type = parse_type(node->left->value);

        if (sym_lookup_current(var_name))
        {
            fprintf(stderr, "Error: Variable %s already declared in this scope.\n", var_name);
            exit(1);
        }

        sym_insert(var_name, SYMBOL_VARIABLE, var_type);
    }
    else if (strcmp(node->node_type, "DECLARACION_ARRAY") == 0)
    {
        char *var_name = node->value;
        Type var_type = parse_type(node->left->value);
        var_type.is_array = true;
        var_type.array_size = node->ivalue;
        if (sym_lookup_current(var_name))
        {
            fprintf(stderr, "Error: Array %s already declared in this scope.\n", var_name);
            exit(1);
        }
        sym_insert(var_name, SYMBOL_VARIABLE, var_type);
    }
    else if (strcmp(node->node_type, "FUNCTION") == 0)
    {
        enter_scope();
        for (int i = 0; i < node->list->size; i++)
        {
            Node *param_node = (Node *)node->list->items[i];
            char *param_name = param_node->value;
            Type param_type = parse_type(param_node->left->value);
            if (sym_lookup_current(param_name))
            {
                fprintf(stderr, "Error: Parameter %s already declared in this scope.\n", param_name);
                exit(1);
            }
            sym_insert(param_name, SYMBOL_VARIABLE, param_type);
        }
        analyze_symbols(node->right);
        exit_scope();
        return;
    }
    else if (strcmp(node->node_type, "ASSIGN") == 0)
    {
        if (node->left && (strcmp(node->left->node_type, "ID") == 0))
        {
            char *var_name = node->left->value;
            Symbol *sym = sym_lookup(var_name);
            if (!sym)
            {
                fprintf(stderr, "Error: Variable %s not declared.\n", var_name);
                exit(1);
            }
            if (sym->type.is_array)
            {
                if (strcmp(node->right->node_type, "ARRAY_VALUES") != 0)
                {
                    fprintf(stderr, "Error: Cannot assign non-array value to array %s.\n", var_name);
                    exit(1);
                }
                for (int i = 0; i < node->right->list->size; i++) {
                    Node *elem = (Node *)node->right->list->items[i];
                    Type elem_type = parse_type(elem->node_type);
                    check_type_compatible(sym->type, elem_type, var_name);
                }
            } else {
                Type value_type;
                if (strcmp(node->right->node_type, "EXEC") == 0)
                {
                    char *func_name = node->right->value;
                    Symbol *func_sym = sym_lookup(func_name);
                    if (!func_sym || func_sym->symbol_type != SYMBOL_FUNCTION)
                    {
                        fprintf(stderr, "Error: Function %s not declared.\n", func_name);
                        exit(1);
                    }
                    value_type = func_sym->type;
                }
                else
                {
                    value_type = parse_type(node->right->node_type);
                }
                check_type_compatible(sym->type, value_type, var_name);
            }
        }
    }
    else if (strcmp(node->node_type, "EXEC") == 0)
    {
        char *func_name = node->value;
        Symbol *sym = sym_lookup(func_name);
        if (!sym || sym->symbol_type != SYMBOL_FUNCTION)
        {
            fprintf(stderr, "Error: Function %s not declared.\n", func_name);
            exit(1);
        }
    }
    analyze_symbols(node->left);
    analyze_symbols(node->right);
    analyze_symbols(node->extra);

    if (node->list)
    {
        for (int i = 0; i < node->list->size; i++)
        {
            analyze_symbols((Node *)node->list->items[i]);
        }
    }
}

void check_type_compatible(Type expected, Type actual, char *var_name)
{
    if (expected.base != actual.base)
    {
        if (expected.is_array) {
            fprintf(stderr, "Error: Type mismatch for array %s.\n", var_name);
            exit(1);
        } else {
            fprintf(stderr, "Error: Type mismatch for variable %s.\n", var_name);
            exit(1);
        }
    }
}