# Primeros Pasos

Esta guía describe cómo instalar las dependencias, compilar la cadena de herramientas y ejecutar programas A3 sobre TinyVM.

## Prerrequisitos

Instala los siguientes paquetes (nombres para Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install build-essential flex bison python3 python3-pip
```

Si deseas flashear TinyVM a una placa ESP32, también necesitarás el IDE de Arduino o `arduino-cli`.

## Estructura del Repositorio

```
TLP_proyecto/
├── language/          # Léxico, parser, semántica, AST y traductor
├── vm/                # Firmware TinyVM para ESP32
└── docs/              # Fuentes de documentación MkDocs
```

## Compilar la Cadena de Herramientas

Entra al directorio `language` y ejecuta el Makefile:

```bash
cd language
make clean
make
```

Artefactos resultantes:

- `a3c` — compilador para programas `.a3` y genera bytecode TinyVM (`.vmcode`).

## Traducir un Programa

1. Escribe tu programa (por ejemplo, `program.a3`).
2. Ejecuta el compilador para verificar la sintaxis y generar el bytecode:

    ```bash
    ./a3c < program.a3
    ```

4. Copia `program.vmcode` a la tarjeta SD usada por TinyVM (ruta predeterminada `/program.vmcode`).

## Desplegar en TinyVM

1. Abre `vm/vm_complete.ino` en Arduino IDE.
2. Ajusta `VMCODE_FILE` o el cableado SD si tu hardware difiere.
3. Flashea el firmware a la ESP32.
4. Inserta la SD con `program.vmcode` y reinicia la placa.
5. Usa el monitor serie (115200 baudios) para ver depuración, dumps de registros y traps.

## Usar la Extensión de VS Code

En el marketplace de VS Code puedes descargar la extensión A3 Linter para que tus programas sean mucho más fácil de escribir.

![A3 Linter Extension](image.png)
