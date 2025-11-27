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
    "setSpeed",
    "stopMotors",
    "delay",
};
static char *builtin_constants[] = {
    "INPUT",
    "OUTPUT",
    "HIGH",
    "LOW",
};

static bool function_context_active = false;
static Type current_function_return_type;
static const char *current_function_name = NULL;
void analyze_symbols(Node *node);
void register_builtin_functions(void);
void register_builtin_constants(void);
void register_functions(Node *node);
void check_type_compatible(Type expected, Type actual, char *var_name);
static Type infer_expression_type(Node *expr);
static bool is_numeric_base(BaseType base);
static const char *base_type_name(BaseType base);
Type parse_type(const char *type_str)
{
    Type type;
    type.base = INT;
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
    else if (strcmp(type_str, "VOID") == 0)
    {
        type.base = VOID;
    }
    else
    {
        fprintf(stderr, "Error: Unknown type %s.\n", type_str ? type_str : "<null>");
        exit(1);
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
        if (var_type.base == VOID)
        {
            fprintf(stderr, "Error: Variable %s cannot be of type void.\n", var_name);
            exit(1);
        }

        if (sym_lookup_current(var_name))
        {
            fprintf(stderr, "Error: Variable %s already declared in this scope.\n", var_name);
            exit(1);
        }

        sym_insert(var_name, SYMBOL_VARIABLE, var_type);

        if (node->right)
        {
            Node *init_expr = node->right;
            if (strcmp(init_expr->node_type, "ASSIGN") == 0 && init_expr->right)
            {
                init_expr = init_expr->right;
            }
            if (strcmp(init_expr->node_type, "ARRAY_VALUES") == 0)
            {
                if (!init_expr->list || init_expr->list->size != 1)
                {
                    fprintf(stderr, "Error: Scalar initialization for %s requires exactly one value.\n", var_name);
                    exit(1);
                }
                init_expr = (Node *)init_expr->list->items[0];
            }
            Type init_type = infer_expression_type(init_expr);
            if (init_type.base == VOID)
            {
                fprintf(stderr, "Error: Variable %s cannot be initialized with void expression.\n", var_name);
                exit(1);
            }
            check_type_compatible(var_type, init_type, var_name);
        }
    }
    else if (strcmp(node->node_type, "DECLARACION_ARRAY") == 0)
    {
        char *var_name = node->value;
        Type var_type = parse_type(node->left->value);
        if (var_type.base == VOID)
        {
            fprintf(stderr, "Error: Array %s cannot be of type void.\n", var_name);
            exit(1);
        }
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
        Type function_type = parse_type(node->left->value);
        enter_scope();
        for (int i = 0; i < node->list->size; i++)
        {
            Node *param_node = (Node *)node->list->items[i];
            char *param_name = param_node->value;
            Type param_type = parse_type(param_node->left->value);
            if (param_type.base == VOID)
            {
                fprintf(stderr, "Error: Parameter %s cannot be of type void.\n", param_name);
                exit(1);
            }
            if (sym_lookup_current(param_name))
            {
                fprintf(stderr, "Error: Parameter %s already declared in this scope.\n", param_name);
                exit(1);
            }
            sym_insert(param_name, SYMBOL_VARIABLE, param_type);
        }
        bool previous_context = function_context_active;
        Type previous_return_type = current_function_return_type;
        const char *previous_function_name = current_function_name;
        function_context_active = true;
        current_function_return_type = function_type;
        current_function_name = node->value;
        analyze_symbols(node->right);
        function_context_active = previous_context;
        current_function_return_type = previous_return_type;
        current_function_name = previous_function_name;
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
                    Type elem_type = infer_expression_type(elem);
                    if (elem_type.base == VOID)
                    {
                        fprintf(stderr, "Error: Cannot assign void expression to array %s.\n", var_name);
                        exit(1);
                    }
                    check_type_compatible(sym->type, elem_type, var_name);
                }
            } else {
                Type value_type = infer_expression_type(node->right);
                if (value_type.base == VOID)
                {
                    fprintf(stderr, "Error: Cannot assign void expression to variable %s.\n", var_name);
                    exit(1);
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
    else if (strcmp(node->node_type, "RETURN") == 0)
    {
        if (!function_context_active)
        {
            fprintf(stderr, "Error: RETURN statement outside of function.\n");
            exit(1);
        }
        if (!node->left)
        {
            fprintf(stderr, "Error: RETURN statement must include an expression.\n");
            exit(1);
        }
        if (current_function_return_type.base == VOID)
        {
            fprintf(stderr, "Error: Void function %s cannot contain return statements.\n",
                    current_function_name ? current_function_name : "<anonymous>");
            exit(1);
        }
        Type return_expr_type = infer_expression_type(node->left);
        if (return_expr_type.base == VOID)
        {
            fprintf(stderr, "Error: Function %s cannot return a void expression.\n", current_function_name ? current_function_name : "<anonymous>");
            exit(1);
        }
        if (return_expr_type.base != current_function_return_type.base)
        {
            fprintf(stderr, "Error: Return type mismatch in function %s. Expected %s but found %s.\n",
                    current_function_name ? current_function_name : "<anonymous>",
                    base_type_name(current_function_return_type.base),
                    base_type_name(return_expr_type.base));
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

static bool is_numeric_base(BaseType base)
{
    return base == INT || base == DOUBLE;
}

static const char *base_type_name(BaseType base)
{
    switch (base)
    {
    case INT:
        return "int";
    case DOUBLE:
        return "double";
    case CHAR:
        return "char";
    case BOOL:
        return "bool";
    case VOID:
        return "void";
    default:
        return "<unknown>";
    }
}

static Type infer_expression_type(Node *expr)
{
    if (!expr)
    {
        fprintf(stderr, "Error: Expected expression but found none.\n");
        exit(1);
    }

    const char *kind = expr->node_type;
    if (strcmp(kind, "INT") == 0)
    {
        return make_type(INT);
    }
    if (strcmp(kind, "DOUBLE") == 0)
    {
        return make_type(DOUBLE);
    }
    if (strcmp(kind, "BOOL") == 0)
    {
        return make_type(BOOL);
    }
    if (strcmp(kind, "CHAR") == 0)
    {
        return make_type(CHAR);
    }
    if (strcmp(kind, "ID") == 0)
    {
        Symbol *sym = sym_lookup(expr->value);
        if (!sym)
        {
            fprintf(stderr, "Error: Variable %s not declared.\n", expr->value);
            exit(1);
        }
        if (sym->type.is_array)
        {
            fprintf(stderr, "Error: Array %s requires index access.\n", expr->value);
            exit(1);
        }
        return sym->type;
    }
    if (strcmp(kind, "ID_ARRAY") == 0)
    {
        Symbol *sym = sym_lookup(expr->value);
        if (!sym)
        {
            fprintf(stderr, "Error: Array %s not declared.\n", expr->value);
            exit(1);
        }
        if (!sym->type.is_array)
        {
            fprintf(stderr, "Error: %s is not declared as an array.\n", expr->value);
            exit(1);
        }
        if (!expr->left)
        {
            fprintf(stderr, "Error: Array %s access missing index expression.\n", expr->value);
            exit(1);
        }
        Type index_type = infer_expression_type(expr->left);
        if (index_type.base != INT || index_type.is_array)
        {
            fprintf(stderr, "Error: Array %s index must be an integer expression.\n", expr->value);
            exit(1);
        }
        Type elem_type = sym->type;
        elem_type.is_array = false;
        elem_type.array_size = 0;
        return elem_type;
    }
    if (strcmp(kind, "EXEC") == 0)
    {
        Symbol *sym = sym_lookup(expr->value);
        if (!sym || sym->symbol_type != SYMBOL_FUNCTION)
        {
            fprintf(stderr, "Error: Function %s not declared.\n", expr->value ? expr->value : "<anonymous>");
            exit(1);
        }
        return sym->type;
    }
    if (strcmp(kind, "NOT") == 0)
    {
        Type operand = infer_expression_type(expr->left);
        if (operand.base != BOOL || operand.is_array)
        {
            fprintf(stderr, "Error: NOT operator requires boolean expression.\n");
            exit(1);
        }
        return make_type(BOOL);
    }
    if (strcmp(kind, "AND") == 0 || strcmp(kind, "OR") == 0)
    {
        Type left = infer_expression_type(expr->left);
        Type right = infer_expression_type(expr->right);
        if (left.base != BOOL || right.base != BOOL || left.is_array || right.is_array)
        {
            fprintf(stderr, "Error: Logical operators require boolean operands.\n");
            exit(1);
        }
        return make_type(BOOL);
    }
    if (strcmp(kind, "EQ") == 0 || strcmp(kind, "NEQ") == 0)
    {
        Type left = infer_expression_type(expr->left);
        Type right = infer_expression_type(expr->right);
        if (left.is_array || right.is_array)
        {
            fprintf(stderr, "Error: Equality operators do not support array operands.\n");
            exit(1);
        }
        if (left.base != right.base)
        {
            fprintf(stderr, "Error: Equality operators require operands of the same type.\n");
            exit(1);
        }
        return make_type(BOOL);
    }
    if (strcmp(kind, "LT") == 0 || strcmp(kind, "LEQ") == 0 ||
        strcmp(kind, "GT") == 0 || strcmp(kind, "GEQ") == 0)
    {
        Type left = infer_expression_type(expr->left);
        Type right = infer_expression_type(expr->right);
        if (!is_numeric_base(left.base) || !is_numeric_base(right.base) ||
            left.is_array || right.is_array)
        {
            fprintf(stderr, "Error: Relational operators require numeric operands.\n");
            exit(1);
        }
        if (left.base != right.base)
        {
            fprintf(stderr, "Error: Relational operator operands must be of the same type.\n");
            exit(1);
        }
        return make_type(BOOL);
    }
    if (strcmp(kind, "ADD") == 0 || strcmp(kind, "MINUS") == 0 ||
        strcmp(kind, "MULT") == 0 || strcmp(kind, "DIV") == 0)
    {
        Type left = infer_expression_type(expr->left);
        Type right = infer_expression_type(expr->right);
        if (!is_numeric_base(left.base) || !is_numeric_base(right.base) ||
            left.is_array || right.is_array)
        {
            fprintf(stderr, "Error: Arithmetic operators require numeric operands.\n");
            exit(1);
        }
        if (left.base != right.base)
        {
            fprintf(stderr, "Error: Arithmetic operands must be of the same type.\n");
            exit(1);
        }
        return left;
    }
    if (strcmp(kind, "ARRAY_VALUES") == 0)
    {
        fprintf(stderr, "Error: Array literals cannot be used in scalar expressions without explicit handling.\n");
        exit(1);
    }

    fprintf(stderr, "Error: Unsupported expression kind %s.\n", kind ? kind : "<null>");
    exit(1);
}