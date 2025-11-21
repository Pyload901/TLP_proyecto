CC=gcc
CFLAGS=-Wall -std=c11 -Wextra -O2 -D_POSIX_C_SOURCE=200809L
LEX=flex

all: parser
parser: lexer.yy.c parser.o ast.o symtab.o semantic.o translator.o main.o
	$(CC) $(CFLAGS) -o $@ lexer.yy.c parser.o ast.o symtab.o semantic.o translator.o main.o -lfl
symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c -o $@ symtab.c
semantic.o: semantic.c semantic.h symtab.h ast.h
	$(CC) $(CFLAGS) -c -o $@ semantic.c
translator.o: translator.c translator.h ast.h
	$(CC) $(CFLAGS) -c -o $@ translator.c
lexer.yy.c: lexer.l tokens.h
	$(LEX) -o $@ lexer.l

parser.o: parser.c tokens.h ast.h
	$(CC) $(CFLAGS) -c -o $@ parser.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c -o $@ ast.c

clean:
	rm -f parser lexer.yy.c parser.o ast.o symtab.o semantic.o translator.o main.o