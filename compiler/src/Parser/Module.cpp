#include "Parser/Module.hpp"
#include "Parser/Function.hpp"
using namespace TinyScript;

void NodeModule::parse(Tokenizer &tk)
{
    Logger::log(tk.get_debug_info(), "Parsing module");
    Logger::start_scope();

    while (!tk.is_eof())
    {
        switch(tk.get_look().type)
        {
            case TokenType::Func: parse_child<NodeFunction>(tk); break;
            default:
            {
                Token error = tk.skip_token();
                Logger::error(tk.get_debug_info(), 
                    "Unexpected token '" + error.data + "'");
            }
        }
    }

    Logger::end_scope();
}
