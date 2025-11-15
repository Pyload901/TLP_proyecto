#ifndef COMPILER_H
#define COMPILER_H

#include "arduino_ir.h"

// Arduino language compiler that generates Arduino bytecode
typedef struct {
    const char *src;
    ArduinoModule module;
    int error_count;
} ArduinoCompiler;

// Compiles a robot program to Arduino bytecode module
// Returns 1 on success, 0 on failure
int compile_source(ArduinoCompiler *c, const char *src);

#endif