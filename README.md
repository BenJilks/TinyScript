# TinyScript
A small lightweight, interpretive scripting language

To run a script:

    make
    ./TinyScript <script file>

Hello world:

    include "IO.tiny"
    
    func main()
        println("Hello, World!")

# Documentation
TinyScript is a dynamically typed language at core, but static types can be assigned. There are no ';'s at the end of lines, all whitespace is ignored including new-lines and indentations. Single line comments are defined with "# ..." and multi-line comments with "#* ... *#". The code is compiled into bytecode and runs on the internal virtual machine.

## Functions
Functions can be declared in the main scope with the format "func name(args...) block". A block can be either a single statement or multiply statements wrapped in "{ ... }" notation. Return statements are given with the 'ret' keyword. For Example:

    func add(a, b)
        ret a + b

Parameter types can be assigned by adding ": type" to the end. As well as return types by adding the same notation to then end of function declarations

    func add(a: int, b: int): int
        ret a + b

Code execution starts at the "main" function, no code can be outside a function scope.

## Variables
Variables are declared by simply assigning them to a value, this will make them a dynamic type that can change during execution. However, if you specify a static type

    var = "String" # Creates a new variable and assigning a string
    var = 20       # As 'var' is dynamic type, its type can change

    i: int = 5     # Variable 'i' is assigned the static type of int
    i = "Apple"    # Cannot be done, as it's a string

You have 4 primitive types: int, float, char and bool. Ints and floats will act as longs and doubles on 64bit systems only (NOTE: not yet implemented). 'String' and 'List' are objects and not primitive types.

## Basic Input/Output
There are three main functions: 'print', 'println' and 'input'. The print function will convert any object given to a string (same behavior as the 'str' function) and print that to console. The println function will do the same, adding a new line at the end. The input function will just ask the user for input and return the next line given. To include variables in your string, you can use the string add operator to concatenate. Anything on the right of a string operation will automatically get converted to a string.

    name = input("Enter your name: ")
    num = as_int(input("Enter a number: "))
    println("Hello, " + name + "! Your number was " + num)

## Lists / Arrays
A list is declared as "[items, ...]" and can hold multiple types at once. The add operator can be used to concatenate arrays together

    list1 = ["Test", 20, 3.16, false]
    list2 = [true, 'a']
    println(list1 + list2)

Output: [Test, 20, 3.16, false, true, a]

Multi-dimensional lists can be declared as such

    arr = [[1, 2], [3, 4], [5, 6]]
    println(arr[1][0])

Output: 3

## Loops
There are three types of loops. A while loop is declared as "while condition". A for loop is declared as "for i = start to end". An iterative for loop is declared as "for item in iterable".

    while running
        println("Is Running")

    for i = 0 to 10
        println(i)

    for prime in [2, 3, 5, 7, 11, 13, 17, 19, 23]
        println(prime)

## Classes
Classes are declared as "class name { ... }". Attributes can only be defined in the scope of the class. Methods are defined using the same function notation. Attributes can be referred to either inside the 'self' object or on there own. The function with the same name as the class is the constructor.

TODO: fix crash when error + get attr

    class Cat
    {
        name: String
        colour: String
        age: int

        func Cat(name, colour, age)
        {
            self.name = name
            self.colour = colour
            self.age = age
        }

        func meow()
            println(name + " says 'Meow!'")
    }

To create a new object, simply call the constructor. To access the properties from a dynamic variable, you must state the type like this: 'object.type:property'

    fluffy = Cat("Fluffy", "brown", 5)
    fluffy.Cat:meow()
    println(fluffy.Cat:colour)

However, if it's a static type variable you can access it without the type

    fluffy: Cat = Cat("Fluffy", "brown", 5)
    fluffy.meow()
    println(fluffy.colour)

## Operator Overloading
Operator overloading can be done by naming a method with that operators name. Operations are: 'operator_add', 'operator_subtract', 'operator_multiply', 'operator_divide', 'operator_to_string', 'operator_get_index', 'operator_set_index' and 'operator_it'

    class Vec2
    {
        x y

        func Vec2(x, y)
        {
            self.x = x
            self.y = y
        }

        func operator_add(right: Vec2)
            ret Vec2(x + right.x, y + right.y)
        
        func operator_subtract(right: Vec2)
            ret Vec2(x - right.x, y - right.y)
        
        func operator_multiply(right: Vec2)
            ret Vec2(x * right.x, y * right.y)
        
        func operator_divide(right: Vec2)
            ret Vec2(x / right.x, y / right.y)
        
        func operator_to_string()
            ret "(" + x + ", " + y + ")"
        
        func operator_it()
            ret [x, y]
    }

To use operators, it does not have to be a static type

    a = Vec2(1.0, 2.0)
    b = Vec2(3.3, 4.0)
    c = Vec2(5.0, 6.6)
    println(a * b + c / a - b)

Output: (5, 7.3)
