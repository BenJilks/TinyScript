#ifndef BIN_FILE
#define BIN_FILE

/* Bin file managment */
void BinFile_Open(char* file_path, char* mode);
void BinFile_Close();

/* Write to file */
void BinFile_WriteInt(int i);
void BinFile_WriteChar(char c);
void BinFile_WriteString(char* str);
void BinFile_WriteData(char* data, int size);

/* Read from file */
int BinFile_ReadInt();
char BinFile_ReadChar();
void BinFile_ReadString(char* str);
void BinFile_ReadData(char* data, int size);

#endif
