#ifndef __KMD_H__
#define __KMD_H__

typedef struct
{
	short x;
	short y;
	short z;
	WORD pad;	// always 0xFFFF
}KMD_VERT;

void KMD_Load(char *fn);
GLuint KMD_DrawPoints();
void KMD_Export();

#endif