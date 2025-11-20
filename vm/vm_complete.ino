#ifndef UNIT_TESTING
#include <Arduino.h>
#endif

// --- VM Configuration ---
#define VM_STACK_SIZE 1024  // 1KB Stack for local variables/expressions
#define VM_HEAP_SIZE  2048  // 2KB Heap for dynamic data
#define NUM_REGISTERS 8     // R0-R7

// --- Opcodes ---
enum Opcode {
    HALT = 0x00,
    LOAD = 0x01,  // LOAD R, Val      : R = Val
    ADD  = 0x02,  // ADD R1, R2       : R1 = R1 + R2
    SUB  = 0x03,  // SUB R1, R2       : R1 = R1 - R2
    MUL  = 0x04,  // MUL R1, R2       : R1 = R1 * R2
    DIV  = 0x05,  // DIV R1, R2       : R1 = R1 / R2
    MOV  = 0x06,  // MOV R1, R2       : R1 = R2
    PRINT= 0x07,  // PRINT R          : Serial.println(R)
    JMP  = 0x08,  // JMP Addr         : PC = Addr
    JZ   = 0x09,  // JZ R, Addr       : If R == 0, PC = Addr
    JNZ  = 0x0A,  // JNZ R, Addr      : If R != 0, PC = Addr
    PUSH = 0x0B,  // PUSH R           : Stack.push(R)
    POP  = 0x0C,  // POP R            : R = Stack.pop()
    STORE= 0x0D,  // STORE R, Addr    : Heap[Addr] = R
    LOADM= 0x0E,  // LOADM R, Addr    : R = Heap[Addr]
    EQ   = 0x0F,  // EQ R1, R2        : R1 = (R1 == R2)
    LT   = 0x10,  // LT R1, R2        : R1 = (R1 < R2)
    GT   = 0x11,  // GT R1, R2        : R1 = (R1 > R2)
    DEBUG= 0xFF   // DEBUG            : Dump registers
};

// --- Instruction Format ---
// 3 Bytes: [OPCODE] [ARG1] [ARG2]
struct Instruction {
    uint8_t opcode;
    uint8_t arg1;
    uint8_t arg2;
};

// --- VM Class ---
class TinyVM {
public:
    int32_t registers[NUM_REGISTERS];
    int32_t stack[VM_STACK_SIZE];
    uint8_t heap[VM_HEAP_SIZE];
    
    uint16_t sp; // Stack Pointer
    uint16_t pc; // Program Counter
    bool running;

    const uint8_t* program;
    size_t programSize;

    TinyVM() {
        reset();
    }

    void reset() {
        for(int i=0; i<NUM_REGISTERS; i++) registers[i] = 0;
        for(int i=0; i<VM_STACK_SIZE; i++) stack[i] = 0;
        for(int i=0; i<VM_HEAP_SIZE; i++) heap[i] = 0;
        sp = 0;
        pc = 0;
        running = false;
        program = nullptr;
        programSize = 0;
    }

    void loadProgram(const uint8_t* code, size_t size) {
        program = code;
        programSize = size;
        pc = 0;
        running = true;
        Serial.println("Program Loaded.");
    }

    void step() {
        if (!running || pc >= programSize) {
            running = false;
            return;
        }

        // Fetch
        if (pc + 3 > programSize) {
            Serial.println("Error: Unexpected end of program");
            running = false;
            return;
        }

        uint8_t op = program[pc];
        uint8_t arg1 = program[pc + 1];
        uint8_t arg2 = program[pc + 2];
        pc += 3;

        // Execute
        switch (op) {
            case HALT:
                Serial.println("HALT encountered.");
                running = false;
                break;
            
            case LOAD: // LOAD R, Val (Val is small 8-bit immediate here for simplicity, or we can combine args)
                // For this simple VM, let's assume ARG2 is an immediate value 0-255
                if (arg1 < NUM_REGISTERS) {
                    registers[arg1] = arg2;
                }
                break;

            case ADD:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] += registers[arg2];
                }
                break;

            case SUB:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] -= registers[arg2];
                }
                break;
            
            case MUL:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] *= registers[arg2];
                }
                break;

            case DIV:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    if (registers[arg2] != 0)
                        registers[arg1] /= registers[arg2];
                    else
                        Serial.println("Error: Division by Zero");
                }
                break;

            case MOV:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] = registers[arg2];
                }
                break;

            case PRINT:
                if (arg1 < NUM_REGISTERS) {
                    Serial.print("OUT: ");
                    Serial.println(registers[arg1]);
                }
                break;

            case JMP:
                // JMP Addr (16-bit address from arg1 and arg2)
                pc = (arg1 << 8) | arg2;
                break;

            case JZ:
                // JZ R, Addr (8-bit address in arg2)
                if (arg1 < NUM_REGISTERS && registers[arg1] == 0) {
                    pc = arg2;
                }
                break;

            case JNZ:
                // JNZ R, Addr (8-bit address in arg2)
                if (arg1 < NUM_REGISTERS && registers[arg1] != 0) {
                    pc = arg2;
                }
                break;

            case PUSH:
                if (arg1 < NUM_REGISTERS) {
                    if (sp < VM_STACK_SIZE) {
                        stack[sp++] = registers[arg1];
                    } else {
                        Serial.println("Error: Stack Overflow");
                        running = false;
                    }
                }
                break;

            case POP:
                if (arg1 < NUM_REGISTERS) {
                    if (sp > 0) {
                        registers[arg1] = stack[--sp];
                    } else {
                        Serial.println("Error: Stack Underflow");
                        running = false;
                    }
                }
                break;

            case STORE:
                // STORE R, Addr (Addr is arg2, 8-bit immediate)
                if (arg1 < NUM_REGISTERS) {
                    if (arg2 < VM_HEAP_SIZE) {
                        heap[arg2] = (uint8_t)registers[arg1];
                    }
                }
                break;

            case LOADM:
                // LOADM R, Addr (Addr is arg2, 8-bit immediate)
                if (arg1 < NUM_REGISTERS) {
                    if (arg2 < VM_HEAP_SIZE) {
                        registers[arg1] = heap[arg2];
                    }
                }
                break;

            case EQ:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] = (registers[arg1] == registers[arg2]) ? 1 : 0;
                }
                break;

            case LT:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] = (registers[arg1] < registers[arg2]) ? 1 : 0;
                }
                break;

            case GT:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] = (registers[arg1] > registers[arg2]) ? 1 : 0;
                }
                break;

            case DEBUG:
                dumpRegisters();
                break;

            default:
                Serial.print("Unknown Opcode: ");
                Serial.println(op, HEX);
                running = false;
                break;
        }
    }

    void run() {
        while (running) {
            step();
        }
    }

    void dumpRegisters() {
        Serial.println("--- Registers ---");
        for (int i = 0; i < NUM_REGISTERS; i++) {
            Serial.print("R"); Serial.print(i); Serial.print(": ");
            Serial.println(registers[i]);
        }
        Serial.println("-----------------");
    }
};

// --- Example Program ---
// Calculates 5 + 10 and prints the result
const uint8_t exampleProgram[] = {
    LOAD, 0, 5,    // R0 = 5
    LOAD, 1, 10,   // R1 = 10
    ADD,  0, 1,    // R0 = R0 + R1
    PRINT, 0, 0,   // Print R0
    HALT, 0, 0     // Stop
};

TinyVM vm;

#ifndef UNIT_TESTING
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("TinyVM Starting...");

    vm.loadProgram(exampleProgram, sizeof(exampleProgram));
    vm.run();
}

void loop() {
    // Nothing to do here
    delay(1000);
}
#endif
