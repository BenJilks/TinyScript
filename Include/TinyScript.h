#ifndef TINY_SCRIPT
#define TINY_SCRIPT
#include <stdio.h>
#include "Tokens.h"

/* Error handling unit */
void Error(char* msg);
void Abort(char* msg);
void UnknownSymbolError(char* symbol);

/* IO unit */
void InitIO(char* in);
void CloseIO();

/* Read write functions */
void WriteLine(char* line);
char ReadChar();
void FileBack();
int IsFileEnd();

/* Define a write line function that allows for the inclusion of variables */
#define WriteLineVar(line, ...) \
{ \
	char msg[80]; \
	sprintf(msg, line, ##__VA_ARGS__); \
	WriteLine(msg); \
}

/* Output functions */
int GetLineCount();
void WriteOutputToFile(char* file_path);
char* GetOutput();
int GetOutputSize();

/* Define symbol types */
#define SYMBOL_GLOBAL_VARIABLE 0
#define SYMBOL_LOCAL_VARIABLE  1
#define SYMBOL_FUNCTION        2

/* Define data types */
#define DT_INT   0
#define DT_FLOAT 1
#define DT_CHAR  2
#define DT_VOID  3
#define DT_SIZE  4

/* Symbol table unit */
typedef struct Symbol
{
	/* Symbol data */
	char name[80];
	int type;

	/* Register data */
	int data_type;
	int location;
	int params[80];
	int param_size;
} Symbol;
Symbol* CreateSymbol(char* name, int type);
Symbol* FindSymbol(char* name);
void StartScope();
void RegisterSymbol(Symbol* symbol);
void PopScope();
char* GenerateLabel();
void Emit(char* label);

/* Tokenizer unit */
typedef struct Token
{
	char data[80];
	int id;
} Token;
Token NextToken();

/* General parser data */
Token look;
static void InitParser()
{
	look = NextToken();
}
static void Match(char* expect, int id)
{
	if (look.id == id)
	{
		look = NextToken();
		return;
	}

	char msg[80];
	sprintf(msg, "Expected '%s' got '%s' instead", expect, look.data);
	Abort(msg);
}

/* Memory unit */
void StartStackFrame(char* type);
Symbol* CreateGlobalVariable(char* name, char* type);
Symbol* CreateLocalVariable(char* name, char* type);
void CreateFunction(char* name, char* type, Symbol* params[80], int param_size);
void ParseCall(Symbol* symbol);
int ParseLoad();
int ParseStore(char* name, int type);
void CorrectTypeing(int aim, int type);
int GetReturnType();

/* Expression parsing unit */
int ParseExpression();

/* Declaration block parsing unit */
void ParseProgramBody();

/* Block parsing unity */
void ParseBlock();

#endif
