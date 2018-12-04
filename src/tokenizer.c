#include "tokenizer.h"
#include <ctype.h>
#include <string.h>

// Create a new tokenizer from file path and return it
struct Tokenizer open_tokenizer(const char *file_path)
{
	struct Tokenizer tk;
	tk.file = fopen(file_path, "r");
	tk.buffer = '\0';
	tk.look = next(&tk);
    tk.has_error = 0;
    tk.line_no = 0;
	return tk;
}

static char read_char(struct Tokenizer *tk)
{
	char c;
	if (tk->buffer != '\0')
	{
		c = tk->buffer;
		tk->buffer = '\0';
		return c;
	}

	c = fgetc(tk->file);
	if (c == '\n')
		tk->line_no++;
	return c;
}

static int check_keyword(char *data)
{
	if (!strcmp(data, "func")) return TK_FUNC;
	else if(!strcmp(data, "class")) return TK_CLASS;
    else if(!strcmp(data, "return")) return TK_RETURN;
    else if(!strcmp(data, "import")) return TK_IMPORT;
    else if(!strcmp(data, "within")) return TK_WITHIN;
	else if(!strcmp(data, "if")) return TK_IF;
    else if(!strcmp(data, "else")) return TK_ELSE;
    else if(!strcmp(data, "elif")) return TK_ELSE_IF;
    else if(!strcmp(data, "while")) return TK_WHILE;
	else if(!strcmp(data, "for")) return TK_FOR;
    else if(!strcmp(data, "break")) return TK_BREAK;
    else if(!strcmp(data, "continue")) return TK_CONTINUE;
    else if(!strcmp(data, "to")) return TK_TO;
	else if(!strcmp(data, "true")) return TK_BOOL;
	else if(!strcmp(data, "false")) return TK_BOOL;
	return TK_NAME;
}

// Read a name token from the tokenizer
static struct Token read_name(struct Tokenizer *tk, char first)
{
	struct Token t;
	int i = 0;
	char c = first;
	while (isalpha(c) || isdigit(c) || c == '_')
	{
		t.data[i++] = c;
		c = read_char(tk);
	}
	tk->buffer = c;

	t.data[i] = '\0';
	t.type = check_keyword(t.data);
	return t;
}

// Read the next number in the file (int of float)
static struct Token read_number(struct Tokenizer *tk, char first)
{
	struct Token t;

	int i = 0, point_count = 0;
	char c = first;
	while (isdigit(c) || c == '.')
	{
		t.data[i++] = c;
		if (c == '.')
			point_count++;
		c = read_char(tk);
	}
	tk->buffer = c;
	t.data[i] = '\0';

	// Check number format
	if (point_count > 1)
		printf("Error: Invalid float '%s'\n",
			t.data);
	else if (point_count == 1)
		t.type = TK_FLOAT;
	else
		t.type = TK_INT;
	return t;
}

static struct Token read_string(struct Tokenizer *tk)
{
	struct Token t;
	t.type = TK_STRING;

	int i = 0;
	char c;
	while ((c = read_char(tk)) != '"')
		t.data[i++] = c;
	t.data[i] = '\0';
	return t;
}

static struct Token read_char_data(struct Tokenizer *tk)
{
	struct Token t;
	t.type = TK_CHAR;
	t.data[0] = read_char(tk);
	t.data[1] = '\0';
	read_char(tk);
	return t;
}

static struct Token read_multi_char(struct Tokenizer *tk, struct Token t)
{
    char next = read_char(tk);
    if (next == t.data[0])
    {
        t.data[1] = next;
        t.data[2] = '\0';
        t.type = TK_EQUAL;
        return t;
    }

    tk->buffer = next;
    return t;
}

static void read_comment(struct Tokenizer *tk)
{
	while (read_char(tk) != '\n')
		continue;
}

// Read the next token in the file and return it
struct Token next(struct Tokenizer *tk)
{
	char c;
	while ((c = read_char(tk)) != EOF)
	{
		// Skip white space
		if (isspace(c))
			continue;
	
		struct Token t;
		t.data[0] = c;
		t.data[1] = '\0';
		t.type = -1;

		// Check for single char tokens
		switch(c)
		{
			case '+': t.type = TK_ADD; break;
			case '-': t.type = TK_SUB; break;
			case '*': t.type = TK_MUL; break;
			case '/': t.type = TK_DIV; break;
            case '>': t.type = TK_MORE_THAN; break;
            case '<': t.type = TK_LESS_THAN; break;
			case '.': t.type = TK_IN; break;
            case ':': t.type = TK_OF; break;
			case ',': t.type = TK_NEXT; break;
			case '(': t.type = TK_OPEN_ARG; break;
			case ')': t.type = TK_CLOSE_ARG; break;
			case '{': t.type = TK_OPEN_BLOCK; break;
			case '}': t.type = TK_CLOSE_BLOCK; break;
			case '#': read_comment(tk); continue;
			case '=': t.type = TK_ASSIGN; return read_multi_char(tk, t);
		}

		// If it found a single char token, then return is
		if (t.type != -1)
			return t;

		// Otherwise, check for multi-tokens
		if (isalpha(c)) return read_name(tk, c);
		if (isdigit(c)) return read_number(tk, c);
		if (c == '"') return read_string(tk);
		if (c == '\'') return read_char_data(tk);

		// Could not decode this token
		F_ERROR(tk, "Unexpected token '%c'", c);
	}

	struct Token eof_t;
	strcpy(eof_t.data, "EOF");
	eof_t.type = -1;
	return eof_t;
}

// Check if the current look token matches what is expected, then goto the next look
struct Token match(struct Tokenizer *tk, const char *name, int type)
{
	if (tk->look.type != type)
	{
		F_ERROR(tk, "Expected token '%s', got '%s' instead",
			name, tk->look.data);
	}

	struct Token look = tk->look;
	tk->look = next(tk);
	return look;
}

void error(struct Tokenizer *tk, const char *msg)
{
	printf("Error at line %i: %s\n", tk->line_no, msg);
    tk->has_error = 1;
}

void close_tokenizer(struct Tokenizer tk)
{
	fclose(tk.file);
}
