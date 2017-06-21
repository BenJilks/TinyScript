all:
	gcc Src/*.c Src/Compiler/*.c Src/Runtime/*.c -IInclude -o TinyScript; ./TinyScript
