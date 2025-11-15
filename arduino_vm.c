#include "arduino_ir.h"
#include <string.h>
#include <math.h>

typedef struct {
    ArduinoValue *stack;
    int sp;                    // stack pointer
    int capacity;
    FILE *output;             // output file for generated Arduino code
    ArduinoValue *variables;  // global variables
    int var_count;
    int in_setup;             // flag to track if we're in setup()
    int in_loop;              // flag to track if we're in loop()
    int indent_level;         // for code formatting
} ArduinoVM;

void arduino_vm_init(ArduinoVM *vm, FILE *output) {
    vm->stack = NULL;
    vm->sp = 0;
    vm->capacity = 0;
    vm->output = output;
    vm->variables = NULL;
    vm->var_count = 0;
    vm->in_setup = 0;
    vm->in_loop = 0;
    vm->indent_level = 0;
}

void arduino_vm_free(ArduinoVM *vm) {
    free(vm->stack);
    free(vm->variables);
    vm->stack = NULL;
    vm->variables = NULL;
    vm->sp = vm->capacity = 0;
    vm->var_count = 0;
}

static void push(ArduinoVM *vm, ArduinoValue v) {
    if (vm->sp + 1 > vm->capacity) {
        vm->capacity = vm->capacity ? vm->capacity * 2 : 256;
        vm->stack = (ArduinoValue *)realloc(vm->stack, sizeof(ArduinoValue) * vm->capacity);
    }
    vm->stack[vm->sp++] = v;
}

static ArduinoValue pop(ArduinoVM *vm) {
    ArduinoValue zero;
    memset(&zero, 0, sizeof(zero));
    return vm->sp > 0 ? vm->stack[--vm->sp] : zero;
}

static ArduinoValue peek(ArduinoVM *vm) {
    ArduinoValue zero;
    memset(&zero, 0, sizeof(zero));
    return vm->sp > 0 ? vm->stack[vm->sp - 1] : zero;
}

// Helper to write indented code
static void write_indent(ArduinoVM *vm) {
    for (int i = 0; i < vm->indent_level; i++) {
        fprintf(vm->output, "  ");
    }
}

// Convert value to appropriate Arduino type string
static const char* arduino_type_to_string(ArduinoType type) {
    switch (type) {
        case ARD_TYPE_INT: return "int";
        case ARD_TYPE_BOOL: return "bool";
        case ARD_TYPE_CHAR: return "char";
        case ARD_TYPE_FLOAT: return "float";
        case ARD_TYPE_BYTE: return "byte";
        case ARD_TYPE_LONG: return "long";
        default: return "int";
    }
}

// Generate Arduino code for binary operations
static void generate_binary_op(ArduinoVM *vm, const char *op) {
    ArduinoValue b = pop(vm);
    ArduinoValue a = pop(vm);
    ArduinoValue result;
    
    write_indent(vm);
    fprintf(vm->output, "(");
    
    // Generate the expression
    switch (a.type) {
        case ARD_TYPE_INT:
            fprintf(vm->output, "%d", a.as.i);
            break;
        case ARD_TYPE_FLOAT:
            fprintf(vm->output, "%.6f", a.as.f);
            break;
        case ARD_TYPE_BOOL:
            fprintf(vm->output, "%s", a.as.b ? "true" : "false");
            break;
        case ARD_TYPE_CHAR:
            fprintf(vm->output, "'%c'", a.as.c);
            break;
        default:
            fprintf(vm->output, "%d", a.as.i);
    }
    
    fprintf(vm->output, " %s ", op);
    
    switch (b.type) {
        case ARD_TYPE_INT:
            fprintf(vm->output, "%d", b.as.i);
            break;
        case ARD_TYPE_FLOAT:
            fprintf(vm->output, "%.6f", b.as.f);
            break;
        case ARD_TYPE_BOOL:
            fprintf(vm->output, "%s", b.as.b ? "true" : "false");
            break;
        case ARD_TYPE_CHAR:
            fprintf(vm->output, "'%c'", b.as.c);
            break;
        default:
            fprintf(vm->output, "%d", b.as.i);
    }
    
    fprintf(vm->output, ")");
    
    // For now, just push a dummy result
    result.type = a.type;
    result.as.i = 0;
    push(vm, result);
}

int arduino_vm_run(ArduinoVM *vm, ArduinoModule *module) {
    // Generate Arduino program header
    fprintf(vm->output, "// Generated Arduino code\n");
    fprintf(vm->output, "// Translated from custom language\n\n");
    
    // Generate variable declarations
    for (int i = 0; i < module->var_count; i++) {
        ArduinoVariable *var = &module->variables[i];
        fprintf(vm->output, "%s %s", arduino_type_to_string(var->type), var->name);
        
        if (var->is_array) {
            fprintf(vm->output, "[%d]", var->array_size);
        }
        
        // Initialize with default value if specified
        if (!var->is_array) {
            switch (var->type) {
                case ARD_TYPE_INT:
                    if (var->initial_value.as.i != 0) {
                        fprintf(vm->output, " = %d", var->initial_value.as.i);
                    }
                    break;
                case ARD_TYPE_FLOAT:
                    if (var->initial_value.as.f != 0.0) {
                        fprintf(vm->output, " = %.6f", var->initial_value.as.f);
                    }
                    break;
                case ARD_TYPE_BOOL:
                    fprintf(vm->output, " = %s", var->initial_value.as.b ? "true" : "false");
                    break;
                case ARD_TYPE_CHAR:
                    if (var->initial_value.as.c != 0) {
                        fprintf(vm->output, " = '%c'", var->initial_value.as.c);
                    }
                    break;
                default:
                    break;
            }
        }
        
        fprintf(vm->output, ";\n");
    }
    
    if (module->var_count > 0) {
        fprintf(vm->output, "\n");
    }
    
    // Execute setup function
    fprintf(vm->output, "void setup() {\n");
    vm->indent_level = 1;
    vm->in_setup = 1;
    
    ArduinoFunction *setup = &module->setup_func;
    for (int ip = 0; ip < setup->chunk.count; ) {
        int op = setup->chunk.code[ip++];
        
        switch (op) {
            case ARD_OP_CONST: {
                int k = setup->chunk.code[ip++];
                push(vm, setup->consts[k]);
                break;
            }
            
            case ARD_OP_PIN_MODE: {
                int pin = setup->chunk.code[ip++];
                int mode = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "pinMode(%d, %s);\n", pin, 
                    mode == ARD_INPUT ? "INPUT" : 
                    mode == ARD_OUTPUT ? "OUTPUT" : "INPUT_PULLUP");
                break;
            }
            
            case ARD_OP_SERIAL_BEGIN: {
                int baud = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "Serial.begin(%d);\n", baud);
                break;
            }
            
            case ARD_OP_DIGITAL_WRITE: {
                int pin = setup->chunk.code[ip++];
                int value = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "digitalWrite(%d, %s);\n", pin, 
                    value ? "HIGH" : "LOW");
                break;
            }
            
            case ARD_OP_DELAY: {
                int ms = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "delay(%d);\n", ms);
                break;
            }
            
            case ARD_OP_STORE_VAR: {
                int var_idx = setup->chunk.code[ip++];
                ArduinoValue val = pop(vm);
                write_indent(vm);
                fprintf(vm->output, "%s = ", module->variables[var_idx].name);
                
                switch (val.type) {
                    case ARD_TYPE_INT:
                        fprintf(vm->output, "%d", val.as.i);
                        break;
                    case ARD_TYPE_FLOAT:
                        fprintf(vm->output, "%.6f", val.as.f);
                        break;
                    case ARD_TYPE_BOOL:
                        fprintf(vm->output, "%s", val.as.b ? "true" : "false");
                        break;
                    case ARD_TYPE_CHAR:
                        fprintf(vm->output, "'%c'", val.as.c);
                        break;
                    default:
                        fprintf(vm->output, "%d", val.as.i);
                }
                
                fprintf(vm->output, ";\n");
                break;
            }
            
            case ARD_OP_COMMENT: {
                int str_idx = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "// %s\n", setup->strings[str_idx]);
                break;
            }
            
            case ARD_OP_SETUP_START:
                // Already in setup function, just continue
                break;
                
            case ARD_OP_JMP: {
                int target = setup->chunk.code[ip++];
                ip = target;
                break;
            }
            
            case ARD_OP_JZ: {
                int target = setup->chunk.code[ip++];
                ArduinoValue cond = pop(vm);
                if (cond.as.i == 0) {
                    ip = target;
                }
                break;
            }
            
            case ARD_OP_JNZ: {
                int target = setup->chunk.code[ip++];
                ArduinoValue cond = pop(vm);
                if (cond.as.i != 0) {
                    ip = target;
                }
                break;
            }
            
            case ARD_OP_LABEL: {
                int label_id = setup->chunk.code[ip++];
                (void)label_id; // Labels are just markers, skip them
                break;
            }
            
            case ARD_OP_SETUP_END:
                goto setup_end;
                
            default:
                fprintf(stderr, "Unknown opcode in setup: %d (at ip=%d)\n", op, ip-1);
                // Skip this instruction and continue
                break;
        }
    }
    
setup_end:
    vm->indent_level = 0;
    vm->in_setup = 0;
    fprintf(vm->output, "}\n\n");
    
    // Execute loop function
    fprintf(vm->output, "void loop() {\n");
    vm->indent_level = 1;
    vm->in_loop = 1;
    
    ArduinoFunction *loop = &module->loop_func;
    for (int ip = 0; ip < loop->chunk.count; ) {
        int op = loop->chunk.code[ip++];
        
        switch (op) {
            case ARD_OP_CONST: {
                int k = loop->chunk.code[ip++];
                push(vm, loop->consts[k]);
                break;
            }
            
            case ARD_OP_LOAD_VAR: {
                int var_idx = loop->chunk.code[ip++];
                ArduinoValue val;
                if (var_idx >= 0 && var_idx < module->var_count) {
                    val.type = module->variables[var_idx].type;
                    val.as.i = 0; // Default value
                } else {
                    val.type = ARD_TYPE_INT;
                    val.as.i = 0;
                }
                push(vm, val);
                break;
            }
            
            case ARD_OP_JMP: {
                int target = loop->chunk.code[ip++];
                ip = target;
                break;
            }
            
            case ARD_OP_JZ: {
                int target = loop->chunk.code[ip++];
                ArduinoValue cond = pop(vm);
                if (cond.as.i == 0) {
                    ip = target;
                }
                break;
            }
            
            case ARD_OP_JNZ: {
                int target = loop->chunk.code[ip++];
                ArduinoValue cond = pop(vm);
                if (cond.as.i != 0) {
                    ip = target;
                }
                break;
            }
            
            case ARD_OP_LABEL: {
                int label_id = loop->chunk.code[ip++];
                (void)label_id; // Labels are just markers, skip them
                break;
            }
            
            case ARD_OP_PIN_MODE: {
                int pin = loop->chunk.code[ip++];
                int mode = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "pinMode(%d, %s);\n", pin, 
                    mode == ARD_INPUT ? "INPUT" : 
                    mode == ARD_OUTPUT ? "OUTPUT" : "INPUT_PULLUP");
                break;
            }
            
            case ARD_OP_DIGITAL_READ: {
                int pin = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "digitalRead(%d)", pin);
                ArduinoValue result;
                result.type = ARD_TYPE_BOOL;
                result.as.b = 0;
                push(vm, result);
                break;
            }
            
            case ARD_OP_ANALOG_READ: {
                int pin = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "analogRead(%d)", pin);
                ArduinoValue result;
                result.type = ARD_TYPE_INT;
                result.as.i = 0;
                push(vm, result);
                break;
            }
            
            case ARD_OP_DIGITAL_WRITE: {
                int pin = loop->chunk.code[ip++];
                int value = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "digitalWrite(%d, %s);\n", pin, 
                    value ? "HIGH" : "LOW");
                break;
            }
            
            case ARD_OP_ANALOG_WRITE: {
                int pin = loop->chunk.code[ip++];
                int value = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "analogWrite(%d, %d);\n", pin, value);
                break;
            }
            
            case ARD_OP_DELAY: {
                int ms = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "delay(%d);\n", ms);
                break;
            }
            
            case ARD_OP_SERIAL_PRINT: {
                ArduinoValue val = pop(vm);
                write_indent(vm);
                fprintf(vm->output, "Serial.print(");
                
                switch (val.type) {
                    case ARD_TYPE_INT:
                        fprintf(vm->output, "%d", val.as.i);
                        break;
                    case ARD_TYPE_FLOAT:
                        fprintf(vm->output, "%.6f", val.as.f);
                        break;
                    case ARD_TYPE_BOOL:
                        fprintf(vm->output, "%s", val.as.b ? "true" : "false");
                        break;
                    case ARD_TYPE_CHAR:
                        fprintf(vm->output, "'%c'", val.as.c);
                        break;
                    default:
                        fprintf(vm->output, "%d", val.as.i);
                }
                
                fprintf(vm->output, ");\n");
                break;
            }
            
            case ARD_OP_SERIAL_PRINTLN: {
                ArduinoValue val = pop(vm);
                write_indent(vm);
                fprintf(vm->output, "Serial.println(");
                
                switch (val.type) {
                    case ARD_TYPE_INT:
                        fprintf(vm->output, "%d", val.as.i);
                        break;
                    case ARD_TYPE_FLOAT:
                        fprintf(vm->output, "%.6f", val.as.f);
                        break;
                    case ARD_TYPE_BOOL:
                        fprintf(vm->output, "%s", val.as.b ? "true" : "false");
                        break;
                    case ARD_TYPE_CHAR:
                        fprintf(vm->output, "'%c'", val.as.c);
                        break;
                    default:
                        fprintf(vm->output, "%d", val.as.i);
                }
                
                fprintf(vm->output, ");\n");
                break;
            }
            
            case ARD_OP_ADD:
                generate_binary_op(vm, "+");
                break;
            case ARD_OP_SUB:
                generate_binary_op(vm, "-");
                break;
            case ARD_OP_MUL:
                generate_binary_op(vm, "*");
                break;
            case ARD_OP_DIV:
                generate_binary_op(vm, "/");
                break;
            case ARD_OP_MOD:
                generate_binary_op(vm, "%");
                break;
                
            case ARD_OP_EQ:
                generate_binary_op(vm, "==");
                break;
            case ARD_OP_NE:
                generate_binary_op(vm, "!=");
                break;
            case ARD_OP_LT:
                generate_binary_op(vm, "<");
                break;
            case ARD_OP_LE:
                generate_binary_op(vm, "<=");
                break;
            case ARD_OP_GT:
                generate_binary_op(vm, ">");
                break;
            case ARD_OP_GE:
                generate_binary_op(vm, ">=");
                break;
                
            case ARD_OP_AND:
                generate_binary_op(vm, "&&");
                break;
            case ARD_OP_OR:
                generate_binary_op(vm, "||");
                break;
                
            case ARD_OP_NOT: {
                ArduinoValue val = pop(vm);
                write_indent(vm);
                fprintf(vm->output, "!(");
                
                switch (val.type) {
                    case ARD_TYPE_BOOL:
                        fprintf(vm->output, "%s", val.as.b ? "true" : "false");
                        break;
                    default:
                        fprintf(vm->output, "%d", val.as.i);
                }
                
                fprintf(vm->output, ")");
                
                ArduinoValue result;
                result.type = ARD_TYPE_BOOL;
                result.as.b = !val.as.b;
                push(vm, result);
                break;
            }
            
            case ARD_OP_POP: {
                pop(vm);
                break;
            }
            
            case ARD_OP_COMMENT: {
                int str_idx = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "// %s\n", loop->strings[str_idx]);
                break;
            }
            
            case ARD_OP_LOOP_START:
                // Already in loop function, just continue
                break;
                
            case ARD_OP_LOOP_END:
                goto loop_end;
                
            default:
                fprintf(stderr, "Unknown opcode in loop: %d (at ip=%d)\n", op, ip-1);
                // Skip this instruction and continue
                break;
        }
    }
    
loop_end:
    vm->indent_level = 0;
    vm->in_loop = 0;
    fprintf(vm->output, "}\n");
    
    return 1;
}