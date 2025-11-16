%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
int    yylex(void);
void   yyerror(const char *s);
Node *root; // output
%}
%union {
    long ival;
    char *id;
    double dval;
    bool bval;
    Node *node;
    List *list;
}
%token START
%token END
%token INTVAL
%token DOUBLEVAL
%token TRUE FALSE 
%token ID
%token ADD MINUS MULT DIV
%token FUNCTION EXEC
%token LPAREN RPAREN COMMA SEMICOLON
%token OR AND NOT LT LEQ GT GEQ EQ NEQ
%token ASSIGN

%type <node> Programa Asignacion Expresion Bloque BloqueAux
%type <id> ID
%type <ival> INTVAL
%type <dval> DOUBLEVAL
%type <bval> TRUE FALSE

%%
Programa : Bloque { root = $1;}
            ;
Bloque : START BloqueAux END { $$ = $2;}
        ;
BloqueAux: Asignacion SEMICOLON { $$ = $1; }
        ;
Asignacion: ID ASSIGN Expresion { $$ = new_assignment_node($1, $3); }
            ;
Expresion: INTVAL { $$ = new_int_node($1); }
        ;

%%
void yyerror(const char *s) {
    fprintf(stderr, "parse error: %s\n", s);
}

void ast_print(Node *node, int indent) {
    if (node == NULL) return;
    for (int i = 0; i < indent; i++) printf("  ");
    if(strcmp(node->type, "INTVAL") == 0) {
        printf("INT: %d\n", node->ivalue);
    } else if (strcmp(node->type, "ASSIGN") == 0) {
        printf("ASSIGN: %s\n", node->value);
        ast_print(node->right, indent + 1);
    } else {
        printf("UNKNOWN NODE TYPE\n");
    }
}

extern FILE *yyin;     /* provided by Flex */
extern Node *root;     /* from parser.y */

int main(int argc, char **argv){
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror(argv[1]); return 1; }
    }
    if (yyparse() == 0) {
        puts("Parse OK\nAST:");
        if (root) ast_print(root, 0); else puts("(root is null)");
    }
    if (yyin && yyin != stdin) fclose(yyin);
    return 0;
}