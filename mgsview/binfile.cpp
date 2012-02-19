#include "stdafx.h"
#include "binfile.h"

BINFILE *binopen(char* fn)
{
	DWORD size;
	FILE *inFile = fopen(fn, "rb");
	DWORD err = GetLastError();
	BINFILE *handle = (BINFILE*)malloc(sizeof(BINFILE));
	fseek(inFile, 0, SEEK_END);
	size = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	handle->base=handle->data = handle->current = (BYTE*)malloc(size);
	handle->size = size;
	fread(handle->data, size, 1, inFile);
	fclose(inFile);
	handle->eof = false;
	return(handle);
}
BINFILE *binopen(void *data, DWORD size)
{
	BINFILE *handle = (BINFILE*)calloc(1, sizeof(BINFILE));
	handle->size = size;
	handle->base = handle->current = (BYTE*)data;
	handle->eof = false;
	return(handle);
}
void binclose(BINFILE *handle)
{
	if(handle)
	{
		if(handle->base)
			free(handle->base);
		free(handle);
	}
	handle = NULL;
}
void binseek(BINFILE *handle, DWORD offset, BYTE origin)
{
	switch(origin)
	{
	case BIN_SET:
		handle->current = handle->base + offset;
		break;
	case BIN_END:
		handle->current = handle->base+handle->size+offset;
		break;
	case BIN_CUR:
		handle->current = handle->current+offset;
		break;
	default:
		break;
	};
}
DWORD binread(void *buffer, size_t size, size_t count, BINFILE *handle)
{
	//BYTE *test = (handle->current + (size*count));
	if((handle->current + (size*count)) < handle->base+handle->size){
		memcpy(buffer, handle->current, size*count);
		handle->current = handle->current + (size*count);
		return(size*count);
	}
	else{
		memcpy(buffer, handle->current, handle->size - (DWORD)(handle->current + (size*count)));
		handle->current = handle->base + handle->size;
		handle->eof = true;
	}
}

BYTE bingetc(BINFILE *handle){
	BYTE c;
	memcpy(&c, handle->current, 1);	
	handle->current++;
	return c;
}

bool bineof(BINFILE *handle){
	return handle->eof;
}

DWORD bintell(BINFILE *handle)
{
	return(handle->current-handle->base);

}
