//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                 OpenGL Extensions Header
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#ifndef __GLEXTENSIONS__
#define __GLEXTENSIONS__

#ifndef GL_GLEXT_VERSION
#include <GL/glext.h>
#endif

#include <math.h>

void ValidateUserConfig();

// Extensions Functions Declarations
// This cannot have the same name as the prototype functions
// in glext.h, else compiler complains about object being
// redeclared as different type of symbol (func->variable).
extern PFNGLCLIENTACTIVETEXTUREARBPROC          p_glClientActiveTexture;
extern PFNGLMULTITEXCOORD4FARBPROC              p_glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC             p_glMultiTexCoord4fvARB;
extern PFNGLACTIVETEXTUREARBPROC                p_glActiveTextureARB;
extern PFNGLSECONDARYCOLOR3UBVEXTPROC           p_glSecondaryColor3ubvEXT;
extern PFNGLSECONDARYCOLOR3UBEXTPROC            p_glSecondaryColor3ubEXT;
extern PFNGLSECONDARYCOLORPOINTEREXTPROC        p_glSecondaryColorPointerEXT;
extern PFNGLFOGCOORDFEXTPROC                    p_glFogCoordfEXT;
extern PFNGLFOGCOORDPOINTEREXTPROC              p_glFogCoordPointerEXT;
extern PFNGLSECONDARYCOLOR3FVEXTPROC            p_glSecondaryColor3fvEXT;
extern PFNGLSECONDARYCOLOR3FEXTPROC             p_glSecondaryColor3fEXT;

extern PFNGLCOLORTABLEEXTPROC                   p_glColorTableEXT;
extern PFNGLCOLORSUBTABLEEXTPROC                p_glColorSubTableEXT;
extern PFNGLGETCOLORTABLEEXTPROC                p_glGetColorTableEXT;
extern PFNGLGETCOLORTABLEPARAMETERIVEXTPROC     p_glGetColorTableParameterivEXT;
extern PFNGLGETCOLORTABLEPARAMETERFVEXTPROC     p_glGetColorTableParameterfvEXT;

extern PFNGLBLENDFUNCSEPARATEEXTPROC            p_glBlendFuncSeparateEXT;


void APIENTRY DummyV( const void *a );

inline GLenum OGLUnit(int tmu)
{
#if 0
#if GLIDE_NUM_TMU >= 3
	switch(tmu)
	{
		case 2: return GL_TEXTURE0;
		case 1: return GL_TEXTURE1;
		case 0: return GL_TEXTURE2;
	}
#elif GLIDE_NUM_TMU == 2
	switch(tmu)
	{
		case 1: return GL_TEXTURE0;
		case 0: return GL_TEXTURE1;
	}
#endif

	return GL_TEXTURE0;
#else
	return GL_TEXTURE0+tmu;
#endif
}

inline void OGLOne2DUnit(GLenum unit)
{
#if GLIDE_NUM_TMU >= 3
	p_glActiveTextureARB(GL_TEXTURE2);
	DGL(glDisable)(GL_TEXTURE_2D);
#endif
#if GLIDE_NUM_TMU >= 2
	p_glActiveTextureARB(GL_TEXTURE1);
	DGL(glDisable)(GL_TEXTURE_2D);
#endif
	p_glActiveTextureARB(GL_TEXTURE0);
	DGL(glDisable)(GL_TEXTURE_2D);

	p_glActiveTextureARB(unit);
	DGL(glEnable)(GL_TEXTURE_2D);
}

inline void OGLCoords4(int tmu, GLfloat *vect)
{
	if(p_glMultiTexCoord4fvARB)
	{
		p_glMultiTexCoord4fvARB(OGLUnit(tmu), vect);
	}
	else if(tmu == GR_TMU0)
	{
		DGL(glTexCoord4fv)(vect);
	}
}

#endif
