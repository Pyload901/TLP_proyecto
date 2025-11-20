/**
 * ESP32 Minimal VM - Arduino Sketch Example
 * 
 * Demonstrates:
 * - VM initialization
 * - Bytecode loading
 * - Program execution
 * - Stack-based function calls
 * - Array operations
 * 
 * Memory Footprint:
 * - Stack: 64 KB (grows down)
 * - Heap: 128 KB (grows up)
 * - Total: ~192 KB (leaves ~130 KB for sketch code on ESP32)
 */

#include "tiny_vm.h"

// Global VM instance
TinyVM vm;

// ============================================================================
// EXAMPLE PROGRAMS (Pre-compiled Bytecode)
// ============================================================================

/**
 * Example 1: Simple arithmetic
 * 
 * Pseudocode:
 *   R0 = 5
 *   R1 = 3
 *   R0 = R0 + R1  (ADD: R0 + R1 -> R0)
 *   PRINT R0       (output 8)
 *   HALT
 * 
 * Bytecode (15 bytes):
 */
const uint8_t program_arithmetic[] = {
  // LOADI R0, 5       [0x21, 0x00, 0x05]
  0x21, 0x00, 0x05,
  
  // LOADI R1, 3       [0x21, 0x01, 0x03]
  0x21, 0x01, 0x03,
  
  // ADD R0, R1        [0x10, 0x00, 0x01]  (result in R0)
  0x10, 0x00, 0x01,
  
  // PRINT R0          [0x60, 0x00, 0x00]
  0x60, 0x00, 0x00,
  
  // HALT              [0x01, 0x00, 0x00]
  0x01, 0x00, 0x00,
};
const uint16_t program_arithmetic_size = sizeof(program_arithmetic);

/**
 * Example 2: Conditional jump (if-else)
 * 
 * Pseudocode:
 *   R0 = 10
 *   R1 = 5
 *   CMP R0, R1       (compare, set flags)
 *   if (R0 > R1) PRINT 1
 *   else PRINT 0
 * 
 * Bytecode:
 */
const uint8_t program_conditional[] = {
  // LOADI R0, 10
  0x21, 0x00, 0x0A,
  
  // LOADI R1, 5
  0x21, 0x01, 0x05,
  
  // CMP R0, R1        [0x1B, 0x00, 0x01]
  0x1B, 0x00, 0x01,
  
  // JLE skip (jump to 0x003B if R0 <= R1)  [0x46, 0x00, 0x0F]
  // Address 0x000F = skip the "then" branch
  0x46, 0x00, 0x0F,
  
  // LOADI R0, 1       (then: R0 > R1, output 1)
  0x21, 0x00, 0x01,
  
  // PRINT R0
  0x60, 0x00, 0x00,
  
  // JMP end (skip else)  [0x40, 0x00, 0x15]
  0x40, 0x00, 0x15,
  
  // LOADI R0, 0       (else: output 0)
  0x21, 0x00, 0x00,
  
  // PRINT R0
  0x60, 0x00, 0x00,
  
  // HALT
  0x01, 0x00, 0x00,
};
const uint16_t program_conditional_size = sizeof(program_conditional);

/**
 * Example 3: Loop (sum 1 to N)
 * 
 * Pseudocode:
 *   R0 = 0  (accumulator)
 *   R1 = 1  (counter)
 *   R2 = 10 (limit)
 * loop:
 *   ADD R0, R1       (accumulate)
 *   LOADI R3, 1
 *   ADD R1, R3       (increment counter)
 *   CMP R1, R2       (check if counter > limit)
 *   JLE loop         (if R1 <= R2, jump back)
 *   PRINT R0         (output sum = 55)
 *   HALT
 * 
 * Bytecode:
 */
const uint8_t program_loop[] = {
  // LOADI R0, 0
  0x21, 0x00, 0x00,
  
  // LOADI R1, 1
  0x21, 0x01, 0x01,
  
  // LOADI R2, 10
  0x21, 0x02, 0x0A,
  
  // [LOOP START at 0x0009]
  // ADD R0, R1        [0x10, 0x00, 0x01]
  0x10, 0x00, 0x01,
  
  // LOADI R3, 1
  0x21, 0x03, 0x01,
  
  // ADD R1, R3        [0x10, 0x01, 0x03]
  0x10, 0x01, 0x03,
  
  // CMP R1, R2        [0x1B, 0x01, 0x02]
  0x1B, 0x01, 0x02,
  
  // JLE loop (jump back to 0x0009)  [0x46, 0x00, 0x09]
  0x46, 0x00, 0x09,
  
  // PRINT R0
  0x60, 0x00, 0x00,
  
  // HALT
  0x01, 0x00, 0x00,
};
const uint16_t program_loop_size = sizeof(program_loop);

/**
 * Example 4: Array operations
 * 
 * Pseudocode:
 *   ALLOC 16         (allocate 16 bytes on heap)
 *   R0 = 5
 *   STORE heap[0], R0 (write 5 to array[0])
 *   R0 = 10
 *   STORE heap[4], R0 (write 10 to array[1])
 *   LOAD_MEM R1, heap (read array[0])
 *   PRINT R1         (output 5)
 *   HALT
 * 
 * Bytecode:
 */
const uint8_t program_array[] = {
  // ALLOC 16          [0x26, 0x10, 0x00]
  0x26, 0x10, 0x00,
  
  // LOADI R0, 5
  0x21, 0x00, 0x05,
  
  // STORE R6, R0      [0x23, 0x06, 0x00]  (R6 has heap base from ALLOC)
  0x23, 0x06, 0x00,
  
  // LOADI R0, 10
  0x21, 0x00, 0x0A,
  
  // LOADI R1, 4       (offset in heap)
  0x21, 0x01, 0x04,
  
  // ADD R1, R6        [0x10, 0x01, 0x06]  (R1 = R1 + R6 = 4 + heap_base)
  0x10, 0x01, 0x06,
  
  // STORE R1, R0      [0x23, 0x01, 0x00]
  0x23, 0x01, 0x00,
  
  // LOAD_MEM R1, R6   [0x24, 0x01, 0x06]  (read from heap base)
  0x24, 0x01, 0x06,
  
  // PRINT R1
  0x60, 0x01, 0x00,
  
  // HALT
  0x01, 0x00, 0x00,
};
const uint16_t program_array_size = sizeof(program_array);

/**
 * Example 5: Function call
 * 
 * Pseudocode:
 *   main:
 *     R0 = 5
 *     R1 = 3
 *     PUSH R0         (push arg 1)
 *     PUSH R1         (push arg 2)
 *     CALL add (0x18)
 *     PRINT R6        (R6 has result)
 *     HALT
 * 
 *   add:  (at 0x18)
 *     POP R0          (pop arg 1)
 *     POP R1          (pop arg 2)
 *     ADD R0, R1      (R0 = R0 + R1)
 *     LOAD R6, R0     (move result to R6)
 *     RET
 */
const uint8_t program_function[] = {
  // === MAIN (starts at 0x0000) ===
  
  // LOADI R0, 5
  0x21, 0x00, 0x05,
  
  // LOADI R1, 3
  0x21, 0x01, 0x03,
  
  // PUSH R0           [0x30, 0x00, 0x00]
  0x30, 0x00, 0x00,
  
  // PUSH R1           [0x30, 0x01, 0x00]
  0x30, 0x01, 0x00,
  
  // CALL add (0x18)   [0x50, 0x00, 0x18]
  0x50, 0x00, 0x18,
  
  // PRINT R6
  0x60, 0x06, 0x00,
  
  // HALT
  0x01, 0x00, 0x00,
  
  // === ADD FUNCTION (starts at 0x18) ===
  // POP R0            [0x31, 0x00, 0x00]
  0x31, 0x00, 0x00,
  
  // POP R1            [0x31, 0x01, 0x00]
  0x31, 0x01, 0x00,
  
  // ADD R0, R1        [0x10, 0x00, 0x01]
  0x10, 0x00, 0x01,
  
  // LOAD R6, R0       [0x20, 0x06, 0x00]  (move R0 to R6)
  0x20, 0x06, 0x00,
  
  // RET               [0x52, 0x00, 0x00]
  0x52, 0x00, 0x00,
};
const uint16_t program_function_size = sizeof(program_function);

// ============================================================================
// ARDUINO SETUP & LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  ESP32 Minimal VM - Example Programs");
  Serial.println("========================================\n");
  
  // Initialize VM
  if (!vm.initialize()) {
    Serial.println("FATAL: VM initialization failed!");
    while (1);
  }
  
  // Run examples
  run_example(1, program_arithmetic, program_arithmetic_size, "Arithmetic");
  run_example(2, program_conditional, program_conditional_size, "Conditional");
  run_example(3, program_loop, program_loop_size, "Loop");
  run_example(4, program_array, program_array_size, "Array");
  run_example(5, program_function, program_function_size, "Function Call");
  
  Serial.println("\n========================================");
  Serial.println("  All examples completed!");
  Serial.println("========================================\n");
}

void loop() {
  // Nothing - all examples run in setup
  delay(10000);
}

// ============================================================================
// HELPER: Run example program
// ============================================================================

void run_example(int num, const uint8_t* bytecode, uint16_t size, const char* name) {
  Serial.print("\n[Example ");
  Serial.print(num);
  Serial.print("] ");
  Serial.println(name);
  Serial.println("----------------------------------------");
  
  // Load program
  if (!vm.load_program(bytecode, size)) {
    Serial.println("ERROR: Failed to load program");
    return;
  }
  
  // Execute
  vm.execute();
  
  // Show results
  Serial.println("\n[Output above] ^");
  Serial.println("----------------------------------------\n");
}

// ============================================================================
// OPTIONAL: Debug helpers for bytecode inspection
// ============================================================================

void print_bytecode_as_instructions(const uint8_t* bytecode, uint16_t size) {
  Serial.println("\nBytecode Disassembly:");
  for (uint16_t pc = 0; pc < size; pc += 3) {
    if (pc + 2 >= size) break;
    
    Instruction instr = Instruction::unpack(&bytecode[pc]);
    Serial.print("0x");
    Serial.print(pc, HEX);
    Serial.print(": ");
    Serial.print(vm.opcode_name(instr.opcode));
    Serial.print(" ARG1=");
    Serial.print(instr.arg1);
    Serial.print(" ARG2=");
    Serial.println(instr.arg2);
  }
}
