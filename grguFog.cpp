//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Fog Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <math.h>
#include <string.h>

#include "GlOgl.h"
#include "GLRender.h"
#include "OGLTables.h"


//*************************************************
//* download a fog table
//* Fog is applied after color combining and before alpha blending.
//*************************************************
#ifndef NEW_FOG
FX_ENTRY void FX_CALL
grFogTable( const GrFog_t *ft )
{
#ifdef OGL_DONE
    GlideMsg( "grFogTable( --- )\n" );
#endif

    if ( InternalConfig.FogEnable )
    {
        memcpy( Glide.FogTable, (GrFog_t *)ft, GR_FOG_TABLE_SIZE * sizeof( FxU8 ) );
        Glide.FogTable[ GR_FOG_TABLE_SIZE ] = 255;

        for ( int i = 0; i < GR_FOG_TABLE_SIZE; i++ )
        {
            for ( unsigned int j = intStartEnd[ i ]; j < intStartEnd[ i + 1 ]; j++ )
            {
                OpenGL.FogTable[ j ] = (FxU8)( Glide.FogTable[ i ] + 
                    ( Glide.FogTable[ i + 1 ] - Glide.FogTable[ i ] ) * ( j - intStartEnd[ i ] ) / 
                    intEndMinusStart[ i ] );
            }
        }
    }
}
#else
FX_ENTRY void FX_CALL
grFogTable( const GrFog_t *ft )
{
#ifdef OGL_DONE
    GlideMsg( "grFogTable( --- )\n" );
#endif

    if ( InternalConfig.FogEnable )
    {
        memcpy( Glide.FogTable, (GrFog_t *)ft, GR_FOG_TABLE_SIZE * sizeof( FxU8 ) );
    }
}
#endif

//*************************************************
FX_ENTRY void FX_CALL
grFogColorValue( GrColor_t fogcolor )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grFogColorValue( %x )\n", fogcolor );
#endif
    EnterGLThread();
    
    RenderDrawTriangles( );

    Glide.State.FogColorValue = fogcolor;
    ConvertColorF( fogcolor, 
                   OpenGL.FogColor[ 0 ], 
                   OpenGL.FogColor[ 1 ], 
                   OpenGL.FogColor[ 2 ], 
                   OpenGL.FogColor[ 3 ] );
    DGL(glFogfv)( GL_FOG_COLOR, &OpenGL.FogColor[0] );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL
grFogMode( GrFogMode_t mode )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grFogMode( 0x%X )\n", mode );
#endif

    static GrFogMode_t  modeSource, 
                        modeAdd;
    static GLfloat      ZeroColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };

		if ( !InternalConfig.FogEnable ) return;

    EnterGLThread();

    RenderDrawTriangles( );

    modeSource = mode & ( GR_FOG_WITH_TABLE | GR_FOG_WITH_ITERATED_ALPHA );
    modeAdd = mode & ( GR_FOG_MULT2 | GR_FOG_ADD2 );

    if ( modeSource )
    {
        OpenGL.Fog = true;
        if ( InternalConfig.EXT_fog_coord )
        {
            DGL(glEnable)( GL_FOG );
        }
    }
    else
    {
        OpenGL.Fog = false;
        DGL(glDisable)( GL_FOG );
    }

    switch ( modeAdd )
    {
    case GR_FOG_MULT2:
    case GR_FOG_ADD2:
        DGL(glFogfv)( GL_FOG_COLOR, &ZeroColor[ 0 ] );
        break;

    default:
        DGL(glFogfv)( GL_FOG_COLOR, &OpenGL.FogColor[ 0 ] );
        break;
    }
    
    Glide.State.FogMode = modeSource;
//    Glide.State.FogMode = mode;

    LeaveGLThread();
}

//*************************************************
#ifndef NEW_FOG
FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t *fogtable, float density )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateExp( ---, %-4.2f )\n", density );
#endif
    
    float f;
    float scale;
    float dp;
    
    dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
    scale = 255.0F / ( 1.0F - (float) exp( -dp ) );
    
    for ( int i = 0; i < GR_FOG_TABLE_SIZE; i++ )
    {
        dp = density * guFogTableIndexToW( i );
        f = ( 1.0F - (float) exp( -dp ) ) * scale;
        
        if ( f > 255.0F )
        {
            f = 255.0F;
        }
        else if ( f < 0.0F )
        {
            f = 0.0F;
        }
        
        fogtable[ i ] = (GrFog_t) f;
    }
}
#else
FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t *fogtable, float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -dp ) );

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -dp ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
}
#endif

//*************************************************
#ifndef NEW_FOG
FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t *fogtable, float density )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateExp2( ---, %-4.2f )\n", density );
#endif

    float Temp;

    for ( int i = 0; i < GR_FOG_TABLE_SIZE; i++ )
    {
        Temp = ( 1.0f - (float) exp( ( -density)  * guFogTableIndexToW( i ) ) * 
               (float)exp( (-density)  * guFogTableIndexToW( i ) ) )  * 255.0f;
        fogtable[ i ] = (FxU8) Temp;
    }
}
#else
FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t *fogtable, float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * guFogTableIndexToW( GR_FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -( dp * dp ) ) );

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
     dp = density * guFogTableIndexToW( i );
     f = ( 1.0F - ( float ) exp( -( dp * dp ) ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;
     fogtable[i] = ( GrFog_t ) f;
  }
} /* guFogGenerateExp2 */

#endif

//*************************************************
#ifndef NEW_FOG
FX_ENTRY void FX_CALL
guFogGenerateLinear( GrFog_t *fogtable,
                     float nearZ, float farZ )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guFogGenerateLinear( ---, %-4.2f, %-4.2f )\n", nearZ, farZ );
#endif

    int Start, 
        End, 
        i;

    for( Start = 0; Start < GR_FOG_TABLE_SIZE; Start++ )
    {
        if ( guFogTableIndexToW( Start ) >= nearZ )
        {
            break;
        }
    }
    for( End = 0; End < GR_FOG_TABLE_SIZE; End++ )
    {
        if ( guFogTableIndexToW( End ) >= farZ )
        {
            break;
        }
    }

    ZeroMemory( fogtable, GR_FOG_TABLE_SIZE );
    for( i = Start; i <= End; i++ )
    {
        fogtable[ i ] = (FxU8)((float)( End - Start ) / 255.0f * (float)( i - Start ));
    }

    for( i = End; i < GR_FOG_TABLE_SIZE; i++ )
    {
        fogtable[ i ] = 255;
    }
}
#else
FX_ENTRY void FX_CALL
guFogGenerateLinear( GrFog_t *fogtable,
                     float nearZ, float farZ )
{
   int i;
   float world_w;
   float f;

  for ( i = 0; i < GR_FOG_TABLE_SIZE; i++ ) {
    world_w = guFogTableIndexToW( i );
    if ( world_w > 65535.0F )
      world_w = 65535.0F;

    f = ( world_w - nearZ ) / ( farZ - nearZ );
    if ( f > 1.0F )
      f = 1.0F;
    else if ( f < 0.0F )
      f = 0.0F;
    f *= 255.0F;
    fogtable[i] = ( GrFog_t ) f;
  }
} /* guFogGenerateLinear */
#endif

//*************************************************
//* convert a fog table index to a floating point eye-space w value
//*************************************************
FX_ENTRY float FX_CALL
guFogTableIndexToW( int i )
{
#ifdef OGL_DONE
    GlideMsg( "guFogTableIndexToW( %d )\n", i );
#endif
#ifdef OGL_DEBUG
    if ( ( i < 0 ) ||
         ( i >= GR_FOG_TABLE_SIZE ) )
    {
        Error( "Error on guFogTableIndexToW( %d )\n", i );
    }
#endif

    return tableIndexToW[ i ];
}

