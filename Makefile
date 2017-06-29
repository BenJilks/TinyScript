all:
	gcc Src/*.c Src/Compiler/*.c Src/Runtime/*.c -IInclude -o TinyScript;
debug:
	gcc -m32 Src/*.c Src/Compiler/*.c Src/Runtime/*.c -IInclude -o TinyScript;
