//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Render File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include "GlOgl.h"
#include "GLRender.h"
#include "Glextensions.h"
#include "PGTexture.h"

#include "OGLTables.h"

//#include <algorithm>
//using namespace std;

#ifndef min
#define min(x, y) ((x)<(y)?(x):(y))
#endif

#ifndef max
#define max(x, y) ((x)<(y)?(y):(x))
#endif

//**************************************************************
// Defines
//**************************************************************

#define DEBUG_MIN_MAX( var, maxvar, minvar )    \
    if ( var > maxvar ) maxvar = var;           \
    if ( var < minvar ) minvar = var;

//**************************************************************
// extern variables and functions prototypes
//**************************************************************

//**************************************************************
// Local variables
//**************************************************************

// The functions for the color combine
ALPHAFACTORFUNCPROC AlphaFactorFunc;
COLORFACTORFUNCPROC ColorFactor3Func;
COLORFUNCTIONPROC   ColorFunctionFunc;

// Snapping constant
static const float vertex_snap_compare = 4096.0f;
static const float vertex_snap = float( 3L << 18 );

// Standard structs for the render
RenderStruct OGLRender;

// Variables for the Add functions
static TColorStruct     Local, 
                        Other, 
                        CFactor;
static TColorStruct     *pC,
                        *pC2;
static TVertexStruct    *pV;
static TTextureStruct   *pTS;
static TFogStruct       *pF;
static float            hAspect, wAspect;
static const float maxoow = 65536.0;

//**************************************************************
// Functions definitions
//**************************************************************
static float OGLFogDistance(float w)
{
	  //w =  max(w, D1OVER65535);
    
    int i;
    
    float oow = 65535;
    if(w != 0.0f)
    {
    	oow = 1.0f/w;
    }
    
    for (i = 0; ((i < GR_FOG_TABLE_SIZE-1) && (tableIndexToW[i] < oow)); i++);
    
    return Glide.FogTable[i] * D1OVER255;
}

// Initializes the render and allocates memory
void RenderInitialize( void )
{
    OGLRender.NumberOfTriangles = 0;

    OGLRender.TColor    = new TColorStruct[ MAXTRIANGLES + 1 ];
    OGLRender.TColor2   = new TColorStruct[ MAXTRIANGLES + 1 ];
    OGLRender.TTexture  = new TTextureStruct[ MAXTRIANGLES + 1 ];
    OGLRender.TVertex   = new TVertexStruct[ MAXTRIANGLES + 1 ];
    OGLRender.TFog      = new TFogStruct[ MAXTRIANGLES + 1 ];

#ifdef OGL_DEBUG
    OGLRender.FrameTriangles = 0;
    OGLRender.MaxTriangles = 0;
    OGLRender.MaxSequencedTriangles = 0;
    OGLRender.MinX = OGLRender.MinY = OGLRender.MinZ = OGLRender.MinW = 99999999.0f;
    OGLRender.MaxX = OGLRender.MaxY = OGLRender.MaxZ = OGLRender.MaxW = -99999999.0f;
    OGLRender.MinS = OGLRender.MinT = OGLRender.MinF = 99999999.0f;
    OGLRender.MaxS = OGLRender.MaxT = OGLRender.MaxF = -99999999.0f;

    OGLRender.MinR = OGLRender.MinG = OGLRender.MinB = OGLRender.MinA = 99999999.0f;
    OGLRender.MaxR = OGLRender.MaxG = OGLRender.MaxB = OGLRender.MaxA = -99999999.0f;
#endif
}

// Shutdowns the render and frees memory
void RenderFree( void )
{
    delete[] OGLRender.TColor;
    delete[] OGLRender.TColor2;
    delete[] OGLRender.TTexture;
    delete[] OGLRender.TVertex;
    delete[] OGLRender.TFog;
}

void RenderUpdateArrays( void )
{
	/* JH: OK this is all bad, don't use vertex array yet */
    DGL(glVertexPointer)( 3, GL_FLOAT, 4 * sizeof( GLfloat ), &OGLRender.TVertex[0] );
    DGL(glColorPointer)( 4, GL_FLOAT, 0, &OGLRender.TColor[0] );
    if ( InternalConfig.ARB_multitexture )
    {
        p_glClientActiveTexture( GL_TEXTURE0_ARB );
    }
    DGL(glTexCoordPointer)( 4, GL_FLOAT, 0, &OGLRender.TTexture[0] );
    if ( InternalConfig.ARB_multitexture )
    {
        p_glClientActiveTexture( GL_TEXTURE1_ARB );
        DGL(glTexCoordPointer)( 4, GL_FLOAT, 0, &OGLRender.TTexture[0] );
    }
    p_glSecondaryColorPointerEXT( 3, GL_FLOAT, 4 * sizeof( GLfloat ), &OGLRender.TColor2[0] );
    if ( InternalConfig.EXT_fog_coord )
    {
        //p_glFogCoordPointerEXT( 1, GL_FLOAT, &OGLRender.TFog[0] );
        p_glFogCoordPointerEXT( GL_FLOAT, 0, &OGLRender.TFog[0] );
    }

#ifdef OPENGL_DEBUG
    GLErro( "Render::UpdateArrays" );
#endif
}

#ifndef OGL_DEBUG_HEAVY
// Draw the current saved triangles
void RenderDrawTriangles( void )
{
#else
void RenderDrawTriangles_traced( const char *fn, const int line )
{
	GlideMsg("%s:%d: RenderDrawTriangles()\n", fn, line);
#endif
    //bool use_two_tex = false;
    int tmu;
    
    if ( ! OGLRender.NumberOfTriangles )
    {
        return;
    }

		for(tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
		{
			Textures->MakeReady( tmu );
    }

    if ( OpenGL.Blend )
    {
        DGL(glEnable)( GL_BLEND );
    }
    else
    {
        DGL(glDisable)( GL_BLEND );
    }

    // Alpha Fix
    if ( Glide.State.AlphaOther != GR_COMBINE_OTHER_TEXTURE )
    {
        DGL(glDisable)( GL_ALPHA_TEST );
    }
    else 
    {
        if ( Glide.State.AlphaTestFunction != GR_CMP_ALWAYS )
        {
            DGL(glEnable)( GL_ALPHA_TEST );
        }
    }
    
    if( !OpenGL.Blend && Glide.State.ChromaKeyMode )
    {
        DGL(glAlphaFunc)( GL_GEQUAL, 0.5 );
        DGL(glEnable)( GL_ALPHA_TEST );
        
        DGL(glBegin)( GL_TRIANGLES );
        for ( int i = 0; i < OGLRender.NumberOfTriangles; i++ )
        {
            DGL(glColor3fv)( &OGLRender.TColor[ i ].ar );
            p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].ar );
            p_glFogCoordfEXT( OGLRender.TFog[ i ].af );
            OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].as);

#if GLIDE_NUM_TMU >= 2
            if(InternalConfig.NumTMU >= 2)
            	 OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].as);
#endif
#if GLIDE_NUM_TMU >= 3
            if(InternalConfig.NumTMU >= 3)
            	 OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].as);
#endif
          
            DGL(glVertex3fv)( &OGLRender.TVertex[ i ].ax );
            
            DGL(glColor3fv)( &OGLRender.TColor[ i ].br );
            p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].br );
            p_glFogCoordfEXT( OGLRender.TFog[ i ].bf );
            OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].bs);

#if GLIDE_NUM_TMU >= 2
            if(InternalConfig.NumTMU >= 2)
            	 OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].bs);
#endif
#if GLIDE_NUM_TMU >= 3
            if(InternalConfig.NumTMU >= 3)
            	 OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].bs);
#endif
            DGL(glVertex3fv)( &OGLRender.TVertex[ i ].bx );
            
            DGL(glColor3fv)( &OGLRender.TColor[ i ].cr );
            p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].cr );
            p_glFogCoordfEXT( OGLRender.TFog[ i ].cf );
            OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].cs);

#if GLIDE_NUM_TMU >= 2
            if(InternalConfig.NumTMU >= 2)
            	 OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].cs);
#endif
#if GLIDE_NUM_TMU >= 3
            if(InternalConfig.NumTMU >= 3)
            	 OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].cs);
#endif
            DGL(glVertex3fv)( &OGLRender.TVertex[ i ].cx );
        }
        DGL(glEnd)( );
        
        DGL(glDisable)( GL_ALPHA_TEST );
    }
    else
    {
        if ( InternalConfig.EXT_vertex_array )
        {
            DGL(glDrawArrays)( GL_TRIANGLES, 0, OGLRender.NumberOfTriangles * 3 );
        }
        else
        {
            DGL(glBegin)( GL_TRIANGLES );
            for ( int i = 0; i < OGLRender.NumberOfTriangles; i++ )
            {
                DGL(glColor4fv)( &OGLRender.TColor[ i ].ar );
                p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].ar );
                p_glFogCoordfEXT( OGLRender.TFog[ i ].af );
                OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].as);
#if GLIDE_NUM_TMU >= 2
                if(InternalConfig.NumTMU >= 2)
                    OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].as);
#endif
#if GLIDE_NUM_TMU >= 3
                if(InternalConfig.NumTMU >= 3)
                    OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].as);
#endif
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].ax );
                
                DGL(glColor4fv)( &OGLRender.TColor[ i ].br );
                p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].br );
                p_glFogCoordfEXT( OGLRender.TFog[ i ].bf );
                OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].bs);
#if GLIDE_NUM_TMU >= 2
                if(InternalConfig.NumTMU >= 2)
                    OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].bs);
#endif
#if GLIDE_NUM_TMU >= 3
                if(InternalConfig.NumTMU >= 3)
                    OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].bs);
#endif
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].bx );
                
                DGL(glColor4fv)( &OGLRender.TColor[ i ].cr );
                p_glSecondaryColor3fvEXT( &OGLRender.TColor2[ i ].cr );
                p_glFogCoordfEXT( OGLRender.TFog[ i ].cf );
                OGLCoords4(GR_TMU0, &OGLRender.TTexture[i].tmu[0].cs);
#if GLIDE_NUM_TMU >= 2
                if(InternalConfig.NumTMU >= 2)
                    OGLCoords4(GR_TMU1, &OGLRender.TTexture[i].tmu[1].cs);
#endif
#if GLIDE_NUM_TMU >= 3
                if(InternalConfig.NumTMU >= 3)
                    OGLCoords4(GR_TMU2, &OGLRender.TTexture[i].tmu[2].cs);
#endif
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].cx );
            }
            DGL(glEnd)( );
        }
    }
  
    if ( ! InternalConfig.EXT_secondary_color )
    {
        DGL(glBlendFunc)( GL_ONE, GL_ONE );
        DGL(glEnable)( GL_BLEND );
        DGL(glDisable)( GL_TEXTURE_2D );

        if ( OpenGL.DepthBufferType )
        {
            DGL(glPolygonOffset)( 1.0f, 0.5f );
        }
        else
        {
            DGL(glPolygonOffset)( -1.0f, -0.5f );
        }

        DGL(glEnable)( GL_POLYGON_OFFSET_FILL );

        if (InternalConfig.EXT_vertex_array )
        {
            DGL(glColorPointer)( 4, GL_FLOAT, 0, &OGLRender.TColor2 );
            DGL(glDrawArrays)( GL_TRIANGLES, 0, OGLRender.NumberOfTriangles * 3 );
            DGL(glColorPointer)( 4, GL_FLOAT, 0, &OGLRender.TColor );
        }
        else
        {
            DGL(glBegin)( GL_TRIANGLES );
            for ( int i = 0; i < OGLRender.NumberOfTriangles; i++ )
            {
                DGL(glColor4fv)( &OGLRender.TColor2[ i ].ar );
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].ax );

                DGL(glColor4fv)( &OGLRender.TColor2[ i ].br );
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].bx );

                DGL(glColor4fv)( &OGLRender.TColor2[ i ].cr );
                DGL(glVertex3fv)( &OGLRender.TVertex[ i ].cx );
            }
            DGL(glEnd)( );
        }

        if ( Glide.State.DepthBiasLevel )
        {
            DGL(glPolygonOffset)( 1.0f, OpenGL.DepthBiasLevel );
        }
        else
        {
            DGL(glDisable)( GL_POLYGON_OFFSET_FILL );
        }

        if ( OpenGL.Blend )
        {
            DGL(glBlendFunc)( OpenGL.SrcBlend, OpenGL.DstBlend );
        }
    }

#ifdef OGL_DEBUG
    if ( OGLRender.NumberOfTriangles > OGLRender.MaxSequencedTriangles )
    {
        OGLRender.MaxSequencedTriangles = OGLRender.NumberOfTriangles;
    }

    GLErro( "Render::DrawTriangles" );
#endif

    OGLRender.NumberOfTriangles = 0;
}

static inline bool HasWDiff(int tmu)
{
	switch(tmu)
	{
		case GR_TMU0:
			return (Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU0) != 0;
		case GR_TMU1:
			return (Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU1) != 0;
 		case GR_TMU2:
 			return (Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU1) != 0;
	}
	return false;
}

static inline bool HasSTDiff(int tmu)
{
	switch(tmu)
	{
		case GR_TMU0:
			//return (Glide.State.STWHint & GR_STWHINT_ST_DIFF_TMU0) != 0;
			return true; /* always there is first ST set */
		case GR_TMU1:
			return (Glide.State.STWHint & GR_STWHINT_ST_DIFF_TMU1) != 0;
 		case GR_TMU2:
 			return (Glide.State.STWHint & GR_STWHINT_ST_DIFF_TMU1) != 0;
	}
	return false;
}

#ifndef OGL_DEBUG_HEAVY
void RenderAddTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c, bool unsnap )
{
#else
void RenderAddTriangle_traced( const GrVertex *a, const GrVertex *b, const GrVertex *c, bool unsnap, const char *fn, const int line)
{
	GlideMsg( "%s:%d: RenderAddTriangle XY ( %f %f, %f %f, %f %f)\n", fn, line, a->x, a->y, b->x, b->y, c->x, c->y);
	GlideMsg( "%s:%d: RenderAddTriangle QQ1( %f %f, %f %f, %f %f)\n", fn, line, a->oow, a->tmuvtx[0].oow, b->oow, b->tmuvtx[0].oow, c->oow, c->tmuvtx[0].oow);
	GlideMsg( "%s:%d: RenderAddTriangle ST1( %f %f, %f %f, %f %f)\n", fn, line, a->tmuvtx[0].sow, a->tmuvtx[0].tow, b->tmuvtx[0].sow, b->tmuvtx[0].tow, c->tmuvtx[0].sow, c->tmuvtx[0].tow);
#endif
    pC = &OGLRender.TColor[ OGLRender.NumberOfTriangles ];
    pC2 = &OGLRender.TColor2[ OGLRender.NumberOfTriangles ];
    pV = &OGLRender.TVertex[ OGLRender.NumberOfTriangles ];
    pTS = &OGLRender.TTexture[ OGLRender.NumberOfTriangles ];

    ZeroMemory( pC2, sizeof( TColorStruct ) );

    if ( Glide.ALocal )
    {
        switch ( Glide.State.AlphaLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.aa = a->a * D1OVER255;
            Local.ba = b->a * D1OVER255;
            Local.ca = c->a * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.aa = Local.ba = Local.ca = OpenGL.ConstantColor[ 3 ];
            break;

        case GR_COMBINE_LOCAL_DEPTH:
            Local.aa = a->z * D1OVER255;
            Local.ba = b->z * D1OVER255;
            Local.ca = c->z * D1OVER255;
            break;
        }
    }

    if ( Glide.AOther )
    {
        switch ( Glide.State.AlphaOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.aa = a->a * D1OVER255;
            Other.ba = b->a * D1OVER255;
            Other.ca = c->a * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.aa = Other.ba = Other.ca = OpenGL.ConstantColor[ 3 ];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            if ( OpenGL.Texture[0] )
            {
                Other.aa = Other.ba = Other.ca = 1.0f;
            }
            else
            {
                Other.aa = Other.ba = Other.ca = 0.0f;
            }
            break;
        }
    }

    if ( Glide.CLocal )
    {
        switch ( Glide.State.ColorCombineLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.ar = a->r * D1OVER255;
            Local.ag = a->g * D1OVER255;
            Local.ab = a->b * D1OVER255;
            Local.br = b->r * D1OVER255;
            Local.bg = b->g * D1OVER255;
            Local.bb = b->b * D1OVER255;
            Local.cr = c->r * D1OVER255;
            Local.cg = c->g * D1OVER255;
            Local.cb = c->b * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.ar = Local.br = Local.cr = OpenGL.ConstantColor[ 0 ];
            Local.ag = Local.bg = Local.cg = OpenGL.ConstantColor[ 1 ];
            Local.ab = Local.bb = Local.cb = OpenGL.ConstantColor[ 2 ];
            break;
        }
    }

    if ( Glide.COther )
    {
        switch ( Glide.State.ColorCombineOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.ar = a->r * D1OVER255;
            Other.ag = a->g * D1OVER255;
            Other.ab = a->b * D1OVER255;
            Other.br = b->r * D1OVER255;
            Other.bg = b->g * D1OVER255;
            Other.bb = b->b * D1OVER255;
            Other.cr = c->r * D1OVER255;
            Other.cg = c->g * D1OVER255;
            Other.cb = c->b * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.ar = Other.br = Other.cr = OpenGL.ConstantColor[ 0 ];
            Other.ag = Other.bg = Other.cg = OpenGL.ConstantColor[ 1 ];
            Other.ab = Other.bb = Other.cb = OpenGL.ConstantColor[ 2 ];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            if ( OpenGL.Texture[0] )
            {
                Other.ar = Other.ag = Other.ab = 1.0f;
                Other.br = Other.bg = Other.bb = 1.0f;
                Other.cr = Other.cg = Other.cb = 1.0f;
            }
            else
            {
                Other.ar = Other.ag = Other.ab = 0.0f;
                Other.br = Other.bg = Other.bb = 0.0f;
                Other.cr = Other.cg = Other.cb = 0.0f;
            }
            break;
        }
    }

    ColorFunctionFunc( pC, pC2, &Local, &Other );

    switch ( Glide.State.AlphaFunction )
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->aa = pC->ba = pC->ca = 0.0f;
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->aa = Local.aa;
        pC->ba = Local.ba;
        pC->ca = Local.ca;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = ( 1.0f - AlphaFactorFunc( Local.aa, Other.aa ) ) * Local.aa;
        pC->ba = ( 1.0f - AlphaFactorFunc( Local.ba, Other.ba ) ) * Local.ba;
        pC->ca = ( 1.0f - AlphaFactorFunc( Local.ca, Other.ca ) ) * Local.ca;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa;
        pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba;
        pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * Other.ca;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa;
        pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba + Local.ba;
        pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * Other.ca + Local.ca;
//      pC2->aa =  Local.aa;
//      pC2->ba =  Local.ba;
//      pC2->ca =  Local.ca;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa );
        pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba );
        pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * ( Other.ca - Local.ca );
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa;
        pC->ba = AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ) + Local.ba;
        pC->ca = AlphaFactorFunc( Local.ca, Other.ca ) * ( Other.ca - Local.ca ) + Local.ca;
//      pC2->aa =  Local.ba;
//      pC2->ba =  Local.ba;
//      pC2->ca =  Local.ca;
        break;
    }

    if ( Glide.State.ColorCombineInvert )
    {
        pC->ar = 1.0f - pC->ar - pC2->ar;
        pC->ag = 1.0f - pC->ag - pC2->ag;
        pC->ab = 1.0f - pC->ab - pC2->ab;
        pC->br = 1.0f - pC->br - pC2->br;
        pC->bg = 1.0f - pC->bg - pC2->bg;
        pC->bb = 1.0f - pC->bb - pC2->bb;
        pC->cr = 1.0f - pC->cr - pC2->cr;
        pC->cg = 1.0f - pC->cg - pC2->cg;
        pC->cb = 1.0f - pC->cb - pC2->cb;
        pC2->ar = pC2->ag = pC2->ab = 0.0f;
        pC2->br = pC2->bg = pC2->bb = 0.0f;
        pC2->cr = pC2->cg = pC2->cb = 0.0f;
    }

    if ( Glide.State.AlphaInvert )
    {
        pC->aa = 1.0f - pC->aa - pC2->aa;
        pC->ba = 1.0f - pC->ba - pC2->ba;
        pC->ca = 1.0f - pC->ca - pC2->ca;
        pC2->aa = pC2->ba = pC2->ca = 0.0f;
    }
    
    // Z-Buffering
    if ( ( Glide.State.DepthBufferMode == GR_DEPTHBUFFER_DISABLE ) || 
         ( Glide.State.DepthFunction == GR_CMP_ALWAYS ) )
    {
        pV->az = 0.0f;
        pV->bz = 0.0f;
        pV->cz = 0.0f;
    }
    else
    if ( OpenGL.DepthBufferType )
    {
        pV->az = a->ooz * D1OVER65535;
        pV->bz = b->ooz * D1OVER65535;
        pV->cz = c->ooz * D1OVER65535;
    }
    else
    {
       /*
        * For silly values of oow, depth buffering doesn't
        * seem to work, so map them to a sensible z.  When
        * games use these silly values, they probably don't
        * use z buffering anyway.
        */
        if ( a->oow > 1.0f )
        {
            pV->az = pV->bz = pV->cz = 1.0f;
        }
        else 
        if ( InternalConfig.PrecisionFix )
        {   // Fix precision to 16 integer bits.
            FxU16 w;
            w      = (FxU16)((a->oow / D1OVER65535) + 0.5f);
            pV->az = (float)w * D1OVER65535;
            w      = (FxU16)((b->oow / D1OVER65535) + 0.5f);
            pV->bz = (float)w * D1OVER65535;
            w      = (FxU16)((c->oow / D1OVER65535) + 0.5f);
            pV->cz = (float)w * D1OVER65535;
        }
        else
        {
            pV->az = a->oow;
            pV->bz = b->oow;
            pV->cz = c->oow;
        }
    }

    if ( ( unsnap ) &&
         ( a->x > vertex_snap_compare ) )
    {
        pV->ax = a->x - vertex_snap;
        pV->ay = a->y - vertex_snap;
        pV->bx = b->x - vertex_snap;
        pV->by = b->y - vertex_snap;
        pV->cx = c->x - vertex_snap;
        pV->cy = c->y - vertex_snap;
    }
    else
    {
        pV->ax = a->x;
        pV->ay = a->y;
        pV->bx = b->x;
        pV->by = b->y;
        pV->cx = c->x;
        pV->cy = c->y;
    }
    
    float a_oow = a->oow*maxoow;
		float b_oow = b->oow*maxoow;
		float c_oow = c->oow*maxoow;
		float a_sow = 0;
		float a_tow = 0;
		float b_sow = 0;
		float b_tow = 0;
		float c_sow = 0;
		float c_tow = 0;

		for(int tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
		{
			if(OpenGL.Texture[tmu])
			{
				if(HasSTDiff(tmu))
				{
					a_sow = a->tmuvtx[tmu].sow;
					a_tow = a->tmuvtx[tmu].tow;
					b_sow = b->tmuvtx[tmu].sow;
					b_tow = b->tmuvtx[tmu].tow;
					c_sow = c->tmuvtx[tmu].sow;
					c_tow = c->tmuvtx[tmu].tow;
				}
				
#ifdef GLIDE3
				if(Glide.State.SpaceMode == GR_CLIP_COORDS)
				{
					pTS->tmu[tmu].as = a_sow*a->tmuvtx[tmu].oow;
					pTS->tmu[tmu].at = a_tow*a->tmuvtx[tmu].oow;
					pTS->tmu[tmu].aq = 0.0f;
					pTS->tmu[tmu].aoow = a->tmuvtx[tmu].oow;
	
					pTS->tmu[tmu].bs = b_sow*b->tmuvtx[tmu].oow;
					pTS->tmu[tmu].bt = b_tow*b->tmuvtx[tmu].oow;
					pTS->tmu[tmu].bq = 0.0f;
					pTS->tmu[tmu].boow = b->tmuvtx[tmu].oow;
	
					pTS->tmu[tmu].cs = c_sow*c->tmuvtx[tmu].oow;
					pTS->tmu[tmu].ct = c_tow*c->tmuvtx[tmu].oow;
					pTS->tmu[tmu].cq = 0.0f;
					pTS->tmu[tmu].coow = c->tmuvtx[tmu].oow;
				}
				else
#endif
				{
					if(HasWDiff(tmu))
					{
	    			a_oow = a->tmuvtx[tmu].oow*maxoow;
						b_oow = b->tmuvtx[tmu].oow*maxoow;
						c_oow = c->tmuvtx[tmu].oow*maxoow;
					}

					if(Textures->GetAspect(tmu, &hAspect, &wAspect))
					{
						pTS->tmu[tmu].as = a_sow * wAspect * maxoow;
						pTS->tmu[tmu].at = a_tow * hAspect * maxoow;
						pTS->tmu[tmu].aq = 0;
						pTS->tmu[tmu].aoow = a_oow;

						pTS->tmu[tmu].bs = b_sow * wAspect * maxoow;
						pTS->tmu[tmu].bt = b_tow * hAspect * maxoow;
						pTS->tmu[tmu].bq = 0;
						pTS->tmu[tmu].boow = b_oow;

						pTS->tmu[tmu].cs = c_sow * wAspect * maxoow;
						pTS->tmu[tmu].ct = c_tow * hAspect * maxoow;
						pTS->tmu[tmu].cq = 0;
						pTS->tmu[tmu].coow = c_oow;
					}
				}
			} // OpenGL.Texture[i]
		} // for

    if( InternalConfig.FogEnable )
    {
        pF = &OGLRender.TFog[ OGLRender.NumberOfTriangles ];
#ifdef NEW_FOG
        if ( Glide.State.FogMode == GR_FOG_WITH_TABLE )
#else
        if ( Glide.State.FogMode & GR_FOG_WITH_TABLE )
#endif
        {
#ifndef NEW_FOG
            pF->af = (float)OpenGL.FogTable[ (FxU16)(1.0f / a->oow) ] * D1OVER255;
            pF->bf = (float)OpenGL.FogTable[ (FxU16)(1.0f / b->oow) ] * D1OVER255;
            pF->cf = (float)OpenGL.FogTable[ (FxU16)(1.0f / c->oow) ] * D1OVER255;
#else
            pF->af = OGLFogDistance(a->oow);
            pF->bf = OGLFogDistance(b->oow);
            pF->cf = OGLFogDistance(c->oow);
#endif
        }
        else
        {
            pF->af = a->a * D1OVER255;
            pF->bf = b->a * D1OVER255;
            pF->cf = c->a * D1OVER255;
        }
//        if ( Glide.State.FogMode & GR_FOG_ADD2 )
//        {
//            pF->af = 1.0f - pF->af;
//            pF->bf = 1.0f - pF->bf;
//            pF->cf = 1.0f - pF->cf;
//        }
        
    #ifdef OGL_DEBUG
       DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
       DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
       DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
    #endif
    }

#ifdef OGL_DEBUG
    DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );
    DEBUG_MIN_MAX( pC->br, OGLRender.MaxR, OGLRender.MinR );
    DEBUG_MIN_MAX( pC->cr, OGLRender.MaxR, OGLRender.MinR );
    
    DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );
    DEBUG_MIN_MAX( pC->bg, OGLRender.MaxG, OGLRender.MinG );
    DEBUG_MIN_MAX( pC->cg, OGLRender.MaxG, OGLRender.MinG );

    DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );
    DEBUG_MIN_MAX( pC->bb, OGLRender.MaxB, OGLRender.MinB );
    DEBUG_MIN_MAX( pC->cb, OGLRender.MaxB, OGLRender.MinB );

    DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );
    DEBUG_MIN_MAX( pC->ba, OGLRender.MaxA, OGLRender.MinA );
    DEBUG_MIN_MAX( pC->ca, OGLRender.MaxA, OGLRender.MinA );

    DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );
    DEBUG_MIN_MAX( pV->bz, OGLRender.MaxZ, OGLRender.MinZ );
    DEBUG_MIN_MAX( pV->cz, OGLRender.MaxZ, OGLRender.MinZ );

    DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );
    DEBUG_MIN_MAX( pV->bx, OGLRender.MaxX, OGLRender.MinX );
    DEBUG_MIN_MAX( pV->cx, OGLRender.MaxX, OGLRender.MinX );

    DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );
    DEBUG_MIN_MAX( pV->by, OGLRender.MaxY, OGLRender.MinY );
    DEBUG_MIN_MAX( pV->cy, OGLRender.MaxY, OGLRender.MinY );

//    DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );
//    DEBUG_MIN_MAX( pTS->bs, OGLRender.MaxS, OGLRender.MinS );
//    DEBUG_MIN_MAX( pTS->cs, OGLRender.MaxS, OGLRender.MinS );

//    DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
//    DEBUG_MIN_MAX( pTS->bt, OGLRender.MaxT, OGLRender.MinT );
//    DEBUG_MIN_MAX( pTS->ct, OGLRender.MaxT, OGLRender.MinT );

    OGLRender.FrameTriangles++;
#endif
    
    OGLRender.NumberOfTriangles++;

    if ( OGLRender.NumberOfTriangles >= ( MAXTRIANGLES - 1 ) )
    {
        RenderDrawTriangles( );
    }

#ifdef OPENGL_DEBUG
    GLErro( "Render::AddTriangle" );
#endif
}

void RenderAddLine( const GrVertex *a, const GrVertex *b, bool unsnap )
{
    pC  = &OGLRender.TColor[ MAXTRIANGLES ];
    pC2 = &OGLRender.TColor2[ MAXTRIANGLES ];
    pV  = &OGLRender.TVertex[ MAXTRIANGLES ];
    pTS = &OGLRender.TTexture[ MAXTRIANGLES ];
    pF  = &OGLRender.TFog[ MAXTRIANGLES ];

    // Color Stuff, need to optimize it
    ZeroMemory( pC2, sizeof( TColorStruct ) );

    float atmuoow0 = a->oow;
    float btmuoow0 = b->oow;

    if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU0 ) != 0)
    {
        atmuoow0 = a->tmuvtx[0].oow;
        btmuoow0 = b->tmuvtx[0].oow;
    }

#if GLIDE_NUM_TMU >= 2
    float atmuoow1 = atmuoow0;
    float btmuoow1 = btmuoow0;

    if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU1 ) != 0)
    {
        atmuoow1 = a->tmuvtx[1].oow;
        btmuoow1 = b->tmuvtx[1].oow;
    }
#endif

#if GLIDE_NUM_TMU >= 3
    float atmuoow2 = atmuoow1;
    float btmuoow2 = btmuoow1;

    if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU2 ) != 0)
    {
        atmuoow2 = a->tmuvtx[2].oow;
        btmuoow2 = b->tmuvtx[2].oow;
    }
#endif

    if ( Glide.ALocal )
    {
        switch ( Glide.State.AlphaLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.aa = a->a * D1OVER255;
            Local.ba = b->a * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.aa = Local.ba = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_LOCAL_DEPTH:
            Local.aa = a->z;
            Local.ba = b->z;
            break;
        }
    }

    if ( Glide.AOther )
    {
        switch ( Glide.State.AlphaOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.aa = a->a * D1OVER255;
            Other.ba = b->a * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.aa = Other.ba = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.aa = Other.ba = 1.0f;
            break;
        }
    }

    if ( Glide.CLocal )
    {
        switch ( Glide.State.ColorCombineLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.ar = a->r * D1OVER255;
            Local.ag = a->g * D1OVER255;
            Local.ab = a->b * D1OVER255;
            Local.br = b->r * D1OVER255;
            Local.bg = b->g * D1OVER255;
            Local.bb = b->b * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.ar = Local.br = OpenGL.ConstantColor[0];
            Local.ag = Local.bg = OpenGL.ConstantColor[1];
            Local.ab = Local.bb = OpenGL.ConstantColor[2];
            break;
        }
    }

    if ( Glide.COther )
    {
        switch ( Glide.State.ColorCombineOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.ar = a->r * D1OVER255;
            Other.ag = a->g * D1OVER255;
            Other.ab = a->b * D1OVER255;
            Other.br = b->r * D1OVER255;
            Other.bg = b->g * D1OVER255;
            Other.bb = b->b * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.ar = Other.br = OpenGL.ConstantColor[0];
            Other.ag = Other.bg = OpenGL.ConstantColor[1];
            Other.ab = Other.bb = OpenGL.ConstantColor[2];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.ar = Other.ag = Other.ab = 1.0f;
            Other.br = Other.bg = Other.bb = 1.0f;
            break;
        }
    }

    switch ( Glide.State.ColorCombineFunction )
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->ar = pC->ag = pC->ab = 0.0f; 
        pC->br = pC->bg = pC->bb = 0.0f; 
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
        pC->ar = Local.ar;
        pC->ag = Local.ag;
        pC->ab = Local.ab;
        pC->br = Local.br;
        pC->bg = Local.bg;
        pC->bb = Local.bb;
        break;

    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->ar = pC->ag = pC->ab = Local.aa;
        pC->br = pC->bg = pC->bb = Local.ba;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC->br = CFactor.br * Other.br;
        pC->bg = CFactor.bg * Other.bg;
        pC->bb = CFactor.bb * Other.bb;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC->br = CFactor.br * Other.br;
        pC->bg = CFactor.bg * Other.bg;
        pC->bb = CFactor.bb * Other.bb;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        pC2->br = Local.br;
        pC2->bg = Local.bg;
        pC2->bb = Local.bb;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC->br = CFactor.br * Other.br;
        pC->bg = CFactor.bg * Other.bg;
        pC->bb = CFactor.bb * Other.bb;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        pC2->br = pC2->bg = pC2->bb = Local.ba;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * (Other.ar - Local.ar);
        pC->ag = CFactor.ag * (Other.ag - Local.ag);
        pC->ab = CFactor.ab * (Other.ab - Local.ab);
        pC->br = CFactor.br * (Other.br - Local.br);
        pC->bg = CFactor.bg * (Other.bg - Local.bg);
        pC->bb = CFactor.bb * (Other.bb - Local.bb);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
        if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
            ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
            (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
        {
            pC->ar = Local.ar;
            pC->ag = Local.ag;
            pC->ab = Local.ab;
            pC->br = Local.br;
            pC->bg = Local.bg;
            pC->bb = Local.bb;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC->br = CFactor.br * (Other.br - Local.br);
            pC->bg = CFactor.bg * (Other.bg - Local.bg);
            pC->bb = CFactor.bb * (Other.bb - Local.bb);
            pC2->ar = Local.ar;
            pC2->ag = Local.ag;
            pC2->ab = Local.ab;
            pC2->br = Local.br;
            pC2->bg = Local.bg;
            pC2->bb = Local.bb;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
            ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
            (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
        {
            pC->ar = pC->ag = pC->ab = Local.aa;
            pC->br = pC->bg = pC->bb = Local.ba;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC->br = CFactor.br * (Other.br - Local.br);
            pC->bg = CFactor.bg * (Other.bg - Local.bg);
            pC->bb = CFactor.bb * (Other.bb - Local.bb);
            pC2->ar = pC2->ag = pC2->ab = Local.aa;
            pC2->br = pC2->bg = pC2->bb = Local.ba;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = ( 1.0f - CFactor.ar ) * Local.ar;
        pC->ag = ( 1.0f - CFactor.ag ) * Local.ag;
        pC->ab = ( 1.0f - CFactor.ab ) * Local.ab;
        pC->br = ( 1.0f - CFactor.br ) * Local.br;
        pC->bg = ( 1.0f - CFactor.bg ) * Local.bg;
        pC->bb = ( 1.0f - CFactor.bb ) * Local.bb;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        pC2->br = Local.br;
        pC2->bg = Local.bg;
        pC2->bb = Local.bb;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * -Local.ar;
        pC->ag = CFactor.ag * -Local.ag;
        pC->ab = CFactor.ab * -Local.ab;
        pC->br = CFactor.br * -Local.br;
        pC->bg = CFactor.bg * -Local.bg;
        pC->bb = CFactor.bb * -Local.bb;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        pC2->br = pC2->bg = pC2->bb = Local.ba;
        break;
    }

    switch ( Glide.State.AlphaFunction )
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->aa = pC->ba = 0.0f;
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->aa = Local.aa;
        pC->ba = Local.ba;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = ((1.0f - AlphaFactorFunc( Local.aa, Other.aa )) * Local.aa);
        pC->ba = ((1.0f - AlphaFactorFunc( Local.ba, Other.ba )) * Local.ba);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa);
        pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa);
        pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * Other.ba + Local.ba);
//      pC2->aa =  Local.aa;
//      pC2->ba =  Local.ba;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ));
        pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ));
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa);
        pC->ba = (AlphaFactorFunc( Local.ba, Other.ba ) * ( Other.ba - Local.ba ) + Local.ba);
//      pC2->aa =  Local.aa;
//      pC2->ba =  Local.ba;
        break;
    }

    if ( Glide.State.ColorCombineInvert )
    {
        pC->ar = 1.0f - pC->ar - pC2->ar;
        pC->ag = 1.0f - pC->ag - pC2->ag;
        pC->ab = 1.0f - pC->ab - pC2->ab;
        pC->br = 1.0f - pC->br - pC2->br;
        pC->bg = 1.0f - pC->bg - pC2->bg;
        pC->bb = 1.0f - pC->bb - pC2->bb;
        pC2->ar = pC2->ag = pC2->ab = 0.0f;
        pC2->br = pC2->bg = pC2->bb = 0.0f;
    }

    if ( Glide.State.AlphaInvert )
    {
        pC->aa = 1.0f - pC->aa - pC2->aa;
        pC->ba = 1.0f - pC->ba - pC2->ba;
        pC2->aa = pC2->ba = 0.0f;
    }
    
    // Z-Buffering
    if ( ( Glide.State.DepthBufferMode == GR_DEPTHBUFFER_DISABLE ) || 
         ( Glide.State.DepthBufferMode == GR_CMP_ALWAYS ) )
    {
        pV->az = 0.0f;
        pV->bz = 0.0f;
    }
    else 
    if ( OpenGL.DepthBufferType )
    {
        pV->az = a->ooz * D1OVER65535;
        pV->bz = b->ooz * D1OVER65535;
    }
    else
    {
       /*
        * For silly values of oow, depth buffering doesn't
        * seem to work, so map them to a sensible z.  When
        * games use these silly values, they probably don't
        * use z buffering anyway.
        */
        if ( a->oow > 1.0f )
        {
            pV->az = pV->bz = 1.0f;
        }
        else 
        if ( InternalConfig.PrecisionFix )
        {   // Fix precision to 16 integer bits.
            FxU16 w;
            w      = (FxU16)((a->oow / D1OVER65535) + 0.5f);
            pV->az = (float)w * D1OVER65535;
            w      = (FxU16)((b->oow / D1OVER65535) + 0.5f);
            pV->bz = (float)w / D1OVER65535;
        }
        else
        {
            pV->az = a->oow;
            pV->bz = b->oow;
        }
    }

    if ( ( unsnap ) &&
         ( a->x > vertex_snap_compare ) )
    {
        pV->ax = a->x - vertex_snap;
        pV->ay = a->y - vertex_snap;
        pV->bx = b->x - vertex_snap;
        pV->by = b->y - vertex_snap;
    }
    else
    {
        pV->ax = a->x;
        pV->ay = a->y;
        pV->bx = b->x;
        pV->by = b->y;
    }

		for(int tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
		{
    	if(OpenGL.Texture[tmu])
    	{
        Textures->GetAspect(tmu, &hAspect, &wAspect );

        pTS->tmu[tmu].as = a->tmuvtx[tmu].sow * wAspect; // / a->oow;
        pTS->tmu[tmu].at = a->tmuvtx[tmu].tow * hAspect; // / a->oow;
        pTS->tmu[tmu].bs = b->tmuvtx[tmu].sow * wAspect; // / b->oow;
        pTS->tmu[tmu].bt = b->tmuvtx[tmu].tow * hAspect; // / b->oow;

        pTS->tmu[tmu].aq = pTS->tmu[tmu].bq = 0.0f;
        pTS->tmu[tmu].aoow = atmuoow0;
        pTS->tmu[tmu].boow = btmuoow0;
    	}
    }
    
    if ( InternalConfig.FogEnable )
    {
#ifndef NEW_FOG
        pF->af = (float)OpenGL.FogTable[ (FxU16)(1.0f / a->oow) ] * D1OVER255;
        pF->bf = (float)OpenGL.FogTable[ (FxU16)(1.0f / b->oow) ] * D1OVER255;
#else
        pF->af = OGLFogDistance(a->oow);
        pF->bf = OGLFogDistance(b->oow);
#endif

    #ifdef OGL_DEBUG
        DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
        DEBUG_MIN_MAX( pF->bf, OGLRender.MaxF, OGLRender.MinF );
    #endif
    }

#ifdef OGL_DEBUG
    DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );
    DEBUG_MIN_MAX( pC->br, OGLRender.MaxR, OGLRender.MinR );
    
    DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );
    DEBUG_MIN_MAX( pC->bg, OGLRender.MaxG, OGLRender.MinG );

    DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );
    DEBUG_MIN_MAX( pC->bb, OGLRender.MaxB, OGLRender.MinB );

    DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );
    DEBUG_MIN_MAX( pC->ba, OGLRender.MaxA, OGLRender.MinA );

    DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );
    DEBUG_MIN_MAX( pV->bz, OGLRender.MaxZ, OGLRender.MinZ );

    DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );
    DEBUG_MIN_MAX( pV->bx, OGLRender.MaxX, OGLRender.MinX );

    DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );
    DEBUG_MIN_MAX( pV->by, OGLRender.MaxY, OGLRender.MinY );

//    DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );
//    DEBUG_MIN_MAX( pTS->bs, OGLRender.MaxS, OGLRender.MinS );

//    DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
//    DEBUG_MIN_MAX( pTS->bt, OGLRender.MaxT, OGLRender.MinT );
#endif
    
    int tmu;
    for(tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
    {
        Textures->MakeReady( tmu );
    }

    if ( OpenGL.Blend )
    {
        DGL(glEnable)( GL_BLEND );
    }
    else
    {
        DGL(glDisable)( GL_BLEND );
    }

    // Alpha Fix
    if ( Glide.State.AlphaOther != GR_COMBINE_OTHER_TEXTURE )
    {
        DGL(glDisable)( GL_ALPHA_TEST );
    }
    else 
    {
        if ( Glide.State.AlphaTestFunction != GR_CMP_ALWAYS )
        {
            DGL(glEnable)( GL_ALPHA_TEST );
        }
    }
    
    DGL(glBegin)( GL_LINES );
        DGL(glColor4fv)( &pC->ar );
        p_glSecondaryColor3fvEXT( &pC2->ar );
        OGLCoords4(GR_TMU0, &pTS->tmu[0].as);
#if GLIDE_NUM_TMU >= 2
        if(InternalConfig.NumTMU >= 2)
            OGLCoords4(GR_TMU1, &pTS->tmu[1].as);
#endif
#if GLIDE_NUM_TMU >= 3
        if(InternalConfig.NumTMU >= 3)
            OGLCoords4(GR_TMU2, &pTS->tmu[2].as);
#endif
        p_glFogCoordfEXT( pF->af );
        DGL(glVertex3fv)( &pV->ax );

        DGL(glColor4fv)( &pC->br );
        p_glSecondaryColor3fvEXT( &pC2->br );
        OGLCoords4(GR_TMU0, &pTS->tmu[0].bs);
#if GLIDE_NUM_TMU >= 2
        if(InternalConfig.NumTMU >= 2)
            OGLCoords4(GR_TMU1, &pTS->tmu[1].bs);
#endif
#if GLIDE_NUM_TMU >= 3
        if(InternalConfig.NumTMU >= 3)
            OGLCoords4(GR_TMU2, &pTS->tmu[2].bs);
#endif
        p_glFogCoordfEXT( pF->bf );
        DGL(glVertex3fv)( &pV->bx );
    DGL(glEnd)();

#ifdef OPENGL_DEBUG
    GLErro( "Render::AddLine" );
#endif
}

void RenderAddPoint( const GrVertex *a, bool unsnap )
{
	  int tmu;
	
    pC  = &OGLRender.TColor[ MAXTRIANGLES ];
    pC2 = &OGLRender.TColor2[ MAXTRIANGLES ];
    pV  = &OGLRender.TVertex[ MAXTRIANGLES ];
    pTS = &OGLRender.TTexture[ MAXTRIANGLES ];
    pF  = &OGLRender.TFog[ MAXTRIANGLES ];

    // Color Stuff, need to optimize it
    ZeroMemory( pC2, sizeof( TColorStruct ) );

    if ( Glide.ALocal )
    {
        switch ( Glide.State.AlphaLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.aa = a->a * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.aa = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_LOCAL_DEPTH:
            Local.aa = a->z;
            break;
        }
    }

    if ( Glide.AOther )
    {
        switch ( Glide.State.AlphaOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.aa = a->a * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.aa = OpenGL.ConstantColor[3];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.aa = 1.0f;
            break;
        }
    }

    if ( Glide.CLocal )
    {
        switch ( Glide.State.ColorCombineLocal )
        {
        case GR_COMBINE_LOCAL_ITERATED:
            Local.ar = a->r * D1OVER255;
            Local.ag = a->g * D1OVER255;
            Local.ab = a->b * D1OVER255;
            break;

        case GR_COMBINE_LOCAL_CONSTANT:
            Local.ar = OpenGL.ConstantColor[0];
            Local.ag = OpenGL.ConstantColor[1];
            Local.ab = OpenGL.ConstantColor[2];
            break;
        }
    }

    if ( Glide.COther )
    {
        switch ( Glide.State.ColorCombineOther )
        {
        case GR_COMBINE_OTHER_ITERATED:
            Other.ar = a->r * D1OVER255;
            Other.ag = a->g * D1OVER255;
            Other.ab = a->b * D1OVER255;
            break;

        case GR_COMBINE_OTHER_CONSTANT:
            Other.ar = OpenGL.ConstantColor[0];
            Other.ag = OpenGL.ConstantColor[1];
            Other.ab = OpenGL.ConstantColor[2];
            break;

        case GR_COMBINE_OTHER_TEXTURE:
            Other.ar = Other.ag = Other.ab = 1.0f;
            break;
        }
    }

    switch ( Glide.State.ColorCombineFunction )
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->ar = pC->ag = pC->ab = 0.0f; 
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
        pC->ar = Local.ar;
        pC->ag = Local.ag;
        pC->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->ar = pC->ag = pC->ab = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * Other.ar;
        pC->ag = CFactor.ag * Other.ag;
        pC->ab = CFactor.ab * Other.ab;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * (Other.ar - Local.ar);
        pC->ag = CFactor.ag * (Other.ag - Local.ag);
        pC->ab = CFactor.ab * (Other.ab - Local.ab);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
        if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
            ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
            (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
        {
            pC->ar = Local.ar;
            pC->ag = Local.ag;
            pC->ab = Local.ab;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC2->ar = Local.ar;
            pC2->ag = Local.ag;
            pC2->ab = Local.ab;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
            ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
            (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
        {
            pC->ar = pC->ag = pC->ab = Local.aa;
        }
        else
        {
            ColorFactor3Func( &CFactor, &Local, &Other );
            pC->ar = CFactor.ar * (Other.ar - Local.ar);
            pC->ag = CFactor.ag * (Other.ag - Local.ag);
            pC->ab = CFactor.ab * (Other.ab - Local.ab);
            pC2->ar = pC2->ag = pC2->ab = Local.aa;
        }
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = ( 1.0f - CFactor.ar ) * Local.ar;
        pC->ag = ( 1.0f - CFactor.ag ) * Local.ag;
        pC->ab = ( 1.0f - CFactor.ab ) * Local.ab;
        pC2->ar = Local.ar;
        pC2->ag = Local.ag;
        pC2->ab = Local.ab;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        ColorFactor3Func( &CFactor, &Local, &Other );
        pC->ar = CFactor.ar * -Local.ar;
        pC->ag = CFactor.ag * -Local.ag;
        pC->ab = CFactor.ab * -Local.ab;
        pC2->ar = pC2->ag = pC2->ab = Local.aa;
        break;
    }

    switch ( Glide.State.AlphaFunction )
    {
    case GR_COMBINE_FUNCTION_ZERO:
        pC->aa = 0.0f;
        break;

    case GR_COMBINE_FUNCTION_LOCAL:
    case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
        pC->aa = Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = ((1.0f - AlphaFactorFunc( Local.aa, Other.aa )) * Local.aa);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa);
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * Other.aa + Local.aa;
//      pC2->aa =  Local.aa;
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
        pC->aa = (AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ));
        break;

    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
        pC->aa = AlphaFactorFunc( Local.aa, Other.aa ) * ( Other.aa - Local.aa ) + Local.aa;
//      pC2->aa =  Local.aa;
        break;
    }

    if ( Glide.State.ColorCombineInvert )
    {
        pC->ar = 1.0f - pC->ar - pC2->ar;
        pC->ag = 1.0f - pC->ag - pC2->ag;
        pC->ab = 1.0f - pC->ab - pC2->ab;
        pC2->ar = pC2->ag = pC2->ab = 0.0f;
    }

    if ( Glide.State.AlphaInvert )
    {
        pC->aa = 1.0f - pC->aa - pC2->aa;
        pC2->aa = 0.0f;
    }
    
    // Z-Buffering
    if ( ( Glide.State.DepthBufferMode == GR_DEPTHBUFFER_DISABLE ) || 
         ( Glide.State.DepthBufferMode == GR_CMP_ALWAYS ) )
    {
        pV->az = 0.0f;
    }
    else
    if ( OpenGL.DepthBufferType )
    {
        pV->az = a->ooz * D1OVER65535;
    }
    else
    {
        if ( a->oow > 1.0f )
        {
            pV->az = 1.0f;
        }
        else 
        if ( InternalConfig.PrecisionFix )
        {   // Fix precision to 16 integer bits.
            FxU16 w = (FxU16)((a->oow / D1OVER65535) + 0.5f);
            pV->az  = (float)w * D1OVER65535;
        }
        else
        {
            pV->az = a->oow;
        }
    }

    if ( ( unsnap ) &&
         ( a->x > vertex_snap_compare ) )
    {
        pV->ax = a->x - vertex_snap;
        pV->ay = a->y - vertex_snap;
    }
    else
    {
        pV->ax = a->x;
        pV->ay = a->y;
    }

		for(tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
		{
			if(OpenGL.Texture[tmu])
			{
				Textures->GetAspect(tmu, &hAspect, &wAspect);
	
				pTS->tmu[tmu].as = a->tmuvtx[tmu].sow * wAspect;
				pTS->tmu[tmu].at = a->tmuvtx[tmu].tow * hAspect;
	
				pTS->tmu[tmu].aq = 0.0f;
				pTS->tmu[tmu].aoow = a->oow; // !
			}
		}

    if( InternalConfig.FogEnable )
    {
#ifndef NEW_FOG
        pF->af = (float)OpenGL.FogTable[ (FxU16)(1.0f / a->oow) ] * D1OVER255;
#else
        pF->af = OGLFogDistance(a->oow);
#endif

    #ifdef OGL_DEBUG
        DEBUG_MIN_MAX( pF->af, OGLRender.MaxF, OGLRender.MinF );
    #endif
    }

#ifdef OGL_DEBUG
    DEBUG_MIN_MAX( pC->ar, OGLRender.MaxR, OGLRender.MinR );
    
    DEBUG_MIN_MAX( pC->ag, OGLRender.MaxG, OGLRender.MinG );

    DEBUG_MIN_MAX( pC->ab, OGLRender.MaxB, OGLRender.MinB );

    DEBUG_MIN_MAX( pC->aa, OGLRender.MaxA, OGLRender.MinA );

    DEBUG_MIN_MAX( pV->az, OGLRender.MaxZ, OGLRender.MinZ );

    DEBUG_MIN_MAX( pV->ax, OGLRender.MaxX, OGLRender.MinX );

    DEBUG_MIN_MAX( pV->ay, OGLRender.MaxY, OGLRender.MinY );

//    DEBUG_MIN_MAX( pTS->as, OGLRender.MaxS, OGLRender.MinS );

//    DEBUG_MIN_MAX( pTS->at, OGLRender.MaxT, OGLRender.MinT );
#endif
    for(tmu = 0; tmu < InternalConfig.NumTMU; tmu++)
    {
        Textures->MakeReady( tmu );
    }

    if ( OpenGL.Blend )
    {
        DGL(glEnable)( GL_BLEND );
    }
    else
    {
        DGL(glDisable)( GL_BLEND );
    }

    // Alpha Fix
    if ( Glide.State.AlphaOther != GR_COMBINE_OTHER_TEXTURE )
    {
        DGL(glDisable)( GL_ALPHA_TEST );
    }
    else 
    {
        if ( Glide.State.AlphaTestFunction != GR_CMP_ALWAYS )
        {
            DGL(glEnable)( GL_ALPHA_TEST );
        }
    }
    
    DGL(glBegin)( GL_POINTS );
        DGL(glColor4fv)( &pC->ar );
        p_glSecondaryColor3fvEXT( &pC2->ar );
        OGLCoords4(GR_TMU0, &pTS->tmu[0].as);
#if GLIDE_NUM_TMU >= 2
        if(InternalConfig.NumTMU >= 2)
            OGLCoords4(GR_TMU1, &pTS->tmu[1].as);
#endif
#if GLIDE_NUM_TMU >= 3
        if(InternalConfig.NumTMU >= 3)
            OGLCoords4(GR_TMU2, &pTS->tmu[2].as);
#endif
        p_glFogCoordfEXT( pF->af );
        DGL(glVertex3fv)( &pV->ax );
    DGL(glEnd)();

#ifdef OPENGL_DEBUG
    GLErro( "Render::AddPoint" );
#endif
}

// Color Factor functions

void FASTCALL ColorFactor3Zero( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = 0.0f;
    Result->br = Result->bg = Result->bb = 0.0f;
    Result->cr = Result->cg = Result->cb = 0.0f;
}

void FASTCALL ColorFactor3Local( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = ColorComponent->ar;
    Result->ag = ColorComponent->ag;
    Result->ab = ColorComponent->ab;
    Result->br = ColorComponent->br;
    Result->bg = ColorComponent->bg;
    Result->bb = ColorComponent->bb;
    Result->cr = ColorComponent->cr;
    Result->cg = ColorComponent->cg;
    Result->cb = ColorComponent->cb;
}

void FASTCALL ColorFactor3OtherAlpha( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = OtherAlpha->aa;
    Result->br = Result->bg = Result->bb = OtherAlpha->ba;
    Result->cr = Result->cg = Result->cb = OtherAlpha->ca;
}

void FASTCALL ColorFactor3LocalAlpha( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = ColorComponent->aa;
    Result->br = Result->bg = Result->bb = ColorComponent->ba;
    Result->cr = Result->cg = Result->cb = ColorComponent->ca;
}

void FASTCALL ColorFactor3OneMinusLocal( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = 1.0f - ColorComponent->ar;
    Result->ag = 1.0f - ColorComponent->ag;
    Result->ab = 1.0f - ColorComponent->ab;
    Result->br = 1.0f - ColorComponent->br;
    Result->bg = 1.0f - ColorComponent->bg;
    Result->bb = 1.0f - ColorComponent->bb;
    Result->cr = 1.0f - ColorComponent->cr;
    Result->cg = 1.0f - ColorComponent->cg;
    Result->cb = 1.0f - ColorComponent->cb;
}

void FASTCALL ColorFactor3OneMinusOtherAlpha( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = 1.0f - OtherAlpha->aa;
    Result->br = Result->bg = Result->bb = 1.0f - OtherAlpha->ba;
    Result->cr = Result->cg = Result->cb = 1.0f - OtherAlpha->ca;
}

void FASTCALL ColorFactor3OneMinusLocalAlpha( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = 1.0f - ColorComponent->aa;
    Result->br = Result->bg = Result->bb = 1.0f - ColorComponent->ba;
    Result->cr = Result->cg = Result->cb = 1.0f - ColorComponent->ca;
}

void FASTCALL ColorFactor3One( TColorStruct *Result, TColorStruct *ColorComponent, TColorStruct *OtherAlpha )
{
    Result->ar = Result->ag = Result->ab = 1.0f;
    Result->br = Result->bg = Result->bb = 1.0f;
    Result->cr = Result->cg = Result->cb = 1.0f;
}

// Alpha Factor functions

float AlphaFactorZero( float LocalAlpha, float OtherAlpha )
{
    return 0.0f;
}

float AlphaFactorLocal( float LocalAlpha, float OtherAlpha )
{
    return LocalAlpha;
}

float AlphaFactorOther( float LocalAlpha, float OtherAlpha )
{
    return OtherAlpha;
}

float AlphaFactorOneMinusLocal( float LocalAlpha, float OtherAlpha )
{
    return 1.0f - LocalAlpha;
}

float AlphaFactorOneMinusOther( float LocalAlpha, float OtherAlpha )
{
    return 1.0f - OtherAlpha;
}

float AlphaFactorOne( float LocalAlpha, float OtherAlpha )
{
    return 1.0f;
}

// Color functions

void ColorFunctionZero( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    pC->ar = pC->ag = pC->ab = 0.0f; 
    pC->br = pC->bg = pC->bb = 0.0f; 
    pC->cr = pC->cg = pC->cb = 0.0f; 
}

void ColorFunctionLocal( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    pC->ar = Local->ar;
    pC->ag = Local->ag;
    pC->ab = Local->ab;
    pC->br = Local->br;
    pC->bg = Local->bg;
    pC->bb = Local->bb;
    pC->cr = Local->cr;
    pC->cg = Local->cg;
    pC->cb = Local->cb;
}

void ColorFunctionLocalAlpha( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    pC->ar = pC->ag = pC->ab = Local->aa;
    pC->br = pC->bg = pC->bb = Local->ba;
    pC->cr = pC->cg = pC->cb = Local->ca;
}

void ColorFunctionScaleOther( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = CFactor.ar * Other->ar;
    pC->ag = CFactor.ag * Other->ag;
    pC->ab = CFactor.ab * Other->ab;
    pC->br = CFactor.br * Other->br;
    pC->bg = CFactor.bg * Other->bg;
    pC->bb = CFactor.bb * Other->bb;
    pC->cr = CFactor.cr * Other->cr;
    pC->cg = CFactor.cg * Other->cg;
    pC->cb = CFactor.cb * Other->cb;
}

void ColorFunctionScaleOtherAddLocal( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = CFactor.ar * Other->ar;
    pC->ag = CFactor.ag * Other->ag;
    pC->ab = CFactor.ab * Other->ab;
    pC->br = CFactor.br * Other->br;
    pC->bg = CFactor.bg * Other->bg;
    pC->bb = CFactor.bb * Other->bb;
    pC->cr = CFactor.cr * Other->cr;
    pC->cg = CFactor.cg * Other->cg;
    pC->cb = CFactor.cb * Other->cb;
    pC2->ar = Local->ar;
    pC2->ag = Local->ag;
    pC2->ab = Local->ab;
    pC2->br = Local->br;
    pC2->bg = Local->bg;
    pC2->bb = Local->bb;
    pC2->cr = Local->cr;
    pC2->cg = Local->cg;
    pC2->cb = Local->cb;
}

void ColorFunctionScaleOtherAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = CFactor.ar * Other->ar;
    pC->ag = CFactor.ag * Other->ag;
    pC->ab = CFactor.ab * Other->ab;
    pC->br = CFactor.br * Other->br;
    pC->bg = CFactor.bg * Other->bg;
    pC->bb = CFactor.bb * Other->bb;
    pC->cr = CFactor.cr * Other->cr;
    pC->cg = CFactor.cg * Other->cg;
    pC->cb = CFactor.cb * Other->cb;
    pC2->ar = pC2->ag = pC2->ab = Local->aa;
    pC2->br = pC2->bg = pC2->bb = Local->ba;
    pC2->cr = pC2->cg = pC2->cb = Local->ca;
}

void ColorFunctionScaleOtherMinusLocal( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = CFactor.ar * (Other->ar - Local->ar);
    pC->ag = CFactor.ag * (Other->ag - Local->ag);
    pC->ab = CFactor.ab * (Other->ab - Local->ab);
    pC->br = CFactor.br * (Other->br - Local->br);
    pC->bg = CFactor.bg * (Other->bg - Local->bg);
    pC->bb = CFactor.bb * (Other->bb - Local->bb);
    pC->cr = CFactor.cr * (Other->cr - Local->cr);
    pC->cg = CFactor.cg * (Other->cg - Local->cg);
    pC->cb = CFactor.cb * (Other->cb - Local->cb);
}

void ColorFunctionScaleOtherMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
        ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
        (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
    {
        pC->ar = Local->ar;
        pC->ag = Local->ag;
        pC->ab = Local->ab;
        pC->br = Local->br;
        pC->bg = Local->bg;
        pC->bb = Local->bb;
        pC->cr = Local->cr;
        pC->cg = Local->cg;
        pC->cb = Local->cb;
    }
    else
    {
        ColorFactor3Func( &CFactor, Local, Other );
        pC->ar = CFactor.ar * (Other->ar - Local->ar);
        pC->ag = CFactor.ag * (Other->ag - Local->ag);
        pC->ab = CFactor.ab * (Other->ab - Local->ab);
        pC->br = CFactor.br * (Other->br - Local->br);
        pC->bg = CFactor.bg * (Other->bg - Local->bg);
        pC->bb = CFactor.bb * (Other->bb - Local->bb);
        pC->cr = CFactor.cr * (Other->cr - Local->cr);
        pC->cg = CFactor.cg * (Other->cg - Local->cg);
        pC->cb = CFactor.cb * (Other->cb - Local->cb);
        pC2->ar = Local->ar;
        pC2->ag = Local->ag;
        pC2->ab = Local->ab;
        pC2->br = Local->br;
        pC2->bg = Local->bg;
        pC2->bb = Local->bb;
        pC2->cr = Local->cr;
        pC2->cg = Local->cg;
        pC2->cb = Local->cb;
    }
}

void ColorFunctionScaleOtherMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    if ((( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_ALPHA ) ||
        ( Glide.State.ColorCombineFactor == GR_COMBINE_FACTOR_TEXTURE_RGB )) &&
        (  Glide.State.ColorCombineOther == GR_COMBINE_OTHER_TEXTURE ) )
    {
        pC->ar = pC->ag = pC->ab = Local->aa;
        pC->br = pC->bg = pC->bb = Local->ba;
        pC->cr = pC->cg = pC->cb = Local->ca;
    }
    else
    {
        ColorFactor3Func( &CFactor, Local, Other );
        pC->ar = CFactor.ar * (Other->ar - Local->ar);
        pC->ag = CFactor.ag * (Other->ag - Local->ag);
        pC->ab = CFactor.ab * (Other->ab - Local->ab);
        pC->br = CFactor.br * (Other->br - Local->br);
        pC->bg = CFactor.bg * (Other->bg - Local->bg);
        pC->bb = CFactor.bb * (Other->bb - Local->bb);
        pC->cr = CFactor.cr * (Other->cr - Local->cr);
        pC->cg = CFactor.cg * (Other->cg - Local->cg);
        pC->cb = CFactor.cb * (Other->cb - Local->cb);
        pC2->ar = pC2->ag = pC2->ab = Local->aa;
        pC2->br = pC2->bg = pC2->bb = Local->ba;
        pC2->cr = pC2->cg = pC2->cb = Local->ca;
    }
}

void ColorFunctionMinusLocalAddLocal( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = ( 1.0f - CFactor.ar ) * Local->ar;
    pC->ag = ( 1.0f - CFactor.ag ) * Local->ag;
    pC->ab = ( 1.0f - CFactor.ab ) * Local->ab;
    pC->br = ( 1.0f - CFactor.br ) * Local->br;
    pC->bg = ( 1.0f - CFactor.bg ) * Local->bg;
    pC->bb = ( 1.0f - CFactor.bb ) * Local->bb;
    pC->cr = ( 1.0f - CFactor.cr ) * Local->cr;
    pC->cg = ( 1.0f - CFactor.cg ) * Local->cg;
    pC->cb = ( 1.0f - CFactor.cb ) * Local->cb;
    pC2->ar = Local->ar;
    pC2->ag = Local->ag;
    pC2->ab = Local->ab;
    pC2->br = Local->br;
    pC2->bg = Local->bg;
    pC2->bb = Local->bb;
    pC2->cr = Local->cr;
    pC2->cg = Local->cg;
    pC2->cb = Local->cb;
}

void ColorFunctionMinusLocalAddLocalAlpha( TColorStruct * pC, TColorStruct * pC2, TColorStruct * Local, TColorStruct * Other )
{
    ColorFactor3Func( &CFactor, Local, Other );
    pC->ar = CFactor.ar * -Local->ar;
    pC->ag = CFactor.ag * -Local->ag;
    pC->ab = CFactor.ab * -Local->ab;
    pC->br = CFactor.br * -Local->br;
    pC->bg = CFactor.bg * -Local->bg;
    pC->bb = CFactor.bb * -Local->bb;
    pC->cr = CFactor.cr * -Local->cr;
    pC->cg = CFactor.cg * -Local->cg;
    pC->cb = CFactor.cb * -Local->cb;
    pC2->ar = pC2->ag = pC2->ab = Local->aa;
    pC2->br = pC2->bg = pC2->bb = Local->ba;
    pC2->cr = pC2->cg = pC2->cb = Local->ca;
}
