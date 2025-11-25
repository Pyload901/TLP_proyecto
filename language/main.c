#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"
#include "semantic.h"
#include "translator.h"
extern FILE *yyin; 
extern int yylineno;

int main (int argc, char **argv) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Failed to open input file");
        return 1;
    }
    Node* ast = parse_program();
    ast_print(ast, 0);
    analyze_program(ast);
    if (!translate_program(ast, "program.vmcode")) {
        fprintf(stderr, "Code generation failed. See diagnostics above.\n");
        fclose(yyin);
        return 1;
    }
    fclose(yyin);
    return 0;
}