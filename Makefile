CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L
LEX=flex
YACC=bison

all: parser

parser: parser.tab.c lex.yy.c ast.o
	$(CC) $(CFLAGS) -DYYDEBUG=1 -o $@ parser.tab.c lex.yy.c ast.o -lfl

parser.tab.c parser.tab.h: small_parser.y
	$(YACC) -Wall -Wcounterexamples -d -o parser.tab.c small_parser.y

lex.yy.c: lexer.l parser.tab.h
	$(LEX) -o lex.yy.c lexer.l

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

clean:
	rm -f parser parser.tab.c parser.tab.h lex.yy.c *.o
