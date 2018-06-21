#include "VM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Type t_int = {"int", INT, 4};
Type t_float = {"float", FLOAT, 4};
Type t_string = {"string", STRING, 8};
Type t_bool = {"bool", BOOL, 1};
Type t_char = {"char", CHAR, 1};

#define ERROR(...) \
	printf(__VA_ARGS__); \
	exit(0)

#define OP_ERROR(left, right, op) \
    printf("Invalid operation of %s %s %s\n", \
        left.type->name, #op, right.type->name); \
    exit(0)

#define OP_FUNC(name, op, overload, str_func) \
	Object name(Object left, Object right) \
	{ \
		Object obj; \
		switch(left.type->prim) \
		{ \
			case INT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_int; obj.i = left.i op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.i op right.f; break; \
					case CHAR: obj.type = &t_int; obj.i = left.i op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case FLOAT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_float; obj.f = left.f op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.f op right.f; break; \
					case CHAR: obj.type = &t_float; obj.f = left.f op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case CHAR: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_int; obj.i = left.c op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.c op right.f; break; \
					case CHAR: obj.type = &t_char; obj.c = left.c op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case BOOL: \
				OP_ERROR(left, right, op); \
				break; \
			case OBJECT: \
				if (left.type->overload != -1) \
				{ \
					sp++; \
					CallFunc(left.type->overload); \
					sp -= 2; \
					return stack[sp+1]; \
				} \
				OP_ERROR(left, right, op); \
				break; \
			case STRING: \
				sp++; \
				str_func(stack, &sp); \
				sp -= 1; \
				return stack[sp-1]; \
		} \
		return obj; \
	}

#define COMPARE_FUNC(name, op) \
	Object name(Object left, Object right) \
	{ \
		Object obj; \
		switch(left.type->prim) \
		{ \
			case INT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.i op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.i op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.i op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case FLOAT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.f op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.f op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.f op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case CHAR: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.c op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.c op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.c op right.c; break; \
					default: OP_ERROR(left, right, op); break; \
				} \
				break; \
			case BOOL: \
				OP_ERROR(left, right, op); \
				break; \
		} \
		return obj; \
	}

Object Equals(Object left, Object right)
{
	Object obj;
	switch(left.type->prim)
	{
		case INT:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.i == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.i == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.i == right.c; break;
				default: OP_ERROR(left, right, ==); break;
			}
			break;
		case FLOAT:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.f == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.f == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.f == right.c; break;
				default: OP_ERROR(left, right, ==); break;
			}
			break;
		case CHAR:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.c == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.c == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.c == right.c; break;
				default: OP_ERROR(left, right, ==); break;
			}
			break;
		case BOOL:
			switch(right.type->prim)
			{
				case BOOL: obj.type = &t_bool; obj.i = left.c == right.c; break;
				default: OP_ERROR(left, right, ==); break;
			}
			break;
		case STRING:
			switch(right.type->prim)
			{
				case STRING: obj.type = &t_bool; obj.i = !strcmp(left.p->str, right.p->str); break;
				default: OP_ERROR(left, right, ==); break;
			}
			break;
        case OBJECT:
            switch(right.type->prim)
			{
				case OBJECT: obj.type = &t_bool; obj.i = left.p->v == right.p->v; break;
				default: OP_ERROR(left, right, ==); break;
			}
            break;
	}
	return obj;
}
