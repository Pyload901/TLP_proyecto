#include "vm.h"
#include <stdlib.h>
#include <string.h>

// Stack operations
static void push(VM *vm, ArduinoValue val) {
    if (vm->sp >= vm->capacity) {
        vm->capacity = vm->capacity ? vm->capacity * 2 : 256;
        vm->stack = realloc(vm->stack, sizeof(ArduinoValue) * vm->capacity);
    }
    vm->stack[vm->sp++] = val;
}

static ArduinoValue pop(VM *vm) {
    if (vm->sp <= 0) {
        ArduinoValue zero = {0};
        return zero;
    }
    return vm->stack[--vm->sp];
}

// Utility functions
static void write_indent(VM *vm) {
    for (int i = 0; i < vm->indent_level; i++) {
        fprintf(vm->output, "  ");
    }
}

void vm_init(VM *vm, FILE *output) {
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

void vm_free(VM *vm) {
    free(vm->stack);
    free(vm->variables);
}

int vm_run(VM *vm, ArduinoModule *module) {
    // Generate Arduino program header
    fprintf(vm->output, "// Generated Arduino code\n");
    fprintf(vm->output, "// Translated from custom robot language\n");
    fprintf(vm->output, "// High-level robot control functions\n\n");
    
    // Generate custom function definitions
    fprintf(vm->output, "// Custom robot control functions\n");
    fprintf(vm->output, "void avanzar() {\n");
    fprintf(vm->output, "  // Move forward - both motors forward\n");
    fprintf(vm->output, "  digitalWrite(3, HIGH);  // Left motor forward\n");
    fprintf(vm->output, "  digitalWrite(4, LOW);\n");
    fprintf(vm->output, "  digitalWrite(5, HIGH);  // Right motor forward\n");
    fprintf(vm->output, "  digitalWrite(6, LOW);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void retroceder() {\n");
    fprintf(vm->output, "  // Move backward - both motors backward\n");
    fprintf(vm->output, "  digitalWrite(3, LOW);   // Left motor backward\n");
    fprintf(vm->output, "  digitalWrite(4, HIGH);\n");
    fprintf(vm->output, "  digitalWrite(5, LOW);   // Right motor backward\n");
    fprintf(vm->output, "  digitalWrite(6, HIGH);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void girar_izquierda() {\n");
    fprintf(vm->output, "  // Turn left - left motor backward, right motor forward\n");
    fprintf(vm->output, "  digitalWrite(3, LOW);   // Left motor backward\n");
    fprintf(vm->output, "  digitalWrite(4, HIGH);\n");
    fprintf(vm->output, "  digitalWrite(5, HIGH);  // Right motor forward\n");
    fprintf(vm->output, "  digitalWrite(6, LOW);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void girar_derecha() {\n");
    fprintf(vm->output, "  // Turn right - left motor forward, right motor backward\n");
    fprintf(vm->output, "  digitalWrite(3, HIGH);  // Left motor forward\n");
    fprintf(vm->output, "  digitalWrite(4, LOW);\n");
    fprintf(vm->output, "  digitalWrite(5, LOW);   // Right motor backward\n");
    fprintf(vm->output, "  digitalWrite(6, HIGH);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void detener() {\n");
    fprintf(vm->output, "  // Stop all motors\n");
    fprintf(vm->output, "  digitalWrite(3, LOW);\n");
    fprintf(vm->output, "  digitalWrite(4, LOW);\n");
    fprintf(vm->output, "  digitalWrite(5, LOW);\n");
    fprintf(vm->output, "  digitalWrite(6, LOW);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void encender_led() {\n");
    fprintf(vm->output, "  // Turn on LED\n");
    fprintf(vm->output, "  digitalWrite(13, HIGH);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "void apagar_led() {\n");
    fprintf(vm->output, "  // Turn off LED\n");
    fprintf(vm->output, "  digitalWrite(13, LOW);\n");
    fprintf(vm->output, "}\n\n");
    
    fprintf(vm->output, "int leer_sensor() {\n");
    fprintf(vm->output, "  // Read sensor from analog pin A0\n");
    fprintf(vm->output, "  return analogRead(A0);\n");
    fprintf(vm->output, "}\n\n");
    
    // Execute setup function
    ArduinoFunction *setup = &module->setup_func;
    vm->in_setup = 1;
    vm->indent_level = 1;
    int ip = 0;
    
    while (ip < setup->chunk.count) {
        ArduinoOpCode op = setup->chunk.code[ip++];
        
        switch (op) {
            case ARD_OP_SETUP_START:
                fprintf(vm->output, "void setup() {\n");
                break;
                
            case ARD_OP_SERIAL_BEGIN: {
                int baud = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "Serial.begin(%d);\n", baud);
                break;
            }
            
            case ARD_OP_PIN_MODE: {
                int pin = setup->chunk.code[ip++];
                int mode = setup->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "pinMode(%d, %s);\n", pin, 
                       mode == ARD_OUTPUT ? "OUTPUT" : "INPUT");
                break;
            }
            
            case ARD_OP_SETUP_END:
                fprintf(vm->output, "}\n\n");
                vm->in_setup = 0;
                goto setup_done;
                
            default:
                break;
        }
    }
    
setup_done:
    // Execute loop function
    ArduinoFunction *loop = &module->loop_func;
    vm->in_loop = 1;
    vm->indent_level = 1;
    ip = 0;
    
    while (ip < loop->chunk.count) {
        ArduinoOpCode op = loop->chunk.code[ip++];
        
        switch (op) {
            case ARD_OP_LOOP_START:
                fprintf(vm->output, "void loop() {\n");
                break;
                
            case ARD_OP_VAR_DECL: {
                int name_idx = loop->chunk.code[ip++];
                int initial_value = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "int %s = %d;\n", loop->strings[name_idx], initial_value);
                break;
            }
            
            case ARD_OP_IF_START: {
                int cond_idx = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "if (%s) {\n", loop->strings[cond_idx]);
                vm->indent_level++;
                break;
            }
            
            case ARD_OP_IF_END: {
                vm->indent_level--;
                write_indent(vm);
                fprintf(vm->output, "}\n");
                break;
            }
            
            case ARD_OP_CALL_CUSTOM: {
                int str_idx = loop->chunk.code[ip++];
                if (str_idx < loop->string_count) {
                    write_indent(vm);
                    fprintf(vm->output, "%s();\n", loop->strings[str_idx]);
                }
                break;
            }
            
            case ARD_OP_DELAY: {
                int delay_ms = loop->chunk.code[ip++];
                write_indent(vm);
                fprintf(vm->output, "delay(%d);\n", delay_ms);
                break;
            }
            
            case ARD_OP_LOOP_END:
                fprintf(vm->output, "}\n");
                vm->in_loop = 0;
                goto loop_done;
                
            default:
                break;
        }
    }
    
loop_done:
    return 1;
}