# Quick Reference & Summary

## FILES CREATED

1. **vm_architecture.md** - Formal specification of VM components (memory, registers, opcodes, flags)
2. **tiny_vm.h** - C++ header file with data structures and VM class
3. **tiny_vm.cpp** - VM implementation (execution engine, memory management)
4. **vm_example.ino** - Arduino sketch demonstrating 5 example programs
5. **execution_flow.md** - Detailed execution traces and memory layouts
6. **compiler_design.md** - Code generation from AST to bytecode
7. **this file** - Quick reference summary

---

## 1. VM AT A GLANCE

### Memory Layout

```
Stack (64 KB)     ← grows downward
    ↓
[Guard zone]
    ↓
Heap (128 KB)     ← grows upward
```

### Key Resources

- **8 Registers** (R0-R7): 32-bit signed integers
- **1 Flags Register**: 8 bits (zero, eq, neq, lt, gt, le, ge, carry)
- **Stack**: 64 KB, for function calls and expression evaluation
- **Heap**: 128 KB, for dynamic arrays

### Instruction Format

```
[OPCODE (8-bit)] [ARG1 (8-bit)] [ARG2 (8-bit)]
         │
         └─ Defines operation type
```

**Total**: 50 opcodes covering arithmetic, memory, control flow, I/O

---

## 2. INSTRUCTION CHEAT SHEET

### Arithmetic (Results → R0)

| Mnemonic | Opcode | Effect               | Notes                         |
| -------- | ------ | -------------------- | ----------------------------- |
| ADD      | 0x10   | R0 = ARG1 + ARG2     | Sets carry/zero flags         |
| SUB      | 0x11   | R0 = ARG1 - ARG2     |                               |
| MUL      | 0x12   | R0 = ARG1 \* ARG2    |                               |
| DIV      | 0x13   | R0 = ARG1 / ARG2     | Div by 0 → R0 = 0             |
| MOD      | 0x14   | R0 = ARG1 % ARG2     |                               |
| AND      | 0x15   | R0 = ARG1 & ARG2     | Bitwise                       |
| OR       | 0x16   | R0 = ARG1 \| ARG2    | Bitwise                       |
| XOR      | 0x17   | R0 = ARG1 ^ ARG2     | Bitwise                       |
| NOT      | 0x18   | R0 = ~ARG1           | Bitwise                       |
| SHL      | 0x19   | R0 = ARG1 << ARG2    | Shift left                    |
| SHR      | 0x1A   | R0 = ARG1 >> ARG2    | Shift right                   |
| CMP      | 0x1B   | Compare ARG1 vs ARG2 | Sets EQ/NEQ/LT/GT/LE/GE flags |

### Memory

| Mnemonic  | Opcode | Effect                           |
| --------- | ------ | -------------------------------- |
| LOAD      | 0x20   | R[ARG1] = R[ARG2]                |
| LOADI     | 0x21   | R[ARG1] = ARG2 (8-bit signed)    |
| LOADI16   | 0x22   | R[ARG1] = next 16-bit word       |
| STORE     | 0x23   | M[R[ARG1]] = R[ARG2]             |
| LOAD_MEM  | 0x24   | R[ARG1] = M[R[ARG2]]             |
| LOAD_ADDR | 0x25   | R[ARG1] = heap_base + ARG2       |
| ALLOC     | 0x26   | Allocate ARG1 bytes; result → R6 |
| FREE      | 0x27   | Free last allocation (LIFO)      |

### Stack

| Mnemonic | Opcode | Effect                              |
| -------- | ------ | ----------------------------------- |
| PUSH     | 0x30   | Stack[SP--] = R[ARG1]               |
| POP      | 0x31   | R[ARG1] = Stack[++SP]               |
| PEEK     | 0x32   | R[ARG1] = Stack[SP + ARG2] (no pop) |

### Control Flow

| Mnemonic | Opcode | Effect                   | Condition       |
| -------- | ------ | ------------------------ | --------------- |
| JMP      | 0x40   | PC = (ARG1 << 8) \| ARG2 | Always          |
| JZ       | 0x42   | Jump                     | zero flag set   |
| JNZ      | 0x43   | Jump                     | zero flag clear |
| JLT      | 0x44   | Jump                     | lt flag set     |
| JGT      | 0x45   | Jump                     | gt flag set     |
| JLE      | 0x46   | Jump                     | le flag set     |
| JGE      | 0x47   | Jump                     | ge flag set     |
| JEQ      | 0x48   | Jump                     | eq flag set     |
| JNEQ     | 0x49   | Jump                     | neq flag set    |

### Functions

| Mnemonic | Opcode | Effect                              |
| -------- | ------ | ----------------------------------- |
| CALL     | 0x50   | Push PC+2; PC = (ARG1 << 8) \| ARG2 |
| RET      | 0x52   | PC = Pop()                          |

### I/O & System

| Mnemonic | Opcode | Effect                       |
| -------- | ------ | ---------------------------- |
| PRINT    | 0x60   | Output R[ARG1] to serial     |
| PRINTC   | 0x61   | Output R[ARG1] as char       |
| READ     | 0x62   | R[ARG1] = read from serial   |
| TRAP     | 0x63   | System call (ARG1 = trap ID) |
| DEBUG    | 0x64   | Debug output                 |

---

## 3. REGISTERS

```
R0: Accumulator (arithmetic results)
R1-R5: General purpose / local variables
R6: Return value (function results)
R7: Frame pointer (reserved)
```

**All 32-bit signed integers** (-2,147,483,648 to 2,147,483,647)

For 64-bit doubles: Use adjacent register pairs (R0-R1, R2-R3, etc.)

---

## 4. FLAGS REGISTER

Set by: **CMP** and arithmetic operations

| Bit | Flag  | Set When                |
| --- | ----- | ----------------------- |
| 0   | carry | Arithmetic overflow     |
| 1   | zero  | Result is 0             |
| 2   | eq    | CMP: operands equal     |
| 3   | neq   | CMP: operands not equal |
| 4   | gt    | CMP: arg1 > arg2        |
| 5   | lt    | CMP: arg1 < arg2        |
| 6   | ge    | CMP: arg1 >= arg2       |
| 7   | le    | CMP: arg1 <= arg2       |

---

## 5. QUICK PROGRAM EXAMPLES

### Example 1: Simple Math

```cpp
// R0 = 5 + 3
LOADI R0, 5         // 0x21, 0x00, 0x05
LOADI R1, 3         // 0x21, 0x01, 0x03
ADD R0, R1          // 0x10, 0x00, 0x01 (result in R0)
PRINT R0            // 0x60, 0x00, 0x00
HALT                // 0x01, 0x00, 0x00
```

### Example 2: Conditional

```cpp
// if (x > 5) print(1) else print(0)
LOADI R0, 10        // x = 10
LOADI R1, 5
CMP R0, R1          // 0x1B, 0x00, 0x01
JLE 0x0F            // 0x46, 0x00, 0x0F (jump if x <= 5)
LOADI R0, 1         // then: print 1
PRINT R0
JMP 0x15            // skip else
LOADI R0, 0         // else: print 0
PRINT R0
HALT
```

### Example 3: Loop (sum 1 to 5)

```cpp
LOADI R0, 0         // sum = 0
LOADI R1, 1         // i = 1
LOADI R2, 5         // limit = 5
// Loop:
ADD R0, R1          // sum += i
LOADI R3, 1
ADD R1, R3          // i++
CMP R1, R2
JLE 0x09            // loop back if i <= 5
PRINT R0            // output sum
HALT
```

### Example 4: Function Call

```cpp
// main:
LOADI R0, 5         // arg1 = 5
LOADI R1, 3         // arg2 = 3
PUSH R0
PUSH R1
CALL 0x18           // call add()
PRINT R6            // R6 has result
HALT

// add function at 0x18:
POP R0              // pop arg2
POP R1              // pop arg1
ADD R0, R1
LOAD R6, R0         // move result to R6
RET
```

### Example 5: Array

```cpp
ALLOC 16            // allocate 16 bytes
LOADI R0, 100
STORE R6, R0        // arr[0] = 100
LOADI R0, 200
LOADI R1, 4
ADD R1, R6          // calculate arr[1] address
STORE R1, R0        // arr[1] = 200
LOAD_MEM R1, R6     // load arr[0] into R1
PRINT R1            // output 100
HALT
```

---

## 6. MEMORY USAGE BREAKDOWN (ESP32)

```
Total ESP32 SRAM: 320 KB

Used by VM:
  - Registers & state: 1 KB
  - Stack: 64 KB
  - Heap: 128 KB
  Subtotal: 193 KB

Available for:
  - Sketch code: ~50 KB
  - Other: ~77 KB

Total: 320 KB
```

**Stack capacity**: ~4000 function frames (16 bytes each)
**Heap capacity**: ~32 large arrays (4 KB each)

---

## 7. DATA TYPE ENCODING

| Type     | Size | Range         | Storage              |
| -------- | ---- | ------------- | -------------------- |
| int      | 4B   | -2³¹ to 2³¹-1 | Native register      |
| char     | 1B   | 0-255         | In register as int32 |
| bool     | 1B   | 0/1           | In register as int32 |
| array[N] | N×4B | N elements    | Heap allocated       |
| double   | 8B   | IEEE 754      | 2 adjacent registers |

---

## 8. CALLING CONVENTION

### Function Prologue (Caller)

1. Evaluate arguments
2. PUSH arguments right-to-left
3. CALL function_address

### Function Prologue (Callee)

1. Arguments on stack (accessible via SP + offset)
2. Allocate locals (via LOADI into registers or push)

### Function Return (Callee)

1. Move result to R6 (if needed)
2. RET

### Function Epilogue (Caller)

1. Result available in R6
2. Continue execution

---

## 9. PERFORMANCE ESTIMATES

| Operation      | Time (µs on ESP32 @ 160MHz) |
| -------------- | --------------------------- |
| LOADI          | 0.05                        |
| ADD/SUB        | 0.1                         |
| MUL            | 0.2                         |
| DIV            | 2.0                         |
| LOAD/STORE     | 0.1                         |
| PUSH/POP       | 0.15                        |
| CALL/RET       | 0.15                        |
| JMP            | 0.05                        |
| PRINT (serial) | 10-100                      |

**Program execution**: ~1-2 ms for typical 50-100 instruction program (excluding I/O)

---

## 10. COMMON PITFALLS

### Stack Overflow

- Max recursion depth: ~4000 functions
- Each frame: ~16 bytes minimum
- Guard zone: Bottom 4 bytes of stack

### Heap Fragmentation

- Uses LIFO allocation (stack-like)
- FREE only deallocates last allocation
- For better control: explicitly manage with ALLOC/FREE pairs

### Lost Return Values

- Must check R6 immediately after CALL
- Don't call another function without saving R6
- Use PUSH/POP to preserve R6 across calls

### Register Clobbering

- Arithmetic always writes to R0
- Save any needed values to stack first
- Use different registers for independent values

---

## 11. DEBUGGING COMMANDS

```cpp
vm.dump_state();          // Print PC, SP, registers, flags
vm.dump_registers();      // Print all 8 registers
vm.dump_stack(10);        // Print top 10 stack entries
```

Example output:

```
=== VM STATE ===
PC: 0xF
SP: 0xFFF3
FP: 0xFFFF
HP: 0x10
Flags: 0x04 (zero bit set)
Halted: NO

--- REGISTERS ---
R0: 42
R1: 100
R2: 5
R3-R7: 0
================
```

---

## 12. INTEGRATION CHECKLIST

- [ ] Create `codegen.h/c` for code generation
- [ ] Implement instruction emitter
- [ ] Add symbol table management
- [ ] Map language constructs to VM instructions
- [ ] Test with simple programs first
- [ ] Compile full language test suite
- [ ] Verify bytecode on SD card
- [ ] Load bytecode on ESP32
- [ ] Run and debug with `dump_state()`
- [ ] Optimize critical paths

---

## 13. TRAPS (System Calls)

Via `OP_TRAP` instruction with trap ID in ARG1:

| ID   | Function          | Effect                |
| ---- | ----------------- | --------------------- |
| 0x01 | TRAP_FLUSH_OUTPUT | Flush serial buffer   |
| 0x02 | TRAP_DELAY        | Delay R0 milliseconds |
| 0xFF | TRAP_DEBUG_DUMP   | Dump VM state         |

Example:

```cpp
LOADI R0, 500       // delay 500ms
TRAP 0x02           // 0x63, 0x02, 0x00
```

---

## 14. EXAMPLE COMPILATION FLOW

```
Source file (my_program.src):
  int x = 10;
  print(x * 2);
        │
        ▼
    Lexer (Flex)
        │
        ▼
    Tokens: [ID(x), ASSIGN, INT(10), PRINT, ID(x), MULT, INT(2)]
        │
        ▼
    Parser (Yacc)
        │
        ▼
    AST: Program
      ├─ Declaration(x, 10)
      └─ Call(print, Mul(x, 2))
        │
        ▼
    Semantic Analysis
      (Type checking, symbol resolution)
        │
        ▼
    Code Generation (NEW)
        │
        ▼
    Bytecode:
      LOADI R0, 10
      LOADI R1, 2
      MUL R0, R1
      PRINT R0
      HALT
        │
        ▼
    Write to program.vm (binary)
        │
        ▼
    Load on ESP32 & Execute
```

---

## 15. MINIMAL WORKING EXAMPLE

Arduino sketch:

```cpp
#include "tiny_vm.h"

TinyVM vm;

// Bytecode: R0=5; R1=3; R0+=R1; print R0
const uint8_t prog[] = {
  0x21, 0x00, 0x05,  // LOADI R0, 5
  0x21, 0x01, 0x03,  // LOADI R1, 3
  0x10, 0x00, 0x01,  // ADD R0, R1
  0x60, 0x00, 0x00,  // PRINT R0
  0x01, 0x00, 0x00   // HALT
};

void setup() {
  Serial.begin(115200);
  vm.initialize();
  vm.load_program(prog, sizeof(prog));
  vm.execute();        // Outputs: 8
  vm.dump_state();
}

void loop() { delay(1000); }
```

Output:

```
8
=== VM STATE ===
PC: 0xF
SP: 0xFFFF
Flags: 0x00
Halted: YES
```

---

**Quick Reference Complete**
