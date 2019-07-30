#ifndef VM_H
#define VM_H

typedef struct VMState
{
    int pc;
    int sp;
    int bp;
    int depth;
    char *stack;
} VMState;
typedef void (*VMFunc)(VMState *state);

void vm_init();
void register_external(const char *name, VMFunc func);
int vm_run(char *code, int start, char *return_value);

#endif // VM_H
