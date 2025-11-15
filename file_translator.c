#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arduino_ir.h"
#include "ast.h"

// External declarations for flex/bison
extern FILE* yyin;
extern int yyparse();
extern Node* program_root; // Assuming this is set by the parser

// Function to convert simple AST (from ast.h) to Arduino IR
ArduinoModule* compile_simple_ast_to_arduino(Node* root);
void compile_ast_node(ArduinoModule* module, Node* node);

// File-based translator functions
ArduinoModule* translate_file_to_arduino(const char* filename);
void write_arduino_code_to_file(ArduinoModule* module, const char* output_filename);

// Implementation of file-based translation
ArduinoModule* translate_file_to_arduino(const char* filename) {
    FILE* input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    // Set input for flex/bison
    yyin = input_file;
    
    // Parse the file
    if (yyparse() != 0) {
        fprintf(stderr, "Error: Parsing failed\n");
        fclose(input_file);
        return NULL;
    }
    
    fclose(input_file);
    
    // Convert AST to Arduino code
    ArduinoModule* module = compile_simple_ast_to_arduino(program_root);
    
    return module;
}

// Convert simple AST to Arduino-compatible format
ArduinoModule* compile_simple_ast_to_arduino(Node* root) {
    if (!root) {
        printf("No AST provided, creating demo program...\n");
        return compile_to_arduino(NULL); // Use existing demo
    }
    
    // For now, create a simplified Arduino module based on the AST
    ArduinoModule* module = (ArduinoModule*)malloc(sizeof(ArduinoModule));
    arduino_module_init(module);
    
    // Setup function
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SETUP_START);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SERIAL_BEGIN);
    arduino_chunk_emit(&module->setup_func.chunk, 9600);
    
    // Setup motor pins
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
    
    // Loop function - process AST nodes
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_START);
    
    // Process the AST and generate corresponding opcodes
    compile_ast_node(module, root);
    
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_END);
    
    return module;
}

// Recursive function to process AST nodes
void compile_ast_node(ArduinoModule* module, Node* node) {
    if (!node) return;
    
    if (strcmp(node->type, "function_call") == 0) {
        // Handle function calls like exec avanzar();
        if (node->value) {
            int str_idx = arduino_function_add_string(&module->loop_func, node->value);
            arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_CALL_CUSTOM);
            arduino_chunk_emit(&module->loop_func.chunk, str_idx);
        }
    }
    else if (strcmp(node->type, "delay") == 0) {
        // Handle delay calls
        arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_DELAY);
        arduino_chunk_emit(&module->loop_func.chunk, node->ivalue);
    }
    else if (strcmp(node->type, "exec") == 0) {
        // Handle exec statements
        if (node->left && node->left->type && strcmp(node->left->type, "function_call") == 0) {
            compile_ast_node(module, node->left);
        }
    }
    
    // Process child nodes
    if (node->left) compile_ast_node(module, node->left);
    if (node->right) compile_ast_node(module, node->right);
    if (node->extra) compile_ast_node(module, node->extra);
    
    // Process list items
    if (node->list) {
        for (int i = 0; i < node->list->size; i++) {
            compile_ast_node(module, node->list->items[i]);
        }
    }
}

// Write Arduino code to .ino file
void write_arduino_code_to_file(ArduinoModule* module, const char* output_filename) {
    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", output_filename);
        return;
    }
    
    // Temporarily redirect stdout to file
    FILE* old_stdout = stdout;
    stdout = output_file;
    
    // Generate Arduino code
    arduino_vm_run(module);
    
    // Restore stdout
    stdout = old_stdout;
    fclose(output_file);
    
    printf("Arduino code generated successfully: %s\n", output_filename);
}

// Main function for file translation
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file> [output_file]\n", argv[0]);
        printf("Example: %s my_robot.prog robot_code.ino\n", argv[0]);
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = (argc >= 3) ? argv[2] : "generated_robot.ino";
    
    printf("Translating %s to Arduino code...\n", input_file);
    
    // Translate the file
    ArduinoModule* module = translate_file_to_arduino(input_file);
    if (!module) {
        fprintf(stderr, "Translation failed!\n");
        return 1;
    }
    
    // Write to .ino file
    write_arduino_code_to_file(module, output_file);
    
    // Cleanup
    arduino_module_free(module);
    
    printf("Translation complete!\n");
    return 0;
}