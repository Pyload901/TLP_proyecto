# VM Architecture - Visual Diagrams

## 1. INSTRUCTION EXECUTION CYCLE

```
                    ┌────────────────────┐
                    │  START EXECUTION   │
                    └─────────┬──────────┘
                              │
                    ┌─────────▼──────────┐
                    │  Check Halted?     │
                    │  PC >= code_size?  │
                    └┬────────────────┬──┘
                     │ NO             │ YES
                     │                │
                     ▼                ▼
            ┌────────────────┐   ┌──────────┐
            │  FETCH (PC+=3) │   │  HALT    │
            └────────┬───────┘   └──────────┘
                     │
            ┌────────▼──────────┐
            │  DECODE (opcode,  │
            │   arg1, arg2)     │
            └────────┬──────────┘
                     │
            ┌────────▼──────────────────────────┐
            │  EXECUTE                          │
            │  └─ Arithmetic: ADD, SUB, etc.    │
            │  └─ Memory: LOAD, STORE, ALLOC    │
            │  └─ Control: JMP, CALL, RET       │
            │  └─ I/O: PRINT, READ              │
            └────────┬──────────────────────────┘
                     │
            ┌────────▼────────────┐
            │ Update State:       │
            │ - PC (already done) │
            │ - Registers         │
            │ - Flags             │
            │ - Memory            │
            └────────┬────────────┘
                     │
                     └──────────────┐
                                    │ Loop
                                    │
                                    └──► Back to Start
```

---

## 2. MEMORY LAYOUT (ESP32)

```
  ADDRESS    CONTENT              SIZE
  ─────────────────────────────────────────────────
  0x00000    System/WiFi/RTOS     ~80 KB
             (managed by Arduino)

  ~0x14000   Sketch Globals       ~50 KB
             (variables, etc.)

  ~0x2E000   VM STATE             1 KB
             ┌─────────────────┐
             │ Registers       │  32 B (8 × 4B)
             │ Flags           │  1 B
             │ PC, SP, HP, FP  │  8 B
             │ Other state     │  ~950 B
             └─────────────────┘

  ~0x2E400   STACK (64 KB)        64 KB
             ↓ Grows downward
             ┌─────────────────┐
             │ Frame N         │
             │ ─ Return Addr   │
             │ ─ Locals        │
             ├─────────────────┤
             │ ...             │
             ├─────────────────┤
             │ Frame 1         │
             │ ─ Arg 2         │
             │ ─ Arg 1         │
             ├─────────────────┤
             │ Guard (4 B)     │  ← SP starts at 0xFFFF
             └─────────────────┘

  ~0x3E400   HEAP (128 KB)        128 KB
             ↑ Grows upward
             ┌─────────────────┐
             │ Array 1         │  ← HP starts at 0x00
             ├─────────────────┤
             │ Array 2         │
             ├─────────────────┤
             │ ...             │
             ├─────────────────┤
             │ Free space      │
             ├─────────────────┤
             │ Guard (4 B)     │
             └─────────────────┘

  0x50000    End of SRAM          320 KB total
```

---

## 3. REGISTER FILE

```
  ┌─────────────────────────────┐
  │  REGISTERS (R0-R7)          │
  │  Each 32-bit signed int     │
  ├──────────┬──────────────────┤
  │ R0       │ Accumulator      │  ← Arithmetic results
  │ R1-R5    │ General Purpose  │  ← Local variables
  │ R6       │ Return Value     │  ← Function results
  │ R7       │ Frame Pointer    │  ← Reserved
  └──────────┴──────────────────┘

  Range: -2,147,483,648 to 2,147,483,647
```

---

## 4. FLAGS REGISTER

```
  FLAGS (8-bit)
  ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
  │  7  │  6  │  5  │  4  │  3  │  2  │  1  │  0  │
  ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
  │ le  │ ge  │ lt  │ gt  │ neq │ eq  │zero │carry│
  └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
   Set by CMP instruction
   zero/carry: set by arithmetic
```

---

## 5. INSTRUCTION FORMAT

```
  Memory Layout:
  ┌────────────┬────────────┬────────────┐
  │  Byte 0    │  Byte 1    │  Byte 2    │
  ├────────────┼────────────┼────────────┤
  │ OPCODE     │ ARG1       │ ARG2       │
  │ (8 bits)   │ (8 bits)   │ (8 bits)   │
  └────────────┴────────────┴────────────┘

  Example: ADD R0, R1
  ┌────────────┬────────────┬────────────┐
  │ 0x10 (ADD) │ 0x00 (R0)  │ 0x01 (R1)  │
  └────────────┴────────────┴────────────┘

  Total: 3 bytes per instruction
```

---

## 6. STACK FRAME LAYOUT

```
  Function Call Stack

  Caller:          Callee:
                   ┌─────────────────┐
                   │ (Other data)    │
  PUSH Arg2   ─┐   ├─────────────────┤
  PUSH Arg1   ─┼──→│ Arg N ...       │
  CALL func   ─┤   │ Arg 2           │
              ─┤   │ Arg 1           │ ◄─ SP + offset
              ─┤   │ Return Addr     │
              ─┘   ├─────────────────┤
                   │ Frame Ptr (opt) │
                   ├─────────────────┤
                   │ Local 1         │
                   │ Local 2         │
                   │ ...             │  ◄─ R7 (FP)
                   ├─────────────────┤
                   │ (grows down)    │  ◄─ SP
                   └─────────────────┘
```

---

## 7. FUNCTION CALL SEQUENCE (TIME DIAGRAM)

```
  Caller Code              Action              Callee Code
  ─────────────────────────────────────────────────────────
  LOADI R0, 5
  PUSH R0                  │ Push arg1
                           │
  LOADI R0, 3
  PUSH R0                  │ Push arg2
                           │
  CALL func_addr           │ Save PC
                           ▼
                      EXECUTE func
                           │
                           │
                      POP R0        ← Retrieve arg1
                      POP R1        ← Retrieve arg2
                      ADD R0, R1
                      LOAD R6, R0   ← Save result
                      RET           │ Restore PC
                           │
                           ▼
  (Next instruction) ◄─────┘ Continue
  PRINT R6
```

---

## 8. CONDITIONAL JUMP DECISION TREE

```
  CMP R0, R1  (Compare R0 vs R1)
       │
       ├─── Check: R0 == R1?
       │    ├─ YES → eq=1, neq=0, zero=1
       │    └─ NO  → eq=0, neq=1, zero=0
       │
       ├─── Check: R0 < R1?
       │    ├─ YES → lt=1, ge=0
       │    └─ NO  → lt=0, ge=1
       │
       ├─── Check: R0 > R1?
       │    ├─ YES → gt=1, le=0
       │    └─ NO  → gt=0, le=1
       │
       └─── Check: R0 >= R1?
            ├─ YES → ge=1, lt=0
            └─ NO  → ge=0, lt=1

  Then use flags for:
  JEQ   (jump if eq=1)
  JNEQ  (jump if neq=1)
  JLT   (jump if lt=1)
  JGT   (jump if gt=1)
  JLE   (jump if le=1)
  JGE   (jump if ge=1)
  JZ    (jump if zero=1)
  JNZ   (jump if zero=0)
```

---

## 9. OPCODE CATEGORIES

```
┌─────────────────────────────────────────────────┐
│         OPCODE DISTRIBUTION (50 total)          │
├─────────────────────────────────────────────────┤
│                                                 │
│  Arithmetic (11):  │████████████                │
│  ├─ ADD, SUB, MUL  │                            │
│  ├─ DIV, MOD       │                            │
│  ├─ AND, OR, XOR   │                            │
│  ├─ NOT, SHL, SHR  │                            │
│                                                 │
│  Memory (8):       │██████                      │
│  ├─ LOAD, LOADI    │                            │
│  ├─ LOADI16        │                            │
│  ├─ STORE, LOAD_MEM│                            │
│  ├─ LOAD_ADDR      │                            │
│  ├─ ALLOC, FREE    │                            │
│                                                 │
│  Stack (3):        │███                         │
│  ├─ PUSH, POP, PEEK│                            │
│                                                 │
│  Control (9):      │██████████                  │
│  ├─ JMP, JZ, JNZ   │                            │
│  ├─ JLT, JGT, etc  │                            │
│                                                 │
│  Functions (3):    │███                         │
│  ├─ CALL, RET, NOP │                            │
│                                                 │
│  I/O (5):          │█████                       │
│  ├─ PRINT, PRINTC  │                            │
│  ├─ READ, TRAP     │                            │
│  ├─ DEBUG          │                            │
│                                                 │
│  Special (1):      │█                           │
│  ├─ HALT           │                            │
│  ├─ CMP            │                            │
│                                                 │
└─────────────────────────────────────────────────┘
```

---

## 10. EXECUTION TIMELINE (Example: 5+3)

```
  Bytecode:  [0x21, 0x00, 0x05] [0x21, 0x01, 0x03] [0x10, 0x00, 0x01] [0x60, 0x00, 0x00] [0x01, 0x00, 0x00]

  ┌────────────────────────────────────────────────────────────────────────────────┐
  │ TIMELINE                                                                       │
  ├────────────────────────────────────────────────────────────────────────────────┤
  │                                                                                │
  │ T=0: PC=0                                                                     │
  │      FETCH [0x21, 0x00, 0x05]                                                │
  │      DECODE: opcode=LOADI, arg1=0, arg2=5                                    │
  │      EXECUTE: R0 = 5                                                         │
  │      PC = 3                                                                  │
  │                                                                                │
  │ T=1: PC=3                                                                     │
  │      FETCH [0x21, 0x01, 0x03]                                                │
  │      DECODE: opcode=LOADI, arg1=1, arg2=3                                    │
  │      EXECUTE: R1 = 3                                                         │
  │      PC = 6                                                                  │
  │                                                                                │
  │ T=2: PC=6                                                                     │
  │      FETCH [0x10, 0x00, 0x01]                                                │
  │      DECODE: opcode=ADD, arg1=0, arg2=1                                      │
  │      EXECUTE: R0 = R0 + R1 = 5 + 3 = 8                                       │
  │      FLAGS: zero=0, carry=0                                                  │
  │      PC = 9                                                                  │
  │                                                                                │
  │ T=3: PC=9                                                                     │
  │      FETCH [0x60, 0x00, 0x00]                                                │
  │      DECODE: opcode=PRINT, arg1=0                                            │
  │      EXECUTE: Serial.println(8)                                              │
  │      OUTPUT: 8                                                               │
  │      PC = 12                                                                 │
  │                                                                                │
  │ T=4: PC=12                                                                    │
  │      FETCH [0x01, 0x00, 0x00]                                                │
  │      DECODE: opcode=HALT                                                     │
  │      EXECUTE: halted = true                                                  │
  │      STOP                                                                    │
  │                                                                                │
  └────────────────────────────────────────────────────────────────────────────────┘

  Total cycles: 5
  Total time: ~0.5 µs (excluding PRINT which takes ~100 µs)
```

---

## 11. ARITHMETIC OPERATION: ADD R0, R1

```
  Before:                 Operation:              After:
  ┌─────────────┐         ┌──────────────┐        ┌─────────────┐
  │ R0 = 5      │         │ ADD R0, R1   │        │ R0 = 8      │
  │ R1 = 3      │    →    │ v1 = R0 = 5  │   →    │ R1 = 3      │
  │ R2..R7 = ?  │         │ v2 = R1 = 3  │        │ R2..R7 = ?  │
  │ FLAGS = 0   │         │ v1 + v2 = 8  │        │ FLAGS = 0   │
  └─────────────┘         │ No overflow  │        │ zero=0      │
                          │ R0 = 8       │        │ carry=0     │
                          └──────────────┘        └─────────────┘

  Flags updated:
  ├─ zero = (8 == 0) ? 1 : 0  → 0
  └─ carry = (overflow) ? 1 : 0  → 0
```

---

## 12. ALLOCATION STRATEGY

```
  Variable Allocation Decision Tree:

  Declare variable
         │
         ├─ Symbol count < 6?
         │  ├─ YES: Use REGISTER
         │  │   ├─ R0 for 1st local
         │  │   ├─ R1 for 2nd local
         │  │   ├─ R2 for 3rd local
         │  │   ├─ R3 for 4th local
         │  │   ├─ R4 for 5th local
         │  │   └─ R5 for 6th local
         │  │
         │  └─ NO: Use STACK
         │      ├─ FP - 4 for 7th
         │      ├─ FP - 8 for 8th
         │      └─ FP - 12 for 9th...
         │
         └─ (R6 reserved, R7=FP)
```

---

## 13. COMPILATION PIPELINE

```
  ┌──────────────┐
  │ Source Code  │
  └──────┬───────┘
         │ Token stream
         ▼
  ┌──────────────┐
  │   LEXER      │ (flex)
  └──────┬───────┘
         │ Token stream
         ▼
  ┌──────────────┐
  │   PARSER     │ (yacc)
  └──────┬───────┘
         │ AST
         ▼
  ┌──────────────┐
  │  SEMANTIC    │ Symbol table, type checking
  │  ANALYSIS    │
  └──────┬───────┘
         │ Annotated AST
         ▼
  ┌──────────────────────────┐
  │  CODE GENERATION (NEW)   │ AST → Instructions
  └──────┬───────────────────┘
         │ Bytecode stream
         ▼
  ┌──────────────┐
  │  EMITTER     │ Pack 3-byte instructions
  └──────┬───────┘
         │ Binary bytecode
         ▼
  ┌──────────────┐
  │ Bytecode     │
  │ File (*.vm)  │ Save to SD card
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │ ESP32 Loads  │
  │ from SD      │
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │ VM Executes  │
  └──────────────┘
```

---

## 14. HEAP ALLOCATION (LIFO)

```
  Initial:
  ┌─────────────┐
  │ HP = 0      │ (Heap Pointer)
  │ alloc_depth=0│
  └─────────────┘

  After ALLOC 16:
  ┌─────────────┐
  │ HP = 16     │ ← updated
  │ alloc_depth=1│
  │ alloc_stack:│
  │ [0]=0       │ ← saves original HP
  └─────────────┘
  Heap: [16 bytes allocated]

  After ALLOC 32:
  ┌─────────────┐
  │ HP = 48     │ ← updated
  │ alloc_depth=2│
  │ alloc_stack:│
  │ [0]=0       │
  │ [1]=16      │ ← saves prev HP
  └─────────────┘
  Heap: [16 bytes][32 bytes allocated]

  After FREE:
  ┌─────────────┐
  │ HP = 16     │ ← restored from stack[1]
  │ alloc_depth=1│
  │ alloc_stack:│
  │ [0]=0       │
  └─────────────┘
  Heap: [16 bytes] [32 bytes freed]

  After FREE (again):
  ┌─────────────┐
  │ HP = 0      │ ← restored from stack[0]
  │ alloc_depth=0│
  └─────────────┘
  Heap: [ALL freed]
```

---

## 15. LOOP EXECUTION FLOW

```
  Code:
    i = 0; sum = 0;
    loop: if (i >= 5) goto end;
          sum = sum + i;
          i = i + 1;
          goto loop;
    end: print sum;

  Bytecode:
    0x00: LOADI R0, 0     (i = 0)
    0x03: LOADI R1, 0     (sum = 0)
    0x06: LOADI R2, 5     (limit = 5)
    0x09: [LOOP START]
    0x09: CMP R0, R2
    0x0C: JGE 0x1A        (i >= 5? goto end)
    0x0F: ADD R1, R0      (sum = sum + i)
    0x12: LOADI R3, 1
    0x15: ADD R0, R3      (i = i + 1)
    0x18: JMP 0x09        (goto loop)
    0x1B: [END]
    0x1B: PRINT R1
    0x1E: HALT

  Execution:
  ┌────────────────────────────────────────┐
  │ PC=0x00: i=0                           │
  │ PC=0x03: sum=0                         │
  │ PC=0x06: limit=5                       │
  ├────────────────────────────────────────┤
  │ [ITERATION 1]                          │
  │ PC=0x09: CMP 0, 5 → lt flag           │
  │ PC=0x0C: JGE (no jump, i < 5)         │
  │ PC=0x0F: sum = 0 + 0 = 0              │
  │ PC=0x12: r3 = 1                       │
  │ PC=0x15: i = 0 + 1 = 1                │
  │ PC=0x18: JMP 0x09                     │
  ├────────────────────────────────────────┤
  │ [ITERATION 2]                          │
  │ PC=0x09: CMP 1, 5 → lt flag           │
  │ PC=0x0C: JGE (no jump)                │
  │ PC=0x0F: sum = 0 + 1 = 1              │
  │ PC=0x15: i = 1 + 1 = 2                │
  │ PC=0x18: JMP 0x09                     │
  ├────────────────────────────────────────┤
  │ ... (iterations 3, 4, 5)              │
  ├────────────────────────────────────────┤
  │ [AFTER ITERATION 5]                    │
  │ PC=0x09: CMP 5, 5 → eq flag           │
  │ PC=0x0C: JGE (JUMP to 0x1B, i >= 5)  │
  │ PC=0x1B: PRINT 10 (sum = 0+1+2+3+4)  │
  │ OUTPUT: 10                             │
  │ PC=0x1E: HALT                          │
  └────────────────────────────────────────┘
```

---

**End of Visual Diagrams**
