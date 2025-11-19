#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symtab.h"

SymTab *current_scope = NULL;
void symtab_init(void) {
    current_scope = (SymTab *) malloc(sizeof(SymTab));
    current_scope->size = 0;
    current_scope->symbols = NULL;
    current_scope->parent = NULL;
}
SymTab* sym_create() {
    SymTab* symtab = (SymTab*) malloc(sizeof(SymTab));
    symtab->size = 0;
    symtab->symbols = NULL;
    symtab->parent = NULL;
    return symtab;
}
void enter_scope() {
    SymTab* new_scope = sym_create();
    new_scope->parent = current_scope;
    current_scope = new_scope;
}
void exit_scope() {
    if (current_scope->parent != NULL) {
        SymTab* old_scope = current_scope;
        current_scope = current_scope->parent;
        free(old_scope->symbols);
        free(old_scope);
    }
}
Symbol *sym_lookup_current(const char *name) {
    if (!current_scope) return NULL;
    for (Symbol *sym = current_scope->symbols; sym; sym = sym->next) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
    }
    return NULL;
}

/* Busca hacia arriba en todos los scopes anidados */
Symbol *sym_lookup(const char *name) {
    for (SymTab *s = current_scope; s; s = s->parent) {
        for (Symbol *sym = s->symbols; sym; sym = sym->next) {
            if (strcmp(sym->name, name) == 0) {
                return sym;
            }
        }
    }
    return NULL;
}

Symbol *sym_insert(const char *name, SymbolType symbol_type, Type type) {
    if (!current_scope) {
        fprintf(stderr, "sym_insert: no current scope\n");
        return NULL;
    }

    if (sym_lookup_current(name)) {
        fprintf(stderr, "Error semántico: '%s' ya está declarado en este ámbito\n", name);
        return NULL;
    }

    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Out of memory creating symbol\n");
        exit(1);
    }

    sym->name = strdup(name); 
    sym->symbol_type = symbol_type;
    sym->type = type;
    sym->param_count = 0;
    sym->param_type = NULL;

    sym->next = current_scope->symbols;
    current_scope->symbols = sym;

    return sym;
}

/* Inserta una función con información de parámetros */
Symbol *sym_insert_function(const char *name, Type return_type, int param_count, Type *param_types) {
    Symbol *sym = sym_insert(name, SYMBOL_FUNCTION, return_type);
    if (!sym) return NULL;

    if (param_count > 0 && param_types) {
        sym->param_count = param_count;
        sym->param_type = malloc(sizeof(Type) * param_count);
        if (!sym->param_type) {
            fprintf(stderr, "Out of memory copying param types\n");
            exit(1);
        }
        for (int i = 0; i < param_count; ++i) {
            sym->param_type[i] = param_types[i];
        }
    }
    return sym;
}