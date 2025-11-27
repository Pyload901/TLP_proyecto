# Herramientas y Flujo de Trabajo

A3 incluye utilidades auxiliares para agilizar el desarrollo, las pruebas y la documentación.

## Utilidades de Línea de Comandos

| Binario       | Ubicación    | Descripción |
|---------------|--------------|-------------|
| `a3c`      | `language/`  | Valida la corrección léxica y sintáctica y emite bytecode de TinyVM. |
| `integration_tests.py` | raíz del repo | Ejecuta regresiones para parser y traductor. |

Flujo de trabajo típico:

```bash
cd language
make
./a3c < test.a3
```

## Extensión de VS Code

`linter/` contiene una gramática TextMate empaquetada como extensión para VS Code:

En el marketplace de VS Code puedes descargar la extensión A3 Linter para que tus programas sean mucho más fácil de escribir.


## Documentación

Este sitio MkDocs vive en `docs/` con configuración en `mkdocs.yml`. Para contribuir:

```bash
pip install mkdocs mkdocs-material
mkdocs serve
```

Edita los archivos Markdown en `docs/`; MkDocs recarga los cambios en vivo en `http://127.0.0.1:8000`.

## Pruebas y CI

- `integration_tests.py` puede integrarse en CI para asegurar que las nuevas características del lenguaje compilan y se traducen.
- Es posible añadir pruebas unitarias alrededor de TinyVM (`vm/`) compilando con `#define UNIT_TESTING` para aislar las dependencias de Arduino.

## Flujo Sugerido

1. **Diseño**: Actualiza `gramatica` y las comprobaciones semánticas.
2. **Implementación**: Ajusta `translator.c` para emitir los nuevos opcodes.
3. **Regresiones**: Ejecuta las pruebas de compilador y, si aplica, flashea TinyVM con el nuevo bytecode.
4. **Documentación**: Agrega o modifica secciones de este sitio para capturar las nuevas características.

Mantener la documentación junto con la implementación ayuda a que estudiantes y colaboradores sigan la evolución del lenguaje.
