#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"
#include "semantic.h"
extern FILE *yyin; 
extern int yylineno;

int main () {
    yyin = fopen("test.src", "r");
    if (!yyin) {
        perror("Failed to open input file");
        return 1;
    }
    next();
    Node* ast = parse_program();
    ast_print(ast, 0);
    analyze_program(ast);
    fclose(yyin);
    return 0;
}