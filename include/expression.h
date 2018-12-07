#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "tokenizer.h"
#include "compiler.h"

struct Node
{
    int type;
    struct Node *left;
    struct Node *right;
    char is_op;

    union
    {
        int i;
        float f;
        char c, b;
        char s[80];
    };
    char mod_func[80];
    struct Node *args[80];
    int arg_size, has_from;
};

struct Node *parse_expression(struct Tokenizer *tk);
struct SymbolType *compile_node(struct Module *mod, struct Node *node, char addr_mode);
void compile_expression(struct Module *mod, struct Node *node);
void delete_node(struct Node *node);

#endif // EXPRESSION_H
