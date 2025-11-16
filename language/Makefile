CC=gcc
CFLAGS=-Wall -std=c11 -Wextra -O2 -D_POSIX_C_SOURCE=200809L
LEX=flex

all: parser
parser: lexer.yy.c parser.o ast.o
	$(CC) $(CFLAGS) -o $@ lexer.yy.c parser.o ast.o -lfl

lexer.yy.c: lexer.l tokens.h
	$(LEX) -o $@ lexer.l

parser.o: parser.c tokens.h ast.h
	$(CC) $(CFLAGS) -c -o $@ parser.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c -o $@ ast.c

clean:
	rm -f parser lexer.yy.c parser.o ast.o