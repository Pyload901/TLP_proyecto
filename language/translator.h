#pragma once
#include <stdbool.h>
#include "ast.h"

/*
 * Translates the given AST into a TinyVM instruction listing (human
 * readable text where each line contains the mnemonic and its two
 * operands). Returns true on success, false if the AST contains
 * constructs that are not supported by the current translator or if
 * any IO error happens. On failure a diagnostic is printed to stderr.
 */
bool translate_program(Node *root, const char *output_path);
