#include "stdafx.h"
#include <stdio.h>
#include "binfile.h"
#include "vram.h"

typedef struct{
	BYTE magic;
	BYTE ver;
	BYTE isRLE;
	BYTE bpp;

	WORD Xmin;
	WORD Ymin;
	WORD Xmax;
	WORD Ymax;

	WORD hdpi;
	WORD vdpi;

	BYTE palette[48];

	BYTE reserved;
	BYTE numplanes;

	WORD stride;	// decoded bytes per line
	WORD paltype;	// 0 = color, 1 = gray. unreliable
	WORD hres;
	WORD vres;

	WORD mgsmagic;	// always 0x3930
	WORD bppmgs;	// 8 = 4bit, 9 = 8bit
	WORD texX;		// divided by 2
	WORD texY;
	WORD palX;
	WORD palY;
	WORD numcolors;// sometimes this is less than the supported number at bpp
	
	BYTE reserved2[40];
}PCX_HDR;

#define LO(x)  ((BYTE) ((BYTE) (x) & (BYTE) 0x0F))
#define HI(x)  ((BYTE) ((BYTE) (x) >> (BYTE) 4))

BYTE VRAM[1024*512*2];
BYTE runcount = 0;
BYTE runvalue = 0;

// borrowed from psx_vram by agemo
WORD C8toC5(BYTE r, BYTE g, BYTE b){
	WORD outcolor = 0;
	BYTE or, og, ob;
	
	or = r;
	og = g;
	ob = b;

	or >>= 3;
	og >>= 3;
	ob >>= 3;

	outcolor = (or) | (og<<5) | (ob<<10);

	return outcolor;
}



// borrowed from netpbm and slightly, very poorly, modified :)
void PCX_Unpack(BYTE *pixels, BYTE *bitplanes, 
	DWORD bytesperline, DWORD planes, DWORD bitsperpixel)
{
    int  i, j;
    int  npixels;
    unsigned char    *p;
	unsigned char    *pa;

    /*
     * clear the pixel buffer
     */
    //npixels = planes*bytesperline;
	npixels = (bytesperline * 8) / bitsperpixel;
    p    = pixels;
    while (--npixels >= 0)
        *p++ = 0;

    /*
     * do the format conversion
     */
	
	// inc set bit
    for (i = 0; i < planes; i++)
    {
        int pixbit, bits, mask, shift;
		// reset dest
		p = pixels;
		// pixel we will set
        pixbit    = (1 << i);
		// inc source
		shift = 0;
        for (j = 0; j < bytesperline; j++)
        {
			// inc src
            bits = *bitplanes++;		

			// copy one bit to one byte
            for (mask = 0x80; mask != 0; mask >>= 1, p++) 
                if (bits & mask)
                    *p |= pixbit;
        }
    }

	// TODO: this is certainly not the best way to do this
	// it works though :D
	npixels = (bytesperline * 8) / bitsperpixel;
	p = pixels;
	for(i = 0;i<npixels;i+=2){
		BYTE fixed = 0;
		fixed = p[i];
		fixed |= p[i+1]<<4;
		p[i]=fixed;		
	}

	npixels = (bytesperline * 8) / bitsperpixel;
	p = pixels;
	j = 2;
	for(i = 1;i<npixels;i++, j+=2){	
		p[i]=p[j];		
	}
	return;
}


WORD PCX_Decode(BYTE *scanline, BINFILE *inFile, WORD length){
	WORD index = 0;
	DWORD total = 0;
	BYTE byte;
	

	do{

		for(total += runcount; runcount && index < length; runcount--, index++){
			scanline[index] = runvalue;
		}

		if(runcount){
			total -= runcount;
			return total;
		}

		byte = bingetc(inFile);
		//if(binerror(inFile))
			//return(EOF);
		if((byte&0xC0) == 0xC0){
			runcount = byte & 0x3F;
			runvalue = bingetc(inFile);
		}
		else{
			runcount = 1;
			runvalue = byte;
		}
	}while(index<length);
	

	return(total);
}

void VRAM_Save(){
	FILE *outfile;
	outfile = fopen("vram.bin", "wb");
	fwrite(VRAM, 1024*512*2, 1, outfile);	
	fclose(outfile);
}

void VRAM_LoadTexture(BINFILE *inFile, bool draw){
	PCX_HDR header;
	

	DWORD y = 0;
	BYTE *scanline;
	BYTE *scan4;

	

	//inFile = binopen(data, size);
	binread(&header, sizeof(PCX_HDR), 1, inFile);
	DWORD xsize = header.Xmax - header.Xmin +1;
	DWORD ysize = header.Ymax - header.Ymin +1;	
	WORD scanlen = header.numplanes*header.stride;
	scanline = (BYTE*)malloc(scanlen);
	
	runcount = 0;
	runvalue = 0;
	if(header.numplanes == 1 && header.bpp == 8){
		// we are 8 bit
		if(header.isRLE){
			// decode RLE
			for(y=0; y< ysize; y++)
			{
				PCX_Decode(scanline, inFile, scanlen);	
				// copy data proper directly into vram
				if(draw)
					memcpy(&VRAM[(y+header.texY)*1024*2+header.texX*2], scanline, scanlen);

			}
		}
		else{
			int breakp = 1;
			// decode straight
		}		
		if(header.ver == 5){
			binseek(inFile, -769, SEEK_END);
				if(bingetc(inFile) == 0x0C){
				// read number of actual colors defined in extra data
				// this is 24-bit color, wtf :/
				BYTE pal[768];
				WORD color[256];
				memset(color, 0, 256*2);
				int j = 0;
				binread(&pal, header.numcolors, 3, inFile);
				for(int i = 0; i<header.numcolors*3; i+=3, j++)
					color[j] = C8toC5(pal[i], pal[i+1], pal[i+2]);
				if(draw){
					memcpy(&VRAM[(header.palY+1)*1024*2+header.palX*2], color, header.numcolors*2);
					//memcpy(&VRAM[(header.palY-32)*1024*2+header.palX*2], color, header.numcolors*2);
				}
			}
		}
	}
	else if(header.numplanes == 4 && header.bpp == 1){
		// we are 4 
		WORD cols = (header.stride*8)/header.bpp;
		scan4 = (BYTE*)malloc(cols*2);
		WORD copylen;
		if(header.isRLE){
			// decode RLE
			for(y=0; y< ysize; y++)
			{
				
				PCX_Decode(scanline, inFile, scanlen);	

				PCX_Unpack(scan4, scanline, header.stride, header.numplanes, header.bpp);				
				if(draw)
					memcpy(&VRAM[(y+header.texY)*1024*2+header.texX*2], scan4, xsize/2);

			}
		}
		else{
			// decode straight
		}
		free(scan4);
		int j = 0;
		WORD color[16];
		memset(color, 0, 16*2);
		for(int i = 0; i<header.numcolors*3; i+=3, j++)
			color[j] = C8toC5(header.palette[i], header.palette[i+1], header.palette[i+2]);
		
		if(draw){
			memcpy(&VRAM[(header.palY+1)*1024*2+header.palX*2], color, header.numcolors*2);
			//memcpy(&VRAM[(header.palY+1-32)*1024*2+header.palX*2], color, header.numcolors*2);						
		}
		
	}

	//binclose(inFile);
	free(scanline);
	
}

void VRAM_SetTPage(WORD x, WORD y);
void VRAM_GetTexture(WORD x, WORD y);