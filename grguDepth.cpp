//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*               Depth (Z/W-Buffer) Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include "GlOgl.h"
#include "GLRender.h"

//*************************************************
//* Changes Depth Buffer Mode
//*************************************************
FX_ENTRY void FX_CALL
grDepthBufferMode( GrDepthBufferMode_t mode )
{
#ifdef OGL_DONE
    GlideMsg( "grDepthBufferMode( %d )\n", mode );
#endif
    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.DepthBufferMode = mode;

    /*
    * In AddTriangle etc.  Use of z or w for
    * depth buffering is determined by the
    * value of OpenGL.DepthBufferType.  So
    * I set it here.
    */
    switch ( mode )
    {
    case GR_DEPTHBUFFER_DISABLE:
        OpenGL.DepthBufferType = 0;
        DGL(glDisable)( GL_DEPTH_TEST );
        return;

    case GR_DEPTHBUFFER_ZBUFFER:
    case GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS:
        OpenGL.DepthBufferType = 1;
        OpenGL.ZNear = ZBUFFERNEAR;
        OpenGL.ZFar = ZBUFFERFAR;
        break;

    case GR_DEPTHBUFFER_WBUFFER:
    case GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS:
        OpenGL.DepthBufferType = 0;
        OpenGL.ZNear = WBUFFERNEAR;
        OpenGL.ZFar = WBUFFERFAR;
        break;
    }

    DGL(glEnable)( GL_DEPTH_TEST );

    DGL(glMatrixMode)( GL_PROJECTION );
    DGL(glLoadIdentity)( );

    if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
    {
        DGL(glOrtho)( Glide.State.ClipMinX, Glide.State.ClipMaxX,
                 Glide.State.ClipMinY, Glide.State.ClipMaxY,
                 OpenGL.ZNear, OpenGL.ZFar );
        DGL(glViewport)( OpenGL.ClipMinX, OpenGL.ClipMinY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }
    else
    {
        DGL(glOrtho)( Glide.State.ClipMinX, Glide.State.ClipMaxX, 
                 Glide.State.ClipMaxY, Glide.State.ClipMinY, 
                 OpenGL.ZNear, OpenGL.ZFar );
        DGL(glViewport)( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }

    DGL(glMatrixMode)( GL_MODELVIEW );

#ifdef OPENGL_DEBUG
    GLErro( "grDepthBufferMode" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Enables or Disables Depth Buffer Writing
//*************************************************
FX_ENTRY void FX_CALL
grDepthMask( FxBool enable )
{
#ifdef OGL_DONE
    GlideMsg( "grDepthMask( %d )\n", enable );
#endif
    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.DepthBufferWritting = OpenGL.DepthBufferWritting = enable;

    DGL(glDepthMask)( OpenGL.DepthBufferWritting );

#ifdef OPENGL_DEBUG
    GLErro( "grDepthMask" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Sets the Depth Function to use
//*************************************************
FX_ENTRY void FX_CALL
grDepthBufferFunction( GrCmpFnc_t func )
{
#ifdef OGL_DONE
    GlideMsg( "grDepthBufferFunction( %d )\n", func );
#endif
    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.DepthFunction = func;

    // We can do this just because we know the constant values for both OpenGL and Glide
    // To port it to anything else than OpenGL we NEED to change this code
    OpenGL.DepthFunction = GL_NEVER + func;

    DGL(glDepthFunc)( OpenGL.DepthFunction );

#ifdef OPENGL_DEBUG
    GLErro( "grDepthBufferFunction" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Set the depth bias level
//*************************************************
#ifndef GLIDE3
FX_ENTRY void FX_CALL grDepthBiasLevel( FxI16 level )
#else
FX_ENTRY void FX_CALL grDepthBiasLevel( FxI32 level )
#endif
{
#ifdef OGL_PARTDONE
    GlideMsg( "grDepthBiasLevel( %d )\n", level );
#endif
    EnterGLThread();

    RenderDrawTriangles( );

		Glide.State.DepthBiasLevel = level;

    if(OpenGL.DepthBufferType)
    {
    	OpenGL.DepthBiasLevel = level * 128.0;
    	//OpenGL.DepthBiasLevel = level;
    }
    else
    {
    	OpenGL.DepthBiasLevel = level * 4.0f;
    	//OpenGL.DepthBiasLevel = level;
    }

    DGL(glPolygonOffset)( 1.0f, OpenGL.DepthBiasLevel );

    if ( level != 0 )
    {
        DGL(glEnable)( GL_POLYGON_OFFSET_FILL );
    }
    else
    {
        DGL(glDisable)( GL_POLYGON_OFFSET_FILL );
    }

#ifdef OPENGL_DEBUG
    GLErro( "grDepthBiasLevel" );
#endif
    LeaveGLThread();
}

