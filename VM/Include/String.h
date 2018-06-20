#ifndef STRING_H
#define STRING_H

#include "VM.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void AsString(Object obj, char *str, Object *stack, int *sp);
void StringAdd(Object *stack, int *sp);
void StringMultiply(Object *stack, int *sp);
void StringError(Object *stack, int *sp);

#endif // STRING_H
