%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#define VALOR 1
#define ID 2
#define BOOLOP 3
#define BOOLCON 4
#define TRUE 5
#define FALSE 6
#define IF 7
#define ELSE 8
#define WHILE 9
#define FOR 10
#define TIPO 12
#define COMENTARIO 13
#define CARACTER 14
#define FUNCTION 16
#define MULT 17
#define DIV 18
#define MODULO 19
#define SUMA 20
#define RESTA 21
#define FIN_LINEA 22
#define LPAREN 23
#define RPAREN 24
#define LBRACE 25
#define RBRACE 26
#define LBRACKET 27
#define RBRACKET 28
#define ASIGNACION 29
#define ERROR 30
#define RETURN 31
#define COMA 32
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

"<="                { return BOOLOP; }
">="                { return BOOLOP; }
"=="                { return BOOLOP; }
"!="                { return BOOLOP; }
"<"                 { return BOOLOP; }
">"                 { return BOOLOP; }

"and"               { return BOOLCON; }
"or"                { return BOOLCON; }
"not"               { return BOOLCON; }

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
    "NULL_0", "VALOR", "ID", "BOOLOP", "BOOLCON", "TRUE", "FALSE", "IF", "ELSE", 
    "WHILE", "FOR", "NULL_11", "TIPO", "COMENTARIO", "CARACTER", "NULL_15", "FUNCTION", 
    "MULT", "DIV", "MODULO", "SUMA", "RESTA", "FIN_LINEA", "LPAREN", 
    "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "ASIGNACION", "ERROR", "RETURN", "COMA"
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