# TinyScript
A small lightweight, interpretive scripting language

To run a script:

    make
    ./TinyScript <script file>

Hello world:

    include "IO.tiny"
    
    func main()
        println("Hello, World!")
 
Read a file and print its content:

    include "IO.tiny"
    
    func main()
    {
        file : File = File("test.tiny", "rb")
        println(file.read_all())
        file.close()
    }

