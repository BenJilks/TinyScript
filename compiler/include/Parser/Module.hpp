#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeImportFrom : public Node
    {
    public:
        NodeImportFrom(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::ImportFrom; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void symbolize();

    private:
        Token module;
        Token attr;

    };

    class NodeImport : public Node
    {
    public:
        NodeImport(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::Import; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void symbolize();

    private:
        Token module;

    };

    class NodeExtern : public Node
    {
    public:
        NodeExtern(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::Extern; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);

        inline Symbol get_symb() const { return symb; }

    private:
        Symbol symb;

    };

    class NodeModule : public NodeBlock
    {
    public:
        NodeModule(Node *parent = nullptr) :
            NodeBlock(parent),
            is_compiled_flag(false) {}

        virtual NodeType get_type() { return NodeType::Module; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);

        inline void set_name(string name) { this->name = name; }
        inline string get_name() const { return name; }
        inline void flag_compiled() { is_compiled_flag = true; }
        bool is_compiled() const;

    private:
        string name;
        bool is_compiled_flag;

    };

}
