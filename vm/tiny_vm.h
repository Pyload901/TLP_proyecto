/**
 * Minimal VM for ESP32 - C++ Header
 * 
 * Designed for ultra-lightweight execution on ESP32 with SD card storage.
 * Compact instruction format: 3 bytes per instruction (1 opcode + 2 args)
 * 8 general-purpose registers, unified stack, heap for arrays.
 * 
 * Memory footprint: ~1KB VM state + 64KB stack + 128KB heap
 */

#pragma once

#include <stdint.h>
#include <string.h>
#include <Arduino.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define VM_REGISTER_COUNT       8      // R0-R7
#define VM_STACK_SIZE           (64 * 1024)  // 64 KB stack
#define VM_HEAP_SIZE            (128 * 1024) // 128 KB heap
#define VM_MAX_FUNCTIONS        256    // Max functions in program
#define VM_MAX_INSTRUCTION_SIZE 3      // bytes: [opcode, arg1, arg2]

// ============================================================================
// OPCODES (50 instructions)
// ============================================================================

enum Opcode : uint8_t {
  // Control & Special
  OP_NOP       = 0x00,  // No operation
  OP_HALT      = 0x01,  // Stop execution

  // Arithmetic (Results go to R0)
  OP_ADD       = 0x10,  // R0 = ARG1 + ARG2
  OP_SUB       = 0x11,  // R0 = ARG1 - ARG2
  OP_MUL       = 0x12,  // R0 = ARG1 * ARG2
  OP_DIV       = 0x13,  // R0 = ARG1 / ARG2
  OP_MOD       = 0x14,  // R0 = ARG1 % ARG2

  // Bitwise
  OP_AND       = 0x15,  // R0 = ARG1 & ARG2
  OP_OR        = 0x16,  // R0 = ARG1 | ARG2
  OP_XOR       = 0x17,  // R0 = ARG1 ^ ARG2
  OP_NOT       = 0x18,  // R0 = ~ARG1
  OP_SHL       = 0x19,  // R0 = ARG1 << ARG2
  OP_SHR       = 0x1A,  // R0 = ARG1 >> ARG2

  // Comparison (Sets flags)
  OP_CMP       = 0x1B,  // Compare ARG1 vs ARG2

  // Memory Access
  OP_LOAD      = 0x20,  // R[ARG1] = R[ARG2] (register copy)
  OP_LOADI     = 0x21,  // R[ARG1] = ARG2 (8-bit immediate, sign-extended)
  OP_LOADI16   = 0x22,  // R[ARG1] = next 16-bit word (extended)
  OP_STORE     = 0x23,  // M[R[ARG1]] = R[ARG2] (write to memory)
  OP_LOAD_MEM  = 0x24,  // R[ARG1] = M[R[ARG2]] (read from memory)
  OP_LOAD_ADDR = 0x25,  // R[ARG1] = heap_base + ARG2 (address calc)
  OP_ALLOC     = 0x26,  // Allocate ARG1 bytes; result in R6
  OP_FREE      = 0x27,  // Free last allocation

  // Stack Operations
  OP_PUSH      = 0x30,  // Push R[ARG1] to stack
  OP_POP       = 0x31,  // Pop stack into R[ARG1]
  OP_PEEK      = 0x32,  // R[ARG1] = stack[SP + ARG2] (no pop)

  // Control Flow & Jumps
  OP_JMP       = 0x40,  // PC = (ARG1 << 8) | ARG2 (16-bit address)
  OP_JMPL      = 0x41,  // Extended: PC = next 24-bit address
  OP_JZ        = 0x42,  // Jump if zero flag set
  OP_JNZ       = 0x43,  // Jump if zero flag clear
  OP_JLT       = 0x44,  // Jump if less than
  OP_JGT       = 0x45,  // Jump if greater than
  OP_JLE       = 0x46,  // Jump if less or equal
  OP_JGE       = 0x47,  // Jump if greater or equal
  OP_JEQ       = 0x48,  // Jump if equal
  OP_JNEQ      = 0x49,  // Jump if not equal

  // Function Calls
  OP_CALL      = 0x50,  // Push PC+2; PC = (ARG1 << 8) | ARG2
  OP_CALLL     = 0x51,  // Extended CALL with 24-bit address
  OP_RET       = 0x52,  // Pop PC from stack

  // I/O & System
  OP_PRINT     = 0x60,  // Output R[ARG1] to serial
  OP_PRINTC    = 0x61,  // Output R[ARG1] as char
  OP_READ      = 0x62,  // R[ARG1] = read from serial
  OP_TRAP      = 0x63,  // System call (ARG1 = trap code)
  OP_DEBUG     = 0x64,  // Debug output (dev only)
};

// ============================================================================
// FLAGS REGISTER (8-bit)
// ============================================================================

struct Flags {
  uint8_t carry : 1;  // Bit 0: Overflow from arithmetic
  uint8_t zero  : 1;  // Bit 1: Result is zero
  uint8_t eq    : 1;  // Bit 2: Equal (from CMP)
  uint8_t neq   : 1;  // Bit 3: Not equal
  uint8_t gt    : 1;  // Bit 4: Greater than
  uint8_t lt    : 1;  // Bit 5: Less than
  uint8_t ge    : 1;  // Bit 6: Greater or equal
  uint8_t le    : 1;  // Bit 7: Less or equal

  void clear() { carry = zero = eq = neq = gt = lt = ge = le = 0; }
  
  uint8_t as_byte() const {
    return (carry << 0) | (zero << 1) | (eq << 2) | (neq << 3) |
           (gt << 4) | (lt << 5) | (ge << 6) | (le << 7);
  }
  
  void from_byte(uint8_t b) {
    carry = (b >> 0) & 1; zero = (b >> 1) & 1; eq = (b >> 2) & 1;
    neq   = (b >> 3) & 1; gt   = (b >> 4) & 1; lt = (b >> 5) & 1;
    ge    = (b >> 6) & 1; le   = (b >> 7) & 1;
  }
};

// ============================================================================
// VALUE TYPE (Unified 32/64-bit value representation)
// ============================================================================

struct Value {
  int32_t i32;      // Primary: 32-bit signed integer
  // Note: For doubles, use adjacent registers (R0-R1, R2-R3, etc.)
  
  Value() : i32(0) {}
  Value(int32_t v) : i32(v) {}
  
  // Conversions
  bool as_bool() const { return i32 != 0; }
  int32_t as_int32() const { return i32; }
  
  // Set from int, char, or bool
  void set_int(int32_t v) { i32 = v; }
  void set_char(char c) { i32 = (unsigned char)c; }
  void set_bool(bool b) { i32 = b ? 1 : 0; }
};

// ============================================================================
// INSTRUCTION STRUCTURE (3 bytes)
// ============================================================================

struct Instruction {
  uint8_t opcode;   // Operation code
  uint8_t arg1;     // First argument (register or immediate)
  uint8_t arg2;     // Second argument (register or immediate)
  
  Instruction() : opcode(0), arg1(0), arg2(0) {}
  Instruction(uint8_t op, uint8_t a1, uint8_t a2) 
    : opcode(op), arg1(a1), arg2(a2) {}
  
  // Pack into 3 bytes (for storage)
  void pack(uint8_t* buf) const {
    buf[0] = opcode;
    buf[1] = arg1;
    buf[2] = arg2;
  }
  
  // Unpack from 3 bytes
  static Instruction unpack(const uint8_t* buf) {
    return Instruction(buf[0], buf[1], buf[2]);
  }
};

// ============================================================================
// FUNCTION ENTRY (for function table)
// ============================================================================

struct FunctionEntry {
  uint16_t address;     // Start address of function (in bytecode)
  uint8_t arg_count;    // Number of arguments
  uint8_t local_count;  // Number of local variables
  char name[32];        // Function name (optional, for debugging)
};

// ============================================================================
// HEAP ALLOCATION HEADER (for memory management)
// ============================================================================

struct HeapBlock {
  uint16_t size;        // Size of allocation (including header)
  uint16_t next_free;   // Pointer to next free block (for LIFO)
  
  HeapBlock(uint16_t s) : size(s), next_free(0) {}
};

// ============================================================================
// VM STATE STRUCTURE
// ============================================================================

struct VMState {
  // Registers (8 x 32-bit)
  Value registers[VM_REGISTER_COUNT];
  
  // Program state
  uint16_t pc;          // Program counter
  uint16_t sp;          // Stack pointer (grows downward)
  uint16_t fp;          // Frame pointer (for local variables)
  uint16_t hp;          // Heap pointer (grows upward)
  
  // Flags and execution
  Flags flags;
  bool halted;
  
  // Memory
  uint8_t* stack;       // Stack memory (allocated)
  uint8_t* heap;        // Heap memory (allocated)
  uint8_t* code;        // Bytecode (loaded from SD card)
  uint16_t code_size;   // Size of bytecode
  
  // Function table
  FunctionEntry* function_table;
  uint16_t function_count;
  
  // Allocation tracking (LIFO)
  uint16_t alloc_stack[32];  // Stack of heap allocations
  uint8_t alloc_depth;
  
  VMState() 
    : pc(0), sp(VM_STACK_SIZE - 1), fp(VM_STACK_SIZE - 1), hp(0),
      halted(false), stack(nullptr), heap(nullptr), code(nullptr),
      code_size(0), function_table(nullptr), function_count(0),
      alloc_depth(0) {
    memset(registers, 0, sizeof(registers));
    flags.clear();
    memset(alloc_stack, 0, sizeof(alloc_stack));
  }
};

// ============================================================================
// VM CLASS (Main virtual machine)
// ============================================================================

class TinyVM {
private:
  VMState state;
  
  // Helper methods
  void update_flags_arithmetic(int32_t result, bool carry);
  void update_flags_compare(int32_t a, int32_t b);
  int32_t sign_extend_8(int8_t val);
  
public:
  TinyVM();
  ~TinyVM();
  
  // Initialization
  bool initialize();
  bool load_program(const uint8_t* bytecode, uint16_t size);
  bool load_program_from_file(const char* filename);
  
  // Execution
  void execute();
  void step();  // Single instruction
  void halt() { state.halted = true; }
  bool is_halted() const { return state.halted; }
  
  // Register access
  Value get_register(uint8_t reg) const;
  void set_register(uint8_t reg, Value val);
  void set_register_i32(uint8_t reg, int32_t val);
  
  // Memory access
  Value read_memory(uint16_t addr);
  void write_memory(uint16_t addr, Value val);
  
  // Stack operations
  void push_stack(Value val);
  Value pop_stack();
  Value peek_stack(uint8_t offset);
  
  // Heap operations
  uint16_t alloc_heap(uint16_t size);
  void free_heap();
  
  // Debugging
  void dump_state();
  void dump_registers();
  void dump_stack(uint8_t count);
  const char* opcode_name(uint8_t opcode);
};

// ============================================================================
// INSTRUCTION BUILDERS (for easier code generation)
// ============================================================================

namespace InstructionBuilder {
  inline Instruction nop() { return Instruction(OP_NOP, 0, 0); }
  inline Instruction halt() { return Instruction(OP_HALT, 0, 0); }
  
  inline Instruction add(uint8_t r1, uint8_t r2) { return Instruction(OP_ADD, r1, r2); }
  inline Instruction sub(uint8_t r1, uint8_t r2) { return Instruction(OP_SUB, r1, r2); }
  inline Instruction mul(uint8_t r1, uint8_t r2) { return Instruction(OP_MUL, r1, r2); }
  inline Instruction div(uint8_t r1, uint8_t r2) { return Instruction(OP_DIV, r1, r2); }
  inline Instruction mod(uint8_t r1, uint8_t r2) { return Instruction(OP_MOD, r1, r2); }
  
  inline Instruction and_op(uint8_t r1, uint8_t r2) { return Instruction(OP_AND, r1, r2); }
  inline Instruction or_op(uint8_t r1, uint8_t r2) { return Instruction(OP_OR, r1, r2); }
  inline Instruction xor_op(uint8_t r1, uint8_t r2) { return Instruction(OP_XOR, r1, r2); }
  inline Instruction not_op(uint8_t r) { return Instruction(OP_NOT, r, 0); }
  
  inline Instruction cmp(uint8_t r1, uint8_t r2) { return Instruction(OP_CMP, r1, r2); }
  
  inline Instruction load(uint8_t dst, uint8_t src) { return Instruction(OP_LOAD, dst, src); }
  inline Instruction loadi(uint8_t dst, int8_t imm) { return Instruction(OP_LOADI, dst, (uint8_t)imm); }
  inline Instruction load_mem(uint8_t dst, uint8_t addr_reg) { return Instruction(OP_LOAD_MEM, dst, addr_reg); }
  inline Instruction store(uint8_t addr_reg, uint8_t src) { return Instruction(OP_STORE, addr_reg, src); }
  
  inline Instruction push(uint8_t reg) { return Instruction(OP_PUSH, reg, 0); }
  inline Instruction pop(uint8_t reg) { return Instruction(OP_POP, reg, 0); }
  inline Instruction peek(uint8_t reg, uint8_t offset) { return Instruction(OP_PEEK, reg, offset); }
  
  inline Instruction jmp(uint16_t addr) { return Instruction(OP_JMP, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jz(uint16_t addr) { return Instruction(OP_JZ, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jnz(uint16_t addr) { return Instruction(OP_JNZ, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jlt(uint16_t addr) { return Instruction(OP_JLT, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jgt(uint16_t addr) { return Instruction(OP_JGT, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jle(uint16_t addr) { return Instruction(OP_JLE, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction jge(uint16_t addr) { return Instruction(OP_JGE, (addr >> 8) & 0xFF, addr & 0xFF); }
  
  inline Instruction call(uint16_t addr) { return Instruction(OP_CALL, (addr >> 8) & 0xFF, addr & 0xFF); }
  inline Instruction ret() { return Instruction(OP_RET, 0, 0); }
  
  inline Instruction print(uint8_t reg) { return Instruction(OP_PRINT, reg, 0); }
  inline Instruction printc(uint8_t reg) { return Instruction(OP_PRINTC, reg, 0); }
  
  inline Instruction alloc(uint8_t size) { return Instruction(OP_ALLOC, size, 0); }
  inline Instruction free_mem() { return Instruction(OP_FREE, 0, 0); }
}

#endif // TINY_VM_H
