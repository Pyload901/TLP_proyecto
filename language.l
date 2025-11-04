%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#define VALOR 1
#define ID 2
#define TRUE 3
#define FALSE 4
#define IF 5
#define ELSE 6
#define WHILE 7
#define FOR 8
#define TIPO 9
#define COMENTARIO 10
#define CARACTER 11
#define FUNCTION 12
#define MULT 13
#define DIV 14
#define MODULO 15
#define SUMA 16
#define RESTA 17
#define FIN_LINEA 18
#define LPAREN 19
#define RPAREN 20
#define LBRACE 21
#define RBRACE 22
#define LBRACKET 23
#define RBRACKET 24
#define ASIGNACION 25
#define ERROR 26
#define RETURN 27
#define COMA 28
#define AND 29
#define OR 30
#define NOT 31
#define MENORQUE 32
#define MAYORQUE 33
#define IGUALIGUAL 34
#define DIFERENTE 35
#define MAYORIGUAL 36
#define MENORIGUAL 37
%}

%%
"true"              { return TRUE; }
"false"             { return FALSE; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"bool"              { return TIPO; }
"char"              { return TIPO; }
"int"               { return TIPO; }
"proc"              { return FUNCTION; }
"return"              { return RETURN; }

"<="                { return MENORIGUAL; }
">="                { return MAYORIGUAL; }
"=="                { return IGUALIGUAL; }
"!="                { return DIFERENTE; }
"<"                 { return MENORQUE; }
">"                 { return MAYORQUE; }

"and"               { return AND; }
"or"                { return OR; }
"not"               { return NOT; }

"/"                 { return DIV; }
"*"                 { return MULT; }
"%"                 { return MODULO; }
"+"                 { return SUMA; }
"-"                 { return RESTA; }
"="                 { return ASIGNACION; }

[-+]?[0-9]+(\.[0-9]+)?   { return VALOR; }
[a-zA-Z_][a-zA-Z0-9_]*  { return ID; }

\'[A-Za-z0-9\-_=+\]\[.?~!@#$%^&*()<>]?\'   { return CARACTER; }
";" { return FIN_LINEA; }

"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
","     { return COMA; }


"//".*    {  }
[ \t\n]+               { /* Ignorar espacios en blanco */ }

.   { return ERROR; }
%%
char *token_names[] = {
 "VALOR","ID","TRUE","FALSE","IF","ELSE","WHILE","FOR","TIPO","COMENTARIO","CARACTER","FUNCTION","MULT","DIV","MODULO","SUMA","RESTA","FIN_LINEA","LPAREN","RPAREN","LBRACE","RBRACE","LBRACKET","RBRACKET","ASIGNACION","ERROR","RETURN","COMA","AND","OR","NOT","MENORQUE","MAYORQUE","IGUALIGUAL","DIFERENTE","MAYORIGUAL","MENORIGUAL"
};
int main () {
    int token;
    while((token = yylex())) {
        printf("Línea %d: Token <%s> (%s)\n", yylineno, yytext, token_names[token]);
    }
    return 0;
}

int yywrap() {
    return 1;
}