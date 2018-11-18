
def fib(i):
    if i > 2:
        return fib(i - 1) + fib(i - 2)
    return 1

print(fib(40))
