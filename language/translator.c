#include "translator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define VM_NUM_REGISTERS 8
#define MAX_ARRAY_BINDINGS 32

/* Opcodes subset needed for the current translator */
typedef enum {
    OP_NOP      = 0x00,
    OP_ADD      = 0x01,
    OP_SUB      = 0x02,
    OP_MUL      = 0x03,
    OP_DIV      = 0x04,
    OP_AND      = 0x06,
    OP_OR       = 0x07,
    OP_NOT      = 0x09,
    OP_CMP      = 0x0A,
    OP_LOAD     = 0x10,
    OP_LOADI    = 0x11,
    OP_LOADI16  = 0x12,
    OP_STORE    = 0x13,
    OP_PUSH     = 0x15,
    OP_POP      = 0x16,
    OP_LOADM    = 0x18,
    OP_JMP      = 0x20,
    OP_JZ       = 0x21,
    OP_JNZ      = 0x22,
    OP_JLT      = 0x23,
    OP_JGT      = 0x24,
    OP_JLE      = 0x25,
    OP_JGE      = 0x26,
    OP_CALL     = 0x27,
    OP_RET      = 0x28,
    OP_HALT     = 0x29
} Opcode;

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} BytecodeBuffer;

typedef struct {
    char *name;
    size_t start_offset;   /* byte offset in buffer */
    size_t start_index;    /* instruction index */
    uint8_t param_regs[VM_NUM_REGISTERS];
    size_t param_count;
} FunctionInfo;

static const char *opcode_name(Opcode op) {
    switch (op) {
        case OP_NOP:     return "NOP";
        case OP_ADD:     return "ADD";
        case OP_SUB:     return "SUB";
        case OP_MUL:     return "MUL";
        case OP_DIV:     return "DIV";
        case OP_AND:     return "AND";
        case OP_OR:      return "OR";
        case OP_NOT:     return "NOT";
        case OP_CMP:     return "CMP";
        case OP_LOAD:    return "LOAD";
        case OP_LOADI:   return "LOADI";
        case OP_LOADI16: return "LOADI16";
        case OP_STORE:   return "STORE";
        case OP_PUSH:    return "PUSH";
        case OP_POP:     return "POP";
        case OP_LOADM:   return "LOADM";
        case OP_JMP:     return "JMP";
        case OP_JZ:      return "JZ";
        case OP_JNZ:     return "JNZ";
        case OP_JLT:     return "JLT";
        case OP_JGT:     return "JGT";
        case OP_JLE:     return "JLE";
        case OP_JGE:     return "JGE";
        case OP_CALL:    return "CALL";
        case OP_RET:     return "RET";
        case OP_HALT:    return "HALT";
        default:         return "UNKNOWN";
    }
}

typedef struct {
    char *name;
    uint8_t reg;
} VarBinding;

typedef struct {
    char *name;
    uint16_t base_addr;
    size_t length;
} ArrayBinding;

typedef struct {
    BytecodeBuffer code;
    VarBinding vars[VM_NUM_REGISTERS];
    size_t var_count;
    uint8_t next_var_reg;
    uint8_t used_regs_mask;
    ArrayBinding arrays[MAX_ARRAY_BINDINGS];
    size_t array_count;
    uint16_t heap_top;
    FunctionInfo functions[16];
    size_t function_count;
    FunctionInfo *current_function;
    bool in_function;
    bool failed;
    char error[256];
} Translator;

typedef struct {
    uint8_t reg;
    bool is_temp;
} RegValue;

static void buffer_init(BytecodeBuffer *buf) {
    buf->capacity = 64;
    buf->size = 0;
    buf->data = (uint8_t *) malloc(buf->capacity);
}

static void buffer_reserve(BytecodeBuffer *buf, size_t extra) {
    if (buf->size + extra <= buf->capacity) return;
    while (buf->size + extra > buf->capacity) {
        buf->capacity *= 2;
    }
    buf->data = (uint8_t *) realloc(buf->data, buf->capacity);
}

static size_t emit_instruction(BytecodeBuffer *buf, Opcode op, uint8_t arg1, uint8_t arg2) {
    buffer_reserve(buf, 3);
    size_t pos = buf->size;
    buf->data[buf->size++] = op;
    buf->data[buf->size++] = arg1;
    buf->data[buf->size++] = arg2;
    return pos;
}

static void translator_fail(Translator *tr, const char *msg) {
    if (!tr->failed) {
        tr->failed = true;
        strncpy(tr->error, msg, sizeof(tr->error) - 1);
        tr->error[sizeof(tr->error) - 1] = '\0';
    }
}

static void translator_init(Translator *tr) {
    buffer_init(&tr->code);
    tr->var_count = 0;
    tr->next_var_reg = 1;      /* R0 is reserved as accumulator */
    tr->used_regs_mask = 1;    /* R0 is used */
    tr->array_count = 0;
    tr->heap_top = 0;
    tr->function_count = 0;
    tr->current_function = NULL;
    tr->in_function = false;
    tr->failed = false;
    tr->error[0] = '\0';
}

static void translator_destroy(Translator *tr) {
    free(tr->code.data);
    for (size_t i = 0; i < tr->var_count; ++i) {
        free(tr->vars[i].name);
    }
    for (size_t i = 0; i < tr->array_count; ++i) {
        free(tr->arrays[i].name);
    }
    for (size_t i = 0; i < tr->function_count; ++i) {
        free(tr->functions[i].name);
    }
}

static void translator_clear_vars(Translator *tr) {
    for (size_t i = 0; i < tr->var_count; ++i) {
        free(tr->vars[i].name);
        tr->vars[i].name = NULL;
    }
    tr->var_count = 0;
}

static void translator_clear_arrays(Translator *tr) {
    for (size_t i = 0; i < tr->array_count; ++i) {
        free(tr->arrays[i].name);
        tr->arrays[i].name = NULL;
    }
    tr->array_count = 0;
    tr->heap_top = 0;
}

static void translator_reset_registers(Translator *tr) {
    translator_clear_vars(tr);
    translator_clear_arrays(tr);
    tr->next_var_reg = 1;
    tr->used_regs_mask = 1;
}

static VarBinding *find_var(Translator *tr, const char *name) {
    for (size_t i = 0; i < tr->var_count; ++i) {
        if (strcmp(tr->vars[i].name, name) == 0) {
            return &tr->vars[i];
        }
    }
    return NULL;
}

static ArrayBinding *find_array(Translator *tr, const char *name) {
    for (size_t i = 0; i < tr->array_count; ++i) {
        if (strcmp(tr->arrays[i].name, name) == 0) {
            return &tr->arrays[i];
        }
    }
    return NULL;
}

static ArrayBinding *register_array(Translator *tr, const char *name, size_t length) {
    if (length == 0) {
        translator_fail(tr, "Array size must be greater than zero");
        return NULL;
    }
    if (length > 255) {
        translator_fail(tr, "Array size exceeds supported limit (255 elements)");
        return NULL;
    }
    if (tr->array_count >= MAX_ARRAY_BINDINGS) {
        translator_fail(tr, "Exceeded maximum number of arrays supported");
        return NULL;
    }
    if ((uint32_t) tr->heap_top + length > 255) {
        translator_fail(tr, "Array allocations exceed available address space");
        return NULL;
    }
    ArrayBinding *binding = &tr->arrays[tr->array_count++];
    binding->name = strdup(name);
    binding->base_addr = tr->heap_top;
    binding->length = length;
    tr->heap_top += (uint16_t) length;
    return binding;
}

static FunctionInfo *find_function_info(Translator *tr, const char *name) {
    for (size_t i = 0; i < tr->function_count; ++i) {
        if (strcmp(tr->functions[i].name, name) == 0) {
            return &tr->functions[i];
        }
    }
    return NULL;
}

static VarBinding *register_var(Translator *tr, const char *name) {
    if (tr->used_regs_mask & (1 << tr->next_var_reg)) {
        translator_fail(tr, "Register limit reached (max 7 user registers)");
        return NULL;
    }
    if (tr->var_count >= VM_NUM_REGISTERS - 1) {
        translator_fail(tr, "Too many variables for current translator backend");
        return NULL;
    }
    VarBinding *binding = &tr->vars[tr->var_count++];
    binding->name = strdup(name);
    binding->reg = tr->next_var_reg++;
    tr->used_regs_mask |= (1 << binding->reg);
    return binding;
}

static uint8_t alloc_temp(Translator *tr) {
    for (int i = VM_NUM_REGISTERS - 1; i >= 0; --i) {
        if (!(tr->used_regs_mask & (1 << i))) {
            tr->used_regs_mask |= (1 << i);
            return i;
        }
    }
    translator_fail(tr, "Out of registers for temporaries");
    return 0;
}

static void release_temp(Translator *tr, uint8_t reg) {
    tr->used_regs_mask &= ~(1 << reg);
}

static void emit_move(Translator *tr, uint8_t dst, uint8_t src) {
    if (dst == src) return;
    emit_instruction(&tr->code, OP_LOAD, dst, src);
}

static void patch_address(BytecodeBuffer *buf, size_t instr_offset, uint16_t target) {
    buf->data[instr_offset + 1] = target & 0xFF;
    buf->data[instr_offset + 2] = (target >> 8) & 0xFF;
}

static uint16_t current_address(const Translator *tr) {
    return (uint16_t) tr->code.size;
}

static void emit_load_const(Translator *tr, uint8_t dst, long value) {
    if (value >= 0 && value <= 255) {
        emit_instruction(&tr->code, OP_LOADI, dst, (uint8_t) value);
    } else {
        translator_fail(tr, "Immediate out of supported range (0..255)");
    }
}

static RegValue translate_expression(Translator *tr, Node *expr);
static bool translate_statement(Translator *tr, Node *stmt);
static bool translate_block(Translator *tr, Node *block);
static bool translate_function(Translator *tr, Node *func);
static bool translate_exec(Translator *tr, Node *node);
static bool translate_if(Translator *tr, Node *node);
static bool translate_while(Translator *tr, Node *node);
static bool translate_for(Translator *tr, Node *node);
static bool translate_array_declaration(Translator *tr, Node *node);
static bool translate_array_assignment(Translator *tr, ArrayBinding *array, Node *values);

static RegValue make_error_reg(void) {
    RegValue r = {0, false};
    return r;
}

static RegValue array_element_address(Translator *tr, ArrayBinding *binding, long index) {
    if (index < 0 || (size_t) index >= binding->length) {
        translator_fail(tr, "Array index out of bounds");
        return make_error_reg();
    }
    uint32_t absolute = binding->base_addr + (uint32_t) index;
    if (absolute > 255) {
        translator_fail(tr, "Array address exceeds 8-bit immediate range");
        return make_error_reg();
    }
    uint8_t addr_reg = alloc_temp(tr);
    if (tr->failed) {
        return make_error_reg();
    }
    emit_load_const(tr, addr_reg, (long) absolute);
    RegValue r = {addr_reg, true};
    return r;
}

static RegValue translate_binary_arith(Translator *tr, Node *node, Opcode op) {
    RegValue result = make_error_reg();
    RegValue lhs = translate_expression(tr, node->left);
    if (tr->failed) return result;
    RegValue rhs = translate_expression(tr, node->right);
    if (tr->failed) return result;

    emit_instruction(&tr->code, op, lhs.reg, rhs.reg);
    emit_instruction(&tr->code, OP_LOAD, lhs.reg, 0); /* copy R0 into lhs */
    if (rhs.is_temp) release_temp(tr, rhs.reg);
    return lhs;
}

static RegValue translate_binary_logic(Translator *tr, Node *node, Opcode op) {
    RegValue result = make_error_reg();
    RegValue lhs = translate_expression(tr, node->left);
    if (tr->failed) return result;
    RegValue rhs = translate_expression(tr, node->right);
    if (tr->failed) return result;

    emit_instruction(&tr->code, op, lhs.reg, rhs.reg);
    emit_instruction(&tr->code, OP_LOAD, lhs.reg, 0);
    if (rhs.is_temp) release_temp(tr, rhs.reg);
    return lhs;
}

static RegValue translate_unary_not(Translator *tr, Node *node) {
    RegValue operand = translate_expression(tr, node->left);
    if (tr->failed) return make_error_reg();
    emit_instruction(&tr->code, OP_NOT, operand.reg, 0);
    emit_instruction(&tr->code, OP_LOAD, operand.reg, 0);
    return operand;
}

static RegValue translate_comparison(Translator *tr, Node *node, Opcode jump_opcode) {
    RegValue result = make_error_reg();
    RegValue lhs = translate_expression(tr, node->left);
    if (tr->failed) return result;
    RegValue rhs = translate_expression(tr, node->right);
    if (tr->failed) return result;

    uint8_t dst = alloc_temp(tr);
    if (tr->failed) {
        if (rhs.is_temp) release_temp(tr, rhs.reg);
        if (lhs.is_temp) release_temp(tr, lhs.reg);
        return result;
    }

    emit_instruction(&tr->code, OP_CMP, lhs.reg, rhs.reg);
    size_t jump_true = emit_instruction(&tr->code, jump_opcode, 0, 0);
    emit_load_const(tr, dst, 0);
    size_t jump_end = emit_instruction(&tr->code, OP_JMP, 0, 0);
    uint16_t true_addr = current_address(tr);
    patch_address(&tr->code, jump_true, true_addr);
    emit_load_const(tr, dst, 1);
    uint16_t end_addr = current_address(tr);
    patch_address(&tr->code, jump_end, end_addr);

    if (rhs.is_temp) release_temp(tr, rhs.reg);
    if (lhs.is_temp) release_temp(tr, lhs.reg);

    RegValue out = {dst, true};
    return out;
}

static size_t emit_jump_if_zero(Translator *tr, uint8_t reg) {
    uint8_t zero = alloc_temp(tr);
    if (tr->failed) return (size_t) -1;
    emit_load_const(tr, zero, 0);
    emit_instruction(&tr->code, OP_CMP, reg, zero);
    release_temp(tr, zero);
    return emit_instruction(&tr->code, OP_JZ, 0, 0);
}

static bool translate_exec(Translator *tr, Node *node) {
    if (!node->value) {
        translator_fail(tr, "EXEC node missing target name");
        return false;
    }
    FunctionInfo *info = find_function_info(tr, node->value);
    if (!info) {
        translator_fail(tr, "Call to unknown function");
        return false;
    }
    size_t arg_count = node->list ? node->list->size : 0;
    if (arg_count != info->param_count) {
        translator_fail(tr, "Argument count mismatch in function call");
        return false;
    }
    RegValue args[VM_NUM_REGISTERS];
    for (size_t i = 0; i < arg_count; ++i) {
        args[i] = translate_expression(tr, node->list->items[i]);
        if (tr->failed) {
            for (size_t j = 0; j < i; ++j) {
                if (args[j].is_temp) release_temp(tr, args[j].reg);
            }
            return false;
        }
    }
    for (int reg = 1; reg < VM_NUM_REGISTERS; ++reg) {
        emit_instruction(&tr->code, OP_PUSH, (uint8_t) reg, 0);
    }
    for (size_t i = 0; i < arg_count; ++i) {
        emit_move(tr, info->param_regs[i], args[i].reg);
        if (args[i].is_temp) release_temp(tr, args[i].reg);
    }
    uint16_t addr = (uint16_t) info->start_offset;
    emit_instruction(&tr->code, OP_CALL, addr & 0xFF, (addr >> 8) & 0xFF);
    for (int reg = VM_NUM_REGISTERS - 1; reg >= 1; --reg) {
        emit_instruction(&tr->code, OP_POP, (uint8_t) reg, 0);
    }
    return true;
}

static RegValue translate_expression(Translator *tr, Node *expr) {
    if (!expr) {
        translator_fail(tr, "Unexpected empty expression");
        return make_error_reg();
    }
    const char *kind = expr->node_type;
    if (strcmp(kind, "INT") == 0) {
        uint8_t temp = alloc_temp(tr);
        if (tr->failed) return make_error_reg();
        emit_load_const(tr, temp, expr->ivalue);
        RegValue r = {temp, true};
        return r;
    }
    if (strcmp(kind, "CHAR") == 0) {
        uint8_t temp = alloc_temp(tr);
        if (tr->failed) return make_error_reg();
        emit_load_const(tr, temp, (unsigned char) expr->cvalue);
        RegValue r = {temp, true};
        return r;
    }
    if (strcmp(kind, "BOOL") == 0) {
        uint8_t temp = alloc_temp(tr);
        if (tr->failed) return make_error_reg();
        emit_load_const(tr, temp, expr->bvalue ? 1 : 0);
        RegValue r = {temp, true};
        return r;
    }
    if (strcmp(kind, "ID") == 0) {
        VarBinding *binding = find_var(tr, expr->value);
        if (!binding) {
            translator_fail(tr, "Undeclared identifier encountered during translation");
            return make_error_reg();
        }
        RegValue r = {binding->reg, false};
        return r;
    }
    if (strcmp(kind, "ID_ARRAY") == 0) {
        ArrayBinding *binding = find_array(tr, expr->value);
        if (!binding) {
            translator_fail(tr, "Use of undeclared array");
            return make_error_reg();
        }
        if (!expr->left || strcmp(expr->left->node_type, "INT") != 0) {
            translator_fail(tr, "Array index must be an integer literal");
            return make_error_reg();
        }
        RegValue addr = array_element_address(tr, binding, expr->left->ivalue);
        if (tr->failed) return make_error_reg();
        uint8_t dst = alloc_temp(tr);
        if (tr->failed) {
            if (addr.is_temp) release_temp(tr, addr.reg);
            return make_error_reg();
        }
        emit_instruction(&tr->code, OP_LOADM, dst, addr.reg);
        if (addr.is_temp) release_temp(tr, addr.reg);
        RegValue r = {dst, true};
        return r;
    }
    if (strcmp(kind, "ADD") == 0) {
        return translate_binary_arith(tr, expr, OP_ADD);
    }
    if (strcmp(kind, "MINUS") == 0) {
        return translate_binary_arith(tr, expr, OP_SUB);
    }
    if (strcmp(kind, "MULT") == 0) {
        return translate_binary_arith(tr, expr, OP_MUL);
    }
    if (strcmp(kind, "DIV") == 0) {
        return translate_binary_arith(tr, expr, OP_DIV);
    }
    if (strcmp(kind, "AND") == 0) {
        return translate_binary_logic(tr, expr, OP_AND);
    }
    if (strcmp(kind, "OR") == 0) {
        return translate_binary_logic(tr, expr, OP_OR);
    }
    if (strcmp(kind, "NOT") == 0) {
        return translate_unary_not(tr, expr);
    }
    if (strcmp(kind, "EQ") == 0) {
        return translate_comparison(tr, expr, OP_JZ);
    }
    if (strcmp(kind, "NEQ") == 0) {
        return translate_comparison(tr, expr, OP_JNZ);
    }
    if (strcmp(kind, "LT") == 0) {
        return translate_comparison(tr, expr, OP_JLT);
    }
    if (strcmp(kind, "GT") == 0) {
        return translate_comparison(tr, expr, OP_JGT);
    }
    if (strcmp(kind, "LEQ") == 0) {
        return translate_comparison(tr, expr, OP_JLE);
    }
    if (strcmp(kind, "GEQ") == 0) {
        return translate_comparison(tr, expr, OP_JGE);
    }
    translator_fail(tr, "Expression type not supported by translator yet");
    return make_error_reg();
}

static bool translate_declaration(Translator *tr, Node *node) {
    if (!node->value) {
        translator_fail(tr, "Declaration without identifier");
        return false;
    }
    if (find_var(tr, node->value)) {
        translator_fail(tr, "Variable redeclaration detected");
        return false;
    }
    VarBinding *binding = register_var(tr, node->value);
    if (!binding) return false;

    if (node->right) {
        Node *init_expr = node->right;
        if (strcmp(init_expr->node_type, "ASSIGN") == 0 && init_expr->right) {
            init_expr = init_expr->right;
        }
        if (strcmp(init_expr->node_type, "ARRAY_VALUES") == 0) {
            if (!init_expr->list || init_expr->list->size != 1) {
                translator_fail(tr, "Scalar initialization with array literal requires exactly one element");
                return false;
            }
            init_expr = init_expr->list->items[0];
        }
        RegValue init = translate_expression(tr, init_expr);
        if (tr->failed) return false;
        emit_move(tr, binding->reg, init.reg);
        if (init.is_temp) release_temp(tr, init.reg);
    } else {
        emit_load_const(tr, binding->reg, 0);
    }
    return !tr->failed;
}

static bool translate_array_declaration(Translator *tr, Node *node) {
    if (!node->value || !node->right || strcmp(node->right->node_type, "INT") != 0) {
        translator_fail(tr, "Malformed array declaration");
        return false;
    }
    if (find_var(tr, node->value) || find_array(tr, node->value)) {
        translator_fail(tr, "Array redeclaration detected");
        return false;
    }
    long length = node->right->ivalue;
    if (length <= 0) {
        translator_fail(tr, "Array size must be positive");
        return false;
    }
    ArrayBinding *binding = register_array(tr, node->value, (size_t) length);
    return binding != NULL && !tr->failed;
}

static bool translate_array_assignment(Translator *tr, ArrayBinding *array, Node *values) {
    if (!values || strcmp(values->node_type, "ARRAY_VALUES") != 0) {
        translator_fail(tr, "Arrays can only be assigned from array literals");
        return false;
    }
    size_t provided = values->list ? (size_t) values->list->size : 0;
    if (provided > array->length) {
        translator_fail(tr, "Array literal has more elements than target array");
        return false;
    }
    size_t i = 0;
    for (; i < provided; ++i) {
        Node *expr = values->list->items[i];
        RegValue val = translate_expression(tr, expr);
        if (tr->failed) return false;
        RegValue addr = array_element_address(tr, array, (long) i);
        if (tr->failed) {
            if (val.is_temp) release_temp(tr, val.reg);
            return false;
        }
        emit_instruction(&tr->code, OP_STORE, addr.reg, val.reg);
        if (addr.is_temp) release_temp(tr, addr.reg);
        if (val.is_temp) release_temp(tr, val.reg);
    }
    if (i < array->length) {
        uint8_t zero = alloc_temp(tr);
        if (tr->failed) return false;
        emit_load_const(tr, zero, 0);
        for (; i < array->length; ++i) {
            RegValue addr = array_element_address(tr, array, (long) i);
            if (tr->failed) {
                release_temp(tr, zero);
                return false;
            }
            emit_instruction(&tr->code, OP_STORE, addr.reg, zero);
            if (addr.is_temp) release_temp(tr, addr.reg);
        }
        release_temp(tr, zero);
    }
    return !tr->failed;
}

static bool translate_assignment(Translator *tr, Node *node) {
    if (!node->left || strcmp(node->left->node_type, "ID") != 0) {
        translator_fail(tr, "Only simple assignments are supported");
        return false;
    }
    const char *target = node->left->value;
    VarBinding *binding = find_var(tr, target);
    ArrayBinding *array = find_array(tr, target);
    if (array && !binding) {
        return translate_array_assignment(tr, array, node->right);
    }
    if (!binding) {
        translator_fail(tr, "Assignment to undeclared variable");
        return false;
    }
    Node *rhs = node->right;
    if (rhs && strcmp(rhs->node_type, "ARRAY_VALUES") == 0) {
        if (!rhs->list || rhs->list->size != 1) {
            translator_fail(tr, "Scalar assignment with array literal requires exactly one element");
            return false;
        }
        rhs = rhs->list->items[0];
    }
    RegValue value = translate_expression(tr, rhs);
    if (tr->failed) return false;
    emit_move(tr, binding->reg, value.reg);
    if (value.is_temp) release_temp(tr, value.reg);
    return true;
}

static bool translate_if(Translator *tr, Node *node) {
    if (!node->left || !node->right) {
        translator_fail(tr, "Malformed IF statement");
        return false;
    }
    RegValue cond = translate_expression(tr, node->left);
    if (tr->failed) return false;
    size_t jump_false = emit_jump_if_zero(tr, cond.reg);
    if (cond.is_temp) release_temp(tr, cond.reg);
    if (jump_false == (size_t) -1) return false;
    if (!translate_block(tr, node->right)) return false;
    size_t jump_end = (size_t) -1;
    if (node->extra) {
        jump_end = emit_instruction(&tr->code, OP_JMP, 0, 0);
    }
    uint16_t false_addr = current_address(tr);
    patch_address(&tr->code, jump_false, false_addr);
    if (node->extra) {
        if (!translate_block(tr, node->extra)) return false;
        uint16_t end_addr = current_address(tr);
        patch_address(&tr->code, jump_end, end_addr);
    }
    return !tr->failed;
}

static bool translate_while(Translator *tr, Node *node) {
    if (!node->left || !node->list || node->list->size == 0) {
        translator_fail(tr, "Malformed WHILE statement");
        return false;
    }
    uint16_t loop_start = current_address(tr);
    RegValue cond = translate_expression(tr, node->left);
    if (tr->failed) return false;
    size_t exit_jump = emit_jump_if_zero(tr, cond.reg);
    if (cond.is_temp) release_temp(tr, cond.reg);
    if (exit_jump == (size_t) -1) return false;
    if (!translate_block(tr, node->list->items[0])) return false;
    emit_instruction(&tr->code, OP_JMP, loop_start & 0xFF, (loop_start >> 8) & 0xFF);
    uint16_t exit_addr = current_address(tr);
    patch_address(&tr->code, exit_jump, exit_addr);
    return !tr->failed;
}

static bool translate_for(Translator *tr, Node *node) {
    if (node->left) {
        if (!translate_statement(tr, node->left) || tr->failed) {
            return false;
        }
    }
    uint16_t loop_start = current_address(tr);
    if (!node->right) {
        translator_fail(tr, "FOR loop missing condition");
        return false;
    }
    RegValue cond = translate_expression(tr, node->right);
    if (tr->failed) return false;
    size_t exit_jump = emit_jump_if_zero(tr, cond.reg);
    if (cond.is_temp) release_temp(tr, cond.reg);
    if (exit_jump == (size_t) -1) return false;

    Node *body = (node->list && node->list->size > 0) ? node->list->items[0] : NULL;
    if (body && !translate_block(tr, body)) return false;
    if (node->extra) {
        if (!translate_statement(tr, node->extra) || tr->failed) {
            return false;
        }
    }
    emit_instruction(&tr->code, OP_JMP, loop_start & 0xFF, (loop_start >> 8) & 0xFF);
    uint16_t exit_addr = current_address(tr);
    patch_address(&tr->code, exit_jump, exit_addr);
    return !tr->failed;
}

static bool translate_statement(Translator *tr, Node *stmt) {
    if (!stmt) return true;
    const char *kind = stmt->node_type;
    if (strcmp(kind, "DECLARACION") == 0) {
        return translate_declaration(tr, stmt);
    }
    if (strcmp(kind, "DECLARACION_ARRAY") == 0) {
        return translate_array_declaration(tr, stmt);
    }
    if (strcmp(kind, "ASSIGN") == 0) {
        return translate_assignment(tr, stmt);
    }
    if (strcmp(kind, "BLOCK") == 0) {
        return translate_block(tr, stmt);
    }
    if (strcmp(kind, "IF") == 0) {
        return translate_if(tr, stmt);
    }
    if (strcmp(kind, "WHILE") == 0) {
        return translate_while(tr, stmt);
    }
    if (strcmp(kind, "FOR") == 0) {
        return translate_for(tr, stmt);
    }
    if (strcmp(kind, "RETURN") == 0) {
        if (!tr->in_function) {
            translator_fail(tr, "RETURN outside of function");
            return false;
        }
        Node *expr = stmt->left;
        RegValue value;
        if (expr) {
            value = translate_expression(tr, expr);
            if (tr->failed) return false;
            emit_move(tr, 0, value.reg);
            if (value.is_temp) release_temp(tr, value.reg);
        } else {
            emit_load_const(tr, 0, 0);
        }
        emit_instruction(&tr->code, OP_RET, 0, 0);
        return true;
    }
    if (strcmp(kind, "EXEC") == 0) {
        return translate_exec(tr, stmt);
    }
    translator_fail(tr, "Statement kind not supported in translator yet");
    return false;
}

static bool translate_block(Translator *tr, Node *block) {
    if (!block->list) return true;
    for (int i = 0; i < block->list->size; ++i) {
        if (!translate_statement(tr, block->list->items[i]) || tr->failed) {
            return false;
        }
    }
    return true;
}

static bool translate_function(Translator *tr, Node *func) {
    if (!func->value) {
        translator_fail(tr, "Function without name");
        return false;
    }
    if (tr->function_count >= (sizeof(tr->functions) / sizeof(tr->functions[0]))) {
        translator_fail(tr, "Exceeded maximum number of functions supported");
        return false;
    }

    FunctionInfo *info = &tr->functions[tr->function_count++];
    info->name = strdup(func->value);
    info->start_offset = current_address(tr);
    info->start_index = info->start_offset / 3;
    info->param_count = 0;

    translator_reset_registers(tr);

    if (func->list) {
        for (int i = 0; i < func->list->size; ++i) {
            if (info->param_count >= VM_NUM_REGISTERS - 1) {
                translator_fail(tr, "Too many parameters for available registers");
                return false;
            }
            Node *param = func->list->items[i];
            if (!param || !param->value) {
                translator_fail(tr, "Invalid parameter node");
                return false;
            }
            VarBinding *binding = register_var(tr, param->value);
            if (!binding) return false;
            info->param_regs[info->param_count++] = binding->reg;
        }
    }

    bool previous_in_function = tr->in_function;
    FunctionInfo *previous_function = tr->current_function;
    tr->in_function = true;
    tr->current_function = info;

    if (!func->right) {
        translator_fail(tr, "Function missing body");
        tr->in_function = previous_in_function;
        tr->current_function = previous_function;
        translator_reset_registers(tr);
        return false;
    }

    bool ok = translate_block(tr, func->right);

    if (ok && (tr->code.size < 3 || tr->code.data[tr->code.size - 3] != OP_RET)) {
        emit_instruction(&tr->code, OP_RET, 0, 0);
    }

    tr->in_function = previous_in_function;
    tr->current_function = previous_function;
    translator_reset_registers(tr);
    return ok && !tr->failed;
}

static bool translate_root(Translator *tr, Node *root) {
    if (!root) {
        translator_fail(tr, "Empty AST");
        return false;
    }
    if (strcmp(root->node_type, "PROGRAM") != 0 || !root->list) {
        if (strcmp(root->node_type, "BLOCK") == 0) {
            return translate_block(tr, root);
        }
        return translate_statement(tr, root);
    }

    int total_nodes = root->list->size;
    int main_candidates = 0;
    bool has_functions = false;
    for (int i = 0; i < total_nodes; ++i) {
        Node *node = root->list->items[i];
        if (node && strcmp(node->node_type, "FUNCTION") == 0) {
            has_functions = true;
        } else {
            main_candidates++;
        }
    }

    Node **main_nodes = NULL;
    if (main_candidates > 0) {
        main_nodes = (Node **) calloc(main_candidates, sizeof(Node *));
    }

    size_t jump_pos = (size_t) -1;
    if (has_functions) {
        jump_pos = emit_instruction(&tr->code, OP_JMP, 0, 0);
    }

    int main_index = 0;
    for (int i = 0; i < total_nodes; ++i) {
        Node *node = root->list->items[i];
        if (!node) continue;
        if (strcmp(node->node_type, "FUNCTION") == 0) {
            if (!translate_function(tr, node) || tr->failed) {
                free(main_nodes);
                return false;
            }
        } else if (main_nodes) {
            main_nodes[main_index++] = node;
        }
    }

    if (has_functions) {
        uint16_t main_addr = current_address(tr);
        patch_address(&tr->code, jump_pos, main_addr);
    }

    translator_reset_registers(tr);
    tr->in_function = false;
    tr->current_function = NULL;

    bool ok = true;
    for (int i = 0; i < main_index; ++i) {
        Node *node = main_nodes[i];
        if (!node) continue;
        if (strcmp(node->node_type, "BLOCK") == 0) {
            ok = translate_block(tr, node);
        } else {
            ok = translate_statement(tr, node);
        }
        if (!ok || tr->failed) break;
    }

    free(main_nodes);
    return ok;
}

bool translate_program(Node *root, const char *output_path) {
    Translator tr;
    translator_init(&tr);

    bool ok = translate_root(&tr, root);
    if (ok && !tr.failed) {
        emit_instruction(&tr.code, OP_HALT, 0, 0);
        FILE *out = fopen(output_path, "wb");
        if (!out) {
            fprintf(stderr, "translator: unable to open %s for writing\n", output_path);
            ok = false;
        } else {
            fwrite(tr.code.data, 1, tr.code.size, out);
            fclose(out);
            fprintf(stderr, "translator: wrote %zu bytes to %s\n", tr.code.size, output_path);
        }
    } else {
        ok = false;
    }

    if (tr.failed) {
        fprintf(stderr, "translator error: %s\n", tr.error);
    }

    translator_destroy(&tr);
    return ok;
}
