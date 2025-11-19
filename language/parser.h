#pragma once
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
Node* parse_program();

static int current_token;
static void next() {current_token = yylex();};
static void die(int token) {printf("Syntax error at token %d\n", token); exit(1);}
static bool accept(int token) {if (current_token == token) {next(); return true;} return false;}
static void expect(int token) {if (!accept(token)) die(token);}

