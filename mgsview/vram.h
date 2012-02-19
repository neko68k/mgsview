#include "stdafx.h"
#include "binfile.h"

#ifndef __VRAM_H__
#define __VRAM_H__

void VRAM_Save();
void VRAM_LoadTexture(BINFILE *inFile, bool draw);
void VRAM_SetTPage(WORD x, WORD y);
void VRAM_GetTexture(WORD x, WORD y);


#endif //__VRAM_H__