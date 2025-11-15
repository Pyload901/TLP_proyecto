# Arduino Language Translator

A virtual machine that translates programs written in your custom language into Arduino C++ code.

## Overview

This project implements a complete translation pipeline from your custom programming language to Arduino C++ code:

1. **Lexical Analysis** - Tokenizes your custom language
2. **Parsing** - Builds an Abstract Syntax Tree (AST)
3. **Compilation** - Converts AST to Arduino Intermediate Representation (IR)
4. **Code Generation** - Arduino VM executes IR and generates C++ code

## Architecture

```
Custom Language → Lexer → Parser → AST → Arduino Compiler → Arduino IR → Arduino VM → Arduino C++
```

### Components

- **`arduino_ir.h`** - Defines Arduino-specific opcodes and data structures
- **`arduino_vm.c`** - Virtual machine that generates Arduino C++ code
- **`arduino_compiler.c`** - Compiles AST to Arduino IR
- **`arduino_translator.c`** - Main program tying everything together

## Features

### Language Support

- Variable declarations (`int`, `bool`, `char`, `float`)
- Arithmetic and logical operations
- Control flow (`if/else`, `while`, `for`)
- Function calls (`exec`)

### Arduino Features

- Pin operations (`pinMode`, `digitalWrite`, `digitalRead`)
- Analog I/O (`analogRead`, `analogWrite`)
- Serial communication (`Serial.begin`, `Serial.print`)
- Timing functions (`delay`, `millis`)

### Custom High-Level Robot Functions

The VM includes predefined high-level robot control functions that automatically translate to multiple Arduino operations:

- **`avanzar()`** - Move robot forward (both motors forward)
- **`retroceder()`** - Move robot backward (both motors backward)
- **`girar_izquierda()`** - Turn robot left (left motor backward, right forward)
- **`girar_derecha()`** - Turn robot right (left motor forward, right backward)
- **`detener()`** - Stop all motors
- **`encender_led()`** - Turn on LED (pin 13)
- **`apagar_led()`** - Turn off LED (pin 13)
- **`leer_sensor()`** - Read sensor value from analog pin A0

These functions automatically set up the necessary pin configurations and generate the appropriate `digitalWrite()` calls.

- Timing functions (`delay`, `millis`)

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential flex bison

# macOS
brew install flex bison

# Windows
# Install MinGW-w64 and flex/bison for Windows
```

### Compilation

```bash
make all           # Build the translator
make demo          # Run demonstration
make test          # Run tests
make clean         # Clean build files
```

## Usage

```bash
# Basic usage
./arduino_translator input.txt output.ino

# Example
./arduino_translator example_program.txt my_arduino_code.ino
```

### Example Input Program with Custom Functions

```
start
  // Robot control program
  int sensor_value = 0;
  bool obstacle_detected = false;

  // Initialize robot
  exec Serial_begin(9600);

  // Main robot behavior loop
  while (!obstacle_detected) start
    // Read sensor
    sensor_value = exec leer_sensor();
    exec Serial_print(sensor_value);

    if (sensor_value > 500) start
      // Obstacle detected - avoid it
      exec detener();
      exec encender_led();
      exec delay(200);
      exec girar_derecha();
      exec delay(800);
      exec apagar_led();
    end else start
      // Path clear - move forward
      exec avanzar();
    end

    exec delay(100);
  end

  // Stop robot
  exec detener();
end
```

### Basic Example Input Program

```
start
  int ledPin = 13;
  int sensorPin = A0;
  bool ledState = false;

  exec Serial_begin(9600);
  exec pinMode(ledPin, 1);

  while (true) start
    int sensorValue = exec analogRead(sensorPin);
    exec Serial_print(sensorValue);

    if (sensorValue > 512) start
      exec digitalWrite(ledPin, 1);
    end else start
      exec digitalWrite(ledPin, 0);
    end

    exec delay(100);
  end
end
```

### Generated Arduino Output

```cpp
// Generated Arduino code
int ledPin = 13;
int sensorPin = A0;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop() {
  int sensorValue = analogRead(A0);
  Serial.print(sensorValue);

  if (sensorValue > 512) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  delay(100);
}
```

## Language Mapping

### Basic Arduino Functions

| Custom Language               | Arduino C++              |
| ----------------------------- | ------------------------ |
| `exec pinMode(pin, mode)`     | `pinMode(pin, mode)`     |
| `exec digitalWrite(pin, val)` | `digitalWrite(pin, val)` |
| `exec digitalRead(pin)`       | `digitalRead(pin)`       |
| `exec analogRead(pin)`        | `analogRead(pin)`        |
| `exec Serial_begin(baud)`     | `Serial.begin(baud)`     |
| `exec Serial_print(val)`      | `Serial.print(val)`      |
| `exec delay(ms)`              | `delay(ms)`              |

### High-Level Robot Functions

| Custom Language          | Generated Arduino Code                                                                  |
| ------------------------ | --------------------------------------------------------------------------------------- |
| `exec avanzar()`         | `digitalWrite(3,HIGH); digitalWrite(4,LOW); digitalWrite(5,HIGH); digitalWrite(6,LOW);` |
| `exec retroceder()`      | `digitalWrite(3,LOW); digitalWrite(4,HIGH); digitalWrite(5,LOW); digitalWrite(6,HIGH);` |
| `exec girar_izquierda()` | Left motor backward, right motor forward                                                |
| `exec girar_derecha()`   | Left motor forward, right motor backward                                                |
| `exec detener()`         | `digitalWrite(3,LOW); digitalWrite(4,LOW); digitalWrite(5,LOW); digitalWrite(6,LOW);`   |
| `exec encender_led()`    | `digitalWrite(13, HIGH);`                                                               |
| `exec apagar_led()`      | `digitalWrite(13, LOW);`                                                                |
| `exec leer_sensor()`     | `return analogRead(A0);`                                                                |

### Motor Pin Configuration

- **Motor A**: Pins 3, 4 (Left motor)
- **Motor B**: Pins 5, 6 (Right motor)
- **LED**: Pin 13
- **Sensor**: Analog pin A0

## Implementation Details

### Arduino IR Opcodes

- `ARD_OP_PIN_MODE` - Configure pin mode
- `ARD_OP_DIGITAL_WRITE/READ` - Digital I/O operations
- `ARD_OP_ANALOG_WRITE/READ` - Analog I/O operations
- `ARD_OP_SERIAL_*` - Serial communication
- `ARD_OP_DELAY` - Timing operations
- Standard arithmetic/logical operations

### VM Architecture

- Stack-based execution model
- Separate `setup()` and `loop()` function generation
- Global variable management
- Code formatting and indentation

## Extending the Translator

### Adding New Arduino Functions

1. Add opcode to `arduino_ir.h`
2. Implement VM execution in `arduino_vm.c`
3. Add compilation mapping in `arduino_compiler.c`

### Supporting New Language Features

1. Extend parser grammar
2. Add AST node types
3. Implement compilation in `arduino_compiler.c`

## Files

- `arduino_ir.h` - Arduino IR definitions
- `arduino_vm.c` - Code generation virtual machine
- `arduino_compiler.c` - AST to IR compiler
- `arduino_translator.c` - Main translation program
- `Makefile` - Build system
- `example_program.txt` - Sample input program
- `language.l` - Lexer specification (from your existing project)
- `parser.y` - Parser grammar (from your existing project)

## Integration with Existing Project

This translator integrates with your existing compiler project by:

1. Reusing your lexer (`language.l`) and parser (`parser.y`)
2. Building on your AST definitions
3. Adding Arduino-specific compilation and code generation
4. Providing a complete translation pipeline

The VM follows the same architectural patterns as your `BNF_Compiler_Machine` but targets Arduino C++ instead of generic execution.

## Future Enhancements

- Support for Arduino libraries (Servo, LCD, etc.)
- Optimization passes for generated code
- Error reporting and debugging information
- Support for multiple Arduino board types
- Integration with Arduino IDE build system
