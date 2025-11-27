# Referencia del Lenguaje

Este capítulo describe la sintaxis y la semántica del lenguaje TLP. La especificación coincide con la gramática implementada en `gramatica`, `lexer.l` y las rutinas de análisis semántico ubicadas en `language/`.

## Estructura Léxica

- **Identificadores**: comienzan con una letra y pueden contener letras, dígitos o `_`.
- **Literales**:
  - Enteros: dígitos decimales (`42`, `0`, `1024`).
  - Dobles: dígitos con punto decimal (`3.14`).
  - Caracteres: comillas simples (`'a'`).
  - Booleanos: `true`, `false`.
- **Comentarios**: `// comentario de línea` y `/* comentario de bloque */`.

### Palabras Reservadas

`int`, `float`, `char`, `bool`, `void`, `function`, `return`, `if`, `else`, `for`, `while`, `start`, `end`, `exec`.

## Tipos

| Tipo    | Descripción                         | Valor por defecto |
|---------|-------------------------------------|-------------------|
| `int`   | Entero con signo de 32 bits          | `0`               |
| `float`| Flotante de 64 bits (se descompone en dos registros en la VM) | `0.0` |
| `char`  | Carácter de 8 bits almacenado como `int` | `0`           |
| `bool`  | Booleano almacenado como `0`/`1`     | `false`           |
| `T[]`   | Arreglo unidimensional del tipo `T`  | Inicializado en cero |

Los arreglos se definen con una longitud constante en tiempo de compilación y pueden inicializarse mediante listas literales. Solo se admite una dimensión en la gramática y en el runtime actual de la VM.

## Resumen de la Gramática

```
Programa -> Bloque Programa | Declaracion_funcion Programa | λ
Declaracion_funcion -> TIPO FUNCTION ID ( Parametros_funcion ) Bloque
Parametros_funcion -> TIPO ID Parametros_funcion_aux | λ
Parametros_funcion_aux -> , TIPO ID Parametros_funcion_aux | λ
Retorno_funcion -> RETURN Expresion
Bloque -> START BloqueAux END
BloqueAux -> Instruccion BloqueAux | λ
Instruccion -> Asignacion ; | Declaracion ; | For | While | If | Exec_funcion ; | Retorno_funcion ;
...
```

La gramática continúa con producciones para expresiones, precedencias e indexaciones de arreglos, tal como se detalla en `/gramatica`. La precedencia de operadores sigue el orden del archivo: unarios `!`/`-`, multiplicativos `* / %`, aditivos `+ -`, comparaciones, `&&`, `||`.

## Sentencias

### Declaraciones

```
int counter;
double velocity = 0.0;
bool flags[4];
```

- Inicialización opcional mediante expresiones o literales de arreglo: `int data[3] = [1,2,3];`.
- La longitud de los arreglos debe ser un literal entero conocido en compilación.

### Asignaciones

```
counter = counter + 1;
data[2] = counter;
data = [0, 1, 2];
```

Los literales de arreglo deben coincidir con la longitud declarada.

### Control de Flujo

```
if (sensor > threshold) start
    exec forward_ms(250);
end else start
    exec stopMotors();
end

while (i < 10) start i = i + 1; end

for (int j = 0; j < size; j = j + 1) start
    sum = sum + values[j];
end
```

Cada ciclo se traduce a saltos y comparaciones en TinyVM; consulta **Integración con la VM** para más detalles.

### Funciones

```
int proc add(int a, int b) start
    return a + b;
end
```

- Los argumentos se pasan por valor mediante la pila de TinyVM.
- `return` abandona la función y coloca el valor en el registro `R6`.

### Llamadas Integradas

`exec` invoca traps de TinyVM (definidos en `vm_complete.ino`). Los argumentos se evalúan antes de ejecutar el trap:

```
exec digitalWrite(pin, value);
exec print(counter);
```

## Expresiones

- Unarios: `-x`, `not flag`.
- Aritmética binaria: `+ - * / %`.
- Comparaciones: `== != < <= > >=`.
- Lógicos: `and or` (el traductor aplica cortocircuito).
- Paréntesis para agrupar expresiones.

## Manejo de Errores

El analizador semántico verifica:

- Compatibilidad de tipos en asignaciones y operaciones.
- Límites de arreglos en tiempo de compilación para inicializadores literales y en tiempo de ejecución para accesos indexados.
- Firmas de funciones (definiciones duplicadas, aridad y tipos de parámetros).

Los diagnósticos incluyen línea y columna, y se muestran durante la traducción.

Continúa con **Integración con la VM** para conocer la representación de bajo nivel de estas construcciones.
