#ifndef __OBJEXPORT_H__
#define __OBJEXPORT_H__

#include <gl\GL.h>
#include "KMD.h"

void OBJExport();
void OBJUpdate(KMD_VERT *inVert);
void OBJEndStrip();
void OBJBegin(char *outfn);
void OBJSetTexture(GLuint texid);
void OBJAddMat(char *fn, GLuint id);
void OBJPushQuad(KMD_VERT *vert1, KMD_VERT *vert2, KMD_VERT *vert3, KMD_VERT *vert4);

#endif // __OBJEXPORT_H__