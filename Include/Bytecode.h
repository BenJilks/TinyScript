#ifndef BYTECODE
#define BYTECODE

/* Stack */
#define BC_PUSH        0
#define BC_PUSHC       1
#define BC_PUSHPC      2
#define BC_SET         3 
#define BC_GET         4
#define BC_SETC        5
#define BC_GETC        6
#define BC_FSET        7
#define BC_FGET        8
#define BC_FSETC       9
#define BC_FGETC      10

/* Memory */
#define BC_CPY        11
#define BC_SCPYTO     12
#define BC_SCPYFROM   13
#define BC_INC        14
#define BC_DEC        15
#define BC_SINC       16
#define BC_SDEC       17

/* Stack Frame */
#define BC_SFCPYTO    18
#define BC_SFCPYFROM  19
#define BC_FINC       20
#define BC_FDEC       21
#define BC_CPYARGS    22

/* Maths */
#define BC_ADD        23
#define BC_ADDF       24
#define BC_SUB        25
#define BC_SUBF       26
#define BC_MUL        27
#define BC_MULF       28
#define BC_DIV        29
#define BC_DIVF       30

/* Conditional */
#define BC_EQL        31
#define BC_EQLF       32
#define BC_NEQL       33
#define BC_NEQLF      34
#define BC_MORE       35
#define BC_MOREF      36
#define BC_LESS       37
#define BC_LESSF      38
#define BC_EMORE      39
#define BC_EMOREF     40
#define BC_ELESS      41
#define BC_ELESSF     42

/* Program flow */
#define BC_JUMP       43
#define BC_JUMPIFNOT  44
#define BC_CALL       45
#define BC_CCALL      46
#define BC_RETURN     47
#define BC_IRETURN    48

/* Typeing */
#define BC_ITOF_LEFT  49
#define BC_ITOF_RIGHT 50
#define BC_FTOI       51

#define BC_COUNT      52

static char* bytecode_names[BC_COUNT] = 
{
	"push", "pushc", "pushpc", "set", "get", "setc", "getc", "fset", "fget", "fsetc", "fgetc",
	"cpy", "scpyto", "scpyfrom", "inc", "dec", "sinc", "sdec",
	"sfcpyto", "sfcpyfrom", "finc", "fdec", "cpyargs",
	"add", "addf", "sub", "subf", "mul", "mulf", "div", "divf",
	"eqaul", "equalf", "neql", "neqlf", "more", "moref", "less", "lessf", "emore", "emoref", "eless", "elessf",
	"jump", "jumpifnot", "call", "ccall", "return", "ireturn",
	"itofleft", "itofright", "ftoi"
};

static int bytecode_sizes[BC_COUNT] = 
{
	2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2,
	4, 3, 3, 3, 3, 2, 2,
	3, 3, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 1, 1,
	1, 1, 1
};

#endif
