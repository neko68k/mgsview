#ifndef __BINFILE_H__
#define __BINFILE_H__

#include <stdio.h>

typedef struct
{
	DWORD size;
	BYTE *data;
	BYTE *base;
	BYTE *current;
	char *name;
	bool eof;
}BINFILE;

#define BIN_SET 0
#define BIN_CUR 1
#define BIN_END 2

BINFILE *binopen(char* fn);
BINFILE *binopen(void *data, DWORD size);
void binclose(BINFILE *handle);
void binseek(BINFILE *handle, DWORD offset, BYTE origin);
DWORD binread(void *buffer, size_t size, size_t count, BINFILE *handle);
DWORD bintell(BINFILE *handle);
BYTE bingetc(BINFILE *handle);
bool bineof(BINFILE *handle);
#endif //__BINFILE_H__