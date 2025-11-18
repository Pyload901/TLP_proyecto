# VM Architecture Documentation

This document provides comprehensive documentation for the Virtual Machine (VM) architecture used in the Robot Language to Arduino Translator project.

## 📁 Directory Structure

```
vm/
├── arduino_ir.h    # Intermediate Representation definitions
├── compiler.c      # Source code compiler implementation
├── compiler.h      # Compiler interface and types
├── vm.c           # Virtual Machine implementation
└── vm.h           # VM interface and types
```

## 🔄 Architecture Overview

The VM architecture follows the **BNF_Compiler_Machine** pattern with a clean separation of concerns:

```
Robot Program (.prog) → Compiler → Arduino IR → VM → Arduino C++ (.ino)
```

### Data Flow:

1. **Input**: Custom robot language program (e.g., `my_robot.prog`)
2. **Compiler**: Parses and converts to intermediate representation (IR)
3. **VM**: Executes IR and generates Arduino C++ code
4. **Output**: Ready-to-upload Arduino sketch (`.ino` file)

---

## 📋 File Documentation

### 1. `arduino_ir.h` - Intermediate Representation

**Purpose**: Defines the core data structures and opcodes for the Arduino-specific intermediate representation.

**Key Components**:

#### 🔧 **OpCodes (ArduinoOpCode enum)**

- **Arithmetic**: `ARD_OP_ADD`, `ARD_OP_SUB`, `ARD_OP_MUL`, `ARD_OP_DIV`
- **Logic**: `ARD_OP_EQ`, `ARD_OP_LT`, `ARD_OP_AND`, `ARD_OP_OR`
- **Control Flow**: `ARD_OP_IF_START`, `ARD_OP_IF_END`, `ARD_OP_JMP`
- **Arduino Specific**:
  - `ARD_OP_PIN_MODE` - Configure pin modes
  - `ARD_OP_DIGITAL_WRITE` - Digital pin output
  - `ARD_OP_DELAY` - Time delays
  - `ARD_OP_CALL_CUSTOM` - Robot function calls
- **Program Structure**: `ARD_OP_SETUP_START/END`, `ARD_OP_LOOP_START/END`
- **Variables**: `ARD_OP_VAR_DECL`, `ARD_OP_LOAD_VAR`, `ARD_OP_STORE_VAR`

#### 🗃️ **Data Structures**

- **`ArduinoValue`**: Union for different data types (int, float, bool)
- **`ArduinoChunk`**: Bytecode container with opcodes and constants
- **`ArduinoFunction`**: Function representation with name and bytecode chunk
- **`ArduinoModule`**: Complete program module with setup() and loop() functions

**Importance**:

- Provides **platform-independent** representation of robot programs
- Enables **optimization** and **analysis** before code generation
- Separates **parsing concerns** from **code generation**
- Allows **multiple backends** (Arduino, other microcontrollers)

---

### 2. `compiler.h` & `compiler.c` - Source Code Compiler

**Purpose**: Converts robot language source code into Arduino intermediate representation.

#### **compiler.h - Interface**

```c
typedef struct {
    const char *src;        // Source code
    ArduinoModule module;   // Generated IR module
    int error_count;        // Compilation errors
} ArduinoCompiler;

int compile_source(ArduinoCompiler *c, const char *src);
```

#### **compiler.c - Implementation**

**Key Features**:

#### 🔍 **Lexical Analysis**

- **Token Types**: Keywords (`if`, `start`, `end`, `exec`), identifiers, numbers
- **Tokenizer**: Converts source text into structured tokens
- **Comment Handling**: Skips `//` line comments
- **Whitespace**: Intelligent whitespace and indentation handling

#### 🌳 **Parsing (Recursive Descent)**

- **`parse_program()`**: Main program structure
- **`parse_main_function()`**: Main function body parsing
- **`parse_statement()`**: Individual statement parsing
- **`parse_if_statement()`**: Conditional logic with `start`/`end` blocks
- **`parse_var_declaration()`**: Variable declarations (`int var = value`)
- **`parse_exec_call()`**: Robot function calls (`exec avanzar()`)
- **`parse_delay_call()`**: Timing commands (`delay(1000)`)

#### 📝 **Code Generation**

- **Setup Generation**: Automatically configures Arduino pins
- **IR Emission**: Converts parsed structures to bytecode opcodes
- **String Management**: Handles function names and conditions
- **Error Recovery**: Graceful handling of syntax errors

**Importance**:

- **Language Frontend**: Understands your custom robot syntax
- **Error Detection**: Catches syntax errors early in compilation
- **IR Generation**: Produces optimized intermediate code
- **Extensibility**: Easy to add new language features

---

### 3. `vm.h` & `vm.c` - Virtual Machine

**Purpose**: Executes Arduino IR and generates optimized Arduino C++ code.

#### **vm.h - Interface**

```c
typedef struct {
    ArduinoValue *stack;    // Execution stack
    int sp;                 // Stack pointer
    int capacity;           // Stack capacity
    FILE *output;           // Output file stream
    int indent_level;       // Code formatting
    // ... other VM state
} VM;

void vm_init(VM *vm, FILE *output);
int vm_run(VM *vm, ArduinoModule *module);
void vm_free(VM *vm);
```

#### **vm.c - Implementation**

**Key Features**:

#### 🖥️ **Stack-Based Execution**

- **Execution Stack**: Manages operands and intermediate values
- **Stack Operations**: `push()`, `pop()` for data manipulation
- **Memory Management**: Dynamic stack allocation and cleanup

#### 🏭 **Code Generation Engine**

- **Arduino Functions**: Pre-defined robot control functions:
  - `avanzar()` - Move forward (both motors forward)
  - `retroceder()` - Move backward (both motors backward)
  - `girar_izquierda()` - Turn left (differential drive)
  - `girar_derecha()` - Turn right (differential drive)
  - `detener()` - Stop all motors
  - `encender_led()` - Turn on LED
  - `leer_sensor()` - Read analog sensor

#### 🎯 **Opcode Execution**

- **Setup Function**: Generates `setup()` with pin configurations
- **Loop Function**: Generates `loop()` with program logic
- **Control Structures**: Proper C++ `if`/`else` generation
- **Variable Declarations**: Arduino-compatible variable syntax
- **Function Calls**: Maps custom functions to Arduino implementations

#### ✨ **Code Formatting**

- **Indentation**: Proper code indentation for readability
- **Comments**: Descriptive comments in generated code
- **Pin Documentation**: Clear pin assignment documentation

**Importance**:

- **Code Backend**: Generates optimized Arduino C++ code
- **Hardware Abstraction**: Maps high-level commands to pin operations
- **Code Quality**: Produces readable, maintainable Arduino code
- **Platform Optimization**: Arduino-specific optimizations and conventions

---

## 🔧 Technical Architecture

### Compilation Pipeline:

```
Source Code (.prog)
      ↓
  Lexer (Tokenization)
      ↓
  Parser (AST/IR Generation)
      ↓
  Arduino IR (Bytecode)
      ↓
  VM Execution
      ↓
  Arduino C++ (.ino)
```

### Memory Management:

- **Compiler**: Manages source parsing and IR generation
- **VM**: Handles execution stack and output generation
- **Cleanup**: Proper resource deallocation

### Error Handling:

- **Compile-time**: Syntax error detection and reporting
- **Runtime**: Graceful VM error handling
- **User Feedback**: Clear error messages with context

---

## 🚀 Integration Benefits

### 1. **Modularity**

- Each component has a single responsibility
- Easy to test and debug individual components
- Clean interfaces between modules

### 2. **Extensibility**

- Add new opcodes to `arduino_ir.h`
- Extend parser in `compiler.c` for new syntax
- Add new Arduino functions in `vm.c`

### 3. **Maintainability**

- Clear separation of concerns
- Well-documented interfaces
- Consistent coding patterns

### 4. **Performance**

- Efficient bytecode representation
- Optimized code generation
- Minimal runtime overhead

---

## 🎯 Usage Examples

### Compiling a Robot Program:

```c
ArduinoCompiler compiler;
if (compile_source(&compiler, source_code)) {
    // Compilation successful
    VM vm;
    vm_init(&vm, output_file);
    vm_run(&vm, &compiler.module);
    vm_free(&vm);
}
```

### Adding New Robot Functions:

1. Add opcode to `arduino_ir.h`
2. Update parser in `compiler.c`
3. Add code generation in `vm.c`

This architecture provides a robust, extensible foundation for translating high-level robot programs into efficient Arduino code while maintaining clean separation of concerns and excellent maintainability.
