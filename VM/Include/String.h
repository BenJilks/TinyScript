#ifndef STRING_H
#define STRING_H

#include "VM.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *AsString(Object obj, Object *stack, int *sp);
void RegisterString();

#endif // STRING_H
