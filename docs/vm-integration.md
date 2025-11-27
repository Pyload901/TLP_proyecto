# Integración con la VM

Esta sección describe cómo las construcciones de alto nivel de TLP se mapean al conjunto de instrucciones y al runtime de TinyVM detallado en `vm/vm_complete.ino` y `vm/vm_architecture.md`.

## Modelo de Memoria en Ejecución

TinyVM reserva:

- **Registros (`R0`-`R7`)**: propósito general con `R6` como registro de retorno.
- **Pila (1 KB)**: evaluación de expresiones, marcos de funciones y variables locales.
- **Heap (2 KB)**: arreglos y buffers dinámicos.
- **Buffer de programa**: bytecode cargado desde la tarjeta SD (`/program.vmcode`).

El traductor asigna a cada variable declarada una ubicación estática relativa al marco de pila o al heap. Los arreglos ocupan rangos contiguos indexados por el tamaño del elemento.

## Codificación de Instrucciones

Cada instrucción ocupa 3 bytes: opcode, `arg1`, `arg2`. Los saltos usan dos bytes (little-endian). Inmediatos más grandes emplean `LOADI16` para insertar palabras de 16 bits.

| Construcción TLP  | Patrón Emitido |
|-------------------|----------------|
| `a = b + c;`      | `LOAD R1, VAR_b` → `LOAD R2, VAR_c` → `ADD R1, R2` → `STORE VAR_a, R0` |
| `if (cond) start end`   | Evaluar `cond`, emitir `JZ else_addr`, cuerpo, opcional `JMP end`, bloque else |
| `while (cond)`    | Etiqueta inicio → evaluar → `JZ exit` → cuerpo → `JMP start` → etiqueta salida |
| `for`             | Inicialización → etiqueta bucle → condición + `JZ exit` → cuerpo → incremento → `JMP loop` |
| `array[i]` lectura| Evaluar `i`, sumar base, `LOADM target_reg, idx_reg` |
| `array[i] = v`    | Evaluar `i` y `v`, calcular dirección, `STORE addr_reg, value_reg` |
| `exec B_x(y)`     | Evaluar argumentos en registros y emitir `TRAP trap_id` |

## Banderas y Comparaciones

- `CMP Rx, Ry` establece las banderas zero/lt/gt/le/ge.
- Los saltos condicionales (`JZ`, `JNZ`, `JLT`, `JGT`, `JLE`, `JGE`) dependen de dichas banderas.
- Las expresiones booleanas aplican cortocircuito encadenando saltos y etiquetas para evitar evaluaciones innecesarias.

## Llamadas a Funciones

1. Los argumentos se apilan de derecha a izquierda.
2. `CALL addr` guarda el PC de retorno (dos posiciones en la pila) y salta.
3. La función llamada reserva locales ajustando el puntero de pila.
4. `return expr;` coloca el valor en `R6`, restaura la pila y ejecuta `RET`.
5. La función que llama lee `R6` para obtener el resultado y desapila los argumentos si es necesario.

## Traps de Hardware

`TRAP opcode` despacha a `TinyVM::call_trap`, con soporte para:

- IO digital/analógico (`B_DIGITAL_READ`, `B_DIGITAL_WRITE`, `B_ANALOG_READ`).
- Control de motores (`B_FORWARD`, `B_BACK`, `B_TURN_LEFT`, `B_TURN_RIGHT`, `B_SET_SPEED`).
- Lectura de sensores (`B_READ_IR_LEFT`, `B_READ_IR_RIGHT`).

Los argumentos se colocan convencionalmente en `R0`, `R1`, … antes de emitir la instrucción `TRAP`.

## Integración con `loop()` de Arduino

Cuando `vm_complete.ino` carga el bytecode, puede registrar la etiqueta `.loop` y ejecutar `TinyVM::runLoop()` dentro de `loop()`. Esto permite estructurar los programas como las funciones `setup` + `loop` típicas de Arduino.

Consulta `vm/vm_architecture.md` para profundizar en la definición de opcodes, marcos de pila y tiempos de ejecución.
