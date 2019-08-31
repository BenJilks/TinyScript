#pragma once
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include <vector>
using std::vector;

namespace TinyScript
{

    enum class NodeType
    {
        Program,
        Module,
        Import,
        ImportFrom,
        Extern,
        Class,
        Function,
        Let,
        Assign,
        Return,
        If,
        For,
        While,
        Expression,
        DataType,
    };

    class Node
    {
    public:
        Node(Node *parent, bool is_allocator = false) :
            parent(parent), is_allocator(is_allocator) {}

        virtual ~Node() {}

        Node *get_parent() const { return parent; }
        Node *get_parent(NodeType type);
        string get_prefix() const;
        virtual NodeType get_type() = 0;
        virtual void parse(Tokenizer &tk) = 0;
        virtual Node *copy(Node *parent) = 0;

        // Symbol table functions
        vector<Symbol> lookup_all(string name) const;
        const Symbol &lookup(string name) const;
        const Symbol &lookup(string name, vector<DataType> params) const;
        DataConstruct *find_construct(string name);
        int allocate(int size);
        void push_symbol(Symbol symb);
        inline int get_scope_size() const { return table.get_scope_size(); }
        inline DataConstruct *create_construct(string name) { return table.create_construct(name); }
        inline void add_construct(DataConstruct *construct) { table.add_construct(construct); }

    protected:
        void copy_node(Node *other);
        SymbolTable table;
        string prefix;

        template<typename T>
        T *parse_node(Tokenizer &tk)
        {
            T *node = new T(this);
            node->parse(tk);
            return node;
        }

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

        virtual ~NodeBlock();

        inline int get_child_size() const { return children.size(); }
        inline void add_child(Node *child) { children.push_back(child); }
        inline Node *operator[] (int index) { return children[index]; }

    protected:
        void copy_block(NodeBlock *to);
        vector<Node*> children;

        template<typename T>
        void parse_child(Tokenizer &tk)
        {
            add_child(parse_node<T>(tk));
        }

    };

}
