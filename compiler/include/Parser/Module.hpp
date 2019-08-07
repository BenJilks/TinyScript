#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeImport : public Node
    {
    public:
        NodeImport(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::Import; }
        virtual void parse(Tokenizer &tk);
        void symbolize();

    private:
        Token module;
        Token attr;

    };

    class NodeExtern : public Node
    {
    public:
        NodeExtern(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::Extern; }
        virtual void parse(Tokenizer &tk);

        inline Symbol get_symb() const { return symb; }

    private:
        Symbol symb;

    };

    class NodeModule : public NodeBlock
    {
    public:
        NodeModule(Node *parent = nullptr) :
            NodeBlock(parent) {}

        virtual NodeType get_type() { return NodeType::Module; }
        virtual void parse(Tokenizer &tk);

        inline void set_name(string name) { this->name = name; }
        inline string get_name() const { return name; }

    private:
        string name;

    };

}
