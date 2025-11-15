#ifndef ARDUINO_IR_H
#define ARDUINO_IR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Arduino-specific intermediate representation
// This IR is designed to represent operations that translate to Arduino C++ code

typedef enum {
    // Basic operations (similar to original VM)
    ARD_OP_CONST = 1,       // arg: const_index
    ARD_OP_LOAD_VAR,        // arg: var_index
    ARD_OP_STORE_VAR,       // arg: var_index
    ARD_OP_ADD,
    ARD_OP_SUB,
    ARD_OP_MUL,
    ARD_OP_DIV,
    ARD_OP_MOD,
    ARD_OP_EQ,
    ARD_OP_LT,
    ARD_OP_GT,
    ARD_OP_LE,
    ARD_OP_GE,
    ARD_OP_NE,
    ARD_OP_AND,
    ARD_OP_OR,
    ARD_OP_NOT,
    ARD_OP_NEG,
    
    // Control flow
    ARD_OP_JMP,             // arg: target ip
    ARD_OP_JZ,              // arg: target ip (jump if false)
    ARD_OP_JNZ,             // arg: target ip (jump if true)
    ARD_OP_LABEL,           // arg: label_id
    
    // Arduino-specific operations
    ARD_OP_PIN_MODE,        // args: pin, mode (INPUT=0, OUTPUT=1, INPUT_PULLUP=2)
    ARD_OP_DIGITAL_WRITE,   // args: pin, value (HIGH=1, LOW=0)
    ARD_OP_DIGITAL_READ,    // arg: pin -> push result
    ARD_OP_ANALOG_WRITE,    // args: pin, value (0-255)
    ARD_OP_ANALOG_READ,     // arg: pin -> push result
    ARD_OP_DELAY,           // arg: milliseconds
    ARD_OP_DELAY_MICROS,    // arg: microseconds
    ARD_OP_MILLIS,          // -> push current time in ms
    ARD_OP_MICROS,          // -> push current time in us
    
    // Serial operations
    ARD_OP_SERIAL_BEGIN,    // arg: baud_rate
    ARD_OP_SERIAL_PRINT,    // print top of stack
    ARD_OP_SERIAL_PRINTLN,  // print top of stack with newline
    ARD_OP_SERIAL_READ,     // -> push byte read
    ARD_OP_SERIAL_AVAILABLE, // -> push bytes available
    
    // Function operations
    ARD_OP_CALL,            // args: func_index
    ARD_OP_RETURN,
    ARD_OP_SETUP_START,     // marks beginning of setup()
    ARD_OP_SETUP_END,       // marks end of setup()
    ARD_OP_LOOP_START,      // marks beginning of loop()
    ARD_OP_LOOP_END,        // marks end of loop()
    
    // Variable declarations
    ARD_OP_DECLARE_VAR,     // args: var_index, type, initial_value
    ARD_OP_DECLARE_ARRAY,   // args: var_index, type, size
    
    // Array operations
    ARD_OP_LOAD_ARRAY,      // args: var_index, index -> push value
    ARD_OP_STORE_ARRAY,     // args: var_index, index, value
    
    // Utility
    ARD_OP_POP,
    ARD_OP_DUP,             // duplicate top of stack
    ARD_OP_COMMENT          // arg: string_index (for code comments)
} ArduinoOpCode;

typedef enum {
    ARD_TYPE_INT = 1,
    ARD_TYPE_BOOL,
    ARD_TYPE_CHAR,
    ARD_TYPE_FLOAT,
    ARD_TYPE_BYTE,
    ARD_TYPE_LONG
} ArduinoType;

typedef struct {
    ArduinoType type;
    union {
        long long l;
        int i;
        double f;
        char c;
        int b;
        unsigned char byte_val;
    } as;
} ArduinoValue;

typedef struct {
    int *code;
    int count;
    int capacity;
} ArduinoChunk;

typedef struct {
    char *name;
    ArduinoType type;
    int is_array;
    int array_size;
    ArduinoValue initial_value;
} ArduinoVariable;

typedef struct {
    char *name;
    int param_count;
    ArduinoType *param_types;
    ArduinoType return_type;
    ArduinoChunk chunk;
    ArduinoValue *consts;
    int const_count;
    int const_cap;
    char **strings;        // for comments and string literals
    int string_count;
    int string_cap;
} ArduinoFunction;

typedef struct {
    ArduinoFunction *functions;
    int func_count;
    int func_cap;
    ArduinoVariable *variables;
    int var_count;
    int var_cap;
    ArduinoFunction setup_func;    // setup() function
    ArduinoFunction loop_func;     // loop() function
    ArduinoFunction *user_funcs;   // user-defined functions
    int user_func_count;
    int user_func_cap;
} ArduinoModule;

// Helper functions
static inline void arduino_chunk_init(ArduinoChunk *c) {
    c->code = NULL;
    c->count = 0;
    c->capacity = 0;
}

static inline void arduino_chunk_free(ArduinoChunk *c) {
    free(c->code);
    c->code = NULL;
    c->count = c->capacity = 0;
}

static inline int arduino_chunk_emit(ArduinoChunk *c, int v) {
    if (c->count + 1 > c->capacity) {
        c->capacity = c->capacity ? c->capacity * 2 : 64;
        c->code = (int *)realloc(c->code, sizeof(int) * c->capacity);
    }
    c->code[c->count++] = v;
    return c->count - 1;
}

static inline void arduino_module_init(ArduinoModule *m) {
    m->functions = NULL;
    m->func_count = 0;
    m->func_cap = 0;
    m->variables = NULL;
    m->var_count = 0;
    m->var_cap = 0;
    m->user_funcs = NULL;
    m->user_func_count = 0;
    m->user_func_cap = 0;
    
    // Initialize setup and loop functions
    arduino_chunk_init(&m->setup_func.chunk);
    m->setup_func.name = "setup";
    m->setup_func.param_count = 0;
    m->setup_func.return_type = ARD_TYPE_INT; // void in Arduino
    m->setup_func.consts = NULL;
    m->setup_func.const_count = 0;
    m->setup_func.const_cap = 0;
    m->setup_func.strings = NULL;
    m->setup_func.string_count = 0;
    m->setup_func.string_cap = 0;
    
    arduino_chunk_init(&m->loop_func.chunk);
    m->loop_func.name = "loop";
    m->loop_func.param_count = 0;
    m->loop_func.return_type = ARD_TYPE_INT; // void in Arduino
    m->loop_func.consts = NULL;
    m->loop_func.const_count = 0;
    m->loop_func.const_cap = 0;
    m->loop_func.strings = NULL;
    m->loop_func.string_count = 0;
    m->loop_func.string_cap = 0;
}

static inline int arduino_function_add_const(ArduinoFunction *f, ArduinoValue v) {
    if (f->const_count + 1 > f->const_cap) {
        f->const_cap = f->const_cap ? f->const_cap * 2 : 8;
        f->consts = (ArduinoValue *)realloc(f->consts, sizeof(ArduinoValue) * f->const_cap);
    }
    f->consts[f->const_count] = v;
    return f->const_count++;
}

static inline int arduino_function_add_string(ArduinoFunction *f, const char *str) {
    if (f->string_count + 1 > f->string_cap) {
        f->string_cap = f->string_cap ? f->string_cap * 2 : 8;
        f->strings = (char **)realloc(f->strings, sizeof(char*) * f->string_cap);
    }
    f->strings[f->string_count] = (char *)malloc(strlen(str) + 1);
    strcpy(f->strings[f->string_count], str);
    return f->string_count++;
}

static inline int arduino_module_add_variable(ArduinoModule *m, ArduinoVariable var) {
    if (m->var_count + 1 > m->var_cap) {
        m->var_cap = m->var_cap ? m->var_cap * 2 : 8;
        m->variables = (ArduinoVariable *)realloc(m->variables, sizeof(ArduinoVariable) * m->var_cap);
    }
    m->variables[m->var_count] = var;
    return m->var_count++;
}

// Arduino pin and mode constants
#define ARD_INPUT 0
#define ARD_OUTPUT 1
#define ARD_INPUT_PULLUP 2

#define ARD_HIGH 1
#define ARD_LOW 0

#endif