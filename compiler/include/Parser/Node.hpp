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
        Return,
        If,
        Expression,
    };

    class Node
    {
    public:
        Node(Node *parent, bool is_allocator = false) :
            parent(parent), is_allocator(is_allocator) {}

        Node *get_parent() const { return parent; }
        Node *get_parent(NodeType type);
        virtual NodeType get_type() = 0;
        virtual void parse(Tokenizer &tk) = 0;

        // Symbol table functions
        vector<Symbol> lookup_all(string name) const;
        const Symbol &lookup(string name) const;
        const Symbol &lookup(string name, vector<DataType> params) const;
        DataConstruct *find_construct(string name);
        int allocate(int size);
        inline void push_symbol(Symbol symb) { table.push(symb); }
        inline int get_scope_size() const { return table.get_scope_size(); }

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
        bool is_allocator;

    };

    class NodeBlock : public Node
    {
    public:
        NodeBlock(Node *parent, bool is_allocator = false) :
            Node(parent, is_allocator) {}
        
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
