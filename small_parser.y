%{
#include <stdio.h>
#include "ast.h"
int    yylex(void);
void   yyerror(const char *s);
Node *root; // output
%}
%define api.value.type { Node* }
%union {
    long ival;
    char *id;
    Node *node;
    List *list;
}
%token START
%token END
%token NUMBER
%token ID
%token SUMA RESTA MULT DIV
%token FUNCTION EXEC
%token LPAREN RPAREN COMMA SEMICOLON
%token VALOR
%token TRUE FALSE OR AND NOT LT LEQ GT GEQ EQ NEQ
%token ASIGNACION

%type <node> Programa Exec_funcion Asignacion Expresion Bloque BloqueAux
%type <id> Id

%%
Programa : Bloque { root = $1;}
            ;
Bloque : START BloqueAux END { $$ = $2;}
        ;
BloqueAux: Asignacion SEMICOLON { $$ = create_asignacion_node($1, NULL); }
          | Asignacion SEMICOLON BloqueAux { $$ = create_asignacion_node($1, $3); }
          | Exec_funcion SEMICOLON { $$ = create_exec_node($1, NULL); }
          | Exec_funcion SEMICOLON BloqueAux { $$ = create_exec_node($1, $3); }
          ;