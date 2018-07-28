#include "Debug.hpp"
#include <stdio.h>

static int scope = 0;

void StartScope()
{
    scope++;
}

void EndScope()
{
    scope--;
}

void PrintScope()
{
    for (int i = 0; i < scope; i++)
        printf("    ");
    
    if (scope > 0)
        printf("L ");
}
