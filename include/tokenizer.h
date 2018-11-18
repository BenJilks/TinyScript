#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

enum TokenType
{
	TK_NAME,
	TK_FUNC,
    TK_RETURN,
    TK_IMPORT,
    TK_WITHIN,
	TK_IF,
	TK_FOR,
    TK_OF,

	TK_INT,
	TK_FLOAT,
	TK_STRING,
	TK_CHAR,
	TK_BOOL,

	TK_ADD,
	TK_SUB,
	TK_MUL,
	TK_DIV,
    TK_MORE_THAN,
    TK_LESS_THAN,
    TK_EQUAL,
	TK_IN,
	TK_ASSIGN,
	TK_NEXT,
	TK_OPEN_ARG,
	TK_CLOSE_ARG,
	TK_OPEN_BLOCK,
	TK_CLOSE_BLOCK
};

struct Token
{
	char data[80];
	int type;
};

struct Tokenizer
{
	FILE *file;
	char buffer;
	struct Token look;
	int line_no;
    int has_error;
};

#define F_ERROR(tk, ...) \
{ \
	char msg[80];\
	sprintf(msg, __VA_ARGS__); \
	error(tk, msg); \
}

struct Tokenizer open_tokenizer(const char *file_path);
void close_tokenizer(struct Tokenizer tk);

struct Token next(struct Tokenizer *tk);
struct Token match(struct Tokenizer *tk, const char *name, int type);
void error(struct Tokenizer *tk, const char *msg);

#endif // TOKENIZER_H
