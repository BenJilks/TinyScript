#ifndef RUNTIME_LIB
#define RUNTIME_LIB

/* Define data types */
#define FUNC_SIZE 80
typedef void (*CFunction)(char* ram, int* sp, int* fp);

/* Lib managment functions */
void ResetLib();
int Lib_RegisterFunction(char* name, CFunction func, int ret);

/* Get functions by there ID */
CFunction Lib_GetFunction(int id);
int Lib_GetReturn(int id);
int Lib_GetFunctionIdByName(char* name);

#endif
