#pragma once
#include <stdbool.h>

typedef enum {
    INT,
    DOUBLE,
    CHAR,
    BOOL,
    VOID,
} BaseType;

typedef struct Type {
    BaseType base;
    bool is_array;
    long array_size;
} Type;

static inline Type make_type(BaseType base) {
    Type t;
    t.base = base;
    t.is_array = false;
    t.array_size = 0;
    return t;
}

static inline Type make_array_type(BaseType base, long size) {
    Type t;
    t.base = base;
    t.is_array = true;
    t.array_size = size;
    return t;
}


typedef enum SymbolType {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolType;
typedef struct Symbol {
    char *name;
    Type type;
    SymbolType symbol_type;
    struct Symbol *next;

    int param_count;
    Type *param_type;

    bool is_builtin;
} Symbol;

typedef struct SymTab {
    Symbol *symbols;
    int size;
    struct SymTab *parent;
} SymTab;

extern SymTab *current_scope;

void symtab_init(void);
SymTab* sym_create();
void enter_scope();
void exit_scope();

Symbol* sym_insert(const char *name, SymbolType symbol_type, Type type);
Symbol *sym_insert_function(const char *name, Type return_type,
    int param_count,
    Type *param_types);
Symbol* sym_lookup(const char *name);
Symbol* sym_lookup_current(const char *name);


SymTab* symtab_create(SymTab *parent);
void symtab_destroy(SymTab *tab);
Type parse_type(const char *type_str);
const char* type_to_string(Type type);
bool types_compatible(Type t1, Type t2);
bool type_equals(Type t1, Type t2);
void print_symbol_table(SymTab *tab);

