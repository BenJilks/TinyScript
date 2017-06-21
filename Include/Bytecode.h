#ifndef BYTECODE
#define BYTECODE

/* Stack */
#define BC_PUSH        0
#define BC_PUSHPC      1
#define BC_SET         2 
#define BC_GET         3
#define BC_FSET        4
#define BC_FGET        5

/* Memory */
#define BC_CPY         6
#define BC_SCPYTO      7
#define BC_SCPYFROM    8
#define BC_INC         9
#define BC_DEC        10
#define BC_SINC       11
#define BC_SDEC       12

/* Stack Frame */
#define BC_SFCPYTO    13
#define BC_SFCPYFROM  14
#define BC_FINC       15
#define BC_FDEC       16

/* Maths */
#define BC_ADD        17
#define BC_SUB        18
#define BC_MUL        19
#define BC_DIV        20

/* Conditional */
#define BC_EQL        21
#define BC_NEQL       22
#define BC_MORE       23
#define BC_LESS       24
#define BC_EMORE      25
#define BC_ELESS      26

/* Program flow */
#define BC_JUMP       27
#define BC_JUMPIFNOT  28
#define BC_CALL       29
#define BC_RETURN     30
#define BC_IRETURN    31
#define BC_COUNT      32

static char* bytecode_names[BC_COUNT] = 
{
	"push", "pushpc", "set", "get", "fset", "fget",
	"cpy", "scpyto", "scpyfrom", "inc", "dec", "sinc", "sdec",
	"sfcpyto", "sfcpyfrom", "finc", "fdec",
	"add", "sub", "mul", "div",
	"eqaul", "neql", "more", "less", "emore", "eless",
	"jump", "jumpifnot", "call", "return", "ireturn"
};

static int bytecode_sizes[BC_COUNT] = 
{
	2, 1, 2, 2, 2, 2,
	4, 3, 3, 3, 3, 2, 2,
	3, 3, 2, 2,
	1, 1, 1, 1,
	1, 1, 1, 1, 1, 1,
	2, 2, 2, 1, 1
};

#endif
