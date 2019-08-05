#pragma once
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include <vector>
using std::vector;

namespace TinyScript
{

    enum class NodeType
    {
        Module,
        Function,
        Let,
        Assign,
        Expression,
    };

    class Node
    {
    public:
        Node(Node *parent) :
            parent(parent) {}

        Node *get_parent() const { return parent; }
        virtual NodeType get_type() = 0;
        virtual void parse(Tokenizer &tk) = 0;

        // Symbol table functions
        vector<Symbol> lookup_all(string name) const;
        const Symbol &lookup(string name) const;
        const Symbol &lookup(string name, vector<DataType> params) const;
        DataConstruct *find_construct(string name);
        void push_symbol(Symbol symb) { table.push(symb); }

    protected:
        SymbolTable table;

        template<typename T>
        T *parse_node(Tokenizer &tk)
        {
            T *node = new T(this);
            node->parse(tk);
            return node;
        }
        DataType parse_type(Tokenizer &tk);

    private:
        DataType parse_array_type(Tokenizer &tk, DataType of);

        Node *parent;

    };

    class NodeBlock : public Node
    {
    public:
        NodeBlock(Node *parent) :
            Node(parent) {}
        
        ~NodeBlock();

        inline int get_child_size() const { return children.size(); }
        inline Node *operator[] (int index) { return children[index]; }

    protected:
        inline void add_child(Node *child) { children.push_back(child); }

        template<typename T>
        void parse_child(Tokenizer &tk)
        {
            add_child(parse_node<T>(tk));
        }

    private:
        vector<Node*> children;

    };

}
