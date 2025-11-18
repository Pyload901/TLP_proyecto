#ifndef VM_H
#define VM_H

#include "arduino_ir.h"
#include <stdio.h>

// Arduino VM that executes bytecode and generates Arduino C++ code
typedef struct {
    ArduinoValue *stack;
    int sp;
    int capacity;
    FILE *output;
    ArduinoValue *variables;
    int var_count;
    int in_setup;
    int in_loop;
    int indent_level;
} VM;

// Initialize the VM with output file
void vm_init(VM *vm, FILE *output);

// Execute a module and generate Arduino code
// Returns 1 on success, 0 on failure
int vm_run(VM *vm, ArduinoModule *module);

// Free VM resources
void vm_free(VM *vm);

#endif