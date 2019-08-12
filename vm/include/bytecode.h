#ifndef BYTECODE_H
#define BYTECODE_H

#define BC_OPERATION_SET(GEN, types) \
    GEN(BC_ADD_##types, 0) \
    GEN(BC_SUB_##types, 0) \
    GEN(BC_MUL_##types, 0) \
    GEN(BC_DIV_##types, 0) \
    GEN(BC_MORE_THAN_##types, 0) \
    GEN(BC_LESS_THAN_##types, 0) \
    GEN(BC_MORE_THAN_EQUALS_##types, 0) \
    GEN(BC_LESS_THAN_EQUALS_##types, 0) \
    GEN(BC_EQUALS_##types, 0) \

#define PRIM_CAST_SET(GEN, to) \
    GEN(BC_CAST_INT_##to, 0) \
    GEN(BC_CAST_FLOAT_##to, 0) \
    GEN(BC_CAST_CHAR_##to, 0) \
    GEN(BC_CAST_BOOL_##to, 0)

#define FOR_EACH_CODE(GEN) \
    GEN(BC_PUSH_1, 1) \
    GEN(BC_PUSH_4, 4) \
    GEN(BC_PUSH_X, -1) \
    GEN(BC_POP, 1) \
    GEN(BC_ALLOC, 1) \
     \
    GEN(BC_STORE_LOCAL_4, 1) \
    GEN(BC_STORE_LOCAL_X, 5) \
    GEN(BC_LOAD_LOCAL_4, 1) \
    GEN(BC_LOAD_LOCAL_X, 5) \
    GEN(BC_LOCAL_REF, 1) \
    GEN(BC_COPY, 1) \
    GEN(BC_GET_ARRAY_INDEX, 8) \
    GEN(BC_GET_ATTR, 12) \
    GEN(BC_ASSIGN_REF_X, 4) \
     \
    GEN(BC_CREATE_FRAME, 4) \
    GEN(BC_CALL, 4) \
    GEN(BC_CALL_EXTERNAL, 4) \
    GEN(BC_RETURN, 2) \
    GEN(BC_JUMP, 4) \
    GEN(BC_JUMP_IF_NOT, 4) \
     \
    /* Operations */ \
    BC_OPERATION_SET(GEN, INT_INT) \
    BC_OPERATION_SET(GEN, INT_FLOAT) \
    BC_OPERATION_SET(GEN, INT_CHAR) \
    BC_OPERATION_SET(GEN, FLOAT_INT) \
    BC_OPERATION_SET(GEN, FLOAT_FLOAT) \
    BC_OPERATION_SET(GEN, FLOAT_CHAR) \
    BC_OPERATION_SET(GEN, CHAR_INT) \
    BC_OPERATION_SET(GEN, CHAR_FLOAT) \
    BC_OPERATION_SET(GEN, CHAR_CHAR) \
     \
    /* Casts */ \
    PRIM_CAST_SET(GEN, INT) \
    PRIM_CAST_SET(GEN, FLOAT) \
    PRIM_CAST_SET(GEN, CHAR) \
    PRIM_CAST_SET(GEN, BOOL) \
     \
    GEN(BC_SIZE, 0)

#define GENERATE_ENUM(ENUM, size) ENUM,
#define GENERATE_STRING(STRING, size) #STRING,
#define GENERATE_SIZE(STRING, size) size,

typedef enum Bytecode
{
    FOR_EACH_CODE(GENERATE_ENUM)
} Bytecode;

static const char *bytecode_names[] = 
{
    FOR_EACH_CODE(GENERATE_STRING)
};

static const int bytecode_size[] = 
{
    FOR_EACH_CODE(GENERATE_SIZE)
};

void disassemble(char *code, int size);

#endif // BYTECODE_H
