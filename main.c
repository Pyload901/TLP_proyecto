#include "compiler.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read source file into memory
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(n + 1);
    fread(buf, 1, n, f);
    buf[n] = 0;
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <robot_program.prog> [output.ino]\n", argv[0]);
        fprintf(stderr, "Example: %s my_robot.prog robot.ino\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = (argc >= 3) ? argv[2] : "robot.ino";

    printf("🤖 Arduino Robot Language Translator\n");
    printf("=====================================\n");
    printf("Input:  %s\n", input_file);
    printf("Output: %s\n\n", output_file);

    // Read source code
    char *src = read_file(input_file);
    if (!src) return 1;

    // Compile source to Arduino bytecode
    ArduinoCompiler comp;
    printf("📝 Compiling robot program...\n");
    if (!compile_source(&comp, src)) {
        fprintf(stderr, "❌ Compilation failed with %d errors.\n", comp.error_count);
        free(src);
        return 1;
    }
    printf("✅ Compilation successful!\n\n");

    // Open output file
    FILE *output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "❌ Could not create output file: %s\n", output_file);
        free(src);
        return 1;
    }

    // Execute bytecode and generate Arduino C++
    VM vm;
    vm_init(&vm, output);
    printf("🔧 Generating Arduino code...\n");
    int ok = vm_run(&vm, &comp.module);
    
    if (ok) {
        printf("✅ Arduino code generated successfully!\n");
        printf("\n📄 Generated: %s\n", output_file);
        printf("🚀 Ready to upload to your Arduino!\n");
    } else {
        printf("❌ Code generation failed!\n");
    }

    // Cleanup
    vm_free(&vm);
    fclose(output);
    free(src);

    return ok ? 0 : 1;
}