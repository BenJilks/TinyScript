# TinyScript
A small lightweight, scripting language. Compiles into low level bytecode and has a strict but easy to understand type system

# Example programs

## Hello world
    import io
    
    func main()
        io.log("Hello, world!")

## Fibonacci
    import io

    func fib(int i) -> int
    {
        if i < 3
            return fib(i - 1) + fib(i - 2)
        return 1
    }

    func main() 
        io.log(fib(10))

## Arrays
    import io

    func main()
    {
        # Arrays are static, size cannot change
        let arr = [1, 2, 3]

        # The type will be 'int array[3]'
        io.log(typename arr)

        # This will output '[1, 2, 3]'
        io.log(arr)
    }

## Auto types
    import io

    # Type auto will allow any type
    func add(auto a, auto b) 
        -> typeof a + b
    {
        return a + b
    }

    func main()
    {
        # Type will automatically be determined
        let result = add(3, 5.7)

        # int + float -> float, so will output 'float'
        io.log(typename result)
    }

