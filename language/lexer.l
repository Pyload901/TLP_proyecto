%option noyywrap noinput nounput
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokens.h"
extern yystype yylval;
%}

%%
"start"              { return START; }
"end"              { return END; }
"true"              { yylval.bval = true; return TRUE; }
"false"             { yylval.bval = false; return FALSE; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"for"               { return FOR; }
"bool"              { yylval.type = "bool"; return TIPO; }
"char"              { yylval.type = "char"; return TIPO; }
"int"               { yylval.type = "int"; return TIPO; }
"float"             { yylval.type = "float"; return TIPO; }
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
";" { return SEMICOLON; }

"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
","     { return COMMA; }

[-+]?[0-9]+ { yylval.ival = atol(yytext); return INTVAL; }
[-+]?[0-9]+\.[0-9]+ { yylval.dval = atof(yytext); return DOUBLEVAL; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.id = strdup(yytext); return ID; }

\'[A-Za-z0-9\-_=+\]\[.?~!@#$%^&*()<>]?\'   { yylval.cval = yytext[1]; return CARACTER; }


"//".*    {  }
[ \t\n]+               { /* Ignorar espacios en blanco */ }

.   { return ERROR; }
%%
char *token_names[] = {
 "nil", "INTVAL","ID","TRUE","FALSE","IF","ELSE",
 "WHILE","FOR","TIPO","COMENTARIO","CARACTER",
 "FUNCTION","MULT","DIV","MODULO","SUMA","RESTA",
 "SEMICOLON","LPAREN","RPAREN","LBRACE","RBRACE","LBRACKET",
 "RBRACKET","ASSIGN","ERROR","RETURN","EXEC","COMMA",
 "AND","OR","NOT","LT","GT","EQ","NEQ","GEQ","LEQ",
 "START","END", "DOUBLEVAL"
};
