#include "CodeGen/TinyVMCode.hpp"
extern "C"
{
#include "bytecode.h"
}
using namespace TinyScript::TinyVM;
using namespace TinyScript;

static char find_op_code(string name)
{
    int i;
    for (i = 0; i < BC_SIZE; i++)
        if (string(bytecode_names[i]) == name)
            return i;
    return -1;
}

static string str_upper(string str)
{
    string out;
    for (char c : str)
        out += toupper(c);
    return out;
}

void Code::compile_operation(Token op, DataType ltype, DataType rtype)
{
    // Find the bytecode name of the operation
    string op_name;
    switch(op.type)
    {
        case TokenType::Add: op_name = "ADD"; break;
        case TokenType::Subtract: op_name = "SUB"; break;
        case TokenType::Multiply: op_name = "MUL"; break;
        case TokenType::Divide: op_name = "DIV"; break;
        case TokenType::MoreThan: op_name = "MORE_THAN"; break;
        case TokenType::LessThan: op_name = "LESS_THAN"; break;
        case TokenType::MoreThanEquals: op_name = "MORE_THAN_EQUALS"; break;
        case TokenType::LessThanEquals: op_name = "MORE_THAN_EQUALS"; break;
        case TokenType::Equals: op_name = "EQUALS"; break;
    }

    // Create the bytecode name string for the types and find the code
    string code_name = "BC_" + op_name + "_" + str_upper(ltype.construct->name) + 
        "_" + str_upper(rtype.construct->name);
    char bytecode = find_op_code(code_name);
    if (bytecode == -1)
    {
        // If that operation has not been implemented
        Logger::error(op.debug_info, "Cannot do operation " + 
            ltype.construct->name + 
            " " + op.data + " " + 
            rtype.construct->name);
        return;
    }

    // Write this final operation to the code
    write_byte(bytecode);
}

void Code::compile_call(const Symbol &symb, ExpDataNode *node)
{
    // Allocate the return space
    int return_size = DataType::find_size(symb.type);
    if (return_size > 0)
    {
        write_byte(BC_ALLOC);
        write_byte(return_size);
    }

    // Push the arguments onto the stack
    for (int i = node->args.size() - 1; i >= 0; i--)
        compile_rvalue(node->args[i]);

    // Call the function depending on type
    if (symb.flags & SYMBOL_EXTERNAL)
    {
        // Call external symbol
        write_byte(BC_CALL_EXTERNAL);
        write_int(symb.location);
    }
    else
    {
        // Call local tinyscript function
        write_byte(BC_CALL);
        write_label(Symbol::printout(symb));
    }
}

void Code::compile_rname(ExpDataNode *node)
{
    Token name = node->token;
    Symbol symb = node->symb;

    // Compile function call
    if (node->flags & NODE_CALL)
    {
        compile_call(symb, node);
        return;
    }

    // Push local to the stack
    if (symb.flags & SYMBOL_LOCAL)
    {
        write_byte(BC_LOAD_LOCAL_X);
        write_int(DataType::find_size(symb.type));
        write_byte(symb.location);
    }
}

void Code::compile_ref(ExpDataNode *node)
{
    Symbol lvalue = find_lvalue_location(node->left);
    write_byte(BC_LOCAL_REF);
    write_byte(lvalue.location);
}

void Code::compile_copy(ExpDataNode *node)
{
    compile_rvalue(node->left);
    write_byte(BC_COPY);
    write_byte(DataType::find_size(node->type));
}

void Code::compile_array(ExpDataNode *node)
{
    // Push all items to the stack
    for (ExpDataNode *item : node->args)
        compile_rvalue(item);
}

void Code::compile_cast(ExpDataNode *node)
{
    compile_rvalue(node->left);
    DataType ltype = node->left->type;
    DataType rtype = node->right->type;

    Logger::log(node->token.debug_info, "Cast from " 
        + DataType::printout(ltype) 
        + " to " + DataType::printout(rtype));
    
    // If converting from a ref, don't do anything
    if (!(ltype.flags & DATATYPE_REF))
    {
        string code_name = "BC_CAST_" + str_upper(ltype.construct->name) + 
            "_" + str_upper(rtype.construct->name);
        write_byte(find_op_code(code_name));
    }
}

void Code::compile_typesize(ExpDataNode *node)
{
    DataType type = node->left->type;
    int size = DataType::find_size(type);

    write_byte(BC_PUSH_4);
    write_int(size);
}

void Code::compile_typename(ExpDataNode *node)
{
    DataType type = node->left->type;
    string name = DataType::printout(type);

    write_byte(BC_PUSH_X);
    write_string(name);
}

void Code::compile_arraysize(ExpDataNode *node)
{
    DataType type = node->left->type;
    int size = type.array_size;

    write_byte(BC_PUSH_4);
    write_int(size);
}

void Code::compile_rterm(ExpDataNode *node)
{
    Token value = node->token;
    const char *str = value.data.c_str();

    // Compile term based on type
    switch(value.type)
    {
        case TokenType::Int: write_byte(BC_PUSH_4); write_int(atoi(str)); break;
        case TokenType::Float: write_byte(BC_PUSH_4); write_float(atof(str)); break;
        case TokenType::Bool: write_byte(BC_PUSH_1); write_byte(value.data == "true" ? 1 : 0); break;
        case TokenType::Char: write_byte(BC_PUSH_1); write_byte(value.data[0]); break;
        case TokenType::String: write_byte(BC_PUSH_X); write_string(value.data); break;
        case TokenType::Name: compile_rname(node); break;
        case TokenType::Ref: compile_ref(node); break;
        case TokenType::As: compile_cast(node); break;
        case TokenType::Copy: compile_copy(node); break;
        case TokenType::OpenIndex: compile_array(node); break;
        case TokenType::TypeSize: compile_typesize(node); break;
        case TokenType::TypeName: compile_typename(node); break;
        case TokenType::ArraySize: compile_arraysize(node); break;
    }
}

void Code::compile_index(ExpDataNode *node)
{
    compile_rvalue(node->left);
    compile_rvalue(node->right);

    int element_size = DataType::find_size(*node->left->type.sub_type);
    if (node->left->type.flags & DATATYPE_REF)
    {
        write_byte(BC_PUSH_4);
        write_int(element_size);
        write_byte(BC_MUL_INT_INT);
        write_byte(BC_ADD_INT_INT);
        write_byte(BC_COPY);
        write_byte(DataType::find_size(node->type));
    }
    else
    {
        write_byte(BC_GET_ARRAY_INDEX);
        write_int(node->left->type.array_size * element_size);
        write_int(element_size);
    }
}

void Code::compile_rin(ExpDataNode *node)
{
    Symbol symb = node->right->symb;
    if (symb.flags & SYMBOL_FUNCTION)
    {
        compile_call(symb, node->right);
        return;
    }
    compile_rvalue(node->left);

    int offset = symb.location;
    int size = DataType::find_size(node->right->type);
    int type_size = DataType::find_size(node->left->type);
    if (node->left->type.flags & DATATYPE_REF)
    {
        write_byte(BC_PUSH_4);
        write_int(offset);
        write_byte(BC_ADD_INT_INT);
        write_byte(BC_COPY);
        write_byte(size);
    }
    else
    {
        write_byte(BC_GET_ATTR);
        write_int(offset);
        write_int(size);
        write_int(type_size);
    }
}

void Code::compile_rvalue(ExpDataNode *node)
{
    // Compile the code for pushing the value on to the stack
    if (node->flags & NODE_OPERATION)
    {
        compile_rvalue(node->left);
        compile_rvalue(node->right);
        compile_operation(node->token, node->left->type, 
            node->right->type);
    }
    else if (node->flags & NODE_INDEX)
    {
        compile_index(node);
    }
    else if (node->flags & NODE_IN)
    {
        compile_rin(node);
    }
    else
    {
        compile_rterm(node);
    }
}

bool Code::is_static_lvalue(ExpDataNode *node)
{
    if (node->token.type == TokenType::Name)
    {
        Symbol symb = node->symb;
        if (!(symb.flags & SYMBOL_FUNCTION))
            return true;
    }

    return false;
}

Symbol Code::find_lvalue_location(ExpDataNode *node)
{
    if (node->token.type != TokenType::Name)
        Logger::error(node->token.debug_info, "Not a valid lvalue");
    return node->symb;
}

void Code::compile_lname(ExpDataNode *node)
{
    Token name = node->token;
    Symbol symb = node->symb;

    if (symb.type.flags & DATATYPE_REF)
    {
        write_byte(BC_LOAD_LOCAL_4);
        write_byte(symb.location);
        return;
    }

    if (symb.flags & SYMBOL_LOCAL)
    {
        write_byte(BC_LOCAL_REF);
        write_byte(symb.location);
    }
}

void Code::compile_lterm(ExpDataNode *node)
{
    if (node->token.type == TokenType::Name)
        compile_lname(node);
    else
        compile_rterm(node);
}

void Code::compile_lin(ExpDataNode *node)
{
    Symbol attr = node->right->symb;
    
    compile_lvalue(node->left);
    write_byte(BC_PUSH_4);
    write_int(attr.location);
    write_byte(BC_ADD_INT_INT);
}

void Code::compile_lvalue(ExpDataNode *node)
{
    if (node->flags & NODE_OPERATION)
    {
        compile_lvalue(node->left);
        compile_lvalue(node->right);
        compile_operation(node->token, node->left->type, 
            node->right->type);
        return;
    }

    if (node->flags & NODE_INDEX)
    {
        compile_lvalue(node->left);
        compile_rvalue(node->right);

        int element_size = DataType::find_size(*node->left->type.sub_type);
        write_byte(BC_PUSH_4);
        write_int(element_size);
        write_byte(BC_MUL_INT_INT);
        write_byte(BC_ADD_INT_INT);
        return;
    }

    if (node->flags & NODE_IN)
    {
        compile_lin(node);
        return;
    }

    compile_lterm(node);
}

void Code::compile_rexpression(NodeExpression *node)
{
    node->symbolize();
    compile_rvalue(node->get_data());
}

void Code::compile_lexpression(NodeExpression *node)
{
    node->symbolize();
    compile_lvalue(node->get_data());
}
