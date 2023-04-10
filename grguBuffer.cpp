//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    Buffer functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <stdio.h>

#include "GlOgl.h"
#include "GLRender.h"

#include "platform/window.h"

// extern functions
void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A );


//*************************************************
//* Clear all the Buffers
//*************************************************
FX_ENTRY void FX_CALL
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU16 depth )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "grBufferClear( %d, %d, %d )\n", color, alpha, depth );
#endif

    static GrColor_t    old_color = 0;
    static float        BR = 0.0f, 
                        BG = 0.0f, 
                        BB = 0.0f, 
                        BA = 0.0f;
    static unsigned int Bits;
    
    Bits = 0;
    
    RenderDrawTriangles( );
    
    if ( OpenGL.ColorMask )
    {
        Bits = GL_COLOR_BUFFER_BIT;
        if ( color != old_color )
        {
            old_color = color;
            ConvertColorF( color, BR, BG, BB, BA );
        }
        glClearColor( BR, BG, BB, BA );
    }
    
    if ( Glide.State.DepthBufferWritting )
    {
        glClearDepth( depth * D1OVER65535 );
        Bits |= GL_DEPTH_BUFFER_BIT;
    }

	if ( ! OpenGL.Clipping )
	{
	    glClear( Bits );
	}
	else
	{
		glEnable( GL_SCISSOR_TEST );
		glClear( Bits );
		glDisable( GL_SCISSOR_TEST );
	}

#ifdef OPENGL_DEBUG
    GLErro( "grBufferClear" );
#endif
}

//*************************************************
//* Swaps Front and Back Buffers
//*************************************************
FX_ENTRY void FX_CALL
grBufferSwap( int swap_interval )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "grBufferSwap( %d )\n", swap_interval );
#endif

    RenderDrawTriangles( );
    glFlush( );

#ifdef OGL_DEBUG
    static float    Temp = 1.0f;

    if ( OGLRender.FrameTriangles > OGLRender.MaxTriangles )
    {
        OGLRender.MaxTriangles = OGLRender.FrameTriangles;
    }
    OGLRender.FrameTriangles = 0;
#endif

    SwapBuffers( );

#ifdef OGL_DEBUG
    RDTSC( FinalTick );
    Temp = (float)(FinalTick - InitialTick);
    FpsAux += Temp;
    Frame++;
    RDTSC( InitialTick );
#endif

#ifdef OPENGL_DEBUG
    GLErro( "grBufferSwap" );
#endif
}

//*************************************************
//* Return the number of queued buffer swap requests
//* Always 0, never pending
//*************************************************
FX_ENTRY int FX_CALL
grBufferNumPending( void )
{
#ifdef OGL_DONE
    GlideMsg( "grBufferNumPending( ) = 0\n" );
#endif

    return 0; 
}

//*************************************************
//* Defines the Buffer to Render
//*************************************************
FX_ENTRY void FX_CALL
grRenderBuffer( GrBuffer_t dwBuffer )
{
#ifdef OGL_DONE
    GlideMsg( "grRenderBuffer( %d )\n", dwBuffer );
#endif

    RenderDrawTriangles( );

    Glide.State.RenderBuffer = dwBuffer;

    // Valid parameters are only FRONT and BACK ( 0x0 and 0x1 )
    OpenGL.RenderBuffer = GL_FRONT + dwBuffer;

    glDrawBuffer( OpenGL.RenderBuffer );

#ifdef OPENGL_DEBUG
    GLErro( "grRenderBuffer" );
#endif
}
