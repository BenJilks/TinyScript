#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "bytecode.h"
#include "vm.h"
#include "io.h"

static struct Module mods[80];
static int mod_size;

int LoadFile(const char *file_name)
{
    struct Tokenizer tk;
	struct Module mod;
    char mod_name[80];
    int mod_name_len = strchr(file_name, '.') - file_name;
    memcpy(mod_name, file_name, mod_name_len);
    mod_name[mod_name_len] = '\0';

    tk = open_tokenizer(file_name);
	mod = create_module(mod_name, &tk);
	compile_module(&mod);
	VM_LoadMod(mod.header, mod.code);
#if DEBUG
    disassemble(mod.code, mod.cp);
#endif

    mods[mod_size++] = mod;
    close_tokenizer(tk);
    return tk.has_error;
}

int main(int argc, char **argv)
{
    int i, has_error = 0;
    for (i = 1; i < argc; i++)
        if(LoadFile(argv[i]))
            has_error = 1;
    
    if (!has_error)
    {
        VM_Load_IO();
        if (!VM_Link())
	        VM_CallFuncName("main");
    }
    
    for (i = 0; i < mod_size; i++)
        delete_module(mods[i]);
}
