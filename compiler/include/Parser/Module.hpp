#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeModule;
    class NodeImportFrom : public Node
    {
    public:
        NodeImportFrom(Node *parent) :
            Node(parent) {}
        
        virtual NodeType get_type() { return NodeType::ImportFrom; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void symbolize();
        void register_types();
    
    private:
        Token module;
        Token attr;
        NodeModule *mod;

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

        void set_name(Token name);
        inline Token get_name() const { return name; }
        inline void flag_compiled() { is_compiled_flag = true; }
        bool is_compiled() const;

        void register_types();
        void register_functions();

    private:
        Token name;
        bool is_compiled_flag;

    };

}
