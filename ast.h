#include <stdbool.h>

// small implementation for assignment, bool operations, arithmetic operations and function execution
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
        int ivalue; // for integer literals
        float fvalue; // for float literals
        bool bvalue; // for boolean literals
    };
    Node *left;
    Node *right;
    Node *extra;

    List *list; // for complex items
};
List *new_list();
void list_add(List *list, Node *item);
Node *new_node(const char *type);
Node *new_int_node(int value);
Node *new_float_node(float value);
Node *new_bool_node(bool value);
Node *new_value_node(const char *value);
Node *new_binary_op_node(const char *type, Node *left, Node *right);
Node *new_unary_op_node(const char *type, Node *operand);
Node *new_function_call_node(const char *name, List *args);
Node *new_assignment_node(const char *name, Node *value);
void ast_print(Node *node, int indent);