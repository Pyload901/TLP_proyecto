#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tokens.h"
#include "ast.h"

yystype yylval;
extern FILE *yyin; 

static int current_token;
static void next() {current_token = yylex();};
static void die(int token) {printf("Syntax error at token %d\n", token); exit(1);}
static bool accept(int token) {if (current_token == token) {next(); return true;} return false;}
static void expect(int token) {if (!accept(token)) die(token);}

Node* parse_program();
Node* parse_block();
Node* parse_instruction();
Node* parse_assign(char *id_name);
Node* parse_decla();
Node* parse_expression();
Node* parse_or_exp();
Node* parse_and_exp();
Node* parse_not_exp();
Node* parse_rel_exp();
Node* parse_sum_exp();
Node* parse_mul_exp();
Node* parse_term_exp();
Node* parse_factor_exp();

bool startsNonTerminal() {
    return current_token == TIPO || current_token == INTVAL || current_token == DOUBLEVAL ||
           current_token == TRUE || current_token == FALSE || current_token == LPAREN;
}

static char *consume_identifier(void) {
    if (current_token != ID) {
        die(ID);
    }
    char *name = yylval.id;
    next();
    return name;
}

Node* parse_program() {
    List* blocks = L_new();
    while (current_token != 0 && current_token != T_EOF) {
        L_push(blocks, parse_block());
    }
    return N_block(blocks);
}
Node* parse_block() {
    expect(START);
    List* instructions = L_new();
    while (startsNonTerminal()) {
        L_push(instructions, parse_instruction());
    }
    expect(END);
    return N_block(instructions);
}
Node* parse_instruction() {
    if (current_token == TIPO) {
        next();
    }
    char *id_name = consume_identifier();
    return parse_assign(id_name);
}
Node* parse_assign(char *id_name) {
    expect(ASSIGN);
    Node* expr = parse_expression();
    expect(SEMICOLON);
    Node* assign = N_assign(N_id(id_name), expr);
    free(id_name);
    return assign;
}
Node* parse_expression() {
    return parse_or_exp();
}
Node* parse_or_exp() {
    Node* left = parse_and_exp();
    while (accept(OR)) {
        Node* right = parse_and_exp();
        left = N_bin("OR", left, right);
    }
    return left;
}
Node* parse_and_exp() {
    Node* left = parse_not_exp();
    while (accept(AND)) {
        Node* right = parse_not_exp();
        left = N_bin("AND", left, right);
    }
    return left;
}
Node* parse_not_exp() {
    if (accept(NOT)) {
        Node* operand = parse_rel_exp();
        return N_unary("NOT", operand);
    }
    return parse_rel_exp();
}
Node* parse_rel_exp() {
    Node* left = parse_sum_exp();
    while (current_token == EQ || current_token == NEQ || current_token == LT ||
           current_token == GT || current_token == LEQ || current_token == GEQ) {
        int op = current_token;
        next();
        Node* right = parse_sum_exp();
        const char* op_str = (op == EQ) ? "EQ" : (op == NEQ) ? "NEQ" :
                             (op == LT) ? "LT" : (op == GT) ? "GT" :
                             (op == LEQ) ? "LEQ" : "GEQ";
        left = N_bin(op_str, left, right);
    }
    return left;
}

Node* parse_sum_exp() {
    Node* left = parse_mul_exp();
    while (current_token == ADD || current_token == MINUS) {
        int op = current_token;
        next();
        Node* right = parse_mul_exp();
        const char* op_str = (op == ADD) ? "ADD" : "MINUS";
        left = N_bin(op_str, left, right);
    }
    return left;
}
Node* parse_mul_exp() {
    Node* left = parse_term_exp();
    while (current_token == MULT || current_token == DIV) {
        int op = current_token;
        next();
        Node* right = parse_term_exp();
        const char* op_str = (op == MULT) ? "MULT" : "DIV";
        left = N_bin(op_str, left, right);
    }
    return left;
}
Node* parse_term_exp() {
    if (current_token == INTVAL) {
        long value = yylval.ival;
        next();
        return N_int(value);
    } else if (current_token == DOUBLEVAL) {
        double value = yylval.dval;
        next();
        return N_float(value);
    } else if (current_token == TRUE || current_token == FALSE) {
        bool value = yylval.bval;
        next();
        return N_bool(value);
    } else if (current_token == ID) {
        char* id_name = yylval.id;
        next();
        return N_id(id_name);
    } else {
        expect(LPAREN);
        Node* expr = parse_expression();
        expect(RPAREN);
        return expr;
    }
}

int main () {
    yyin = fopen("test.src", "r");
    if (!yyin) {
        perror("Failed to open input file");
        return 1;
    }
    next();
    Node* ast = parse_program();
    ast_print(ast, 0);
    fclose(yyin);
    return 0;
}