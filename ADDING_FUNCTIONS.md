# Adding Custom Functions to the Arduino VM

## How to Add New High-Level Functions

To add a new custom function like `avanzar`, follow these steps:

### 1. Add Function Name Check in Compiler

In `arduino_compiler.c`, in the `compile_exec_call` function, add your function name to the list:

```c
if (strcmp(func_name, "avanzar") == 0 ||
    strcmp(func_name, "retroceder") == 0 ||
    strcmp(func_name, "my_new_function") == 0) {  // <-- Add here
```

### 2. Add Function Implementation in VM

In `arduino_vm.c`, in the `arduino_vm_run` function, add the function definition:

```c
fprintf(vm->output, "void my_new_function() {\n");
fprintf(vm->output, "  // Your Arduino code here\n");
fprintf(vm->output, "  digitalWrite(7, HIGH);\n");
fprintf(vm->output, "  delay(500);\n");
fprintf(vm->output, "  digitalWrite(7, LOW);\n");
fprintf(vm->output, "}\n\n");
```

### 3. Usage in Your Language

Then you can use it in your custom language programs:

```
start
  exec my_new_function();
end
```

## Current Available Functions

### Robot Movement

- `avanzar()` - Move forward
- `retroceder()` - Move backward
- `girar_izquierda()` - Turn left
- `girar_derecha()` - Turn right
- `detener()` - Stop all motors

### LED Control

- `encender_led()` - Turn on LED (pin 13)
- `apagar_led()` - Turn off LED (pin 13)

### Sensors

- `leer_sensor()` - Read from analog sensor (A0)

## Pin Assignments

The current implementation uses these pins:

- Motor A (Left): Pins 3, 4
- Motor B (Right): Pins 5, 6
- LED: Pin 13
- Sensor: Analog A0

You can modify these pin assignments in the function implementations in `arduino_vm.c`.

## Example: Adding a Servo Function

1. **Add to compiler check:**

```c
strcmp(func_name, "mover_servo") == 0
```

2. **Add function definition:**

```c
fprintf(vm->output, "#include <Servo.h>\n");  // Add at top
fprintf(vm->output, "Servo myServo;\n\n");     // Add after includes

fprintf(vm->output, "void mover_servo() {\n");
fprintf(vm->output, "  myServo.write(90);\n");
fprintf(vm->output, "  delay(1000);\n");
fprintf(vm->output, "  myServo.write(0);\n");
fprintf(vm->output, "}\n\n");
```

3. **Use in your language:**

```
start
  exec mover_servo();
end
```

This modular approach makes it easy to extend the VM with any Arduino functionality you need!
