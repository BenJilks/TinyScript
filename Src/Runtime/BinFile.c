#include "BinFile.h"
#include <stdio.h>
#include <string.h>

/* Bin file data */
FILE* bin_file = NULL;

/* Bin file managment */
void BinFile_Open(char* file_path, char* mode)
{
	if (bin_file != NULL)
		BinFile_Close();
	bin_file = fopen(file_path, mode);
}
void BinFile_Close()
{
	fclose(bin_file);
	bin_file = NULL;
}

/* Write to file */
void BinFile_WriteInt(int i)
{
	char* data = (char*)&i;
	fprintf(bin_file, "%c%c%c%c", 
		data[0], data[1], data[2], data[3]);
}
void BinFile_WriteChar(char c) { fprintf(bin_file, "%c", c); }
void BinFile_WriteString(char* str) { BinFile_WriteChar(strlen(str)); fprintf(bin_file, "%s", str); }
void BinFile_WriteData(char* data, int size) { int i; for (i = 0; i < size; i++) BinFile_WriteChar(data[i]); }

/* Read from file */
int BinFile_ReadInt()
{
	char data[4];/* Bin file managment */
	fscanf(bin_file, "%c%c%c%c", 
		&data[0], &data[1], &data[2], &data[3]);
	return *(int*)data;
}
char BinFile_ReadChar() 
{ 
	char c;
	fscanf(bin_file, "%c", &c); 
	return c; 
}
void BinFile_ReadString(char* str)
{
	int i, size = BinFile_ReadChar();
	for (i = 0; i < size; i++)
		str[i] = BinFile_ReadChar();
	str[i] = '\0';
}
void BinFile_ReadData(char* data, int size)
{
	int i;
	for (i = 0; i < size; i++)
		data[i] = BinFile_ReadChar();
}
