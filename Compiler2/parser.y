
%{
#include <stdio.h>
#include <ctype.h>

#define YYDEBUG 1

extern int yylex();
extern FILE *yyin;

extern int yylineno;
extern char *yytext;

void yyerror(const char *);

struct Node;
Node *out;

%}

%code requires {
	#include <string>
	using namespace std;

	enum class PrimType
	{
		Int,
		Float,
		String,
		Bool,
		Char,
	};

	struct Literal
	{
		union
		{
			int i;
			float f;
			bool b;
			char c;
			string *s;
		};
		PrimType type;
	};

	struct Node
	{
		Node(Node *left, char op, Node *right) :
			left(left), right(right), op(op), val(NULL) {}

		Node(Literal *val) :
			left(NULL), right(NULL), val(val) {}

		~Node()
		{
			if (left != NULL && right != NULL)
			{
				delete left;
				delete right;
			}

			if (val != NULL)
				delete val;
		}

		Node *left;
		Node *right;
		char op;
		Literal *val;
	};
}

%union {
	Literal*	lit_val;
	string*		str_val;
	Node*		node_val;
}

%start program

%token <lit_val> 	INT_LIT
%token <lit_val> 	FLOAT_LIT
%token <lit_val> 	BOOL_LIT
%token <lit_val> 	CHAR_LIT
%token <lit_val> 	STRING_LIT
%token <str_val>	IDENT
%token 			FUNC
%type <node_val> 	expr
%type <lit_val> 	term
%left '+' '-'
%left '*' '/'

%%

program:
	expr		{ out = $1; }

expr:	
	term		{ $$ = new Node($1); }
|	expr '+' expr 	{ $$ = new Node($1, '+', $3); }
|	expr '-' expr 	{ $$ = new Node($1, '-', $3); }
|	expr '*' expr 	{ $$ = new Node($1, '*', $3); }
|	expr '/' expr 	{ $$ = new Node($1, '/', $3); }
|	'(' expr ')'	{ $$ = $2; }

term:
	INT_LIT
|	FLOAT_LIT
|	BOOL_LIT
|	CHAR_LIT
|	STRING_LIT

%%

void yyerror(char const *s)
{
	fprintf(stderr, "Error line %i '%s': %s\n", 
		yylineno, yytext, s);
}

void print_lit(Literal *lit)
{
	switch(lit->type)
	{
		case PrimType::Int: printf("%i ", lit->i); break;
		case PrimType::Float: printf("%.6g ", lit->f); break;
		case PrimType::Bool: printf("%s ", lit->b ? "true" : "false"); break;
		case PrimType::Char: printf("%c ", lit->c); break;
		case PrimType::String: printf("%s ", lit->s->c_str()); break;
	}
}

void print_node(Node *node)
{
	if (node->left == NULL && node->right == NULL)
	{
		print_lit(node->val);
		return;
	}

	print_node(node->left);
	print_node(node->right);
	printf("%c ", node->op);
}

int main(int argc, char *argv[])
{
	yydebug = 0;
	yyin = fopen(argv[1], "r");
	if (yyparse() != 0)
		return 1;
	
	print_node(out);
	printf("\n");	
	delete out;
}
