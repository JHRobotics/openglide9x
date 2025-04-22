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

extern GLint NumberOfTMUs;

void APIENTRY DummyV( const void *a );

/*
	Glide to OpenGL tmu mapping
	
	3x TMU:
		GL_TEXTURE0 -> TMU2
		GL_TEXTURE1 -> TMU2 - invertor
		GL_TEXTURE2 -> TMU1
		GL_TEXTURE3 -> TMU1 - invertor
		GL_TEXTURE4 -> TMU0
		GL_TEXTURE5 -> TMU0 - invertor
		GK_TEXTURE6 -> color combine unit

  2x TMU:
		GL_TEXTURE0 -> TMU1
		GL_TEXTURE1 -> TMU1 - invertor
		GL_TEXTURE2 -> TMU0
		GL_TEXTURE3 -> TMU0 - invertor
		GK_TEXTURE4 -> color combine unit

  1x TMU:
		GL_TEXTURE0 -> TMU0
		GL_TEXTURE1 -> TMU0 - invertor
		GK_TEXTURE2 -> color combine unit
*/	

inline GLenum OGLUnit(int tmu)
{
	if(OpenGL.MultiTextureTMUs >= 8)
	{
#if GLIDE_NUM_TMU >= 3
		switch(tmu)
		{
			case 2: return GL_TEXTURE0;
			case 1: return GL_TEXTURE2;
			case 0: return GL_TEXTURE4;
		}
#elif GLIDE_NUM_TMU == 2
		switch(tmu)
		{
			case 1: return GL_TEXTURE0;
			case 0: return GL_TEXTURE2;
		}
#endif
	}
	else
	{
#if GLIDE_NUM_TMU >= 3
		switch(tmu)
		{
			case 2: return GL_TEXTURE0;
			case 1: return GL_TEXTURE1;
			case 0: return GL_TEXTURE3;
		}
#elif GLIDE_NUM_TMU == 2
		switch(tmu)
		{
			case 1: return GL_TEXTURE0;
			case 0: return GL_TEXTURE1;
		}
#endif
	}
	return GL_TEXTURE0;
}

inline GLenum OGLUnitExtra(int tmu)
{
	if(OpenGL.MultiTextureTMUs >= 8)
	{
		return OGLUnit(tmu)+1;
	}
	
	return GL_ZERO;
}

inline void OGLOne2DUnit(GLenum unit)
{
#if GLIDE_NUM_TMU >= 3
	p_glActiveTextureARB(OGLUnit(GR_TMU2));
	DGL(glDisable)(GL_TEXTURE_2D);
#endif
#if GLIDE_NUM_TMU >= 2
	p_glActiveTextureARB(OGLUnit(GR_TMU1));
	DGL(glDisable)(GL_TEXTURE_2D);
#endif
	p_glActiveTextureARB(OGLUnit(GR_TMU0));
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

inline GLenum OGLColorCombineUnit()
{
	if(OpenGL.MultiTextureTMUs >= 8)
	{
#if GLIDE_NUM_TMU >= 3
	return GL_TEXTURE6;
#elif GLIDE_NUM_TMU >= 2
	return GL_TEXTURE4;
#else
	return GL_TEXTURE2;
#endif
	}
	else
	{
#if GLIDE_NUM_TMU >= 3
	return GL_TEXTURE6;
#elif GLIDE_NUM_TMU >= 2
	return GL_TEXTURE2;
#else
	return GL_TEXTURE1;
#endif
	}
}

extern GLuint emptytex;

inline void OGLInvertor(int tmu, bool inv_color, bool inv_alpha)
{
	GLenum tu = OGLUnitExtra(tmu);
	
	if(tu != GL_ZERO)
	{
		p_glActiveTextureARB(tu);
	
		DGL(glEnable)(GL_TEXTURE_2D);
		DGL(glBindTexture)(GL_TEXTURE_2D, emptytex);
		
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_REPLACE);
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_PREVIOUS);
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND0_RGB,     inv_color ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
	
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_REPLACE);
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_PREVIOUS);
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA,   inv_alpha ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
	}
}

#endif
