
all:
	flex -o lexer.c lexer.lex
	bison -d -o parser.c parser.y
	g++ parser.c lexer.c -o Compiler
	rm parser.c parser.h lexer.c
	./Compiler test.txt

