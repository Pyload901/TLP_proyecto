# Execution Flow & Architecture Guide

## 1. INITIALIZATION SEQUENCE

```
┌──────────────────────────────────────────────────┐
│  ESP32 Arduino startup                           │
└──────────────────────────────────┬────────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────┐
                    │  setup() called          │
                    │  Serial.begin(115200)    │
                    │  delay(1000)             │
                    └──────────┬───────────────┘
                               │
                               ▼
                    ┌──────────────────────────┐
                    │  vm.initialize()         │
                    │  Allocate stack (64KB)   │
                    │  Allocate heap (128KB)   │
                    │  Init registers to 0     │
                    │  Set PC = 0              │
                    │  Set SP = 65535 (top)    │
                    │  Set FP = 65535          │
                    │  Set HP = 0              │
                    │  Return TRUE             │
                    └──────────┬───────────────┘
                               │
                               ▼
                    ┌──────────────────────────┐
                    │  vm.load_program(code)   │
                    │  Store bytecode pointer  │
                    │  Set code_size           │
                    │  Return TRUE             │
                    └──────────┬───────────────┘
                               │
                               ▼
                    ┌──────────────────────────┐
                    │  vm.execute()            │
                    │  Fetch-Decode-Execute    │
                    │  Loop until HALT         │
                    └──────────────────────────┘
```

## 2. FETCH-DECODE-EXECUTE CYCLE

Each VM step executes one 3-byte instruction:

```
STATE: PC=0, SP=65535, R0..R7=0, FLAGS=0

Cycle N:
┌─────────────────────────────────────────────────────┐
│ FETCH PHASE                                         │
├─────────────────────────────────────────────────────┤
│ 1. Check if PC >= code_size                         │
│    If yes: halted = true; return                    │
│ 2. Read 3 bytes from code[PC..PC+2]                │
│    Example: [0x21, 0x00, 0x05]                      │
│ 3. Create Instruction(opcode, arg1, arg2)           │
│ 4. Advance PC by 3                                  │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│ DECODE PHASE                                        │
├─────────────────────────────────────────────────────┤
│ Extract fields:                                     │
│   opcode = 0x21 (LOADI)                             │
│   arg1 = 0x00 (register 0)                          │
│   arg2 = 0x05 (immediate value 5)                   │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│ EXECUTE PHASE: SWITCH(opcode)                       │
├─────────────────────────────────────────────────────┤
│ case OP_LOADI (0x21):                               │
│   R[arg1] = sign_extend(arg2)                       │
│   registers[0].i32 = 5                              │
│   (No flags update for LOADI)                       │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
                PC += 3
              FLAGS = unchanged
           REGISTERS[0] = 5
```

## 3. ARITHMETIC OPERATION EXAMPLE

```
Instruction: ADD R0, R1  (add registers 0 and 1)
Bytecode: [0x10, 0x00, 0x01]

STATE BEFORE:
  PC = 0
  R0 = 5
  R1 = 3
  FLAGS = 0

EXECUTION:
  1. FETCH: Read [0x10, 0x00, 0x01], PC = 3
  2. DECODE: opcode=ADD, arg1=0, arg2=1
  3. EXECUTE (case OP_ADD):
     - v1 = registers[0].i32 = 5
     - v2 = registers[1].i32 = 3
     - res = (int64_t)5 + 3 = 8
     - no overflow
     - registers[0].i32 = 8
     - update_flags_arithmetic(8, false)
       * flags.zero = 0 (8 != 0)
       * flags.carry = 0

STATE AFTER:
  PC = 3
  R0 = 8
  R1 = 3
  FLAGS: zero=0, carry=0
```

## 4. MEMORY ACCESS: LOAD IMMEDIATE

```
Instruction Sequence:
  LOADI R0, 100      (load 100 into R0)
  LOADI R1, -50      (load -50 into R1)

Bytecode:
  [0x21, 0x00, 100]
  [0x21, 0x01, 206]  (206 = -50 as unsigned byte with sign extension)

EXECUTION:
  Step 1: PC=0
    - Read [0x21, 0x00, 100]
    - LOADI: registers[0].i32 = sign_extend_8(100) = 100
    - PC = 3

  Step 2: PC=3
    - Read [0x21, 0x01, 206]
    - LOADI: registers[1].i32 = sign_extend_8(-50) = -50 (C auto sign-extends on cast)
    - PC = 6

RESULT:
  R0 = 100
  R1 = -50
```

## 5. STACK OPERATIONS: FUNCTION CALL

```
Function call: main() calls add(5, 3)

Bytecode Structure:
  Offset  Code
  0x0000: LOADI R0, 5    [0x21, 0x00, 0x05]
  0x0003: LOADI R1, 3    [0x21, 0x01, 0x03]
  0x0006: PUSH R0        [0x30, 0x00, 0x00]
  0x0009: PUSH R1        [0x30, 0x01, 0x00]
  0x000C: CALL add       [0x50, 0x00, 0x12]  (call to 0x0012)
  0x000F: PRINT R6       [0x60, 0x06, 0x00]
  0x0012: (ADD function starts here)
          POP R0         [0x31, 0x00, 0x00]
          POP R1         [0x31, 0x01, 0x00]
          ADD R0, R1     [0x10, 0x00, 0x01]
          LOAD R6, R0    [0x20, 0x06, 0x00]
          RET            [0x52, 0x00, 0x00]

MEMORY STATE PROGRESSION:

Before PUSH:
  SP = 0xFFFF (65535)
  Stack: [empty]

After "PUSH R0" (SP -= 4):
  SP = 0xFFFB (65531)
  Stack[65531..65534] = 5 (R0 value)

After "PUSH R1" (SP -= 4):
  SP = 0xFFF7 (65527)
  Stack[65527..65530] = 3 (R1 value)
  Stack[65531..65534] = 5

After "CALL add" (push return address):
  SP = 0xFFF3 (65523)
  Stack[65523..65526] = 0x000F (return address after CALL)
  Stack[65527..65530] = 3
  Stack[65531..65534] = 5
  PC = 0x0012 (jump to add function)

In add function "POP R0":
  R0 = Stack[65527..65530] = 3
  SP = 0xFFF7 (65527)  -- SP += 4
  Stack[65523..65526] = 0x000F (still there)
  Stack[65531..65534] = 5

In add function "POP R1":
  R1 = Stack[65531..65534] = 5
  SP = 0xFFFB (65531)  -- SP += 4
  Stack[65523..65526] = 0x000F (still there)

In add function "ADD R0, R1":
  R0 = 3 + 5 = 8
  R6 unchanged

In add function "LOAD R6, R0":
  R6 = 8

In add function "RET":
  PC = Stack[65523..65526] = 0x000F
  SP = 0xFFFF (65535)  -- SP += 4

Back in main after CALL:
  PC = 0x000F
  R6 = 8
  SP = 0xFFFF
  "PRINT R6" outputs: 8
```

## 6. CONDITIONAL JUMP: IF-ELSE

```
Code:
  CMP R0, R1         Compare R0 vs R1
  JLE else_block     Jump if R0 <= R1
  [then block]       Execute if R0 > R1
  JMP end
  [else block]       Execute if R0 <= R1
  [end]

Example with R0=10, R1=5:

Bytecode:
  0x00: LOADI R0, 10      [0x21, 0x00, 0x0A]
  0x03: LOADI R1, 5       [0x21, 0x01, 0x05]
  0x06: CMP R0, R1        [0x1B, 0x00, 0x01]
  0x09: JLE 0x15          [0x46, 0x00, 0x15]
  0x0C: LOADI R0, 1       (then branch)
  0x0F: PRINT R0
  0x12: JMP 0x1B
  0x15: LOADI R0, 0       (else branch)
  0x18: PRINT R0
  0x1B: HALT

EXECUTION FLOW:

Step 1: PC=0x00
  LOADI: R0 = 10

Step 2: PC=0x03
  LOADI: R1 = 5

Step 3: PC=0x06
  CMP R0, R1:
    - Compare: 10 > 5
    - Set flags:
      * zero = 0 (10 != 5)
      * eq = 0
      * neq = 1 (10 != 5)
      * lt = 0 (10 is not < 5)
      * gt = 1 (10 > 5)
      * le = 0 (10 is not <= 5)
      * ge = 1 (10 >= 5)

Step 4: PC=0x09
  JLE 0x15 (Jump if LE flag set):
    - flags.le = 0, so NO JUMP
    - PC = 0x0C (normal increment)

Step 5: PC=0x0C
  LOADI: R0 = 1 (then branch executes)

Step 6: PC=0x0F
  PRINT: Output "1"

Step 7: PC=0x12
  JMP 0x1B: PC = 0x1B

Step 8: PC=0x1B
  HALT: Stop

ELSE BRANCH NEVER EXECUTES
```

## 7. HEAP & ARRAY OPERATIONS

```
Instruction Sequence:
  ALLOC 16           Allocate 16 bytes
  LOADI R0, 100      Load value 100
  STORE R6, R0       Store to heap[0]
  LOADI R0, 200      Load value 200
  STORE R6+4, R0     Store to heap[4]
  LOAD_MEM R1, R6    Load from heap[0]
  PRINT R1           Output 100

HEAP MEMORY LAYOUT:

Initial state:
  HP = 0 (heap pointer)
  HEAP: [0, 0, 0, 0, ...]

After ALLOC 16:
  R6 = 0 (base address)
  HP = 16 (next free)
  alloc_stack[0] = 0
  alloc_depth = 1
  HEAP: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...]

After STORE R6, R0 (store 100 at heap[0]):
  HEAP: [100, 0, 0, 0, 100, 0, 0, 0, ...]
         (int32 stored as 4 bytes, little-endian on ESP32)

After storing 200 at heap[4]:
  HEAP: [100, 0, 0, 0, 200, 0, 0, 0, ...]
         [-----heap[0]-----][-----heap[4]-----]

After LOAD_MEM R1, R6:
  R1 = 100 (loaded from heap[0])

PRINT: Output "100"
```

## 8. LOOP EXECUTION

```
Pseudocode:
  Sum = 0
  Count = 1
  Limit = 5
loop:
  Sum = Sum + Count
  Count = Count + 1
  if (Count <= Limit) goto loop
  print Sum

Bytecode:
  0x00: LOADI R0, 0       (Sum = 0)
  0x03: LOADI R1, 1       (Count = 1)
  0x06: LOADI R2, 5       (Limit = 5)
  0x09: ADD R0, R1        [LOOP START]
  0x0C: LOADI R3, 1
  0x0F: ADD R1, R3        (Count++)
  0x12: CMP R1, R2
  0x15: JLE 0x09          (jump back if Count <= Limit)
  0x18: PRINT R0
  0x1B: HALT

EXECUTION TRACE:

PC=0x00: R0=0
PC=0x03: R1=1
PC=0x06: R2=5
PC=0x09: R0 = R0 + R1 = 0 + 1 = 1
PC=0x0C: R3=1
PC=0x0F: R1 = R1 + R3 = 1 + 1 = 2
PC=0x12: CMP R1, R2 -> compare 2 vs 5 -> flags.le = 1
PC=0x15: JLE 0x09 -> jump! PC = 0x09
[LOOP ITERATION 2]
PC=0x09: R0 = R0 + R1 = 1 + 2 = 3
PC=0x0C: R3=1
PC=0x0F: R1 = R1 + R3 = 2 + 1 = 3
PC=0x12: CMP R1, R2 -> compare 3 vs 5 -> flags.le = 1
PC=0x15: JLE 0x09 -> jump! PC = 0x09
[LOOP ITERATION 3]
PC=0x09: R0 = R0 + R1 = 3 + 3 = 6
PC=0x0C: R3=1
PC=0x0F: R1 = R1 + R3 = 3 + 1 = 4
PC=0x12: CMP R1, R2 -> compare 4 vs 5 -> flags.le = 1
PC=0x15: JLE 0x09 -> jump! PC = 0x09
[LOOP ITERATION 4]
PC=0x09: R0 = R0 + R1 = 6 + 4 = 10
PC=0x0C: R3=1
PC=0x0F: R1 = R1 + R3 = 4 + 1 = 5
PC=0x12: CMP R1, R2 -> compare 5 vs 5 -> flags.le = 1
PC=0x15: JLE 0x09 -> jump! PC = 0x09
[LOOP ITERATION 5]
PC=0x09: R0 = R0 + R1 = 10 + 5 = 15
PC=0x0C: R3=1
PC=0x0F: R1 = R1 + R3 = 5 + 1 = 6
PC=0x12: CMP R1, R2 -> compare 6 vs 5 -> flags.le = 0 (6 > 5)
PC=0x15: JLE 0x09 -> NO JUMP! PC = 0x18 (normal)
PC=0x18: PRINT R0 -> Output "15"
PC=0x1B: HALT

RESULT: Sum = 15 (1+2+3+4+5)
```

## 9. MEMORY LAYOUT DURING EXECUTION

```
ESP32 RAM Layout (320 KB total):

┌──────────────────────────────────────────────┐ 0x00000
│  Other ESP32 System (WiFi, FreeRTOS, etc)    │
├──────────────────────────────────────────────┤
│                                              │
│  Sketch code & global variables (~50 KB)     │
│                                              │
├──────────────────────────────────────────────┤ ~0xC800
│                                              │
│  VM STATE STRUCTURES (~1 KB)                 │
│  - 8 registers: 32 bytes                     │
│  - Flags: 1 byte                             │
│  - PC, SP, HP, FP: 8 bytes                   │
│  - Other state: ~500 bytes                   │
│                                              │
├──────────────────────────────────────────────┤
│                                              │
│  VM STACK (64 KB) - Grows Downward           │
│  [Top] ← SP (starts at 65535)                │
│  - Function frames                           │
│  - Local variables                           │
│  - Return addresses                          │
│  - Function arguments                        │
│  [Bottom - Guard: 4 bytes]                   │
│                                              │
├──────────────────────────────────────────────┤
│  GAP (guard region to prevent collision)     │
├──────────────────────────────────────────────┤
│                                              │
│  VM HEAP (128 KB) - Grows Upward             │
│  [Bottom] ← HP (starts at 0)                 │
│  - Dynamic arrays                            │
│  - String data                               │
│  [Top - Guard: 4 bytes]                      │
│                                              │
├──────────────────────────────────────────────┤
│  Remaining ESP32 memory (~60-80 KB)          │
├──────────────────────────────────────────────┤ 0x50000 (320 KB)
```

## 10. MEMORY SAFETY CONSIDERATIONS

### Stack Overflow Check

```cpp
if (state.sp <= 3) {  // Guard zone
  // Stack collision - error condition
  halted = true;
}
```

### Heap Overflow Check

```cpp
if (state.hp + size >= VM_HEAP_SIZE) {
  // Heap allocation failed
  return -1;
}
```

### Array Bounds Access

```cpp
uint16_t addr = (uint16_t)state.registers[a2].i32;
if (addr < VM_HEAP_SIZE) {
  // Safe access
  value = *(int32_t*)&state.heap[addr];
}
```

## 11. PERFORMANCE CHARACTERISTICS

| Operation   | Cycles | Notes                    |
| ----------- | ------ | ------------------------ |
| LOADI       | 1      | Direct register write    |
| ADD/SUB/MUL | 2      | Include flag computation |
| DIV/MOD     | 10-20  | Expensive on ESP32       |
| LOAD/STORE  | 2      | Memory access            |
| CMP         | 2      | Flag computation         |
| JMP         | 1      | PC assignment            |
| CALL        | 2      | Push + PC jump           |
| RET         | 2      | Pop + PC restore         |
| PRINT       | 5-10   | Serial I/O (slow)        |

**Typical program**: 50-100 instructions execute in ~1-2 ms on ESP32 (160 MHz CPU)

## 12. DEBUGGING & MONITORING

### State Dump

```cpp
vm.dump_state();  // Prints PC, SP, registers, flags
```

Output:

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

### Instruction-by-Instruction Monitoring

```cpp
void step_with_trace() {
  Instruction instr = Instruction::unpack(&state.code[state.pc]);
  Serial.print("PC=0x"); Serial.print(state.pc, HEX);
  Serial.print(" OP="); Serial.println(opcode_name(instr.opcode));
  step();
  dump_registers();
}
```

---

**End of Execution Flow Document**
