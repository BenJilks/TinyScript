#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Logger.hpp"
using std::string;
using std::vector;
using std::shared_ptr;

#define DATATYPE_REF    0b1000
#define DATATYPE_ARRAY  0b0100
#define DATATYPE_AUTO   0b0010
#define DATATYPE_NULL   0b0001

#define SYMBOL_FUNCTION             0b10000000000
#define SYMBOL_LOCAL                0b01000000000
#define SYMBOL_ARG                  0b00100000000
#define SYMBOL_GLOBAL               0b00010000000
#define SYMBOL_CONST                0b00001000000
#define SYMBOL_PRIVATE              0b00000100000
#define SYMBOL_READONLY             0b00000010000
#define SYMBOL_EXTERNAL             0b00000001000
#define SYMBOL_TEMPLATE             0b00000000100
#define SYMBOL_MODULE               0b00000000010
#define SYMBOL_NULL                 0b00000000001

namespace TinyScript
{

    struct DataConstruct;
    struct DataType
    {
        DataConstruct *construct;
        int flags;

        int array_size;
        shared_ptr<DataType> sub_type;

        static int find_size(const DataType &type);
        static bool equal(const DataType &a, const DataType &b);
        static bool can_cast_to(const DataType &from, const DataType &to, bool &warning);
        static string printout(const DataType &type);
    };

    class Node;
    struct Symbol
    {
        Symbol() {}
        
        Symbol(string name, DataType type, int flags, int location, Node *parent) :
            name(name), type(type), flags(flags), location(location), parent(parent) {}

        static string printout(const Symbol &symb);

        string name;
        DataType type;
        int flags;

        vector<DataType> params;
        int location;
        Node *parent;
    };

    class SymbolTable
    {
    public:
        SymbolTable() :
            allocator(0),
            scope_size(0) {}
        
        void push(Symbol symb);
        void push_all(vector<Symbol> symbs);
        void patch_params(vector<DataType> params);
        
        vector<Symbol> lookup_all(string name) const;
        const Symbol &lookup(string name) const;
        const Symbol &lookup(string name, vector<DataType> params) const;
        
        vector<Symbol> find_externals() const;
        DataConstruct *find_construct(string name);
        DataConstruct *create_construct(string name);
        inline void add_construct(DataConstruct *construct) { external_constructs.push_back(construct); }

        void new_allocation_space();
        int allocate(int size);
        int get_scope_size() const;

        static bool is_null(const Symbol &symb);
        static const Symbol &get_null() { return null_symbol; }

    private:
        vector<Symbol> symbols;
        vector<DataConstruct> constructs;
        vector<DataConstruct*> external_constructs;
        int scope_size;
        int allocator;
        static Symbol null_symbol;

    };

    struct DataConstruct
    {
        string name;
        int size;
        Node *parent;
        bool is_complete;
    };

    class PrimTypes
    {
    public:
        PrimTypes() {}

        static inline DataConstruct* type_int() { return &dt_int; }
        static inline DataConstruct* type_float() { return &dt_float; }
        static inline DataConstruct* type_char() { return &dt_char; }
        static inline DataConstruct* type_bool() { return &dt_bool; }
        static inline DataConstruct* type_null() { return &dt_null; }

    private:
        static DataConstruct dt_int;
        static DataConstruct dt_float;
        static DataConstruct dt_char;
        static DataConstruct dt_bool;
        static DataConstruct dt_null;
    
    };

}
