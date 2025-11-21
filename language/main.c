#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"
#include "semantic.h"
#include "translator.h"
extern FILE *yyin; 
extern int yylineno;

int main () {
    yyin = fopen("test.src", "r");
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