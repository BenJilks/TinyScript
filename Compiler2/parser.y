
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

	struct Node
	{
		Node(Node *left, char op, Node *right) :
			left(left), right(right), op(op) {}

		Node(int val) :
			left(NULL), right(NULL), val(val) {}

		~Node()
		{
			if (left != NULL && right != NULL)
			{
				delete left;
				delete right;
			}
		}

		Node *left;
		Node *right;
		char op;
		int val;
	};
}

%union {
	int	int_val;
	string*	str_val;
	Node*	node_val;
}

%start program

%token <int_val> 	NUM
%token <str_val>	IDENT
%token 			FUNC
%type <node_val> 	expr
%left '+' '-'
%left '*' '/'

%%

program:
	expr		{ out = $1; }

expr:	
	NUM		{ $$ = new Node($1); }
|	expr '+' expr 	{ $$ = new Node($1, '+', $3); }
|	expr '-' expr 	{ $$ = new Node($1, '-', $3); }
|	expr '*' expr 	{ $$ = new Node($1, '*', $3); }
|	expr '/' expr 	{ $$ = new Node($1, '/', $3); }
|	'(' expr ')'	{ $$ = $2; }

%%

void yyerror(char const *s)
{
	fprintf(stderr, "Error line %i '%s': %s\n", 
		yylineno, yytext, s);
}

int main(int argc, char *argv[])
{
	yydebug = 0;
	yyin = fopen(argv[1], "r");
	if (yyparse() != 0)
		return 1;
	
	printf("%i\n", out->right->right->val);
	delete out;
}

