# VM Architecture

## 1. OVERVIEW

This VM is designed for **minimal memory footprint** and **efficient execution** on ESP32 (with ~320KB SRAM for program execution). The architecture prioritizes:

- **Compact instruction format** (3 bytes per instruction)
- **Shared register-stack model** (8 registers reduce memory pressure)
- **Unified value type** (32-bit integers primary, with optional 64-bit float support)
- **Direct addressing** (no complex memory indirection)

---

## 2. MEMORY LAYOUT

```
ESP32 Memory Organization (for Program Execution)
┌─────────────────────────────────────────┐
│  INSTRUCTION MEMORY (SD Card / EEPROM)  │
│  - Bytecode program                     │
│  - Function table (addresses)           │
│  - Global constant pool                 │
└─────────────────────────────────────────┘

RUNTIME MEMORY (RAM - ~320KB available)
┌─────────────────────────────────────────┐
│  STACK (grows downward)                 │
│  - Expression evaluation                │
│  - Function call frames                 │
│  - Local variables                      │
├─────────────────────────────────────────┤
│  GAP (guard region)                     │
├─────────────────────────────────────────┤
│  HEAP (grows upward)                    │
│  - Dynamic arrays                       │
│  - String data (if needed)              │
├─────────────────────────────────────────┤
│  REGISTERS (R0-R7) - 32 bytes static    │
├─────────────────────────────────────────┤
│  VM STATE STRUCTURES                    │
│  - PC (Program Counter)                 │
│  - SP (Stack Pointer)                   │
│  - HP (Heap Pointer)                    │
│  - Frame pointers                       │
└─────────────────────────────────────────┘
```

---

## 3. REGISTERS

| Register | Purpose         | Special Use                       |
| -------- | --------------- | --------------------------------- |
| R0-R5    | General Purpose | Scratch, operands                 |
| R6       | Accumulator     | Arithmetic results, return values |
| R7       | Reserved        | Stack frame base (FP)             |

**Size**: 32-bit signed integers (support 64-bit doubles via pairs R0-R1, R2-R3, etc.)

---

## 4. INSTRUCTION FORMAT

```
Instruction Structure (3 bytes):
┌──────────┬──────────┬──────────┐
│ OPCODE   │  ARG1    │  ARG2    │
│ (8 bits) │ (8 bits) │ (8 bits) │
└──────────┴──────────┴──────────┘

Total: 1 byte opcode + 2 bytes arguments = 3 bytes per instruction
```

### Instruction Interpretation

- **Opcode**: Defines operation type (arithmetic, memory, control)
- **ARG1**: First operand (register index, memory offset, or value)
- **ARG2**: Second operand (register index or immediate value)

**Immediate Values**: If ARG1/ARG2 ≥ 128, indicates immediate mode (value stored in argument byte)

---

## 5. OPCODE DEFINITIONS

### Arithmetic & Logic (20 opcodes)

| Opcode | Mnemonic | Args | Operation            | Notes              |
| ------ | -------- | ---- | -------------------- | ------------------ |
| 0x00   | NOP      | -    | No-op                | Padding            |
| 0x01   | ADD      | R,R  | R0 = ARG1 + ARG2     | Stores in R0       |
| 0x02   | SUB      | R,R  | R0 = ARG1 - ARG2     |                    |
| 0x03   | MUL      | R,R  | R0 = ARG1 \* ARG2    |                    |
| 0x04   | DIV      | R,R  | R0 = ARG1 / ARG2     | Div by zero = R0:0 |
| 0x05   | MOD      | R,R  | R0 = ARG1 % ARG2     |                    |
| 0x06   | AND      | R,R  | R0 = ARG1 & ARG2     | Bitwise            |
| 0x07   | OR       | R,R  | R0 \| ARG2           | Bitwise            |
| 0x08   | XOR      | R,R  | R0 = ARG1 ^ ARG2     | Bitwise            |
| 0x09   | NOT      | R    | R0 = ~ARG1           | Bitwise NOT        |
| 0x0A   | CMP      | R,R  | Compare ARG1 vs ARG2 | Sets flags         |
| 0x0B   | SHL      | R,I  | R0 = ARG1 << ARG2    | Shift left         |
| 0x0C   | SHR      | R,I  | R0 = ARG1 >> ARG2    | Shift right        |

### Memory Access (10 opcodes)

| Opcode | Mnemonic  | Args | Operation                  | Notes            |
| ------ | --------- | ---- | -------------------------- | ---------------- |
| 0x10   | LOAD      | R,R  | R[ARG1] = R[ARG2]          | Register copy    |
| 0x11   | LOADI     | R,I  | R[ARG1] = ARG2 (immediate) | 8-bit immediate  |
| 0x12   | LOADI16   | R,I  | R[ARG1] = NEXT_WORD        | 16-bit immediate |
| 0x13   | STORE     | R,R  | M[R[ARG1]] = R[ARG2]       | Array store      |
| 0x14   | LOAD_ADDR | R,I  | R[ARG1] = heap_base + ARG2 | Address calc     |
| 0x15   | PUSH      | R    | STACK[SP--] = R[ARG1]      | Push register    |
| 0x16   | POP       | R    | R[ARG1] = STACK[++SP]      | Pop to register  |
| 0x17   | PEEK      | R,I  | R[ARG1] = STACK[SP+ARG2]   | Read without pop |

### Control Flow (10 opcodes)

| Opcode | Mnemonic | Args | Operation                  | Notes                |
| ------ | -------- | ---- | -------------------------- | -------------------- |
| 0x20   | JMP      | ADDR | PC = ARG1 \| (ARG2 << 8)   | Unconditional jump   |
| 0x21   | JZ       | ADDR | if (flags.zero) PC = ADDR  | Jump if zero         |
| 0x22   | JNZ      | ADDR | if (!flags.zero) PC = ADDR | Jump if not zero     |
| 0x23   | JLT      | ADDR | if (flags.lt) PC = ADDR    | Jump if less than    |
| 0x24   | JGT      | ADDR | if (flags.gt) PC = ADDR    | Jump if greater      |
| 0x25   | JLE      | ADDR | if (flags.le) PC = ADDR    | Jump if ≤            |
| 0x26   | JGE      | ADDR | if (flags.ge) PC = ADDR    | Jump if ≥            |
| 0x27   | CALL     | ADDR | Push PC+2; PC = ADDR       | Function call        |
| 0x28   | RET      | -    | PC = Pop()                 | Return from function |
| 0x29   | HALT     | -    | Stop execution             | End program          |

### Special (6 opcodes)

| Opcode | Mnemonic | Args | Operation                         | Notes              |
| ------ | -------- | ---- | --------------------------------- | ------------------ |
| 0x30   | PRINT    | R    | Output R[ARG1]                    | Console output     |
| 0x31   | READ     | R    | R[ARG1] = Input()                 | Console input      |
| 0x32   | ALLOC    | I    | Allocate ARG1 bytes on heap       | Returns base in R6 |
| 0x33   | FREE     | -    | Free last allocation              | LIFO dealloc       |
| 0x34   | TRAP     | I    | System trap (I/O, debug)          | Invoke builtin/extended ops (args in registers; id=ARG1) |
| 0x35   | DEBUG    | R,I  | Debug output R[ARG1] with ID ARG2 | Dev only           |

---

## 6. FLAGS REGISTER

```
FLAGS (8-bit):
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ 7  │ 6  │ 5  │ 4  │ 3  │ 2  │ 1  │ 0  │
├────┴────┴────┴────┴────┴────┼────┼────┤
│    Reserved (0)              │ GE │ LE │ LT │ GT │ NEQ│ EQ │ Zero│Carry│
└──────────────────────────────┴────┴────┴────┴────┴────┴────┴────┴────┘
```

Bits:

- **Bit 0**: Carry flag (overflow from arithmetic)
- **Bit 1**: Zero flag (result == 0)
- **Bit 2**: EQ (equal from CMP)
- **Bit 3**: NEQ (not equal)
- **Bit 4**: GT (greater than)
- **Bit 5**: LT (less than)
- **Bit 6**: GE (greater or equal)
- **Bit 7**: LE (less or equal)

---

## 7. STACK FRAME LAYOUT

```
For each function call:

┌────────────────────┐  <- SP (grows down)
│ Local Var N        │
│ ...                │
│ Local Var 1        │
├────────────────────┤  <- Frame Base (R7)
│ Return Address     │  (2 bytes, pushed by CALL)
├────────────────────┤
│ Prev Frame Ptr     │  (optional, for debugging)
├────────────────────┤
│ Arg N              │  (arguments pushed right-to-left)
│ ...                │
│ Arg 1              │
└────────────────────┘
```

**Call Sequence**:

1. Push arguments (right-to-left)
2. CALL instruction (auto-pushes return address)
3. Function prologue: allocate local vars
4. Execute function body
5. Place return value in R6
6. RET instruction: pop return address, restore SP

---

## 8. DATA TYPES

| Type     | Size           | Range                           | Encoding                  |
| -------- | -------------- | ------------------------------- | ------------------------- |
| int8     | 1B             | -128 to 127                     | Native                    |
| int32    | 4B             | -2,147,483,648 to 2,147,483,647 | Native (register)         |
| char     | 1B             | 0 to 255 (ASCII)                | In register as int32      |
| bool     | 1B             | 0 (false), 1 (true)             | In register as int32      |
| double   | 8B             | IEEE 754                        | Uses 2 adjacent registers |
| array[N] | N \* elem_size | Dynamic                         | Heap allocated            |

**Type System**: Compile-time; runtime is untyped (values are 32-bit or 64-bit words).

---

## 9. MEMORY CONSTRAINTS (ESP32)

- **Total SRAM**: 320 KB
- **Stack Size**: 64 KB (typical)
- **Heap Size**: 128 KB (arrays/dynamic)
- **VM Structures**: ~1 KB (registers, state)
- **Bytecode**: Loaded from SD Card (~4 KB typical per function)

**GC Strategy**: Reference counting on arrays; deallocation via explicit FREE or scope exit.

---

## 10. EXECUTION MODEL

### Fetch-Decode-Execute Cycle

```
Loop:
  1. FETCH:  instruction = memory[PC]
  2. DECODE: extract opcode, ARG1, ARG2
  3. EXECUTE:
     - Perform operation
     - Update PC (usually PC += 3, or conditional jump)
     - Set flags if needed
  4. Repeat until HALT
```

### Example: ADD instruction execution

```
Instruction: [0x01, 0x01, 0x02]  (ADD R1 + R2)
  - OPCODE: 0x01 (ADD)
  - ARG1: 0x01 (register 1)
  - ARG2: 0x02 (register 2)

Execution:
  R0 = R1 + R2
  PC += 3
  flags.zero = (R0 == 0) ? 1 : 0
  flags.carry = (overflow) ? 1 : 0
```

---

## 11. FUNCTION CALL SEQUENCE

### Example: Calling `add(a, b)`

```
Bytecode:
  ... caller code ...
  LOADI R0, a       (load arg1 into R0)
  LOADI R1, b       (load arg2 into R1)
  PUSH R0           (push arg1)
  PUSH R1           (push arg2)
  CALL 0x1000       (jump to add() at address 0x1000)
  ...               (after return, result in R6)

At function address 0x1000:
  [prologue]
  LOAD R2, R7 + 4   (load arg1 from stack)
  LOAD R3, R7 + 2   (load arg2 from stack)
  ADD R2, R3        (R0 = R2 + R3)
  LOAD R6, R0       (move result to R6)
  RET               (return)
```

---

## 12. KEY CONSTRAINTS FOR ESP32 OPTIMIZATION

1. **No dynamic function tables**: Function addresses are hardcoded or stored in a small lookup table at program start.
2. **Limited recursion depth**: Stack frame size ~16 bytes; ~4000 frame depth max.
3. **No garbage collection**: Use LIFO allocation; programs must manage deallocations.
4. **Single-threaded execution**: No concurrency; simplifies state management.
5. **I/O via traps**: Serial output, SD card access through TRAP instruction.
6. **No virtual method dispatch**: All calls are static.

---

## 13. SUMMARY TABLE

| Component        | Capacity        | Notes                       |
| ---------------- | --------------- | --------------------------- |
| Registers        | 8 x 32-bit      | + 1 flags register          |
| Stack            | 64 KB           | ~4000 frames @ 16B each     |
| Heap             | 128 KB          | Array allocation            |
| Code             | 256 KB          | From SD card                |
| Instruction Set  | 50 opcodes      | Arithmetic, memory, control |
| Max program size | ~50 KB bytecode | Including function table    |

---

## 14. LANGUAGE FEATURE MAPPING

| Language Construct     | VM Support                     |
| ---------------------- | ------------------------------ |
| Variable declaration   | LOADI / LOAD_ADDR              |
| Assignment             | LOAD / STORE                   |
| Arrays                 | LOAD_ADDR + STORE/LOAD + ALLOC |
| Arithmetic expressions | ADD/SUB/MUL/DIV/MOD            |
| Boolean operations     | AND/OR/XOR/NOT                 |
| Comparisons            | CMP + conditional jumps        |
| If/else                | JZ/JNZ to branch               |
| Loops (for/while)      | JMP + conditional jumps        |
| Function calls         | CALL/RET + stack frames        |
| Return statements      | LOAD R6 + RET                  |

---

**End of Formal Specification**
