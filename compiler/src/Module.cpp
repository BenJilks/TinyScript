#include "Module.hpp"
extern "C"
{
#include "bytecode.h"
}
using namespace TinyScript;

Module::Module(string name, Tokenizer tk) :
    name(name), tk(tk), exp(&this->tk) {}

Module::Module(string name) :
    name(name), is_external_flag(true), exp(nullptr) {}

Module::Module(Module &&other) :
    Module(other.name, other.tk) {}

CodeGen Module::compile()
{
    if (is_external_flag)
        return compile_external();

    // Import everything
    for (auto import : imports)
    {
        Module *mod = import.first;
        for (string attr : import.second)
        {
            auto symbols = mod->table.lookup_all(tk.get_debug_info(), attr);
            table.push_all(symbols);
        }
    }

    Logger::log(tk.get_debug_info(), "Compiling module '" + name + "'");
    Logger::start_scope();
    for (Function &func : funcs)
        code.append(func.compile(table));
    Logger::end_scope();

    return code;
}

CodeGen Module::compile_external()
{    
    return code;
}

void Module::gen_header(ModuleLibrary *library)
{
    Logger::log(tk.get_debug_info(), "Scanning module '" + name + "'");
    Logger::start_scope();

    while (!tk.is_eof())
    {
        switch (tk.get_look().type)
        {
            case TokenType::Class: parse_class(); break;
            case TokenType::Func: parse_func(); break;
            case TokenType::From: parse_import_from(library); break;
            case TokenType::Eof: break;
            default: 
                Logger::error(tk.get_debug_info(), 
                    "Unexpected token '" + tk.get_look().data + "'");
                tk.skip_token();
                break;
        }
    }

    Logger::end_scope();
}

vector<Symbol> Module::parse_params()
{
    vector<Symbol> params;
    int allocator = -8;

    tk.match(TokenType::OpenArg, "(");
    while (tk.get_look().type != TokenType::CloseArg && !tk.is_eof())
    {
        DataType type = exp.parse_type(table);
        Token name = tk.match(TokenType::Name, "Param Name");
        
        allocator -= DataType::find_size(type);
        params.push_back(Symbol(name.data, type, SYMBOL_LOCAL | SYMBOL_ARG, allocator));

        Logger::log(tk.get_debug_info(), "Found param '" + name.data + 
            "' of type " + type.construct->name);

        if (tk.get_look().type != TokenType::CloseArg)
            tk.match(TokenType::Next, ",");
    }
    tk.match(TokenType::CloseArg, ")");

    return params;
}

static vector<DataType> to_type_list(vector<Symbol> params)
{
    vector<DataType> types;
    types.reserve(params.size());

    for (Symbol &symb : params)
        types.push_back(symb.type);
    
    return types;
}

DataType Module::parse_return_type()
{
    if (tk.get_look().type == TokenType::Gives)
    {
        tk.match(TokenType::Gives, "->");
        return exp.parse_type(table);
    }

    return { PrimTypes::type_null(), 0, 0 };
}

void Module::parse_import_from(ModuleLibrary *library)
{
    tk.match(TokenType::From, "from");
    Token mod_name = tk.match(TokenType::Name, "Module Name");
    tk.match(TokenType::Import, "import");
    Token attr_name = tk.match(TokenType::Name, "Attr Name");

    Module *module = library->load_module(mod_name.data);
    if (imports.find(module) == imports.end())
        imports[module] = vector<string>();
    
    imports[module].push_back(attr_name.data);
}

void Module::parse_class()
{
    tk.match(TokenType::Class, "class");
    Token class_name = tk.match(TokenType::Name, "Name");
    tk.match(TokenType::OpenBlock, "{");
    Logger::log(class_name.debug_info, "Creating class " + class_name.data);
    Logger::start_scope();

    DataConstruct *construct = table.create_construct(class_name.data);
    while (tk.get_look().type != TokenType::CloseBlock && !tk.is_eof())
    {
        DataType type = exp.parse_type(table);
        Token name = tk.match(TokenType::Name, "Name");
        DataConstruct::add_symbol(construct, name.data, type);
        
        Logger::log(name.debug_info, "Created new attr '" + 
            name.data + "' of type '" + 
            DataType::printout(type) + "'");
    }

    Logger::end_scope();
    tk.match(TokenType::CloseBlock, "}");
}

void Module::parse_func()
{
    tk.match(TokenType::Func, "func");
    Token name = tk.match(TokenType::Name, "Function Name");

    vector<Symbol> params = parse_params();
    DataType return_type = parse_return_type();
    DebugInfo start = tk.get_debug_info();
    tk.skip_scope();

    // Create function symbol
    Symbol symb = Symbol(name.data, return_type, 
        SYMBOL_FUNCTION | SYMBOL_GLOBAL, 
        funcs.size());
    symb.params = to_type_list(params);
    table.push(symb);
    
    // Register function position
    funcs.emplace_back(name.data, start, symb, params, return_type, &tk);
    Logger::log(tk.get_debug_info(), "Found function '" + name.data + "'");
}

void Module::add_external_func(string name, DataType type, vector<DataType> params)
{
    Symbol symb(name, type, SYMBOL_FUNCTION | SYMBOL_GLOBAL 
        | SYMBOL_EXTERNAL, 0);
    symb.params = params;
    code.register_external(symb);
    table.push(symb);
}

void ModuleLibrary::add_external(Module *mod)
{
    modules.push_back(mod);
}

Module *ModuleLibrary::load_module(string name)
{
    Module *loaded = find_module(name);
    if (loaded == nullptr)
    {
        Tokenizer tk(name + ".tiny");
        loaded = new Module(name, tk);
        modules.push_back(loaded);
        loaded->gen_header(this);
    }

    return loaded;
}

Module *ModuleLibrary::find_module(string name)
{
    for (Module *mod : modules)
        if (mod->get_name() == name)
            return mod;
    return nullptr;
}

CodeGen ModuleLibrary::compile()
{
    CodeGen code;
    for (Module *mod : modules)
        code.append(mod->compile());

    return code;
}

ModuleLibrary::~ModuleLibrary()
{
    for (Module *mod : modules)
        delete mod;
}
