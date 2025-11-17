#pragma once
#include <stdio.h>
#include <stdbool.h>
typedef struct Node Node;
typedef struct List List;
enum {
    T_EOF = 0,
    ID = 256,
    INTVAL = 257,
    TRUE = 258,
    FALSE = 259,
    IF = 260,
    ELSE = 261,
    WHILE = 262,
    FOR = 263,
    TIPO = 264,
    COMENTARIO = 265,
    CARACTER = 266,
    FUNCTION = 267,
    MULT = 268,
    DIV = 269,
    MODULO = 270,
    ADD = 271,
    MINUS = 272,
    SEMICOLON = 273,
    LPAREN = 274,
    RPAREN = 275,
    LBRACE = 276,
    RBRACE = 277,
    LBRACKET = 278,
    RBRACKET = 279,
    ASSIGN = 280,
    ERROR = 281,
    RETURN = 282,
    COMMA = 283,
    AND = 284,
    OR = 285,
    NOT = 286,
    LT = 287,
    GT = 288,
    EQ = 289,
    NEQ = 290,
    GEQ = 291,
    LEQ = 292,
    START = 293,
    END = 294,
    DOUBLEVAL = 295,
    EXEC = 296,
};

typedef union {
  char *type;
  long  ival;
  double dval;
  bool  bval;
  char cval;
  char *id;
} yystype;

extern yystype yylval;
int   yylex(void);
extern FILE *yyin; 