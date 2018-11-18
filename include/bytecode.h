#ifndef BYTECODE_H
#define BYTECODE_H

enum Bytecode 
{
    BC_PUSH_INT,
    BC_PUSH_FLOAT,
    BC_PUSH_CHAR,
    BC_PUSH_BOOL,
    BC_PUSH_STRING,
    BC_PUSH_ARG,
    BC_PUSH_LOC,
    BC_ASSIGN_LOC,

    BC_CALL,
    BC_CALL_MOD,
    BC_RETURN,
    BC_JUMP_IF_NOT,
    BC_POP,

    BC_ADD,
    BC_SUB,
    BC_MUL,
    BC_DIV,
    BC_MORE_THAN,
    BC_LESS_THAN,
    BC_ASSIGN
};

void decode_header(char *header);
void disassemble(char *code, int len);

#endif // BYTECODE_H
