#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "vm.h"
#include "bytecode.h"

static struct Module mods[80];
static int mod_size;

void LoadFile(const char *file_name)
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
}

int main()
{
    LoadFile("maths.tiny");
	LoadFile("test.tiny");
    VM_Link();
	struct VMObject obj = VM_CallFuncName("main");
    printf("%i\n", obj.i);

    int i;
    for (i = 0; i < mod_size; i++)
        delete_module(mods[i]);
}
