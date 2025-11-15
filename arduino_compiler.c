#include "arduino_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified AST node types for compatibility
typedef enum {
    SIMPLE_NK_PROGRAM, SIMPLE_NK_BLOCK, SIMPLE_NK_VAR_DECL, SIMPLE_NK_ASSIGN, 
    SIMPLE_NK_EXEC, SIMPLE_NK_INT, SIMPLE_NK_FLOAT, SIMPLE_NK_BOOL, SIMPLE_NK_CHAR,
    SIMPLE_NK_ID, SIMPLE_NK_BINOP, SIMPLE_NK_UNOP
} SimpleNodeKind;

typedef enum {
    SIMPLE_TY_INT, SIMPLE_TY_BOOL, SIMPLE_TY_CHAR, SIMPLE_TY_FLOAT
} SimpleTypeKind;

typedef enum {
    SIMPLE_OP_ADD, SIMPLE_OP_SUB, SIMPLE_OP_MUL, SIMPLE_OP_DIV, SIMPLE_OP_MOD,
    SIMPLE_OP_EQ, SIMPLE_OP_NE, SIMPLE_OP_LT, SIMPLE_OP_LE, SIMPLE_OP_GT, SIMPLE_OP_GE,
    SIMPLE_OP_AND, SIMPLE_OP_OR, SIMPLE_OP_NOT, SIMPLE_OP_NEG
} SimpleOpKind;

typedef struct SimpleAstNode SimpleAstNode;
typedef struct SimpleAstList SimpleAstList;

struct SimpleAstList { 
    SimpleAstNode **items; 
    int size, cap; 
};

struct SimpleAstNode {
    SimpleNodeKind k;
    union {
        struct { SimpleAstList *items; } program;
        struct { SimpleAstList *stmts; } block;
        struct { SimpleTypeKind type; char *name; int isArray; long arrSize; SimpleAstNode *initExpr; } vardecl;
        struct { char *name; SimpleAstNode *rhs; } assign;
        struct { char *name; SimpleAstList *args; } exec;
        struct { long ival; } lit_i;
        struct { double fval; } lit_f;
        struct { int bval; } lit_b;
        struct { char cval; } lit_c;
        struct { char *name; } id;
        struct { SimpleOpKind op; SimpleAstNode *left, *right; } binop;
        struct { SimpleOpKind op; SimpleAstNode *expr; } unop;
    };
};

// Forward declaration of the Arduino VM
typedef struct ArduinoVM ArduinoVM;
void arduino_vm_init(ArduinoVM *vm, FILE *output);
int arduino_vm_run(ArduinoVM *vm, ArduinoModule *module);
void arduino_vm_free(ArduinoVM *vm);

typedef struct {
    ArduinoModule *module;
    ArduinoFunction *current_func;  // Currently compiling function
    int var_counter;                // For generating unique variable names
    int label_counter;              // For generating unique labels
} ArduinoCompiler;

// Map types to Arduino types
ArduinoType map_type(SimpleTypeKind tk) {
    switch (tk) {
        case SIMPLE_TY_INT: return ARD_TYPE_INT;
        case SIMPLE_TY_BOOL: return ARD_TYPE_BOOL;
        case SIMPLE_TY_CHAR: return ARD_TYPE_CHAR;
        case SIMPLE_TY_FLOAT: return ARD_TYPE_FLOAT;
        default: return ARD_TYPE_INT;
    }
}

// Map operators to Arduino opcodes
ArduinoOpCode map_binary_op(SimpleOpKind op) {
    switch (op) {
        case SIMPLE_OP_ADD: return ARD_OP_ADD;
        case SIMPLE_OP_SUB: return ARD_OP_SUB;
        case SIMPLE_OP_MUL: return ARD_OP_MUL;
        case SIMPLE_OP_DIV: return ARD_OP_DIV;
        case SIMPLE_OP_MOD: return ARD_OP_MOD;
        case SIMPLE_OP_EQ: return ARD_OP_EQ;
        case SIMPLE_OP_NE: return ARD_OP_NE;
        case SIMPLE_OP_LT: return ARD_OP_LT;
        case SIMPLE_OP_LE: return ARD_OP_LE;
        case SIMPLE_OP_GT: return ARD_OP_GT;
        case SIMPLE_OP_GE: return ARD_OP_GE;
        case SIMPLE_OP_AND: return ARD_OP_AND;
        case SIMPLE_OP_OR: return ARD_OP_OR;
        default: return ARD_OP_ADD;
    }
}

ArduinoOpCode map_unary_op(SimpleOpKind op) {
    switch (op) {
        case SIMPLE_OP_NOT: return ARD_OP_NOT;
        case SIMPLE_OP_NEG: return ARD_OP_NEG;
        default: return ARD_OP_NOT;
    }
}

void arduino_compiler_init(ArduinoCompiler *comp) {
    comp->module = (ArduinoModule*)malloc(sizeof(ArduinoModule));
    arduino_module_init(comp->module);
    comp->current_func = NULL;
    comp->var_counter = 0;
    comp->label_counter = 0;
}

void arduino_compiler_free(ArduinoCompiler *comp) {
    if (comp->module) {
        // Free module resources
        free(comp->module->variables);
        free(comp->module->user_funcs);
        free(comp->module);
    }
}

// Find variable index by name
int find_variable(ArduinoCompiler *comp, const char *name) {
    for (int i = 0; i < comp->module->var_count; i++) {
        if (strcmp(comp->module->variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Add a constant to the current function
int add_constant(ArduinoCompiler *comp, ArduinoValue val) {
    if (!comp->current_func) return -1;
    return arduino_function_add_const(comp->current_func, val);
}

// Forward declaration
void compile_expression(ArduinoCompiler *comp, SimpleAstNode *node);

// Compile literal values
void compile_literal(ArduinoCompiler *comp, SimpleAstNode *node) {
    ArduinoValue val;
    
    switch (node->k) {
        case SIMPLE_NK_INT:
            val.type = ARD_TYPE_INT;
            val.as.i = (int)node->lit_i.ival;
            break;
        case SIMPLE_NK_FLOAT:
            val.type = ARD_TYPE_FLOAT;
            val.as.f = node->lit_f.fval;
            break;
        case SIMPLE_NK_BOOL:
            val.type = ARD_TYPE_BOOL;
            val.as.b = node->lit_b.bval;
            break;
        case SIMPLE_NK_CHAR:
            val.type = ARD_TYPE_CHAR;
            val.as.c = node->lit_c.cval;
            break;
        default:
            return;
    }
    
    int const_idx = add_constant(comp, val);
    arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_CONST);
    arduino_chunk_emit(&comp->current_func->chunk, const_idx);
}

// Compile variable access
void compile_variable_access(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k == SIMPLE_NK_ID) {
        int var_idx = find_variable(comp, node->id.name);
        if (var_idx >= 0) {
            arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_LOAD_VAR);
            arduino_chunk_emit(&comp->current_func->chunk, var_idx);
        }
    }
}

// Compile binary operations
void compile_binary_op(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k != SIMPLE_NK_BINOP) return;
    
    // Compile left and right operands
    compile_expression(comp, node->binop.left);
    compile_expression(comp, node->binop.right);
    
    // Emit operation opcode
    ArduinoOpCode op = map_binary_op(node->binop.op);
    arduino_chunk_emit(&comp->current_func->chunk, op);
}

// Compile unary operations
void compile_unary_op(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k != SIMPLE_NK_UNOP) return;
    
    // Compile operand
    compile_expression(comp, node->unop.expr);
    
    // Emit operation opcode
    ArduinoOpCode op = map_unary_op(node->unop.op);
    arduino_chunk_emit(&comp->current_func->chunk, op);
}

// Compile expressions
void compile_expression(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (!node) return;
    
    switch (node->k) {
        case SIMPLE_NK_INT:
        case SIMPLE_NK_FLOAT:
        case SIMPLE_NK_BOOL:
        case SIMPLE_NK_CHAR:
            compile_literal(comp, node);
            break;
        case SIMPLE_NK_ID:
            compile_variable_access(comp, node);
            break;
        case SIMPLE_NK_BINOP:
            compile_binary_op(comp, node);
            break;
        case SIMPLE_NK_UNOP:
            compile_unary_op(comp, node);
            break;
        default:
            // Handle other expression types as needed
            break;
    }
}

// Compile assignment statement
void compile_assignment(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k != SIMPLE_NK_ASSIGN) return;
    
    // Compile the right-hand side expression
    if (node->assign.rhs) {
        compile_expression(comp, node->assign.rhs);
    }
    
    // Find the variable and emit store instruction
    int var_idx = find_variable(comp, node->assign.name);
    if (var_idx >= 0) {
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_STORE_VAR);
        arduino_chunk_emit(&comp->current_func->chunk, var_idx);
    }
}

// Compile variable declaration
void compile_var_declaration(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k != SIMPLE_NK_VAR_DECL) return;
    
    ArduinoVariable var;
    var.name = (char*)malloc(strlen(node->vardecl.name) + 1);
    strcpy(var.name, node->vardecl.name);
    var.type = map_type(node->vardecl.type);
    var.is_array = node->vardecl.isArray;
    var.array_size = node->vardecl.arrSize;
    
    // Initialize with default value
    memset(&var.initial_value, 0, sizeof(ArduinoValue));
    var.initial_value.type = var.type;
    
    if (node->vardecl.initExpr) {
        // If there's an initial value, set it
        switch (var.type) {
            case ARD_TYPE_INT:
                if (node->vardecl.initExpr->k == SIMPLE_NK_INT) {
                    var.initial_value.as.i = (int)node->vardecl.initExpr->lit_i.ival;
                }
                break;
            case ARD_TYPE_FLOAT:
                if (node->vardecl.initExpr->k == SIMPLE_NK_FLOAT) {
                    var.initial_value.as.f = node->vardecl.initExpr->lit_f.fval;
                }
                break;
            case ARD_TYPE_BOOL:
                if (node->vardecl.initExpr->k == SIMPLE_NK_BOOL) {
                    var.initial_value.as.b = node->vardecl.initExpr->lit_b.bval;
                }
                break;
            case ARD_TYPE_CHAR:
                if (node->vardecl.initExpr->k == SIMPLE_NK_CHAR) {
                    var.initial_value.as.c = node->vardecl.initExpr->lit_c.cval;
                }
                break;
            default:
                break;
        }
    }
    
    arduino_module_add_variable(comp->module, var);
}

// Compile exec function calls
void compile_exec_call(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (node->k != SIMPLE_NK_EXEC) return;
    
    // Map common function calls to Arduino operations
    const char *func_name = node->exec.name;
    
    if (strcmp(func_name, "digitalWrite") == 0) {
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_DIGITAL_WRITE);
        arduino_chunk_emit(&comp->current_func->chunk, 13); // default pin
        arduino_chunk_emit(&comp->current_func->chunk, 1);  // default HIGH
    } else if (strcmp(func_name, "pinMode") == 0) {
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_PIN_MODE);
        arduino_chunk_emit(&comp->current_func->chunk, 13); // default pin
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OUTPUT); // default OUTPUT
    } else if (strcmp(func_name, "delay") == 0) {
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_DELAY);
        arduino_chunk_emit(&comp->current_func->chunk, 1000); // default 1 second
    } else if (strcmp(func_name, "Serial_begin") == 0) {
        arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_SERIAL_BEGIN);
        arduino_chunk_emit(&comp->current_func->chunk, 9600); // default baud rate
    } else if (strcmp(func_name, "Serial_print") == 0) {
        if (node->exec.args && node->exec.args->size >= 1) {
            compile_expression(comp, node->exec.args->items[0]);
            arduino_chunk_emit(&comp->current_func->chunk, ARD_OP_SERIAL_PRINT);
        }
    }
}

// Compile statements
void compile_statement(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (!node) return;
    
    switch (node->k) {
        case SIMPLE_NK_ASSIGN:
            compile_assignment(comp, node);
            break;
        case SIMPLE_NK_VAR_DECL:
            compile_var_declaration(comp, node);
            break;
        case SIMPLE_NK_EXEC:
            compile_exec_call(comp, node);
            break;
        default:
            break;
    }
}

// Compile a block of statements
void compile_block(ArduinoCompiler *comp, SimpleAstNode *node) {
    if (!node || node->k != SIMPLE_NK_BLOCK) return;
    
    for (int i = 0; i < node->block.stmts->size; i++) {
        compile_statement(comp, node->block.stmts->items[i]);
    }
}

// Create a simple demo program
SimpleAstNode* create_simple_demo_program() {
    // Create a simple demo program
    SimpleAstNode *program = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    program->k = SIMPLE_NK_PROGRAM;
    program->program.items = (SimpleAstList*)malloc(sizeof(SimpleAstList));
    program->program.items->items = NULL;
    program->program.items->size = 0;
    program->program.items->cap = 0;
    
    // Create a simple block with some statements
    SimpleAstNode *block = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    block->k = SIMPLE_NK_BLOCK;
    block->block.stmts = (SimpleAstList*)malloc(sizeof(SimpleAstList));
    block->block.stmts->items = (SimpleAstNode**)malloc(sizeof(SimpleAstNode*) * 4);
    block->block.stmts->size = 3;
    block->block.stmts->cap = 4;
    
    // Variable declaration: int ledPin = 13;
    SimpleAstNode *var_decl = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    var_decl->k = SIMPLE_NK_VAR_DECL;
    var_decl->vardecl.type = SIMPLE_TY_INT;
    var_decl->vardecl.name = "ledPin";
    var_decl->vardecl.isArray = 0;
    var_decl->vardecl.arrSize = 0;
    
    // Initial value: 13
    SimpleAstNode *init_val = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    init_val->k = SIMPLE_NK_INT;
    init_val->lit_i.ival = 13;
    var_decl->vardecl.initExpr = init_val;
    
    // pinMode call
    SimpleAstNode *pin_mode = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    pin_mode->k = SIMPLE_NK_EXEC;
    pin_mode->exec.name = "pinMode";
    pin_mode->exec.args = NULL; // Simplified for demo
    
    // digitalWrite call
    SimpleAstNode *digital_write = (SimpleAstNode*)malloc(sizeof(SimpleAstNode));
    digital_write->k = SIMPLE_NK_EXEC;
    digital_write->exec.name = "digitalWrite";
    digital_write->exec.args = NULL; // Simplified for demo
    
    // Add statements to block
    block->block.stmts->items[0] = var_decl;
    block->block.stmts->items[1] = pin_mode;
    block->block.stmts->items[2] = digital_write;
    
    // Add block to program
    program->program.items->items = (SimpleAstNode**)malloc(sizeof(SimpleAstNode*));
    program->program.items->items[0] = block;
    program->program.items->size = 1;
    program->program.items->cap = 1;
    
    return program;
}

// Main compilation function - now uses simplified AST
ArduinoModule* compile_to_arduino(void *program_ptr) {
    SimpleAstNode *program = (SimpleAstNode*)program_ptr;
    
    if (!program) {
        // Create demo program if none provided
        program = create_simple_demo_program();
    }
    
    ArduinoCompiler comp;
    arduino_compiler_init(&comp);
    
    // Set up setup() function compilation
    comp.current_func = &comp.module->setup_func;
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_SETUP_START);
    
    // Add default setup operations
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_SERIAL_BEGIN);
    arduino_chunk_emit(&comp.current_func->chunk, 9600);
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_PIN_MODE);
    arduino_chunk_emit(&comp.current_func->chunk, 13);        // LED pin
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OUTPUT);
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_SETUP_END);
    
    // Set up loop() function compilation
    comp.current_func = &comp.module->loop_func;
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_LOOP_START);
    
    // Compile program if valid
    if (program->k == SIMPLE_NK_PROGRAM) {
        for (int i = 0; i < program->program.items->size; i++) {
            SimpleAstNode *item = program->program.items->items[i];
            
            if (item->k == SIMPLE_NK_BLOCK) {
                compile_block(&comp, item);
            } else {
                compile_statement(&comp, item);
            }
        }
    }
    
    // Add default loop operations (LED blink example)
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_DIGITAL_WRITE);
    arduino_chunk_emit(&comp.current_func->chunk, 13);   // pin
    arduino_chunk_emit(&comp.current_func->chunk, 1);    // HIGH
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_DELAY);
    arduino_chunk_emit(&comp.current_func->chunk, 1000); // 1 second
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_DIGITAL_WRITE);
    arduino_chunk_emit(&comp.current_func->chunk, 13);   // pin
    arduino_chunk_emit(&comp.current_func->chunk, 0);    // LOW
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_DELAY);
    arduino_chunk_emit(&comp.current_func->chunk, 1000); // 1 second
    
    arduino_chunk_emit(&comp.current_func->chunk, ARD_OP_LOOP_END);
    
    return comp.module;
}