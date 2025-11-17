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
Node* parse_expression();
Node* parse_or_exp();
Node* parse_and_exp();
Node* parse_not_exp();
Node* parse_rel_exp();
Node* parse_sum_exp();
Node* parse_mul_exp();
Node* parse_term_exp();
Node* parse_decla();
Node* parse_for();
Node* parse_while();
Node* parse_if();
Node* parse_exec_fun();
List* parse_args();
Node* parse_return();
Node* parse_decla_fun();

bool startsNonTerminal() {
    return current_token == TIPO || current_token == INTVAL || current_token == DOUBLEVAL ||
           current_token == TRUE || current_token == FALSE || current_token == LPAREN || 
           current_token == ID || current_token == FOR || current_token == WHILE ||
           current_token == IF || current_token == EXEC || current_token == RETURN;
}

static char *consume_identifier(void) {
    if (current_token != ID) {
        die(ID);
    }
    char *name = yylval.id;
    next();
    return name;
}
static char* consume_type_name(void) {
    if (current_token != TIPO) {
        die(TIPO);
    }
    char *type_name = yylval.type;
    next();
    return type_name;
}

Node* parse_program() {
    List* blocks = L_new();
    while (current_token != 0 && current_token != T_EOF) {
        if (current_token == TIPO) {
            L_push(blocks, parse_decla_fun());
        } else if (current_token == START) {
            L_push(blocks, parse_block());
        }
    }
    return N_program(blocks);
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
    printf("next token: %d\n", current_token);
    if (current_token == TIPO) {
        char *type_name = consume_type_name();
        Node* decl =  parse_decla(type_name);
        expect(SEMICOLON);
        return decl;
    } else if (current_token == ID) {
        char *id_name = consume_identifier();
        Node *assign = parse_assign(id_name);
        expect(SEMICOLON);
        return assign;
    } else if (current_token == FOR) {
        accept(FOR);
        return parse_for();
    } else if (current_token == WHILE){
        accept(WHILE);
        return parse_while();
    } else if (current_token == IF){
        accept(IF);
        return parse_if();
    } else if (current_token == EXEC){
        accept(EXEC);
        Node* exec_node = parse_exec_fun();
        expect(SEMICOLON);
        return exec_node;
    } else if (current_token == RETURN){
        Node* return_node = parse_return();
        expect(SEMICOLON);
        return return_node;
    } else {
        die(0);
        return NULL;
    }
}
Node* parse_for(){
    expect(LPAREN);
    Node* start_expr = NULL;
    if (current_token == TIPO) {
        char *typename = consume_type_name();
        start_expr = parse_decla(typename);
    } else if (current_token == ID) {
        char *id_name = consume_identifier();
        start_expr = parse_assign(id_name);
    }
    expect(SEMICOLON);

    Node* cond_expr = parse_expression();
    expect(SEMICOLON);

    char *update_id = consume_identifier();
    Node* update_expr = parse_assign(update_id);
    
    expect(RPAREN);

    Node* body = parse_block();
    Node* for_node = N_for(start_expr, cond_expr, update_expr, body);
    return for_node;
}
Node* parse_while(){
    expect(LPAREN);
    Node* cond_expr = parse_expression();
    expect(RPAREN);
    Node* body = parse_block();
    Node* while_node = N_while(cond_expr, body);
    return while_node;
}
Node* parse_if(){
    expect(LPAREN);
    Node* cond_expr = parse_expression();
    expect(RPAREN);
    Node* then_branch = parse_block();
    Node* else_branch = NULL;
    if (accept(ELSE)) {
        else_branch = parse_block();
    }
    Node* if_node = N_if(cond_expr, then_branch, else_branch);
    return if_node;
}

Node* parse_exec_fun() {
    char *func_name = consume_identifier();
    expect(LPAREN);
    List* args = parse_args();
    expect(RPAREN);
    Node* exec_node = N_exec_fun(func_name, args);
    return exec_node;
}
List* parse_args() {
    List* args = L_new();
    L_push(args, parse_expression());
    while (accept(COMMA)) {
        L_push(args, parse_expression());
    }
    return args;
}
Node* parse_return() {
    expect(RETURN);
    Node* expr = parse_expression();
    return N_return(expr);
}
List* parse_decla_fun_args() {
    List* params = L_new();
    char* param_type = consume_type_name();
    char* param_name = consume_identifier();
    Node* param_node = N_decla(param_type, param_name, NULL);
    L_push(params, param_node);
    while (accept(COMMA)) {
        param_type = consume_type_name();
        param_name = consume_identifier();
        param_node = N_decla(param_type, param_name, NULL);
        L_push(params, param_node);
    }
    return params;
}
Node* parse_decla_fun() {
    char* return_type = consume_type_name();
    printf("next token in decla_fun: %d\n", current_token);
    expect(FUNCTION);
    char *func_name = consume_identifier();
    expect(LPAREN);
    List* params = parse_decla_fun_args();
    expect(RPAREN);
    Node* body = parse_block();
    Node* decla_fun_node = N_decla_fun(func_name, params, return_type, body);
    return decla_fun_node;
}

Node* parse_decla(char *typename) {
    char *id_name = consume_identifier();
    if (current_token == SEMICOLON) {
        return N_decla(typename, id_name, NULL);
    } else{
        expect(ASSIGN);
        Node* initial_value = parse_expression();
        return N_decla(typename, id_name, initial_value);
    }
}
Node* parse_assign(char *id_name) {
    expect(ASSIGN);
    Node* expr = parse_expression();
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