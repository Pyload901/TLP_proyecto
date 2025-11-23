#ifndef UNIT_TESTING
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#endif

// --- SD Card Configuration ---
#define CS_PIN 5
#define VMCODE_FILE "/program.vmcode"

// --- VM Configuration ---
#define VM_STACK_SIZE 1024  // 1KB Stack for local variables/expressions
#define VM_HEAP_SIZE  2048  // 2KB Heap for dynamic data
#define NUM_REGISTERS 8     // R0-R7

// --- Opcodes ---
enum Opcode {
    NOP   = 0x00,
    ADD   = 0x01, SUB   = 0x02, MUL   = 0x03, DIV   = 0x04, MOD   = 0x05,
    AND   = 0x06, OR    = 0x07, XOR   = 0x08, NOT   = 0x09, CMP   = 0x0A,
    SHL   = 0x0B, SHR   = 0x0C,
    LOAD   = 0x10, LOADI  = 0x11, LOADI16= 0x12, STORE  = 0x13,
    LOAD_ADDR = 0x14, PUSH   = 0x15, POP    = 0x16, PEEK   = 0x17, LOADM  = 0x18,
    JMP   = 0x20, JZ    = 0x21, JNZ   = 0x22, JLT   = 0x23, JGT   = 0x24,
    JLE   = 0x25, JGE   = 0x26, CALL  = 0x27, RET   = 0x28, HALT  = 0x29,
    PRINT = 0x30, TRAP  = 0x31
};

// --- Instruction Format ---
struct Instruction {
    uint8_t opcode;
    uint8_t arg1;
    uint8_t arg2;
};

// Flags set by CMP
struct Flags {
    bool zero = false;
    bool lt = false;
    bool gt = false;
    bool le = false;
    bool ge = false;
};

// =========================
// === BUILTIN / PIN MAP ===
// =========================

static const int LOGICAL_PIN_COUNT = 12;
static const int pin_map[LOGICAL_PIN_COUNT] = {
    2, 4, 5, 18, 19, 21, 32, 33, 34, 35, 25, 26
};

static inline int resolve_pin(int logical_or_physical) {
    if (LOGICAL_PIN_COUNT > 0 && logical_or_physical >= 0 && logical_or_physical < LOGICAL_PIN_COUNT) {
        return pin_map[logical_or_physical];
    }
    return logical_or_physical;
}

// Builtin TRAP IDs
enum BuiltinID_IO {
    B_DIGITAL_READ  = 40, B_DIGITAL_WRITE = 41, B_ANALOG_READ   = 42,
    B_PWM_WRITE     = 44, B_PIN_MODE      = 45,
    B_FORWARD       = 50, B_BACK          = 51, B_TURN_LEFT     = 52,
    B_TURN_RIGHT    = 53, B_SET_SPEED     = 54, B_STOP          = 55,
    B_READ_IR_LEFT  = 60, B_READ_IR_RIGHT = 61
};

// =========================
// === GLOBAL VARIABLES ===
// =========================

// --- Motor pin declarations ---
const int L_IN1 = 26;
const int L_IN2 = 27;
const int L_ENA = 25;
const int R_IN3 = 33;
const int R_IN4 = 32;
const int R_ENB = 14;
const int SAFETY_DELAY = 500;
int speed_global = 255;

// --- IR Sensors configuration ---
const int sensorIzqPin = 34;
const int sensorDerPin = 35;
int lecturaSensorIzq = 0;
int lecturaSensorDer = 0;
int umbralIzq = 1500;
int umbralDer = 1500;

// Global program storage
uint8_t programBuffer[2048];
size_t programSize = 0;

// =========================
// === FUNCTION IMPLEMENTATIONS ===
// =========================

#ifndef UNIT_TESTING

void pwm_write_pin(int pin, int pwmValue) {
    analogWrite(pin, pwmValue);
}

void setMotor(int side, int pwr) {
    pwr = constrain(pwr, -100, 100);
    
    // Definir pines según el lado (0 = Izquierda, 1 = Derecha)
    int pin1 = (side == 0) ? L_IN1 : R_IN3;
    int pin2 = (side == 0) ? L_IN2 : R_IN4;
    int pinPWM = (side == 0) ? L_ENA : R_ENB;

    int pwmValue = abs(pwr) * 255 / 100;

    if (pwr > 0) {
        // Hacia adelante
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
    } else if (pwr < 0) {
        // Hacia atrás
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
    } else {
        // Detener
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
        pwmValue = 0;
    }
    
    analogWrite(pinPWM, pwmValue);
}

void stopMotors() {
    setMotor(0, 0);
    setMotor(1, 0);
}

void forward_ms(int ms) {
    setMotor(0, speed_global);
    setMotor(1, speed_global);
    if (ms > 0) {
        delay(ms);
        stopMotors();
    }
}

void back_ms(int ms) {
    setMotor(0, -speed_global);
    setMotor(1, -speed_global);
    if (ms > 0) {
        delay(ms);
        stopMotors();
    }
}

void turnLeft_ms(int ms) {
    setMotor(0, -speed_global);
    setMotor(1, speed_global);
    if (ms > 0) {
        delay(ms);
        stopMotors();
    }
}

void turnRight_ms(int ms) {
    setMotor(0, speed_global);
    setMotor(1, -speed_global);
    if (ms > 0) {
        delay(ms);
        stopMotors();
    }
}

void set_speed(int s) {
    speed_global = constrain(s, 0, 255);
}

void initSensors() {
    // Set motor control pins as outputs
    pinMode(L_IN1, OUTPUT);
    pinMode(L_IN2, OUTPUT);
    pinMode(R_IN3, OUTPUT);
    pinMode(R_IN4, OUTPUT);
    
    // Set ADC pins as input
    pinMode(sensorIzqPin, INPUT);
    pinMode(sensorDerPin, INPUT);
    
    // Setup PWM channels for ESP32 if needed (using ledc)
}
#endif

// =========================
// === VM CLASS ===
// =========================

class TinyVM {
public:
    int32_t registers[NUM_REGISTERS];
    int32_t stack[VM_STACK_SIZE];
    uint8_t heap[VM_HEAP_SIZE];
    uint16_t sp, pc;
    bool running;
    const uint8_t* program;
    size_t programSize;
    Flags flags;
    size_t heap_top;
    int loop_start_pc;

    TinyVM() { reset(); }

    void reset() {
        for(int i=0; i<NUM_REGISTERS; i++) registers[i] = 0;
        for(int i=0; i<VM_STACK_SIZE; i++) stack[i] = 0;
        for(int i=0; i<VM_HEAP_SIZE; i++) heap[i] = 0;
        sp = 0; pc = 0; running = false; program = nullptr; programSize = 0;
        flags = Flags(); heap_top = 0;
        loop_start_pc = -1;
    }

    void loadProgram(const uint8_t* code, size_t size) {
        program = code; programSize = size; pc = 0; running = true;
        Serial.println("Program Loaded.");
    }

    void setLoopStart(size_t addr) {
        loop_start_pc = (int)addr;
    }

    void call_trap(uint8_t id) {
        switch (id) {
            case B_DIGITAL_READ: {
                int pin = resolve_pin((int)registers[0]);
                registers[0] = digitalRead(pin);
                break;
            }
            case B_DIGITAL_WRITE: {
                int pin = resolve_pin((int)registers[0]);
                int val = (int)registers[1];
                digitalWrite(pin, val ? HIGH : LOW);
                break;
            }
            case B_ANALOG_READ: {
                int pin = resolve_pin((int)registers[0]);
                registers[0] = analogRead(pin);
                break;
            }
            case B_PWM_WRITE: {
                int pin = resolve_pin((int)registers[0]);
                int pwm = (int)registers[1];
                pwm_write_pin(pin, pwm);
                break;
            }
            case B_PIN_MODE: {
                int pin = resolve_pin((int)registers[0]);
                int mode = (int)registers[1];
                pinMode(pin, mode ? OUTPUT : INPUT);
                break;
            }
            case B_FORWARD: {
                int ms = (int)registers[0];
                forward_ms(ms);
                break;
            }
            case B_BACK: {
                int ms = (int)registers[0];
                back_ms(ms);
                break;
            }
            case B_TURN_LEFT: {
                int ms = (int)registers[0];
                turnLeft_ms(ms);
                break;
            }
            case B_TURN_RIGHT: {
                int ms = (int)registers[0];
                turnRight_ms(ms);
                break;
            }
            case B_SET_SPEED: {
                int s = (int)registers[0];
                set_speed(s);
                break;
            }
            case B_STOP: {
                stopMotors();
                break;
            }
            case B_READ_IR_LEFT: {
                lecturaSensorIzq = analogRead(sensorIzqPin);
                bool result = lecturaSensorIzq < 1500 ? 1 : 0;
                registers[0] = result;
                break;
            }
            case B_READ_IR_RIGHT: {
                lecturaSensorDer = analogRead(sensorDerPin);
                bool result = lecturaSensorDer < 1500 ? 1 : 0;
                registers[0] = result;
                break;
            }
            default:
                Serial.print("Unknown TRAP id: "); Serial.println(id);
                break;
        }
    }

    void step() {
        if (!running || pc >= programSize) {
            running = false;
            return;
        }

        if (pc + 3 > programSize) {
            Serial.println("Error: Unexpected end of program");
            running = false;
            return;
        }

        uint8_t op = program[pc];
        uint8_t arg1 = program[pc + 1];
        uint8_t arg2 = program[pc + 2];
        pc += 3;

        switch (op) {
            case NOP: break;
            case ADD:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] + registers[arg2];
                }
                break;
            case SUB:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] - registers[arg2];
                }
                break;
            case MUL:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] * registers[arg2];
                }
                break;
            case DIV:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    if (registers[arg2] != 0)
                        registers[0] = registers[arg1] / registers[arg2];
                    else
                        registers[0] = 0;
                }
                break;
            case MOD:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    if (registers[arg2] != 0)
                        registers[0] = registers[arg1] % registers[arg2];
                    else
                        registers[0] = 0;
                }
                break;
            case AND:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] & registers[arg2];
                }
                break;
            case OR:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] | registers[arg2];
                }
                break;
            case XOR:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] ^ registers[arg2];
                }
                break;
            case NOT:
                if (arg1 < NUM_REGISTERS) {
                    registers[0] = ~registers[arg1];
                }
                break;
            case CMP:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    int32_t a = registers[arg1];
                    int32_t b = registers[arg2];
                    flags.zero = (a == b);
                    flags.lt   = (a < b);
                    flags.gt   = (a > b);
                    flags.le   = (a <= b);
                    flags.ge   = (a >= b);
                }
                break;
            case SHL:
                if (arg1 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] << (arg2 & 31);
                }
                break;
            case SHR:
                if (arg1 < NUM_REGISTERS) {
                    registers[0] = registers[arg1] >> (arg2 & 31);
                }
                break;
            case LOAD:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    registers[arg1] = registers[arg2];
                }
                break;
            case LOADI:
                if (arg1 < NUM_REGISTERS) {
                    registers[arg1] = (int32_t)arg2;
                }
                break;
            case LOADI16:
                if (arg1 < NUM_REGISTERS) {
                    if (pc + 2 <= programSize) {
                        uint16_t word = program[pc] | (program[pc+1] << 8);
                        pc += 2;
                        registers[arg1] = (int32_t)word;
                    } else {
                        Serial.println("Error: LOADI16 requires 2 more bytes");
                        running = false;
                    }
                }
                break;
            case STORE:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    int idx = registers[arg1];
                    if (idx >= 0 && idx < (int)VM_HEAP_SIZE) {
                        heap[idx] = (uint8_t)registers[arg2];
                    }
                }
                break;
            case LOAD_ADDR:
                if (arg1 < NUM_REGISTERS) {
                    registers[arg1] = (int32_t)(heap_top + arg2);
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
            case PEEK:
                if (arg1 < NUM_REGISTERS) {
                    uint16_t idx = sp + arg2;
                    if (idx < VM_STACK_SIZE) {
                        registers[arg1] = stack[idx];
                    } else {
                        Serial.println("Error: PEEK out of bounds");
                        running = false;
                    }
                }
                break;
            case LOADM:
                if (arg1 < NUM_REGISTERS && arg2 < NUM_REGISTERS) {
                    int idx = registers[arg2];
                    if (idx >= 0 && idx < (int)VM_HEAP_SIZE) {
                        registers[arg1] = heap[idx];
                    } else {
                        Serial.println("Error: LOADM out of bounds");
                        running = false;
                    }
                }
                break;
            case JMP:
                pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                break;
            case JZ:
                if (flags.zero) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case JNZ:
                if (!flags.zero) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case JLT:
                if (flags.lt) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case JGT:
                if (flags.gt) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case JLE:
                if (flags.le) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case JGE:
                if (flags.ge) {
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                }
                break;
            case CALL:
                if (sp + 2 < VM_STACK_SIZE) {
                    uint16_t ret = pc;
                    stack[sp++] = ret & 0xFFFF;
                    stack[sp++] = (ret >> 16);
                    pc = ((uint16_t)arg1) | ((uint16_t)arg2 << 8);
                } else {
                    Serial.println("Error: Stack overflow on CALL");
                    running = false;
                }
                break;
            case RET:
                // pop return address
                if (sp >= 2) {
                    sp -= 2;
                    uint32_t low = (uint32_t)stack[sp];
                    uint32_t high = (uint32_t)stack[sp+1];
                    uint32_t ret = (high << 16) | (low & 0xFFFF);
                    pc = (uint16_t)ret;
                } else {
                    // If stack is empty, we assume we returned from the main loop function
                    // Clean exit from the loop iteration
                    running = false;
                }
                break;
            case HALT:
                running = false;
                Serial.println("HALT encountered.");
                break;
            case PRINT:
                if (arg1 < NUM_REGISTERS) {
                    Serial.println(registers[arg1]);
                }
                break;
            case TRAP:
                call_trap(arg1);
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

    // Execute one iteration of the user's loop function
    void runLoop() {
        if (loop_start_pc == -1) return;
        
        pc = (uint16_t)loop_start_pc;
        sp = 0; // Reset stack for new iteration
        running = true;
        
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

TinyVM vm;

#ifndef UNIT_TESTING

bool initializeSD() {
    Serial.println("\n--- INICIALIZANDO SD CARD ---");
    SPI.begin(18, 19, 23, 5);
    if (!SD.begin(CS_PIN)) {
        Serial.println("FALLO: No se detecta la tarjeta.");
        return false;
    }
    Serial.println("ÉXITO: Tarjeta SD detectada.");
    return true;
}

struct OpcodeMapping {
    const char* name;
    uint8_t opcode;
};

const OpcodeMapping opcodeMap[] = {
    {"NOP", 0x00}, {"ADD", 0x01}, {"SUB", 0x02}, {"MUL", 0x03}, {"DIV", 0x04}, {"MOD", 0x05},
    {"AND", 0x06}, {"OR", 0x07}, {"XOR", 0x08}, {"NOT", 0x09}, {"CMP", 0x0A},
    {"SHL", 0x0B}, {"SHR", 0x0C}, {"LOAD", 0x10}, {"LOADI", 0x11}, {"LOADI16", 0x12}, {"STORE", 0x13},
    {"LOAD_ADDR", 0x14}, {"PUSH", 0x15}, {"POP", 0x16}, {"PEEK", 0x17}, {"LOADM", 0x18},
    {"JMP", 0x20}, {"JZ", 0x21}, {"JNZ", 0x22}, {"JLT", 0x23}, {"JGT", 0x24},
    {"JLE", 0x25}, {"JGE", 0x26}, {"CALL", 0x27}, {"RET", 0x28}, {"HALT", 0x29},
    {"PRINT", 0x30}, {"TRAP", 0x31}
};

const int OPCODE_COUNT = sizeof(opcodeMap) / sizeof(OpcodeMapping);

uint8_t findOpcode(const char* name) {
    for (int i = 0; i < OPCODE_COUNT; i++) {
        if (strcmp(opcodeMap[i].name, name) == 0) {
            return opcodeMap[i].opcode;
        }
    }
    return 0xFF;
}

bool loadProgramFromSD() {
    Serial.println("--- CARGANDO PROGRAMA DESDE SD ---");
    
    File file = SD.open(VMCODE_FILE);
    if (!file) {
        Serial.print("ERROR: No se puede abrir el archivo ");
        Serial.println(VMCODE_FILE);
        return false;
    }
    
    Serial.println("Archivo encontrado, parseando instrucciones...");
    
    programSize = 0;
    char line[128];
    int lineNum = 0;
    
    while (file.available() && programSize < sizeof(programBuffer) - 3) {
        int pos = 0;
        while (file.available() && pos < sizeof(line) - 1) {
            char c = file.read();
            if (c == '\n' || c == '\r') break;
            line[pos++] = c;
        }
        line[pos] = '\0';
        lineNum++;
        
        // Check for loop label marker (supports "# .loop" or "# FUNCTION loop")
        if (strstr(line, "# .loop") != NULL || strstr(line, "# FUNCTION loop") != NULL) {
            vm.setLoopStart(programSize);
            continue;
        }

        if (pos == 0 || line[0] == '#') continue;
        
        char opcode_str[16];
        int arg1, arg2;
        
        if (sscanf(line, "%s %d %d", opcode_str, &arg1, &arg2) == 3) {
            uint8_t opcode = findOpcode(opcode_str);
            if (opcode != 0xFF) {
                programBuffer[programSize++] = opcode;
                programBuffer[programSize++] = (uint8_t)arg1;
                programBuffer[programSize++] = (uint8_t)arg2;
            } else {
                Serial.print("ADVERTENCIA: Opcode desconocido en línea ");
                Serial.print(lineNum);
                Serial.print(": ");
                Serial.println(opcode_str);
            }
        }
    }
    
    file.close();
    
    Serial.print("Programa cargado: ");
    Serial.print(programSize);
    Serial.println(" bytes");
    
    return programSize > 0;
}

#endif

// =========================
// === ARDUINO SETUP/LOOP ===
// =========================

void setup() {
    Serial.begin(115200);
    while(!Serial) delay(10);
    delay(1000);
    
    Serial.println("==============================================");
    Serial.println("    TeoCompis VM - Cargador desde SD");
    Serial.println("==============================================");

    initSensors();

    if (!initializeSD()) {
        Serial.println("ERROR CRÍTICO: No se puede inicializar SD");
        Serial.println("Sistema detenido.");
        return;
    }
    
    if (!loadProgramFromSD()) {
        Serial.println("ERROR CRÍTICO: No se puede cargar el programa desde SD");
        Serial.println("Sistema detenido.");
        return;
    }
    
    Serial.println("--- INICIANDO EJECUCIÓN (SETUP) ---");
    vm.loadProgram(programBuffer, programSize);
    
    // Run setup code (everything before the loop or until HALT)
    // If loop_start_pc is set, we might want to stop before it?
    // But usually setup code ends with HALT or falls through.
    // For now, we just run. If it hits HALT, it stops.
    vm.run();
    
    Serial.println("--- SETUP COMPLETADO ---");
    vm.dumpRegisters();
}

void loop() {
    // Run the user's loop function if defined
    vm.runLoop();
    
    // Optional: small delay to prevent CPU hogging if loop is empty
    // delay(1); 
}

