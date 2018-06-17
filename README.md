# TinyScript
A small lightweight, interpretive scripting language

Hello world:

    include "IO.tiny"
    
    func Main()
        Println("Hello, World!")
 
Read a file and print its content:

    include "IO.tiny"
    
    func Main()
    {
        file : File = File("test.tiny", "rb")
        Println(file.ReadAll())
        file.Close()
    }
