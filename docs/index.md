# Documentación del Lenguaje A3

Bienvenido a la documentación oficial del lenguaje de programación A3. A3 es un lenguaje compacto de estilo C pensado para fines educativos y diseñado para ejecutarse sobre la máquina virtual TinyVM en robots basados en ESP32. Se enfoca en:

- **Semántica predecible** sustentada por una gramática determinista construida con Flex/Bison.
- **Integración estrecha con TinyVM**, utilizando opcodes para aritmética, control de flujo y traps de hardware.
- **Sintaxis accesible** para estudiantes que aprenden programación imperativa, arreglos y composición de funciones.

## Panorama Rápido

| Categoría         | Características |
|-------------------|-----------------|
| Tipos             | `int`, `float`, `char`, `bool`, arreglos unidimensionales |
| Operadores        | Aritméticos (+, -, *, /, %), lógicos (and, or, not), comparaciones (==, !=, <, >, <=, >=) |
| Control de flujo  | `if/else`, `while`, `for` con sintaxis tipo C |
| Funciones         | Declaraciones `proc` con parámetros tipados y `return` |
| E/S y hardware    | `exec` para invocar funciones externas o integradas en TinyVM para sensores, motores y SD |

## Ejemplo Mínimo

```a3
int proc main() start
    int counter = 0;

    while (counter < 5) start
        exec print(counter);
        counter = counter + 1;
    end

    return counter;
end
```

Este programa incrementa un contador, imprime los valores intermedios mediante un trap de la VM y devuelve el resultado final.

## Mapa de la Documentación

El resto del sitio se organiza así:

1. **Primeros Pasos** — requisitos del toolchain, compilación y ejecución en TinyVM.
2. **Referencia del Lenguaje** — gramática completa, tokens léxicos, reglas de tipos y semántica.
3. **Integración con la VM** — mapeo de construcciones A3 hacia instrucciones y traps de TinyVM, además del layout en tiempo de ejecución.
4. **Herramientas** — extensiones de IDE, utilidades de prueba y flujos de trabajo para extender el lenguaje.

Usa la barra lateral de navegación para saltar a cualquier sección. Cada página incluye enlaces cruzados hacia el material de referencia relevante para facilitar la exploración.
