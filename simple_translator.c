#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arduino_ir.h"

// Arduino VM structure definition
typedef struct ArduinoVM {
    ArduinoValue *stack;
    int sp;
    int capacity;
    FILE *output;
    ArduinoValue *variables;
    int var_count;
    int in_setup;
    int in_loop;
    int indent_level;
} ArduinoVM;

// External functions from arduino_compiler.c and arduino_vm.c
extern ArduinoModule* compile_to_arduino(void *program_ptr);
extern int arduino_vm_run(ArduinoVM *vm, ArduinoModule *module);
extern void arduino_vm_init(ArduinoVM *vm, FILE *output);

// Simple text-based parsing for custom functions
typedef enum {
    CMD_EXEC_FUNCTION,
    CMD_DELAY,
    CMD_VARIABLE_DECL,
    CMD_IF_START,
    CMD_IF_END,
    CMD_CONDITION
} CommandType;

typedef struct {
    CommandType type;
    char* function_name;
    char* variable_name;
    char* condition;
    int delay_ms;
    int int_value;
    int indent_level;
} SimpleCommand;

typedef struct {
    SimpleCommand* commands;
    int count;
    int capacity;
} SimpleProgram;

SimpleProgram* parse_simple_file(const char* filename);
ArduinoModule* compile_simple_program(SimpleProgram* program);
void write_arduino_to_file(ArduinoModule* module, const char* output_file);

// Parse a simple text file with commands like:
// exec avanzar();
// delay(1000);
SimpleProgram* parse_simple_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    SimpleProgram* program = malloc(sizeof(SimpleProgram));
    program->commands = malloc(sizeof(SimpleCommand) * 50);
    program->count = 0;
    program->capacity = 50;
    
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (strstr(line, "//") || line[0] == '\n') continue;
        
        // Trim leading whitespace but track indent level
        char* trimmed = line;
        int indent = 0;
        while (*trimmed == ' ' || *trimmed == '\t') {
            trimmed++;
            indent++;
        }
        
        // Parse variable declarations: int variable = value;
        if (strstr(trimmed, "int ") && strstr(trimmed, "=") && strstr(trimmed, ";")) {
            char* var_start = strstr(trimmed, "int ") + 4;
            char* equals = strstr(var_start, "=");
            char* semicolon = strstr(equals, ";");
            
            if (var_start < equals && equals < semicolon) {
                // Extract variable name
                int var_len = equals - var_start;
                char* var_name = malloc(var_len + 1);
                strncpy(var_name, var_start, var_len);
                var_name[var_len] = '\0';
                
                // Remove whitespace from variable name
                char* clean_name = var_name;
                while (*clean_name == ' ') clean_name++;
                char* end = clean_name + strlen(clean_name) - 1;
                while (end > clean_name && *end == ' ') end--;
                *(end + 1) = '\0';
                
                // Extract value
                char* val_start = equals + 1;
                while (*val_start == ' ') val_start++;
                int value = atoi(val_start);
                
                program->commands[program->count].type = CMD_VARIABLE_DECL;
                program->commands[program->count].variable_name = malloc(strlen(clean_name) + 1);
                strcpy(program->commands[program->count].variable_name, clean_name);
                program->commands[program->count].int_value = value;
                program->commands[program->count].function_name = NULL;
                program->commands[program->count].condition = NULL;
                program->commands[program->count].delay_ms = 0;
                program->commands[program->count].indent_level = indent;
                program->count++;
                
                free(var_name);
            }
        }
        
        // Parse if statements: if(condition) start
        else if (strstr(trimmed, "if(") && strstr(trimmed, ") start")) {
            char* cond_start = strstr(trimmed, "if(") + 3;
            char* cond_end = strstr(cond_start, ") start");
            
            if (cond_start < cond_end) {
                int cond_len = cond_end - cond_start;
                char* condition = malloc(cond_len + 1);
                strncpy(condition, cond_start, cond_len);
                condition[cond_len] = '\0';
                
                program->commands[program->count].type = CMD_IF_START;
                program->commands[program->count].condition = condition;
                program->commands[program->count].function_name = NULL;
                program->commands[program->count].variable_name = NULL;
                program->commands[program->count].delay_ms = 0;
                program->commands[program->count].indent_level = indent;
                program->count++;
            }
        }
        
        // Parse end statements
        else if (strstr(trimmed, "end") && strlen(trimmed) <= 5) { // "end" or "end\n"
            program->commands[program->count].type = CMD_IF_END;
            program->commands[program->count].function_name = NULL;
            program->commands[program->count].variable_name = NULL;
            program->commands[program->count].condition = NULL;
            program->commands[program->count].delay_ms = 0;
            program->commands[program->count].indent_level = indent;
            program->count++;
        }
        
        // Parse exec commands
        else if (strstr(trimmed, "exec ") && strstr(trimmed, "();")) {
            char* start = strstr(trimmed, "exec ") + 5;
            char* end = strstr(start, "();");
            if (end > start) {
                int len = end - start;
                char* func_name = malloc(len + 1);
                strncpy(func_name, start, len);
                func_name[len] = '\0';
                
                program->commands[program->count].type = CMD_EXEC_FUNCTION;
                program->commands[program->count].function_name = func_name;
                program->commands[program->count].variable_name = NULL;
                program->commands[program->count].condition = NULL;
                program->commands[program->count].delay_ms = 0;
                program->commands[program->count].indent_level = indent;
                program->count++;
            }
        }
        
        // Parse delay commands
        else if (strstr(trimmed, "delay(") && strstr(trimmed, ");")) {
            char* start = strstr(trimmed, "delay(") + 6;
            char* end = strstr(start, ");");
            if (end > start) {
                int delay_val = atoi(start);
                program->commands[program->count].type = CMD_DELAY;
                program->commands[program->count].function_name = NULL;
                program->commands[program->count].variable_name = NULL;
                program->commands[program->count].condition = NULL;
                program->commands[program->count].delay_ms = delay_val;
                program->commands[program->count].indent_level = indent;
                program->count++;
            }
        }
    }
    
    fclose(file);
    printf("Parsed %d commands from %s\n", program->count, filename);
    return program;
}

// Convert simple program to Arduino module
ArduinoModule* compile_simple_program(SimpleProgram* program) {
    if (!program) return compile_to_arduino(NULL); // Use demo
    
    ArduinoModule* module = malloc(sizeof(ArduinoModule));
    arduino_module_init(module);
    
    // Setup function
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SETUP_START);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SERIAL_BEGIN);
    arduino_chunk_emit(&module->setup_func.chunk, 9600);
    
    // Setup motor pins as outputs
    for (int pin = 3; pin <= 6; pin++) {
        arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_PIN_MODE);
        arduino_chunk_emit(&module->setup_func.chunk, pin);
        arduino_chunk_emit(&module->setup_func.chunk, ARD_OUTPUT);
    }
    
    // LED pin
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_PIN_MODE);
    arduino_chunk_emit(&module->setup_func.chunk, 13);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OUTPUT);
    
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SETUP_END);
    
    // Loop function
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_START);
    
    // Add commands from parsed program
    for (int i = 0; i < program->count; i++) {
        SimpleCommand* cmd = &program->commands[i];
        
        switch (cmd->type) {
            case CMD_VARIABLE_DECL: {
                // Add variable declaration - for now we'll emit as a comment
                int str_idx = arduino_function_add_string(&module->loop_func, cmd->variable_name);
                arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_VAR_DECL);
                arduino_chunk_emit(&module->loop_func.chunk, str_idx);
                arduino_chunk_emit(&module->loop_func.chunk, cmd->int_value);
                break;
            }
            
            case CMD_IF_START: {
                // Start if statement with condition
                int str_idx = arduino_function_add_string(&module->loop_func, cmd->condition);
                arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_IF_START);
                arduino_chunk_emit(&module->loop_func.chunk, str_idx);
                break;
            }
            
            case CMD_IF_END: {
                // End if statement
                arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_IF_END);
                break;
            }
            
            case CMD_EXEC_FUNCTION: {
                // Custom function call
                int str_idx = arduino_function_add_string(&module->loop_func, cmd->function_name);
                arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_CALL_CUSTOM);
                arduino_chunk_emit(&module->loop_func.chunk, str_idx);
                break;
            }
            
            case CMD_DELAY: {
                // Delay command
                arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_DELAY);
                arduino_chunk_emit(&module->loop_func.chunk, cmd->delay_ms);
                break;
            }
            
            default:
                break;
        }
    }
    
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_END);
    
    return module;
}

// Write Arduino code to file
void write_arduino_to_file(ArduinoModule* module, const char* output_file) {
    FILE* file = fopen(output_file, "w");
    if (!file) {
        printf("Error: Cannot create output file %s\n", output_file);
        return;
    }
    
    // Initialize Arduino VM with file output
    ArduinoVM vm;
    arduino_vm_init(&vm, file);
    
    // Generate Arduino code
    arduino_vm_run(&vm, module);
    
    fclose(file);
    printf("Arduino code written to: %s\n", output_file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file> [output_file]\n", argv[0]);
        printf("Example: %s my_robot.prog robot.ino\n", argv[0]);
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = (argc >= 3) ? argv[2] : "generated_robot.ino";
    
    printf("Translating %s to Arduino...\n", input_file);
    
    // Parse the input file
    SimpleProgram* program = parse_simple_file(input_file);
    
    // Compile to Arduino IR
    ArduinoModule* module = compile_simple_program(program);
    
    // Generate .ino file
    write_arduino_to_file(module, output_file);
    
    // Show what commands were translated
    if (program) {
        printf("\nTranslated commands:\n");
        for (int i = 0; i < program->count; i++) {
            SimpleCommand* cmd = &program->commands[i];
            switch (cmd->type) {
                case CMD_VARIABLE_DECL:
                    printf("  - Variable: int %s = %d\n", cmd->variable_name, cmd->int_value);
                    break;
                case CMD_IF_START:
                    printf("  - If statement: if (%s)\n", cmd->condition);
                    break;
                case CMD_IF_END:
                    printf("  - End if block\n");
                    break;
                case CMD_EXEC_FUNCTION:
                    printf("  - Custom function: %s\n", cmd->function_name);
                    break;
                case CMD_DELAY:
                    printf("  - Delay: %d ms\n", cmd->delay_ms);
                    break;
                default:
                    printf("  - Unknown command type\n");
                    break;
            }
        }
        
        // Cleanup
        for (int i = 0; i < program->count; i++) {
            if (program->commands[i].function_name) {
                free(program->commands[i].function_name);
            }
            if (program->commands[i].variable_name) {
                free(program->commands[i].variable_name);
            }
            if (program->commands[i].condition) {
                free(program->commands[i].condition);
            }
        }
        free(program->commands);
        free(program);
    }
    
    // Cleanup is handled automatically
    printf("\nTranslation complete! Upload %s to your Arduino.\n", output_file);
    
    return 0;
}