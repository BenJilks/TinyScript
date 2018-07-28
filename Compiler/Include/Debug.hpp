#pragma once

#define DEBUG 0
#if DEBUG
#define LOG(...) PrintScope(); printf(__VA_ARGS__)
#define START_SCOPE() StartScope()
#define END_SCOPE() EndScope()
#else
#define LOG(...) ;
#define START_SCOPE() ;
#define END_SCOPE() ;
#endif

void StartScope();
void EndScope();
void PrintScope();
