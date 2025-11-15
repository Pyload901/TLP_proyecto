#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Simple lexer states
typedef enum {
    TOK_EOF, TOK_ID, TOK_INT, TOK_IF, TOK_START, TOK_END,
    TOK_EXEC, TOK_DELAY, TOK_LPAREN, TOK_RPAREN, TOK_SEMICOLON,
    TOK_EQUALS, TOK_ASSIGN, TOK_MAIN, TOK_RETURN
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    int value;
} Token;

typedef struct {
    const char *src;
    int pos;
    Token current;
    ArduinoCompiler *comp;
} Parser;

// Forward declarations
static void advance(Parser *p);
static int match(Parser *p, TokenType expected);
static void parse_program(Parser *p);
static void parse_main_function(Parser *p);
static void parse_statement(Parser *p);
static void parse_if_statement(Parser *p);
static void parse_var_declaration(Parser *p);
static void parse_exec_call(Parser *p);
static void parse_delay_call(Parser *p);

// Initialize parser
static void parser_init(Parser *p, const char *src, ArduinoCompiler *comp) {
    p->src = src;
    p->pos = 0;
    p->comp = comp;
    advance(p); // Load first token
}

// Simple lexer - advance to next token
static void advance(Parser *p) {
    // Skip whitespace and comments
    while (p->src[p->pos] && (isspace(p->src[p->pos]) || p->src[p->pos] == '/')) {
        if (p->src[p->pos] == '/' && p->src[p->pos + 1] == '/') {
            // Skip line comment
            while (p->src[p->pos] && p->src[p->pos] != '\n') p->pos++;
        } else {
            p->pos++;
        }
    }

    if (!p->src[p->pos]) {
        p->current.type = TOK_EOF;
        return;
    }

    int start = p->pos;
    char c = p->src[p->pos];

    // Single character tokens
    if (c == '(') { p->current.type = TOK_LPAREN; p->pos++; return; }
    if (c == ')') { p->current.type = TOK_RPAREN; p->pos++; return; }
    if (c == ';') { p->current.type = TOK_SEMICOLON; p->pos++; return; }
    if (c == '=') {
        if (p->src[p->pos + 1] == '=') {
            p->current.type = TOK_EQUALS; p->pos += 2; return;
        } else {
            p->current.type = TOK_ASSIGN; p->pos++; return;
        }
    }

    // Numbers
    if (isdigit(c)) {
        while (isdigit(p->src[p->pos])) p->pos++;
        int len = p->pos - start;
        p->current.text = malloc(len + 1);
        strncpy(p->current.text, p->src + start, len);
        p->current.text[len] = 0;
        p->current.value = atoi(p->current.text);
        p->current.type = TOK_INT;
        return;
    }

    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        while (isalnum(p->src[p->pos]) || p->src[p->pos] == '_') p->pos++;
        int len = p->pos - start;
        p->current.text = malloc(len + 1);
        strncpy(p->current.text, p->src + start, len);
        p->current.text[len] = 0;

        // Check for keywords
        if (strcmp(p->current.text, "if") == 0) p->current.type = TOK_IF;
        else if (strcmp(p->current.text, "start") == 0) p->current.type = TOK_START;
        else if (strcmp(p->current.text, "end") == 0) p->current.type = TOK_END;
        else if (strcmp(p->current.text, "exec") == 0) p->current.type = TOK_EXEC;
        else if (strcmp(p->current.text, "delay") == 0) p->current.type = TOK_DELAY;
        else if (strcmp(p->current.text, "main") == 0) p->current.type = TOK_MAIN;
        else if (strcmp(p->current.text, "return") == 0) p->current.type = TOK_RETURN;
        else if (strcmp(p->current.text, "int") == 0) p->current.type = TOK_ID; // Treat as identifier for now
        else p->current.type = TOK_ID;
        return;
    }

    // Unknown character, skip it
    p->pos++;
    advance(p);
}

static int match(Parser *p, TokenType expected) {
    if (p->current.type == expected) {
        advance(p);
        return 1;
    }
    return 0;
}

// Compile source code
int compile_source(ArduinoCompiler *c, const char *src) {
    c->src = src;
    c->error_count = 0;
    arduino_module_init(&c->module);

    Parser parser;
    parser_init(&parser, src, c);
    
    parse_program(&parser);
    
    return c->error_count == 0;
}

static void parse_program(Parser *p) {
    // Setup Arduino module structure
    ArduinoModule *module = &p->comp->module;
    
    // Initialize setup function
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SETUP_START);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SERIAL_BEGIN);
    arduino_chunk_emit(&module->setup_func.chunk, 9600);
    
    // Setup pins
    for (int pin = 3; pin <= 6; pin++) {
        arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_PIN_MODE);
        arduino_chunk_emit(&module->setup_func.chunk, pin);
        arduino_chunk_emit(&module->setup_func.chunk, ARD_OUTPUT);
    }
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_PIN_MODE);
    arduino_chunk_emit(&module->setup_func.chunk, 13);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OUTPUT);
    arduino_chunk_emit(&module->setup_func.chunk, ARD_OP_SETUP_END);
    
    // Initialize loop function
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_START);
    
    // Parse main function or statements
    while (p->current.type != TOK_EOF) {
        if (p->current.type == TOK_ID && p->current.text && strcmp(p->current.text, "int") == 0) {
            // Look ahead for "main"
            int saved_pos = p->pos;
            advance(p);
            if (p->current.type == TOK_MAIN) {
                parse_main_function(p);
            } else {
                // Restore and parse as variable declaration
                p->pos = saved_pos;
                advance(p);
                parse_var_declaration(p);
            }
        } else {
            parse_statement(p);
        }
    }
    
    arduino_chunk_emit(&module->loop_func.chunk, ARD_OP_LOOP_END);
}

static void parse_main_function(Parser *p) {
    // Already consumed "int main"
    match(p, TOK_LPAREN);
    match(p, TOK_RPAREN);
    
    // Parse function body
    while (p->current.type != TOK_EOF && p->current.type != TOK_RETURN) {
        parse_statement(p);
    }
    
    // Skip return statement
    if (p->current.type == TOK_RETURN) {
        advance(p);
        if (p->current.type == TOK_INT) advance(p); // Skip return value
        match(p, TOK_SEMICOLON);
    }
}

static void parse_statement(Parser *p) {
    switch (p->current.type) {
        case TOK_IF:
            parse_if_statement(p);
            break;
        case TOK_EXEC:
            parse_exec_call(p);
            break;
        case TOK_DELAY:
            parse_delay_call(p);
            break;
        case TOK_ID:
            if (p->current.text && strcmp(p->current.text, "int") == 0) {
                parse_var_declaration(p);
            } else {
                advance(p); // Skip unknown identifiers
            }
            break;
        default:
            advance(p); // Skip unknown tokens
            break;
    }
}

static void parse_if_statement(Parser *p) {
    advance(p); // consume 'if'
    match(p, TOK_LPAREN);
    
    // Parse condition (simple for now - just capture the text)
    char condition[256] = "";
    int cond_pos = 0;
    while (p->current.type != TOK_RPAREN && p->current.type != TOK_EOF) {
        if (p->current.text) {
            if (cond_pos > 0) {
                condition[cond_pos++] = ' ';
            }
            strcpy(condition + cond_pos, p->current.text);
            cond_pos += strlen(p->current.text);
        } else if (p->current.type == TOK_EQUALS) {
            strcpy(condition + cond_pos, " == ");
            cond_pos += 4;
        }
        advance(p);
    }
    condition[cond_pos] = 0;
    
    match(p, TOK_RPAREN);
    match(p, TOK_START);
    
    // Emit if start
    int str_idx = arduino_function_add_string(&p->comp->module.loop_func, condition);
    arduino_chunk_emit(&p->comp->module.loop_func.chunk, ARD_OP_IF_START);
    arduino_chunk_emit(&p->comp->module.loop_func.chunk, str_idx);
    
    // Parse statements until 'end'
    while (p->current.type != TOK_END && p->current.type != TOK_EOF) {
        parse_statement(p);
    }
    
    match(p, TOK_END);
    
    // Emit if end
    arduino_chunk_emit(&p->comp->module.loop_func.chunk, ARD_OP_IF_END);
}

static void parse_var_declaration(Parser *p) {
    advance(p); // consume 'int'
    
    if (p->current.type == TOK_ID) {
        char *var_name = p->current.text;
        advance(p);
        
        if (match(p, TOK_ASSIGN)) {
            int value = 0;
            if (p->current.type == TOK_INT) {
                value = p->current.value;
                advance(p);
            }
            
            // Emit variable declaration
            int str_idx = arduino_function_add_string(&p->comp->module.loop_func, var_name);
            arduino_chunk_emit(&p->comp->module.loop_func.chunk, ARD_OP_VAR_DECL);
            arduino_chunk_emit(&p->comp->module.loop_func.chunk, str_idx);
            arduino_chunk_emit(&p->comp->module.loop_func.chunk, value);
        }
        
        match(p, TOK_SEMICOLON);
    }
}

static void parse_exec_call(Parser *p) {
    advance(p); // consume 'exec'
    
    if (p->current.type == TOK_ID) {
        char *func_name = p->current.text;
        advance(p);
        
        match(p, TOK_LPAREN);
        match(p, TOK_RPAREN);
        match(p, TOK_SEMICOLON);
        
        // Emit function call
        int str_idx = arduino_function_add_string(&p->comp->module.loop_func, func_name);
        arduino_chunk_emit(&p->comp->module.loop_func.chunk, ARD_OP_CALL_CUSTOM);
        arduino_chunk_emit(&p->comp->module.loop_func.chunk, str_idx);
    }
}

static void parse_delay_call(Parser *p) {
    advance(p); // consume 'delay'
    
    match(p, TOK_LPAREN);
    
    if (p->current.type == TOK_INT) {
        int delay_ms = p->current.value;
        advance(p);
        
        // Emit delay
        arduino_chunk_emit(&p->comp->module.loop_func.chunk, ARD_OP_DELAY);
        arduino_chunk_emit(&p->comp->module.loop_func.chunk, delay_ms);
    }
    
    match(p, TOK_RPAREN);
    match(p, TOK_SEMICOLON);
}