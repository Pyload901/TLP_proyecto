%option noyywrap noinput nounput
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Node Node;
typedef struct List List;
    

#include "parser.tab.h"
%}

%%
"start"              { return START; }
"end"              { return END; }
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
"exec"             { return EXEC; }

"<="                { return LEQ; }
">="                { return GEQ; }
"=="                { return EQ; }
"!="                { return NEQ; }
"<"                 { return LT; }
">"                 { return GT; }
"and"               { return AND; }
"or"                { return OR; }
"not"               { return NOT; }

"/"                 { return DIV; }
"*"                 { return MULT; }
"%"                 { return MODULO; }
"+"                 { return ADD; }
"-"                 { return MINUS; }
"="                 { return ASSIGN; }

[-+]?[0-9]+   { yylval.ival = atol(yytext); return VALOR; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.id = strdup(yytext); return ID; }

\'[A-Za-z0-9\-_=+\]\[.?~!@#$%^&*()<>]?\'   { return CARACTER; }
";" { return SEMICOLON; }

"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
","     { return COMMA; }


"//".*    {  }
[ \t\n]+               { /* Ignorar espacios en blanco */ }

.   { return ERROR; }
%%
char *token_names[] = {
 "nil", "VALOR","ID","TRUE","FALSE","IF","ELSE",
 "WHILE","FOR","TIPO","COMENTARIO","CARACTER",
 "FUNCTION","MULT","DIV","MODULO","SUMA","RESTA",
 "SEMICOLON","LPAREN","RPAREN","LBRACE","RBRACE","LBRACKET",
 "RBRACKET","ASSIGN","ERROR","RETURN","EXEC","COMMA",
 "AND","OR","NOT","LT","GT","EQ","NEQ","GEQ","LEQ",
 "START","END"
};
