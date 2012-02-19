#include "stdafx.h"
#include <stdio.h>
#include "dar.h"
#include "binfile.h"
#include "vram.h"

void finddword(BINFILE *file)
{
	while(bintell(file)%4 != 0)
	{	
		binseek(file, 1, SEEK_CUR);
	}
}

void DAR_LoadTextures(char *inFN){
	BINFILE *infile;
	BINFILE *texfile;
	//FILE *outfile;
	void *outbuf = NULL;
	unsigned int *filesize = (unsigned int*)malloc(4);
	memset((void*)filesize, 0, 4);
	unsigned short numfiles;
	char out;

	
	//out = 'a';
	
	unsigned int filecount = 0;

	int i = 0;

	char outfilename[255];
	infile = binopen(inFN);
	//infile = fopen("stg_tex1.dar", "rb");

	if(infile == NULL)
	{
		printf("File not found: %s\n", inFN);
		return;
	}

	//fseek(infile, 4, 0);
	numfiles = bingetc(infile);
	finddword(infile);
	

	for(filecount=0; filecount<=numfiles-1; filecount++)
	{
		//finddword(infile);
		out='a';
		memset(outfilename, 0, 255);
		for(i = 1; i<=255 && (int)out != 0; i++)
		{
			out = bingetc(infile);
			strcpy(&outfilename[i-1], &out);			
			

		}		
		printf("%s, %i\n", outfilename, strlen(outfilename));
		finddword(infile);
		binread(filesize, 4, 1, infile);
		
		finddword(infile);
		
		outbuf = (void*)realloc(outbuf, *filesize+1);		
		DWORD err = GetLastError();
		//outfile = fopen(outfilename, "wb");
		
		if(filecount == numfiles-1)
			binread((void*)outbuf, 1, *filesize, infile);	
		else
			binread((void*)outbuf, 1, *filesize+1, infile);	
		
		texfile = binopen(outbuf, *filesize);
		if(outfilename[0] != 't')	// ignore credits textures, just cuz
			VRAM_LoadTexture(texfile, true);
		else
			VRAM_LoadTexture(texfile, false);
		
	}
	binclose(texfile);
	binclose(infile);
}