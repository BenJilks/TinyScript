#include "Parser/Symbol.hpp"
#include "Parser/Node.hpp"
using namespace TinyScript;

#define REF_SIZE 4

// Define prim type info
DataConstruct PrimTypes::dt_int = { "int", 4, nullptr };
DataConstruct PrimTypes::dt_float = { "float", 4, nullptr };
DataConstruct PrimTypes::dt_char = { "char", 1, nullptr };
DataConstruct PrimTypes::dt_bool = { "bool", 1, nullptr };
DataConstruct PrimTypes::dt_null = { "null", 0, nullptr };

Symbol SymbolTable::null_symbol = Symbol("null", { PrimTypes::type_null(), 0, 0 }, SYMBOL_NULL, 0, nullptr);

int DataType::find_size(const DataType &type)
{
    // If the type is a referance, it's of a constant size
    if (type.flags & DATATYPE_REF)
        return REF_SIZE;
    
    // If it's an array, then return the total array size
    if (type.flags & DATATYPE_ARRAY)
        return find_size(*type.sub_type) * type.array_size;
    
    // Otherwise, return the type size
    if (type.construct == nullptr)
        return 0;
    return type.construct->size;
}

#include <iostream>

bool DataType::equal(const DataType &a, const DataType &b)
{
    if (a.construct == b.construct && a.flags == b.flags)
    {
        if (a.flags & DATATYPE_ARRAY && 
            a.array_size != b.array_size)
        {
            return false;
        }
        
        if (a.flags & (DATATYPE_REF | DATATYPE_ARRAY) && 
            !DataType::equal(*a.sub_type, *b.sub_type))
        {
            return false;
        }
        
        return true;
    }

    return false;
}

void SymbolTable::push(Symbol symb)
{
    symbols.push_back(symb);
}

void SymbolTable::push_all(vector<Symbol> symbs)
{
    for (Symbol symb : symbs)
        symbols.push_back(symb);
}

string DataType::printout(const DataType &type)
{
    if (type.flags & DATATYPE_AUTO)
        return "auto";
    
    string type_name = type.construct->name;
    if (type.flags & DATATYPE_ARRAY)
    {
        type_name = printout(*type.sub_type) + 
            " array[" + std::to_string(type.array_size) + "]";
    }

    if (type.flags & DATATYPE_REF)
    {
        type_name = printout(*type.sub_type) + 
            " ref";
    }
    
    return type_name;
}

bool DataType::can_cast_to(const DataType &from, const DataType &to, bool &warning)
{
    if (from.construct == PrimTypes::type_int())
    {
        if (to.construct == PrimTypes::type_int()) return true;
        if (to.construct == PrimTypes::type_float()) return true;
        if (to.construct == PrimTypes::type_char()) { warning = true; return true; }
        if (to.construct == PrimTypes::type_bool()) return true;
    }
    else if (from.construct == PrimTypes::type_float())
    {
        warning = true;
        if (to.construct == PrimTypes::type_int()) return true;
        if (to.construct == PrimTypes::type_float()) return true;
        if (to.construct == PrimTypes::type_char()) return true;
        if (to.construct == PrimTypes::type_bool()) return true;
    }
    else if (from.construct == PrimTypes::type_char())
    {
        if (to.construct == PrimTypes::type_int()) return true;
        if (to.construct == PrimTypes::type_float()) return true;
        if (to.construct == PrimTypes::type_char()) return true;
        if (to.construct == PrimTypes::type_bool()) return true;
    }
    else if (from.construct == PrimTypes::type_bool())
    {
        if (to.construct == PrimTypes::type_int()) return true;
        if (to.construct == PrimTypes::type_float()) return true;
        if (to.construct == PrimTypes::type_char()) return true;
        if (to.construct == PrimTypes::type_bool()) return true;
    }
    
    return false;
}

static string param_printout(const vector<DataType> &params)
{
    string param_str = "(";
    for (int i = 0; i < params.size(); i++)
    {
        param_str += DataType::printout(params[i]);
        if (i < params.size() - 1)
            param_str += ", ";
    }
    return param_str + ")";
}

string Symbol::printout(const Symbol &symb)
{
    string prefix = "";
    if (symb.parent != nullptr)
        prefix = symb.parent->get_prefix();
    
    string str = prefix + symb.name;
    if (symb.params.size() > 0)
        str += param_printout(symb.params);
    return str;
}

vector<Symbol> SymbolTable::lookup_all(string name) const
{
    vector<Symbol> out;
    for (const Symbol &symb : symbols)
    {
        if (symb.name == name)
            out.push_back(symb);
    }
    
    return out;
}

const Symbol &SymbolTable::lookup(string name) const
{
    for (const Symbol &symb : symbols)
        if (symb.name == name)
            return symb;

    return null_symbol;
}

const Symbol &SymbolTable::lookup(string name, vector<DataType> params) const
{
    for (const Symbol &symb : symbols)
    {
        if (symb.name == name && symb.params.size() == params.size())
        {
            bool is_equal = true;
            for (int i = 0; i < params.size(); i++)
            {
                DataType p1 = symb.params[i];
                DataType p2 = params[i];
                if (!DataType::equal(p1, p2))
                    is_equal = false;
            }

            if (is_equal)
                return symb;
        }
    }
    
    return null_symbol;
}

vector<Symbol> SymbolTable::find_externals() const
{
    vector<Symbol> out;
    for (const Symbol &symb : symbols)
        if (symb.flags & SYMBOL_EXTERNAL)
            out.push_back(symb);
    return out;
}

DataConstruct *SymbolTable::find_construct(string name)
{
    // Search built in constructs
    if (name == "int") return PrimTypes::type_int();
    else if (name == "float") return PrimTypes::type_float();
    else if (name == "char") return PrimTypes::type_char();
    else if (name == "bool") return PrimTypes::type_bool();

    // Search internal constructs
    for (DataConstruct *construct : constructs)
        if (construct->name == name)
            return construct;

    // Search external constructs
    for (DataConstruct *construct : external_constructs)
        if (construct->name == name)
            return construct;
    
    // Could not be found
    return PrimTypes::type_null();
}

vector<DataConstruct*> SymbolTable::find_construct()
{
    vector<DataConstruct*> out;
    out.insert(out.end(), constructs.begin(), constructs.end());
    out.insert(out.end(), external_constructs.begin(), external_constructs.end());
    return out;
}

DataConstruct *SymbolTable::create_construct(string name)
{
    DataConstruct *construct = new DataConstruct { name, 0, nullptr };
    constructs.push_back(construct);
    return construct;
}

void SymbolTable::new_allocation_space()
{
    allocator = 0;
    scope_size = 0;
}

int SymbolTable::allocate(int size)
{
    int start = allocator;
    allocator += size;
    return start;
}

int SymbolTable::get_scope_size() const
{
    return std::max(allocator, scope_size);
}

bool SymbolTable::is_null(const Symbol &symb)
{
    return symb.flags & SYMBOL_NULL;
}

void SymbolTable::patch_params(vector<DataType> params)
{
    int allocator = -8;

    for (int i = 0; i < params.size(); i++)
    {
        allocator -= DataType::find_size(params[i]);
        symbols[i].type = params[i];
        symbols[i].location = allocator;
    }
}

SymbolTable::~SymbolTable()
{
    for (DataConstruct *construct : constructs)
        delete construct;
}
