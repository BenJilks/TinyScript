#ifndef BYTECODE
#define BYTECODE

/* Stack */
#define BC_PUSH        0
#define BC_PUSHPC      1
#define BC_SET         2 
#define BC_GET         3
#define BC_SETC        4
#define BC_GETC        5
#define BC_FSET        6
#define BC_FGET        7
#define BC_FSETC       8
#define BC_FGETC       9

/* Memory */
#define BC_CPY        10
#define BC_SCPYTO     11
#define BC_SCPYFROM   12
#define BC_INC        13
#define BC_DEC        14
#define BC_SINC       15
#define BC_SDEC       16

/* Stack Frame */
#define BC_SFCPYTO    17
#define BC_SFCPYFROM  18
#define BC_FINC       19
#define BC_FDEC       20

/* Maths */
#define BC_ADD        21
#define BC_ADDF       22
#define BC_SUB        23
#define BC_SUBF       24
#define BC_MUL        25
#define BC_MULF       26
#define BC_DIV        27
#define BC_DIVF       28

/* Conditional */
#define BC_EQL        29
#define BC_EQLF       30
#define BC_NEQL       31
#define BC_NEQLF      32
#define BC_MORE       33
#define BC_MOREF      34
#define BC_LESS       35
#define BC_LESSF      36
#define BC_EMORE      37
#define BC_EMOREF     38
#define BC_ELESS      39
#define BC_ELESSF     40

/* Program flow */
#define BC_JUMP       41
#define BC_JUMPIFNOT  42
#define BC_CALL       43
#define BC_RETURN     44
#define BC_IRETURN    45

/* Typeing */
#define BC_ITOF_LEFT  46
#define BC_ITOF_RIGHT 47
#define BC_FTOI       48

#define BC_COUNT      49

static char* bytecode_names[BC_COUNT] = 
{
	"push", "pushpc", "set", "get", "setc", "getc", "fset", "fget", "fsetc", "fgetc",
	"cpy", "scpyto", "scpyfrom", "inc", "dec", "sinc", "sdec",
	"sfcpyto", "sfcpyfrom", "finc", "fdec",
	"add", "addf", "sub", "subf", "mul", "mulf", "div", "divf",
	"eqaul", "equalf", "neql", "neqlf", "more", "moref", "less", "lessf", "emore", "emoref", "eless", "elessf",
	"jump", "jumpifnot", "call", "return", "ireturn",
	"itofleft", "itofright", "ftoi"
};

static int bytecode_sizes[BC_COUNT] = 
{
	2, 1, 2, 2, 2, 2, 2, 2, 2, 2,
	4, 3, 3, 3, 3, 2, 2,
	3, 3, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 1, 1,
	1, 1, 1
};

#endif
