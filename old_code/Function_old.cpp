#include "Function.hpp"
extern "C"
{
#include "bytecode.h"
}
using namespace TinyScript;

Function::Function(string name, const DebugInfo start, const DebugInfo return_code, Symbol func_symb,
    vector<Symbol> params, DataType return_type, Tokenizer *tk) :
    name(name), start(start), return_code(return_code), params(params), return_type(return_type),
    func_symb(func_symb), tk(tk), exp(tk), arg_size(0)
{
    code.register_func(func_symb);

    for (Symbol &symb : params)
        arg_size += DataType::find_size(symb.type);
    return_size = DataType::find_size(return_type);
}

void Function::add_prefix(string prefix)
{
    name = prefix + ":" + name;
    func_symb.name = name;
}

CodeGen Function::compile(SymbolTable &table)
{
    table.new_allocation_space();
    table.start_scope();
    for (const Symbol &symb : params)
        table.push(symb);

    tk->set_pos(start);
    tk->skip_token();
    code.write_byte(BC_CREATE_FRAME);
    int frame_size_loc = code.curr_location();
    code.write_byte(0);

    Logger::log(tk->get_debug_info(), "Compiling function '" + name + "'");
    Logger::start_scope();
    compile_block(table);
    Logger::end_scope();

    code.write_byte(BC_RETURN);
    code.write_byte(0);
    code.write_byte(arg_size);
    table.end_scope();

    char frame_size = table.get_scope_size();
    code.insert(frame_size_loc, &frame_size, 1);
    return code;
}

void Function::compile_block(SymbolTable &table)
{
    if (tk->get_look().type == TokenType::OpenBlock)
    {
        tk->match(TokenType::OpenBlock, "{");
        while (tk->get_look().type != TokenType::CloseBlock && !tk->is_eof())
            compile_statement(table);
        tk->match(TokenType::CloseBlock, "}");
    }
    else
    {
        compile_statement(table);
    }
}

void Function::compile_if(SymbolTable &table)
{
    int label = code.create_label();

    tk->match(TokenType::If, "if");
    ExpNode *condition = exp.parse(table);
    exp.compile_rvalue(condition, code, table);

    code.write_byte(BC_JUMP_IF_NOT);
    code.write_label(label);
    table.start_scope();
    compile_block(table);
    table.end_scope();
    code.register_label(label);

    exp.free_node(condition);
}

void Function::compile_statement(SymbolTable &table)
{
    switch(tk->get_look().type)
    {
        case TokenType::Let: compile_let(table); break;
        case TokenType::Return: compile_return(table); break;
        case TokenType::If: compile_if(table); break;
        default: compile_expression(table); break;
    }
}

void Function::assign_inline(SymbolTable &table, Token name)
{
    tk->match(TokenType::Assign, "=");
    ExpNode *value = exp.parse(table);
    DataType type = value->type;
    int size = DataType::find_size(type);
    int location = table.allocate(size);

    Symbol symb = Symbol(name.data, type, SYMBOL_LOCAL, location);
    exp.compile_rvalue(value, code, table);
    exp.assign_symb(symb, code);

    exp.free_node(value);
    table.push(symb);
}

void Function::compile_let(SymbolTable &table)
{
    tk->match(TokenType::Let, "let");    
    Token name = tk->match(TokenType::Name, "Local Name");
    Logger::log(name.debug_info, "Created symbol '" + name.data + "'");

    if (tk->get_look().type == TokenType::Of)
    {
        // Find data type
        tk->match(TokenType::Of, ":");
        DataType type = exp.parse_type(table);

        // Create new var of that type
        int size = DataType::find_size(type);
        int location = table.allocate(size);
        Symbol symb = Symbol(name.data, type, SYMBOL_LOCAL, location);
        table.push(symb);

        // If there's an assignment, do so
        if (tk->get_look().type == TokenType::Assign)
        {
            tk->match(TokenType::Assign, "=");
            ExpNode *right = exp.parse(table);
            exp.compile_rvalue(right, code, table);
            exp.assign_symb(symb, code);
            exp.free_node(right);
        }
    }
    else
    {
        assign_inline(table, name);
    }
}

void Function::compile_return(SymbolTable &table)
{
    tk->match(TokenType::Return, "return");
    ExpNode *value = exp.parse(table);
    if (!DataType::equal(value->type, return_type))
    {
        Logger::error(value->token.debug_info, "Function " + name + " returns '" + 
            DataType::printout(return_type) + "' not '" + 
            DataType::printout(value->type) + "'");
    }

    Logger::log(tk->get_debug_info(), "Return " + value->type.construct->name);
    exp.compile_rvalue(value, code, table);
    code.write_byte(BC_RETURN);
    code.write_byte(return_size);
    code.write_byte(arg_size);
    exp.free_node(value);
}

void Function::parse_static_assign(ExpNode *left, ExpNode *right, SymbolTable &table)
{
    // Find lvalue location
    Symbol lvalue = exp.find_lvalue_location(left);
    
    // Check assign types
    if (DataType::equal(left->type, right->type))
    {
        Logger::error(left->token.debug_info, "Cannot assign '" + 
            left->type.construct->name + "' value to '" + 
            right->type.construct->name + "'");
    }

    // Compile and assign values
    exp.compile_rvalue(right, code, table);
    exp.assign_symb(lvalue, code);
}

void Function::parse_dynamic_assign(ExpNode *left, ExpNode *right, SymbolTable &table)
{
    exp.compile_rvalue(right, code, table);
    exp.compile_lvalue(left, code, table);
    code.write_byte(BC_ASSIGN_REF_X);
    code.write_int(DataType::find_size(right->type));
}

void Function::parse_assign(ExpNode *left, SymbolTable &table)
{
    tk->match(TokenType::Assign, "=");
    ExpNode *right = exp.parse(table);

    // Depending on if the lvalue is static or not, pick an assign method
    if (exp.is_static_lvalue(left))
        parse_static_assign(left, right, table);
    else
        parse_dynamic_assign(left, right, table);

    // Clean up after
    exp.free_node(left);
    exp.free_node(right);
}

void Function::compile_expression(SymbolTable &table)
{
    ExpNode *head = exp.parse(table);
    if (tk->get_look().type == TokenType::Assign)
    {
        parse_assign(head, table);
        return;
    }

    exp.compile_rvalue(head, code, table);
    int return_size = DataType::find_size(head->type);
    if (return_size > 0)
    {
        code.write_byte(BC_POP);
        code.write_byte(return_size);
    }
    exp.free_node(head);
}
