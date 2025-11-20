/**
 * Minimal VM for ESP32 - C++ Implementation
 * 
 * Complete execution engine with arithmetic, memory, and control flow.
 * Designed to fit in ESP32's limited SRAM (~320KB).
 */

#include "tiny_vm.h"
#include <cmath>

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

TinyVM::TinyVM() {
  // VMState already initialized via default constructor
}

TinyVM::~TinyVM() {
  if (state.stack) free(state.stack);
  if (state.heap) free(state.heap);
  if (state.function_table) free(state.function_table);
}

bool TinyVM::initialize() {
  // Allocate memory regions
  state.stack = (uint8_t*)malloc(VM_STACK_SIZE);
  state.heap = (uint8_t*)malloc(VM_HEAP_SIZE);
  
  if (!state.stack || !state.heap) {
    Serial.println("ERROR: Failed to allocate VM memory");
    return false;
  }
  
  // Initialize memory to zero
  memset(state.stack, 0, VM_STACK_SIZE);
  memset(state.heap, 0, VM_HEAP_SIZE);
  
  // Initialize pointers
  state.sp = VM_STACK_SIZE - 1;
  state.fp = VM_STACK_SIZE - 1;
  state.hp = 0;
  state.pc = 0;
  
  Serial.println("[VM] Initialized: 64KB stack, 128KB heap");
  return true;
}

bool TinyVM::load_program(const uint8_t* bytecode, uint16_t size) {
  if (size == 0 || !bytecode) {
    Serial.println("ERROR: Invalid bytecode");
    return false;
  }
  
  // Store reference to bytecode (assume it's in PROGMEM or stable memory)
  state.code = (uint8_t*)bytecode;
  state.code_size = size;
  state.pc = 0;
  
  Serial.print("[VM] Loaded program: ");
  Serial.print(size);
  Serial.println(" bytes");
  return true;
}

bool TinyVM::load_program_from_file(const char* filename) {
  // This would load from SD card on ESP32
  // Implementation depends on your SD card driver
  // For now, return false
  Serial.print("[VM] SD card loading not implemented: ");
  Serial.println(filename);
  return false;
}

// ============================================================================
// INSTRUCTION EXECUTION
// ============================================================================

void TinyVM::step() {
  if (state.halted || state.pc >= state.code_size) {
    state.halted = true;
    return;
  }
  
  // Fetch instruction (3 bytes)
  if (state.pc + 2 >= state.code_size) {
    state.halted = true;
    return;
  }
  
  Instruction instr = Instruction::unpack(&state.code[state.pc]);
  state.pc += 3;
  
  // Decode and execute
  uint8_t op = instr.opcode;
  uint8_t a1 = instr.arg1;
  uint8_t a2 = instr.arg2;
  
  int32_t result = 0;
  
  switch (op) {
    // ========== CONTROL & SPECIAL ==========
    case OP_NOP:
      break;
      
    case OP_HALT:
      state.halted = true;
      break;
    
    // ========== ARITHMETIC ==========
    case OP_ADD: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      int64_t res = (int64_t)v1 + v2;
      bool carry = (res > INT32_MAX || res < INT32_MIN);
      state.registers[0].i32 = (int32_t)res;
      update_flags_arithmetic((int32_t)res, carry);
      break;
    }
    
    case OP_SUB: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      int64_t res = (int64_t)v1 - v2;
      bool carry = (res > INT32_MAX || res < INT32_MIN);
      state.registers[0].i32 = (int32_t)res;
      update_flags_arithmetic((int32_t)res, carry);
      break;
    }
    
    case OP_MUL: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      int64_t res = (int64_t)v1 * v2;
      bool carry = (res > INT32_MAX || res < INT32_MIN);
      state.registers[0].i32 = (int32_t)res;
      update_flags_arithmetic((int32_t)res, carry);
      break;
    }
    
    case OP_DIV: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      if (v2 == 0) {
        state.registers[0].i32 = 0;
        state.flags.zero = 1;
      } else {
        result = v1 / v2;
        state.registers[0].i32 = result;
        update_flags_arithmetic(result, false);
      }
      break;
    }
    
    case OP_MOD: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      if (v2 == 0) {
        state.registers[0].i32 = 0;
      } else {
        result = v1 % v2;
        state.registers[0].i32 = result;
        update_flags_arithmetic(result, false);
      }
      break;
    }
    
    // ========== BITWISE ==========
    case OP_AND: {
      result = state.registers[a1].i32 & state.registers[a2].i32;
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_OR: {
      result = state.registers[a1].i32 | state.registers[a2].i32;
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_XOR: {
      result = state.registers[a1].i32 ^ state.registers[a2].i32;
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_NOT: {
      result = ~state.registers[a1].i32;
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_SHL: {
      result = state.registers[a1].i32 << (state.registers[a2].i32 & 0x1F);
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_SHR: {
      result = state.registers[a1].i32 >> (state.registers[a2].i32 & 0x1F);
      state.registers[0].i32 = result;
      update_flags_arithmetic(result, false);
      break;
    }
    
    case OP_CMP: {
      int32_t v1 = state.registers[a1].i32;
      int32_t v2 = state.registers[a2].i32;
      update_flags_compare(v1, v2);
      break;
    }
    
    // ========== MEMORY ACCESS ==========
    case OP_LOAD: {
      // R[a1] = R[a2]
      state.registers[a1] = state.registers[a2];
      break;
    }
    
    case OP_LOADI: {
      // R[a1] = a2 (sign-extended 8-bit)
      state.registers[a1].i32 = sign_extend_8((int8_t)a2);
      break;
    }
    
    case OP_LOADI16: {
      // R[a1] = next 16-bit word
      if (state.pc + 1 < state.code_size) {
        uint16_t val = (state.code[state.pc] << 8) | state.code[state.pc + 1];
        state.registers[a1].i32 = (int32_t)val;
        state.pc += 2;
      }
      break;
    }
    
    case OP_STORE: {
      // M[R[a1]] = R[a2]
      uint16_t addr = (uint16_t)state.registers[a1].i32;
      if (addr < VM_HEAP_SIZE) {
        *(int32_t*)&state.heap[addr] = state.registers[a2].i32;
      }
      break;
    }
    
    case OP_LOAD_MEM: {
      // R[a1] = M[R[a2]]
      uint16_t addr = (uint16_t)state.registers[a2].i32;
      if (addr < VM_HEAP_SIZE) {
        state.registers[a1].i32 = *(int32_t*)&state.heap[addr];
      }
      break;
    }
    
    case OP_LOAD_ADDR: {
      // R[a1] = heap_base + a2
      state.registers[a1].i32 = (int32_t)(state.hp + a2);
      break;
    }
    
    case OP_ALLOC: {
      // Allocate a2 bytes on heap; return base in R6
      uint16_t size = a2 ? a2 : 256;  // Default to 256 if a2 = 0
      if (state.hp + size < VM_HEAP_SIZE && state.alloc_depth < 32) {
        state.alloc_stack[state.alloc_depth++] = state.hp;
        state.registers[6].i32 = (int32_t)state.hp;
        state.hp += size;
      } else {
        state.registers[6].i32 = -1;  // Allocation failed
      }
      break;
    }
    
    case OP_FREE: {
      // Free last allocation (LIFO)
      if (state.alloc_depth > 0) {
        state.hp = state.alloc_stack[--state.alloc_depth];
      }
      break;
    }
    
    // ========== STACK OPERATIONS ==========
    case OP_PUSH: {
      push_stack(state.registers[a1]);
      break;
    }
    
    case OP_POP: {
      state.registers[a1] = pop_stack();
      break;
    }
    
    case OP_PEEK: {
      state.registers[a1] = peek_stack(a2);
      break;
    }
    
    // ========== CONTROL FLOW ==========
    case OP_JMP: {
      // PC = (a1 << 8) | a2
      state.pc = ((uint16_t)a1 << 8) | a2;
      break;
    }
    
    case OP_JZ: {
      if (state.flags.zero) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JNZ: {
      if (!state.flags.zero) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JLT: {
      if (state.flags.lt) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JGT: {
      if (state.flags.gt) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JLE: {
      if (state.flags.le) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JGE: {
      if (state.flags.ge) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JEQ: {
      if (state.flags.eq) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    case OP_JNEQ: {
      if (state.flags.neq) {
        state.pc = ((uint16_t)a1 << 8) | a2;
      }
      break;
    }
    
    // ========== FUNCTION CALLS ==========
    case OP_CALL: {
      // Push return address, jump to function
      push_stack(Value((int32_t)(state.pc + 3)));  // Next instruction after CALL
      state.pc = ((uint16_t)a1 << 8) | a2;
      break;
    }
    
    case OP_RET: {
      // Pop return address
      Value ret_addr = pop_stack();
      state.pc = (uint16_t)ret_addr.i32;
      break;
    }
    
    // ========== I/O & DEBUG ==========
    case OP_PRINT: {
      Serial.println(state.registers[a1].i32);
      break;
    }
    
    case OP_PRINTC: {
      char c = (char)(state.registers[a1].i32 & 0xFF);
      Serial.print(c);
      break;
    }
    
    case OP_READ: {
      // Simple: read one line from serial
      // This is blocking - use with caution
      while (!Serial.available());
      int val = Serial.parseInt();
      state.registers[a1].i32 = val;
      break;
    }
    
    case OP_TRAP: {
      // System calls: a1 = trap code
      switch (a1) {
        case 0x01:  // TRAP_FLUSH_OUTPUT
          Serial.flush();
          break;
        case 0x02:  // TRAP_DELAY
          delay(state.registers[0].i32);
          break;
        case 0xFF:  // TRAP_DEBUG_DUMP
          dump_state();
          break;
      }
      break;
    }
    
    case OP_DEBUG: {
      Serial.print("[DEBUG ");
      Serial.print(a2);
      Serial.print("] R");
      Serial.print(a1);
      Serial.print(" = ");
      Serial.println(state.registers[a1].i32);
      break;
    }
    
    default:
      Serial.print("ERROR: Unknown opcode 0x");
      Serial.println(op, HEX);
      state.halted = true;
      break;
  }
}

void TinyVM::execute() {
  while (!state.halted) {
    step();
  }
}

// ============================================================================
// HELPER METHODS
// ============================================================================

void TinyVM::update_flags_arithmetic(int32_t result, bool carry) {
  state.flags.zero = (result == 0) ? 1 : 0;
  state.flags.carry = carry ? 1 : 0;
  // eq/neq/lt/gt/le/ge only set by CMP
}

void TinyVM::update_flags_compare(int32_t a, int32_t b) {
  state.flags.zero = (a == b) ? 1 : 0;
  state.flags.eq = (a == b) ? 1 : 0;
  state.flags.neq = (a != b) ? 1 : 0;
  state.flags.lt = (a < b) ? 1 : 0;
  state.flags.gt = (a > b) ? 1 : 0;
  state.flags.le = (a <= b) ? 1 : 0;
  state.flags.ge = (a >= b) ? 1 : 0;
}

int32_t TinyVM::sign_extend_8(int8_t val) {
  return (int32_t)val;  // C automatically sign-extends on cast
}

// ============================================================================
// REGISTER & MEMORY ACCESS
// ============================================================================

Value TinyVM::get_register(uint8_t reg) const {
  if (reg < VM_REGISTER_COUNT) {
    return state.registers[reg];
  }
  return Value(0);
}

void TinyVM::set_register(uint8_t reg, Value val) {
  if (reg < VM_REGISTER_COUNT) {
    state.registers[reg] = val;
  }
}

void TinyVM::set_register_i32(uint8_t reg, int32_t val) {
  if (reg < VM_REGISTER_COUNT) {
    state.registers[reg].i32 = val;
  }
}

Value TinyVM::read_memory(uint16_t addr) {
  if (addr < VM_HEAP_SIZE) {
    Value v;
    v.i32 = *(int32_t*)&state.heap[addr];
    return v;
  }
  return Value(0);
}

void TinyVM::write_memory(uint16_t addr, Value val) {
  if (addr < VM_HEAP_SIZE) {
    *(int32_t*)&state.heap[addr] = val.i32;
  }
}

// ============================================================================
// STACK OPERATIONS
// ============================================================================

void TinyVM::push_stack(Value val) {
  if (state.sp > 3) {  // Guard: keep at least 4 bytes
    *(int32_t*)&state.stack[state.sp - 3] = val.i32;
    state.sp -= 4;
  }
}

Value TinyVM::pop_stack() {
  Value v;
  if (state.sp < VM_STACK_SIZE - 3) {
    v.i32 = *(int32_t*)&state.stack[state.sp + 1];
    state.sp += 4;
  }
  return v;
}

Value TinyVM::peek_stack(uint8_t offset) {
  Value v;
  uint16_t addr = state.sp + 1 + (offset * 4);
  if (addr < VM_STACK_SIZE) {
    v.i32 = *(int32_t*)&state.stack[addr];
  }
  return v;
}

// ============================================================================
// HEAP OPERATIONS
// ============================================================================

uint16_t TinyVM::alloc_heap(uint16_t size) {
  if (state.hp + size < VM_HEAP_SIZE && state.alloc_depth < 32) {
    uint16_t base = state.hp;
    state.alloc_stack[state.alloc_depth++] = base;
    state.hp += size;
    return base;
  }
  return 0xFFFF;  // Error code
}

void TinyVM::free_heap() {
  if (state.alloc_depth > 0) {
    state.hp = state.alloc_stack[--state.alloc_depth];
  }
}

// ============================================================================
// DEBUGGING
// ============================================================================

void TinyVM::dump_state() {
  Serial.println("\n=== VM STATE ===");
  Serial.print("PC: 0x"); Serial.println(state.pc, HEX);
  Serial.print("SP: 0x"); Serial.println(state.sp, HEX);
  Serial.print("FP: 0x"); Serial.println(state.fp, HEX);
  Serial.print("HP: 0x"); Serial.println(state.hp, HEX);
  Serial.print("Flags: 0x"); Serial.println(state.flags.as_byte(), HEX);
  Serial.print("Halted: "); Serial.println(state.halted ? "YES" : "NO");
  
  dump_registers();
  Serial.println("================\n");
}

void TinyVM::dump_registers() {
  Serial.println("\n--- REGISTERS ---");
  for (int i = 0; i < VM_REGISTER_COUNT; i++) {
    Serial.print("R");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(state.registers[i].i32);
  }
}

void TinyVM::dump_stack(uint8_t count) {
  Serial.println("\n--- STACK ---");
  uint16_t addr = state.sp + 1;
  for (int i = 0; i < count && addr < VM_STACK_SIZE; i++) {
    int32_t val = *(int32_t*)&state.stack[addr];
    Serial.print("0x");
    Serial.print(addr, HEX);
    Serial.print(": ");
    Serial.println(val);
    addr += 4;
  }
}

const char* TinyVM::opcode_name(uint8_t opcode) {
  switch (opcode) {
    case OP_NOP: return "NOP";
    case OP_HALT: return "HALT";
    case OP_ADD: return "ADD";
    case OP_SUB: return "SUB";
    case OP_MUL: return "MUL";
    case OP_DIV: return "DIV";
    case OP_MOD: return "MOD";
    case OP_AND: return "AND";
    case OP_OR: return "OR";
    case OP_XOR: return "XOR";
    case OP_NOT: return "NOT";
    case OP_SHL: return "SHL";
    case OP_SHR: return "SHR";
    case OP_CMP: return "CMP";
    case OP_LOAD: return "LOAD";
    case OP_LOADI: return "LOADI";
    case OP_LOADI16: return "LOADI16";
    case OP_STORE: return "STORE";
    case OP_LOAD_MEM: return "LOAD_MEM";
    case OP_LOAD_ADDR: return "LOAD_ADDR";
    case OP_ALLOC: return "ALLOC";
    case OP_FREE: return "FREE";
    case OP_PUSH: return "PUSH";
    case OP_POP: return "POP";
    case OP_PEEK: return "PEEK";
    case OP_JMP: return "JMP";
    case OP_JMPL: return "JMPL";
    case OP_JZ: return "JZ";
    case OP_JNZ: return "JNZ";
    case OP_JLT: return "JLT";
    case OP_JGT: return "JGT";
    case OP_JLE: return "JLE";
    case OP_JGE: return "JGE";
    case OP_JEQ: return "JEQ";
    case OP_JNEQ: return "JNEQ";
    case OP_CALL: return "CALL";
    case OP_CALLL: return "CALLL";
    case OP_RET: return "RET";
    case OP_PRINT: return "PRINT";
    case OP_PRINTC: return "PRINTC";
    case OP_READ: return "READ";
    case OP_TRAP: return "TRAP";
    case OP_DEBUG: return "DEBUG";
    default: return "???";
  }
}
