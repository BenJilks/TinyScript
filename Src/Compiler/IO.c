#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/* Define input file data */
char* in_file;
int in_file_length;
int in_file_pointer;
int line_count;

/* Define output file data */
char* out_file;
int out_file_pointer;

/* Init the input and output file */
void InitIO(char* in)
{
	/* Open input file */
	FILE* file = fopen(in, "rb");
	if (!file) Abort("Could not open input file");
	fseek(file, 0, SEEK_END);

	/* Read input file length */
	in_file_length = ftell(file);
	in_file_pointer = 0;
	line_count = 1;
	rewind(file);

	/* Read the file data */
	in_file = (char*)malloc(in_file_length);
	fread(in_file, 1, in_file_length, file);
	fclose(file);

	/* Open output file */
	out_file = (char*)malloc(1024);
	out_file_pointer = 0;
}

/* Write a line to the output file */
void WriteLine(char* line)
{
	sprintf(out_file + out_file_pointer, "%s\n", line);
	out_file_pointer += strlen(line) + 1;
}

/* Read a char from the input file */
char ReadChar()
{
	if (in_file_pointer < in_file_length)
	{
		if (IsFileEnd())
			return '\0';
		
		char c = in_file[in_file_pointer++];
		if (c == '\n')
			line_count++;
		return c;
	}
}

/* Reverses the file pointer back one char */
void FileBack()
{
	if (in_file_pointer > 0)
	{
		/* Update line count */
		char c = in_file[in_file_pointer - 1];
		if (c == '\n' || c == '\r')
			line_count--;
	
		/* Move the counter back one */
		in_file_pointer--;
	}
}

/* Returns if the end of the input 
file has been reached */
int IsFileEnd()
{
	return in_file_pointer > in_file_length;
}

/* Close the file data */
void CloseIO()
{
	free(in_file);
	free(out_file);
}

/* Returns how many lines have been processed */
int GetLineCount()
{
	return line_count;
}

/* Writes the output data into a file */
void WriteOutputToFile(char* file_path)
{
	FILE* file = fopen(file_path, "wb");
	fprintf(file, "%s", out_file);
	fclose(file);
}

/* Returns the output code */
char* GetOutput() { return out_file; }
int GetOutputSize() { return out_file_pointer; }
