# TinyVM for ESP32 (Arduino)

A lightweight, register-based virtual machine designed to run on ESP32 (and other microcontrollers) using the Arduino framework. This implementation is contained within a single `.ino` file for simplicity and ease of integration.

## Architecture

- **Registers**: 8 General Purpose Registers (`R0` - `R7`). Each is a 32-bit signed integer (`int32_t`).
- **Stack**: 1024 integers (4KB) for temporary storage and expression evaluation.
- **Heap**: 2048 bytes (2KB) for dynamic data storage.
- **Instruction Format**: Fixed 3-byte length per instruction.
  - Byte 0: `OPCODE`
  - Byte 1: `ARG1` (Usually Destination Register or Operand)
  - Byte 2: `ARG2` (Usually Source Register, Immediate Value, or Address)

## Instruction Set

| Opcode | Name      | Arguments | Description                                            |
| :----- | :-------- | :-------- | :----------------------------------------------------- |
| `0x00` | **HALT**  | -         | Stops execution.                                       |
| `0x01` | **LOAD**  | `R, Val`  | Loads immediate value `Val` (0-255) into Register `R`. |
| `0x02` | **ADD**   | `R1, R2`  | `R1 = R1 + R2`                                         |
| `0x03` | **SUB**   | `R1, R2`  | `R1 = R1 - R2`                                         |
| `0x04` | **MUL**   | `R1, R2`  | `R1 = R1 * R2`                                         |
| `0x05` | **DIV**   | `R1, R2`  | `R1 = R1 / R2`                                         |
| `0x06` | **MOV**   | `R1, R2`  | Copies value of `R2` into `R1`.                        |
| `0x07` | **PRINT** | `R`       | Prints the value of Register `R` to Serial.            |
| `0x08` | **JMP**   | `Addr`    | Unconditional Jump. Address is `(ARG1 << 8) \| ARG2`.  |
| `0x09` | **JZ**    | `R, Addr` | Jump to `Addr` (ARG2) if `R` is Zero.                  |
| `0x0A` | **JNZ**   | `R, Addr` | Jump to `Addr` (ARG2) if `R` is Not Zero.              |
| `0x0B` | **PUSH**  | `R`       | Pushes value of `R` onto the Stack.                    |
| `0x0C` | **POP**   | `R`       | Pops value from Stack into `R`.                        |
| `0x0D` | **STORE** | `R, Addr` | Stores lower 8-bits of `R` into Heap at `Addr` (ARG2). |
| `0x0E` | **LOADM** | `R, Addr` | Loads byte from Heap at `Addr` (ARG2) into `R`.        |
| `0x0F` | **EQ**    | `R1, R2`  | `R1 = 1` if `R1 == R2`, else `0`.                      |
| `0x10` | **LT**    | `R1, R2`  | `R1 = 1` if `R1 < R2`, else `0`.                       |
| `0x11` | **GT**    | `R1, R2`  | `R1 = 1` if `R1 > R2`, else `0`.                       |
| `0xFF` | **DEBUG** | -         | Dumps all register values to Serial.                   |

## Usage

1.  Open `vm_complete.ino` in the Arduino IDE.
2.  Define your bytecode program as a `uint8_t` array.
3.  In `setup()`, load and run the program:

```cpp
const uint8_t myProgram[] = {
    LOAD, 0, 10,   // R0 = 10
    LOAD, 1, 20,   // R1 = 20
    ADD,  0, 1,    // R0 = 30
    PRINT, 0, 0,   // Output: 30
    HALT, 0, 0
};

void setup() {
    Serial.begin(115200);
    vm.loadProgram(myProgram, sizeof(myProgram));
    vm.run();
}
```

## Local Testing

You can verify the VM logic on your PC without flashing the ESP32 using the included test harness.

1.  Navigate to the `test` directory:
    ```bash
    cd vm/test
    ```
2.  Compile and run the tests:
    ```bash
    make run
    ```

This uses a mock Arduino environment (`mock_arduino.h`) to simulate `Serial` and `delay` functions.
