#include "TinyScript.h"
#include <stdio.h>
#include <memory.h>

/* Creates a new token struct */
Token CreateToken(char* data, int data_size, int id)
{
	Token token;
	memcpy(token.data, data, data_size);
	token.data[data_size] = '\0';

	token.id = id;
	return token;
}

/* Checks for white space */
int IsWhiteSpace(char c)
{
	return c == ' ' || c == '	' || c == '\n' || c == '\r';
}

/* Checks for a numeric value */
int IsNumeric(char c)
{
	return c >= '0' && c <= '9';
}

/* Checks for a letter */
int IsLetter(char c)
{
	return (c >= 'a' && c <= 'z') || 
		(c >= 'A' && c <= 'Z');
}

/* Skips over white space */
void SkipWhiteSpace()
{
	while (IsWhiteSpace(ReadChar()))
		continue;
	FileBack();
}

/* Returns if the token is a single char token */
int IsSingleCharToken(char c)
{
	switch(c)
	{
		case '*': return TOKEN_MUL;
		case '/': return TOKEN_DIV;
		case '+': return TOKEN_ADD;
		case '-': return TOKEN_SUB;
		case '>': return TOKEN_MORE;
		case '<': return TOKEN_LESS;
		case '!': return TOKEN_NOT;
		case '(': return TOKEN_OPEN_ARG;
		case ')': return TOKEN_CLOSE_ARG;
		case '=': return TOKEN_ASSIGN;
		case ',': return TOKEN_LIST;
		case ';': return TOKEN_LINE_END;
		case '{': return TOKEN_OPEN_BLOCK;
		case '}': return TOKEN_CLOSE_BLOCK;
		case '[': return TOKEN_OPEN_ARRAY;
		case ']': return TOKEN_CLOSE_ARRAY;
	}
	return TOKEN_NULL;
}

/* Returns if the token is a double char token */
int IsDoubleCharToken(char c1, char c2)
{
	switch(c2)
	{
		case '=':
			switch(c1)
			{
				case '>': return TOKEN_MORE_EQUAL;
				case '<': return TOKEN_LESS_EQUAL;
				case '=': return TOKEN_EQUAL;
				case '!': return TOKEN_NOT_EQUAL;
			}
			break;
		case '&':
			switch(c1)
			{
				case '&': return TOKEN_AND;
			}
		case '|':
			switch(c1)
			{
				case '|': return TOKEN_OR;
			}
	}
	return TOKEN_NULL;
}

/* Returns if a buffer contains a keyword */
int IsKeyword(char buffer[80], int buffer_size)
{
	buffer[buffer_size] = '\0';
	if (!strcmp(buffer, "declare")) return TOKEN_DECLARE;
	else if (!strcmp(buffer, "if")) return TOKEN_IF;
	else if (!strcmp(buffer, "while")) return TOKEN_WHILE;
	else if (!strcmp(buffer, "include")) return TOKEN_INCLUDE;
	else if (!strcmp(buffer, "program")) return TOKEN_PROGRAM;
	else if (!strcmp(buffer, "function")) return TOKEN_FUNCTION;
	else if (!strcmp(buffer, "new")) return TOKEN_NEW;
	else if (!strcmp(buffer, "return")) return TOKEN_RETURN;
	return TOKEN_NULL;
}

/* Returns if the token is an identifier */
int CheckIdentifier(char buffer[80], int buffer_size)
{
	int i;

	/* If the size of the buffer is zero, then it 
	does not contain an identifier, so return false */
	if (buffer_size == 0) return 0;

	/* If the first char is not a letter, then return false */
	if (!IsLetter(buffer[0])) return 0;

	/* If any of the rest of the letters are not letters 
	or numbers, then return false */
	for (i = 1; i < buffer_size; i++)
		if (!(IsLetter(buffer[i]) || IsNumeric(buffer[i])))
			return 0;

	/* Otherwise, return true */
	return 1;
}

/* Returns if the token is a number */
int IsNumber(char buffer[80], int buffer_size)
{
	int i;

	/* A number cannot contains zero chars, to return false */
	if (buffer_size == 0) return 0;

	/* A number can only contains numeric values, so if any are not,
	then return false */
	for (i = 0; i < buffer_size; i++)
		if (!IsNumeric(buffer[i]))
			return 0;

	/* Otherwise, return true */
	return 1;
}

/* Returns if the token is a float */
int IsFloat(char buffer[80], int buffer_size)
{
	int i;

	/* A number cannot contains zero chars, to return false */
	if (buffer_size == 0) return 0;
	
	int point_count = 0;
	for (i = 0; i < buffer_size; i++)
	{
		char c = buffer[i];
		if ((c == 'f' || c == 'F') && i == buffer_size - 1) return 1;
		if (!IsNumeric(c) && c != '.') return 0;
		if (c == '.') point_count++;
	}
	
	if (point_count != 1)
		return 0;
	return 1;
}

/* Returns if the token is a condition */
int IsCondition(char buffer[80], int buffer_size)
{
	buffer[buffer_size] = '\0';
	if (!strcmp(buffer, "true") || !strcmp(buffer, "false"))
		return 1;
	return 0;
}

/* Skips any comments if they are found */
int SkipComments(char c, char c2)
{
	/* Check for multi line comment and skip it */
	if (c == '/' && c2 == '*')
	{
		while (!(c == '*' && c2 == '/') && !IsFileEnd())
		{
			c = c2;
			c2 = ReadChar();
		}
		return 1;
	}

	/* Check for single line comments and skip them */
	if (c == '/' && c2 == '/')
	{
		while (c != '\n' && c != '\r' && !IsFileEnd())
			c = ReadChar();
		return 1;
	}

	return 0;
}

Token NextToken()
{
	SkipWhiteSpace();
	char c = ReadChar();
	char c2 = ReadChar();

	/* Skip the comments */
	while (SkipComments(c, c2))
	{
		SkipWhiteSpace();
		c = ReadChar();
		c2 = ReadChar();
	}

	/* Check double char token */
	int token_id = IsDoubleCharToken(c, c2);
	if (token_id != TOKEN_NULL)
		return CreateToken((char[2]){c, c2}, 2, token_id);
	FileBack();

	/* Check single char token */
	token_id = IsSingleCharToken(c);
	if (token_id != TOKEN_NULL)
		return CreateToken(&c, 1, token_id);

	char buffer[80];
	int buffer_pointer = 0;
	while (!IsSingleCharToken(c) && !IsWhiteSpace(c) && !IsFileEnd())
	{
		buffer[buffer_pointer++] = c;
		c = ReadChar();
	}
	FileBack();
	SkipWhiteSpace();

	/* Check for keywords */
	token_id = IsKeyword(buffer, buffer_pointer);
	if (token_id != TOKEN_NULL)
		return CreateToken(buffer, buffer_pointer, token_id);

	/* Identify the token inside the buffer */
	if (IsCondition(buffer, buffer_pointer)) token_id = TOKEN_BOOL;
	else if (CheckIdentifier(buffer, buffer_pointer)) token_id = TOKEN_IDENTIFIER;
	else if (IsFloat(buffer, buffer_pointer)) token_id = TOKEN_FLOAT;	
	else if (IsNumber(buffer, buffer_pointer)) token_id = TOKEN_NUMBER;	
	if (token_id != TOKEN_NULL) return CreateToken(buffer, buffer_pointer, token_id);

	if (!IsFileEnd())
	{
		char msg[80];

		buffer[buffer_pointer] = '\0';
		sprintf(msg, "Could not recognize token '%s'", buffer);
		Abort(msg);
	}

	Token token;
	memcpy(token.data, "EoF", 4);
	token.id = TOKEN_NULL;
	return token;
}
