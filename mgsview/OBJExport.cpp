#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <direct.h>
#include <set>
#include <vector>
#include <algorithm>
#include <gl\GL.h>
#include "KMD.h"
#include "OBJExport.h"

	typedef struct{
		GLfloat x;
		GLfloat y;
		GLfloat z;
	}vertex;
	
	typedef struct{
		GLfloat u;
		GLfloat v;
		GLfloat padding;
	}texcoord;

	typedef struct{
		GLfloat x;
		GLfloat y;
		GLfloat z;
	}normal;

	typedef struct{
		vertex vert[4];
		texcoord tex[3];
		normal norm[3];

		DWORD vertInd[4];
		DWORD texInd[3];
		DWORD normalInd[3];
		GLuint texID;
	}Quad;




std::vector<vertex*> allVertices;
//std::vector<texcoord*> allTexCoord;
//std::vector<normal*> allNormals;

std::vector<Quad*> allQuads;
std::vector<KMD_VERT*> strip;
FILE *outFile = NULL;
FILE *outMat = NULL;
GLuint currentTex = 0;

// open the file for export and reset memory structures
void OBJBegin(char *outfn)
{
	char *extension = NULL;
	//strcat(outfn, ".obj\0");
	outFile = fopen(outfn, "w");
	extension = strrchr(outfn, '.');
//	strcpy(extension, ".mtl\0");
//	outMat = fopen(outfn, "w");
	fprintf(outFile, "mtllib %s\n", outfn);
	allVertices.clear();
	//allTexCoord.clear();
	//allNormals.clear();
	allQuads.clear();
	strip.clear();
}

void OBJAddMat(char *fn, GLuint id)
{
	char *extension = strrchr(fn, '.');
	strcpy(extension, ".tga\0");
	fprintf(outMat, "newmtl texture%i\n", id);		
	fprintf(outMat, "map_Ka %s\nmap_Kd %s\n\n", fn, fn); 

}

// end strip and process verts into Quads
void OBJEndStrip()
{
	Quad *workingQuad = (Quad*)malloc(sizeof(Quad));
	for(int i = 0; i<strip.size()-4; i+=4)
	{
		//fprintf(outFile, "Face: %i\n", i);
		workingQuad = (Quad*)malloc(sizeof(Quad));
			for(int j = 0; j < 4; j++)
			{
				workingQuad->vert[j].x = (GLfloat)(strip[i+j]->x/65535.0f);
				workingQuad->vert[j].y = (GLfloat)(strip[i+j]->y/65535.0f);
				workingQuad->vert[j].z = (GLfloat)(strip[i+j]->z/65535.0f);
				//fprintf(outFile, "%i: %f, %f, %f\n", j, workingQuad->vert[j].x, workingQuad->vert[j].y, workingQuad->vert[j].z);
			}
		workingQuad->texID = currentTex;
		allQuads.push_back(workingQuad);
		//fprintf(outFile, "\n\n");
	} 
	strip.clear();
}

// update structures with new data
void OBJUpdate(KMD_VERT *inVert)
{	
		vertex *vert = (vertex*)malloc(sizeof(vertex));
		vert->x = (GLfloat)(inVert->x/65535.0f);
		vert->y = (GLfloat)(inVert->y/65535.0f);
		vert->z = (GLfloat)(inVert->z/65535.0f);

		//texcoord *tex = (texcoord*)malloc(sizeof(texcoord));
		//tex->u = inVert->texcoordx;
		//tex->v = inVert->texcoordy;

		//normal *norm = (normal*)malloc(sizeof(normal));
		//norm->x = inVert->normalx;
		//norm->y = inVert->normaly;
		//norm->z = inVert->normalz;
	
		//allNormals.push_back(norm);
		//allTexCoord.push_back(tex);
		allVertices.push_back(vert);
		KMD_VERT *saveVert = (KMD_VERT*)malloc(sizeof(KMD_VERT));
		memcpy(saveVert, inVert, sizeof(KMD_VERT));
		strip.push_back(saveVert);
}

void OBJPushQuad(KMD_VERT *vert1, KMD_VERT *vert2, KMD_VERT *vert3, KMD_VERT *vert4){
	Quad *quad = (Quad*)malloc(sizeof(Quad));
	quad->vert[0].x = vert1->x;
	quad->vert[0].y = vert1->y;
	quad->vert[0].z = vert1->z;

	quad->vert[1].x = vert2->x;
	quad->vert[1].y = vert2->y;
	quad->vert[1].z = vert2->z;

	quad->vert[2].x = vert3->x;
	quad->vert[2].y = vert3->y;
	quad->vert[2].z = vert3->z;

	quad->vert[3].x = vert4->x;
	quad->vert[3].y = vert4->y;
	quad->vert[3].z = vert4->z;

	allQuads.push_back(quad);
}

void findIndex()
{
	for(int k = 0; k < allQuads.size(); k++)
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < allVertices.size(); j++)
			{
				if(allVertices[j]->x == allQuads[k]->vert[i].x && 
					allVertices[j]->y == allQuads[k]->vert[i].y &&
					allVertices[j]->z == allQuads[k]->vert[i].z)
				{
					allQuads[k]->vertInd[i] = j+1;
					break;
				}
			}
			/*for(int j = 0; j < allVertices.size(); j++)
			{
				if( allNormals[j]->x == allQuads[k]->norm[i].x && 
					allNormals[j]->y == allQuads[k]->norm[i].y &&
					allNormals[j]->z == allQuads[k]->norm[i].z)
				{
					allQuads[k]->normalInd[i] = j+1;
					j=0;
					break;
				}
			}
			for(int j = 0; j < allVertices.size(); j++)
			{
				if( allTexCoord[j]->u == allQuads[k]->tex[i].u && 
					allTexCoord[j]->v == allQuads[k]->tex[i].v)
				{
					allQuads[k]->texInd[i] = j+1;
					j=0;
					break;
				}
			}	*/		
		}
	}
}

void writeVerts(vertex *vert)
{
	fprintf(outFile, "v %f %f %f\n", vert->x, vert->y, vert->z);
}

void writeVerts2(Quad *quad)
{
	static int vertindex = 1;
	quad->vertInd[0]=vertindex;
	fprintf(outFile, "v %f %f %f\n", quad->vert[0].x/65535.0f, quad->vert[0].y/65535.0f, quad->vert[0].z/65535.0f);
	vertindex++;
	quad->vertInd[1]=vertindex;
	fprintf(outFile, "v %f %f %f\n", quad->vert[1].x/65535.0f, quad->vert[1].y/65535.0f, quad->vert[1].z/65535.0f);
	vertindex++;
	quad->vertInd[2]=vertindex;
	fprintf(outFile, "v %f %f %f\n", quad->vert[2].x/65535.0f, quad->vert[2].y/65535.0f, quad->vert[2].z/65535.0f);
	vertindex++;
	quad->vertInd[3]=vertindex;
	fprintf(outFile, "v %f %f %f\n", quad->vert[3].x/65535.0f, quad->vert[3].y/65535.0f, quad->vert[3].z/65535.0f);
	vertindex++;
}


void writeNormal(normal *norm)
{
	fprintf(outFile, "vn %f %f %f\n", norm->x, norm->y, norm->z);
}

void writeTex(texcoord*tex)
{
	fprintf(outFile, "vt %f %f \n", tex->u, tex->v);
}

void writeFaces(Quad *Quad)
{
	static GLint currentTex = -1;
	char outString[65535];
	memset(outString, 0, 65535);	
	char finString[65535];
	memset(finString, 0, 65535);
	sprintf(finString, "f ");
	/*if(Quad->texID != currentTex)
	{
		currentTex = Quad->texID;
		fprintf(outFile, "\nusemtl texture%i\n", currentTex);
	}*/
	if(Quad->vertInd[2] != Quad->vertInd[3])		
	{
		for(int i = 0; i < 4; i++)
		{
			sprintf(outString, "%i ", Quad->vertInd[i]);//, Quad->texInd[i], Quad->normalInd[i]);
			strcat(finString, outString);
			memset(outString, 0, 65535);	
		}
	}
	else
	{
		for(int i = 0; i < 3; i++)
		{
			sprintf(outString, "%i ", Quad->vertInd[i]);//, Quad->texInd[i], Quad->normalInd[i]);
			strcat(finString, outString);
			memset(outString, 0, 65535);	
		}
	}
	strcat(finString, "\n");
	fprintf(outFile, finString);
}

// process Quads into indexed lists
// perform export file write and reset all structures
void OBJExport()
{
	//printf("%i\n", allVertices.size());
	
	//findIndex();
	//for_each(allVertices.begin(), allVertices.end(), writeVerts);
	//for_each(allNormals.begin(), allNormals.end(), writeNormal);
	//for_each(allTexCoord.begin(), allTexCoord.end(), writeTex);
	for_each(allQuads.begin(), allQuads.end(), writeVerts2);
	for_each(allQuads.begin(), allQuads.end(), writeFaces);
	//allVertices.clear();
	//allTexCoord.clear();
	//allNormals.clear();
	allQuads.clear();
	strip.clear();
	fclose(outFile);
	//fclose(outMat);
}

void OBJSetTexture(GLuint texid)
{

	currentTex = texid;

}
