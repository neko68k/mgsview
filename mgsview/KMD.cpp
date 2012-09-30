#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <gl\GL.h>
#include <stdio.h>
#include "kmd.h"
#include "OBJExport.h"

//#define SCALE_FCT 65535.0f
#define SCALE_FCT 256.0f

typedef struct
{
	// will have to work on this :D
	DWORD numBlocks;
	DWORD unkNum;
	DWORD boundingBox[6];
}KMD_HDR;

typedef struct
{
	WORD  numObj;
	WORD  numUnk;
	DWORD numFaces;
	WORD boundingBox[6]; 
	DWORD translation[3]; 
	DWORD parentObjIndex;
	DWORD translationUnk;

	// mesh verts
	DWORD numVerts;
	DWORD vertOfs;
	DWORD indexOfs;

	// these are not part of the mesh, unknown
	DWORD numNorms;
	DWORD normOfs;
	DWORD normIndex;

	DWORD ofsUV;	// uv coords
	DWORD ofsUnk;	

	DWORD nullpad;
}KMD_VERT_HDR;

/*typedef struct
{
	short x;
	short y;
	short z;
	WORD pad;	// always 0xFFFF
}KMD_VERT;
*/

BYTE *buffer = NULL;
KMD_VERT_HDR *blockPtr = NULL;
KMD_VERT *vertPtr = NULL;
BYTE *facePtr = NULL;
KMD_HDR *header = NULL;
DWORD numBlocks = 0;

void KMD_Load(char *fn)
{
	DWORD filesize = 0;
	char exportFN[MAX_PATH];
	memset(exportFN, 0, MAX_PATH);
	FILE *inFile = fopen(fn, "rb");
	fseek(inFile, 0, SEEK_END);
	filesize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	buffer = (BYTE*)malloc(filesize);
	fread(buffer, filesize, 1, inFile);
	fclose(inFile);
	header = (KMD_HDR*)buffer;
	blockPtr = (KMD_VERT_HDR*)(buffer + 0x20);
	sprintf(exportFN, "%s.obj", fn);
	OBJBegin(exportFN);
}

void KMD_Export(char *fn)
{
	FILE* outfile = fopen("test_command.obj", "w");
	for(int i = 0; i< header->numBlocks; i++)
	{
		vertPtr = (KMD_VERT*)(buffer+blockPtr[i].vertOfs);
		facePtr = (BYTE*)(buffer+blockPtr[i].indexOfs);
		//fprintf(outfile, "\n\n%8x\n\n%i\n", blockPtr[i].gpuCommand, blockPtr[i].numFaces);
		for(int k = 0; k<blockPtr[i].numVerts; k++)
		{
			fprintf(outfile, "v %f %f %f\n", (GLfloat)(vertPtr[k].x/(SCALE_FCT)), (GLfloat)(vertPtr[k].y/(SCALE_FCT)), (GLfloat)(vertPtr[k].z/(SCALE_FCT)));
		}
		for(int k = 0; k<blockPtr[i].numFaces*4; k+=4)
		{
			fprintf(outfile, "f %i %i %i %i\n", facePtr[k]+1, facePtr[k+1]+1, facePtr[k+2]+1, facePtr[k+3]+1);
		}
	}

	fclose(outfile);
}

GLuint KMD_DrawPoints()
{
	//glBegin(GL_POINTS);
	GLuint dl = glGenLists(1);
	
    glNewList(dl, GL_COMPILE);
	glBegin(GL_QUADS);
	for(int i = 0; i< header->numBlocks; i++)
	{
		glColor3ub(rand()/255, rand()/255, rand()/255);
		vertPtr = (KMD_VERT*)(buffer+blockPtr[i].vertOfs);
		facePtr = (BYTE*)(buffer+blockPtr[i].indexOfs);
		for(int k = 0; k<blockPtr[i].numFaces*4; k+=4)
		{
			
			/*OBJUpdate(&vertPtr[facePtr[k]]);
			OBJUpdate(&vertPtr[facePtr[k+1]]);
			OBJUpdate(&vertPtr[facePtr[k+2]]);
			OBJUpdate(&vertPtr[facePtr[k+3]]);*/
			OBJPushQuad(&vertPtr[facePtr[k]], &vertPtr[facePtr[k+1]], &vertPtr[facePtr[k+2]], &vertPtr[facePtr[k+3]]);
			glVertex3f((GLfloat)(vertPtr[facePtr[k]].x/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k]].y/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k]].z/(SCALE_FCT)));
			glVertex3f((GLfloat)(vertPtr[facePtr[k+1]].x/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+1]].y/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+1]].z/(SCALE_FCT)));
			glVertex3f((GLfloat)(vertPtr[facePtr[k+2]].x/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+2]].y/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+2]].z/(SCALE_FCT)));
			glVertex3f((GLfloat)(vertPtr[facePtr[k+3]].x/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+3]].y/(SCALE_FCT)), (GLfloat)(vertPtr[facePtr[k+3]].z/(SCALE_FCT)));
			//OBJEndStrip();
		}
		//OBJEndStrip();
	}
	glEnd();
	glEndList();
	OBJExport();
	return dl;
}

KMD_VERT_HDR* KMD_NextBlock()
{

	return 0;

}