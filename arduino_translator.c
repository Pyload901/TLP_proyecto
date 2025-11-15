#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arduino_ir.h"

// Forward declarations for Arduino compiler and VM
ArduinoModule* compile_to_arduino(void *program);

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

void arduino_vm_init(ArduinoVM *vm, FILE *output);
int arduino_vm_run(ArduinoVM *vm, ArduinoModule *module);
void arduino_vm_free(ArduinoVM *vm);

void print_usage(const char *program_name) {
    printf("Usage: %s <output_file>\n", program_name);
    printf("  output_file: Generated Arduino C++ file\n");
    printf("\nExample:\n");
    printf("  %s arduino_output.ino\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *output_file = argv[1];
    
    printf("Arduino Translator\n");
    printf("==================\n");
    printf("Output file: %s\n", output_file);
    printf("\n");
    
    // Use built-in demo program
    printf("Using built-in demo program...\n");
    
    // Compile to Arduino IR
    printf("Compiling to Arduino intermediate representation...\n");
    ArduinoModule *module = compile_to_arduino(NULL);
    
    if (!module) {
        printf("✗ Compilation failed\n");
        return 1;
    }
    printf("✓ Compilation successful\n");
    
    // Open output file
    FILE *output = fopen(output_file, "w");
    if (!output) {
        printf("✗ Could not create output file: %s\n", output_file);
        return 1;
    }
    
    // Initialize Arduino VM and generate code
    printf("Generating Arduino C++ code...\n");
    ArduinoVM vm;
    arduino_vm_init(&vm, output);
    
    int result = arduino_vm_run(&vm, module);
    
    if (result) {
        printf("✓ Arduino code generation successful\n");
        printf("\nGenerated Arduino code saved to: %s\n", output_file);
        printf("\nYou can now:\n");
        printf("1. Open %s in the Arduino IDE\n", output_file);
        printf("2. Upload it to your Arduino board\n");
    } else {
        printf("✗ Arduino code generation failed\n");
    }
    
    // Cleanup
    arduino_vm_free(&vm);
    fclose(output);
    
    // Print sample of generated code
    printf("\nSample of generated code:\n");
    printf("========================\n");
    
    FILE *show_output = fopen(output_file, "r");
    if (show_output) {
        char line[256];
        int line_count = 0;
        while (fgets(line, sizeof(line), show_output) && line_count < 15) {
            printf("%s", line);
            line_count++;
        }
        if (line_count >= 15) {
            printf("... (truncated)\n");
        }
        fclose(show_output);
    }
    
    return result ? 0 : 1;
}