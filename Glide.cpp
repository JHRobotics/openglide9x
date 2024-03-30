//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                         Main File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <stdio.h>

#include "GlOgl.h"
#include "Glextensions.h"
#include "PGTexture.h"
#include "PGUTexture.h"

#include "platform/error.h"

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Version ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#include "openglide9x.h"
const char * OpenGLideVersion = OPENGLIDE9X_VERSION_STR; /*"0.09rc9";*/


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// Main Structs
GlideStruct     Glide;
OpenGLStruct    OpenGL;

// Classes
PGTexture       *Textures;
PGUTexture       UTextures;

#ifdef OGL_DEBUG
    // Profiling variables
    FxI64           InitialTick;
    FxI64           FinalTick;
    FxU32           Frame;
    double          Fps;
    double          FpsAux;
#endif

double          ClockFreq;

// Error Function variable
GLIDEERRORFUNCTION ExternErrorFunction;

// Number of Errors
unsigned long NumberOfErrors;

// Support DLL functions

void InitMainVariables( void )
{
    OpenGL.WinOpen = false;
    OpenGL.GlideInit = false;
    NumberOfErrors = 0;
    GetOptions( );
}

bool InitWindow( FxU hWnd )
{
		char Path[MAX_PATH];
		
    if ( !InitialiseOpenGLWindow( hWnd, 0, 0,  OpenGL.WindowWidth, OpenGL.WindowHeight ) ) {
        Error( "Failed to Initialise OpenGL Window!\n" );
        return false;
    }

    if ( !strcmp( (char*)DGL(glGetString)( GL_RENDERER ), "GDI Generic" ) )
        ReportWarning("You are running in a Non-Accelerated OpenGL!!!\nThings can become really slow");

    ValidateUserConfig( );
    GlideMsg( OGL_LOG_SEPARATE );
#ifdef _WIN32
    GetModuleFileNameA(NULL, Path, MAX_PATH-1);;
		Path[MAX_PATH-1] = '\0';
		GlideMsg( "Application EXE: %s\n", Path);
		
		GetModuleFileNameA(glideDLLInt, Path, MAX_PATH-1);;
		Path[MAX_PATH-1] = '\0';
		GlideMsg( "OpenGlide DLL: %s\n", Path);
#endif
    
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( " Setting in Use: \n" );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "Init Full Screen = %s\n", InternalConfig.InitFullScreen ? "true" : "false" );
    GlideMsg( "Fog = %s\n", InternalConfig.FogEnable ? "true" : "false" );
    GlideMsg( "Precision Fix = %s\n", InternalConfig.PrecisionFix ? "true" : "false" );
    GlideMsg( "Wrap 565 to 5551 = %s\n", InternalConfig.Wrap565to5551 ? "true" : "false" );
    GlideMsg( "Texture Memory Size = %d Mb\n", InternalConfig.TextureMemorySize );
    GlideMsg( "Frame Buffer Memory Size = %d Mb\n", InternalConfig.FrameBufferMemorySize );
    GlideMsg( "MMX is %s\n", InternalConfig.MMXEnable ? "present" : "not present" );
    GlideMsg( OGL_LOG_SEPARATE );

#ifdef OGL_DEBUG
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "GlideState size = %d\n", sizeof( GlideState ) );
    GlideMsg( "GrState size = %d\n", sizeof( GrState ) );
    GlideMsg( OGL_LOG_SEPARATE );
#endif

    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "** Glide Calls **\n" );
    GlideMsg( OGL_LOG_SEPARATE );

    return true;
}

//*************************************************
//* Initializes OpenGL
//*************************************************
void InitOpenGL( void )
{
    DGL(glShadeModel)( GL_SMOOTH );

    OpenGL.ZNear = ZBUFFERNEAR;
    OpenGL.ZFar = ZBUFFERFAR;

    DGL(glMatrixMode)( GL_PROJECTION );
    DGL(glLoadIdentity)( );
    DGL(glOrtho)( 0, Glide.WindowWidth, 0, Glide.WindowHeight, OpenGL.ZNear, OpenGL.ZFar );
    DGL(glViewport)( 0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight );

    DGL(glMatrixMode)( GL_MODELVIEW );
    DGL(glLoadIdentity)( );

    DGL(glTexEnvi)( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
//  DGL(glTexEnvi)( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
//  DGL(glTexEnvi)( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
//  DGL(glTexEnvi)( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
//  DGL(glTexEnvi)( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    DGL(glPixelStorei)( GL_PACK_ALIGNMENT, 2);
    DGL(glPixelStorei)( GL_UNPACK_ALIGNMENT, 2);
}
