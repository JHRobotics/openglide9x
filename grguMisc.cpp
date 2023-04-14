//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                     Other Functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include "GlOgl.h"
#include "GLRender.h"

#define GLIDE_MAX_NUM_TMU 1

//*************************************************
//* Sets the External Error Function to call if
//* Glides Generates and Error
//*************************************************
FX_ENTRY void FX_CALL
grErrorSetCallback( void (*function)(const char *string, FxBool fatal) )
{
#ifdef OGL_DONE
    GlideMsg( "grErrorSetCallback( --- )\n" );
#endif

    ExternErrorFunction = function;
}

//*************************************************
//* Sets the Cull Mode
//*************************************************
FX_ENTRY void FX_CALL
grCullMode( GrCullMode_t mode )
{
#ifdef OGL_DONE
    GlideMsg( "grCullMode( %d )\n", mode );
#endif

    RenderDrawTriangles( );

    Glide.State.CullMode = mode;

    switch ( Glide.State.CullMode )
    {
    case GR_CULL_DISABLE:
        glDisable( GL_CULL_FACE );
        glCullFace( GL_BACK );  // This will be called in initialization
        break;

    case GR_CULL_NEGATIVE:
        glEnable( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            glFrontFace( GL_CCW );
        }
        else
        {
            glFrontFace( GL_CW );
        }
        break;

    case GR_CULL_POSITIVE:
        glEnable( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            glFrontFace( GL_CW );
        }
        else
        {
            glFrontFace( GL_CCW );
        }
        break;
    }

#ifdef OPENGL_DEBUG
    GLErro( "grCullMode" );
#endif
}

//*************************************************
//* Set the size and location of the hardware clipping window
//*************************************************
FX_ENTRY void FX_CALL 
grClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grClipWindow( %d, %d, %d, %d )\n", minx, miny, maxx, maxy );
#endif

    RenderDrawTriangles( );

    Glide.State.ClipMinX = minx;
    Glide.State.ClipMaxX = maxx;
    Glide.State.ClipMinY = miny;
    Glide.State.ClipMaxY = maxy;
    // calculate the corresponding OpenGL coords
    // (the multiplication has to come first because the values are integers)
    OpenGL.ClipMinX = OpenGL.WindowWidth * minx / Glide.WindowWidth;
    OpenGL.ClipMaxX = OpenGL.WindowWidth * maxx / Glide.WindowWidth;
    OpenGL.ClipMinY = OpenGL.WindowHeight * miny / Glide.WindowHeight;
    OpenGL.ClipMaxY = OpenGL.WindowHeight * maxy / Glide.WindowHeight;

    if ( ( Glide.State.ClipMinX != 0 ) || 
         ( Glide.State.ClipMinY != 0 ) ||
         ( Glide.State.ClipMaxX != (FxU32) Glide.WindowWidth ) ||
         ( Glide.State.ClipMaxY != (FxU32) Glide.WindowHeight ) )
    {
        OpenGL.Clipping = true;
    }
    else
    {
        OpenGL.Clipping = false;
    }

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
    {
        glOrtho( Glide.State.ClipMinX, Glide.State.ClipMaxX,
                 Glide.State.ClipMinY, Glide.State.ClipMaxY,
                 OpenGL.ZNear, OpenGL.ZFar );
        glViewport( OpenGL.ClipMinX, OpenGL.ClipMinY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
        // Used for the buffer clearing
        glScissor( OpenGL.ClipMinX, OpenGL.ClipMinY,
                   OpenGL.ClipMaxX - OpenGL.ClipMinX,
                   OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }
    else
    {
        glOrtho( Glide.State.ClipMinX, Glide.State.ClipMaxX,
                 Glide.State.ClipMaxY, Glide.State.ClipMinY,
                 OpenGL.ZNear, OpenGL.ZFar );
        glViewport( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
        // Used for the buffer clearing
        glScissor( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                   OpenGL.ClipMaxX - OpenGL.ClipMinX,
                   OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }

    glMatrixMode( GL_MODELVIEW );
}

//*************************************************
FX_ENTRY void FX_CALL 
grDisableAllEffects( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grDisableAllEffects( )\n" );
#endif

    grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    grAlphaTestFunction( GR_CMP_ALWAYS );
    grChromakeyMode( GR_CHROMAKEY_DISABLE );
    grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
    grFogMode( GR_FOG_DISABLE );
}

//*************************************************
FX_ENTRY void FX_CALL
grResetTriStats( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grResetTriStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grTriStats( FxU32 *trisProcessed, FxU32 *trisDrawn )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTriStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grHints( GrHint_t hintType, FxU32 hintMask )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grHints( %d, %d )\n", hintType, hintMask );
#endif

    switch( hintType )
    {
    case GR_HINT_STWHINT:
        Glide.State.STWHint = hintMask;
        break;
    }
}

//*************************************************
/*
FX_ENTRY void FX_CALL
grSplash( float x, float y, float width, float height, FxU32 frame )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSplash( %-4.2f, %-4.2f, %-4.2f, %-4.2f, %lu )\n",
        x, y, width, height, frame );
#endif
}*/

//*************************************************
FX_ENTRY void FX_CALL 
ConvertAndDownloadRle( GrChipID_t        tmu,
                       FxU32             startAddress,
                       GrLOD_t           thisLod,
                       GrLOD_t           largeLod,
                       GrAspectRatio_t   aspectRatio,
                       GrTextureFormat_t format,
                       FxU32             evenOdd,
                       FxU8              *bm_data,
                       long              bm_h,
                       FxU32             u0,
                       FxU32             v0,
                       FxU32             width,
                       FxU32             height,
                       FxU32             dest_width,
                       FxU32             dest_height,
                       FxU16             *tlut )
{
#ifdef OGL_NOTDONE
    GlideMsg( "ConvertAndDownloadRle( %d, %lu, %d, %d, %d, %d, %d, ---, %l, %lu, %lu, %lu, %lu, %lu, %lu, --- )\n",
        tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, bm_h, u0, v0, width, height,
        dest_width, dest_height );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL 
grCheckForRoom( FxI32 n )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grCheckForRoom( %l )\n", n );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grParameterData( FxU32 param, FxU32 components, FxU32 type, FxI32 offset )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grParameterData( %lu, %lu, %lu, %l )\n",
        param, components, type, offset );
#endif
}

//*************************************************
FX_ENTRY int FX_CALL
guEncodeRLE16( void *dst, 
               void *src, 
               FxU32 width, 
               FxU32 height )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guEncodeRLE16( ---, ---, %lu, %lu ) = 1\n", width, height ); 
#endif

    return 1; 
}

//*************************************************
FX_ENTRY FxU32 FX_CALL
guEndianSwapWords( FxU32 value )
{
#ifdef OGL_DONE
    GlideMsg( "guEndianSwapWords( %lu )\n", value );
#endif

    return ( value << 16 ) | ( value >> 16 );
}

//*************************************************
FX_ENTRY FxU16 FX_CALL
guEndianSwapBytes( FxU16 value )
{
#ifdef OGL_DONE
    GlideMsg( "guEndianSwapBytes( %u )\n", value );
#endif

    return ( value << 8 ) | ( value >> 8 );
}

#ifdef GLIDE3

/* Glide3.x */

FX_ENTRY GrProc FX_CALL grGetProcAddress(char *procName)
{
	return NULL;
}

FX_ENTRY void FX_CALL grEnable( GrEnableMode_t mode)
{
	
}

FX_ENTRY void FX_CALL grCoordinateSpace( GrCoordinateSpaceMode_t mode )
{
	
}

FX_ENTRY void FX_CALL grDisable ( GrEnableMode_t mode)
{
	
}

FX_ENTRY void FX_CALL grDrawVertexArray ( FxU32 mode, FxU32 count, void *pointers )
{
	
}

FX_ENTRY void FX_CALL grDrawVertexArrayContiguous ( FxU32 mode, FxU32 count, void *vertex, FxU32 stride )
{
	
}

FX_ENTRY void FX_CALL grFinish ( void )
{
	
}

FX_ENTRY void FX_CALL grFlush ( void )
{
	
}

static inline FxU32 grGet_fill_buffer(FxI32 *dst, FxU32 dstlen, const FxI32 *src, FxU32 srclen)
{
	FxU32 n = 0; 
	while(dstlen >= 4)
	{
		*dst = *src;
		dst++;
		src++;
		dstlen -= 4;
		n++;
	}
	
	return n;
}

static inline void grGet_fill_num(FxU32 dstlen, FxI32 *dst, FxI32 n)
{
	if(dstlen >= 4)
	{
		*dst = n;
	}
}


FX_ENTRY FxBool FX_CALL grGet ( FxU32 pname, FxU32 plength, FxI32 *params )
{
	switch(pname)
	{
		case GR_BITS_DEPTH:
			/* 1 4
			The number of bits of depth (z or w) in the frame buffer. */
			break;
		case GR_BITS_RGBA:
			/*4 16
			The number of bits each of red, green, blue, alpha in the frame buffer. If there is no separate alpha buffer
			(e.g. on Voodoo2, the depth buffer can be used as an alpha buffer), 0 will be returned for alpha bits.*/
			break;
		case GR_BITS_GAMMA:
			/*1 4
			The number of bits for each channel in the gamma table. If gamma correction is not available,
			grGet will fail, and the params array will be unmodified.*/
			grGet_fill_num(plength, params, 8);
			return FXTRUE;
		case GR_FIFO_FULLNESS:
			/*2 8
			How full the FIFO is, as a percentage. The value is returned in two forms: 1.24 fixed point and a hardware-specific format. */
			break;
		case GR_FOG_TABLE_ENTRIES:
			/*1 4
			The number of entries in the hardware fog table.*/
			break;
		case GR_GAMMA_TABLE_ENTRIES:
			/*1 4
			The number of entries in the hardware gamma table. Returns FXFALSE if it is not possible to manipulate gamma
			(e.g. on a Macronix card, or in windowed mode).*/
			grGet_fill_num(plength, params, 256);
			return FXTRUE;
		case GR_GLIDE_STATE_SIZE:
			/*1 4
			Size of buffer, in bytes, needed to save Glide state. See grGlideGetState.*/
			break;
		case GR_GLIDE_VERTEXLAYOUT_SIZE:
			/*1 4
			Size of buffer, in bytes, needed to save the current vertex layout.
			See grGlideGetVertexLayout.*/
			break;
		case GR_IS_BUSY:
			/*1 4
			Returns FXFALSE if idle, FXTRUE if busy.*/
			grGet_fill_buffer(params, plength, {FXFALSE}, 1);
			return FXTRUE;
		case GR_LFB_PIXEL_PIPE:
			/*1 4
			Returns FXTRUE if LFB writes can go through the 3D 
			pixel pipe, FXFALSE otherwise.*/
			break;
		case GR_MAX_TEXTURE_SIZE:
			/* 1 4
			The width of the largest texture supported on this configuration (e.g. Voodoo Graphics returns 256). */
			grGet_fill_num(plength, params, 256);
			return FXTRUE;
		case GR_MAX_TEXTURE_ASPECT_RATIO:
			 /* 1 4
			 The logarithm base 2 of the maximum aspect ratio supported for power-of-two, mipmap-able textures (e.g. Voodoo Graphics returns 3). */
			 grGet_fill_num(plength, params, 3);
			 return FXTRUE;
		case GR_MEMORY_FB:
			/* 1 4
			The total number of bytes per Pixelfx chip if a non-UMA configuration is used, else 0. In non-UMA configurations, the total FB memory is GR_MEMORY_FB * GR_NUM_FB.*/
			break;
		case GR_MEMORY_TMU:
			/* 1 4
			The total number of bytes per Texelfx chip if a non-UMA configuration is used, else FXFALSE. In non-UMA configurations, the total usable texture memory is GR_MEMORY_TMU * GR_NUM_TMU.*/
			break;
		case GR_MEMORY_UMA:
			/* 1 4
			The total number of bytes if a UMA configuration, else 0. */
			grGet_fill_num(plength, params, 4);
			return FXTRUE;
		case GR_NON_POWER_OF_TWO_TEXTURES:
			/* 1 4
			Returns FXTRUE if this configuration supports textures with arbitrary width and height (up to the maximum). Note that only power-of-two textures may be mipmapped. Not implemented in the initial release of Glide 3.0. */
			break;
		case GR_NUM_BOARDS:
			/* 1 4
			The number of installed boards supported by Glide. Valid before a call to grSstWinOpen.*/
			grGet_fill_num(plength, params, 1);
			return FXTRUE;
			//break;
		case GR_NUM_FB:
			/*1 4
			The number of Pixelfx chips present. This number will always be 1 except for SLI configurations.*/
			grGet_fill_num(plength, params, 1);
			return FXTRUE;
		case GR_NUM_SWAP_HISTORY_BUFFER:
			/* 1 4
			Number of entries in the swap history buffer. Each entry is 4 bytes long. */
			break;
		case GR_NUM_TMU:
			/* 1 4
			The number of Texelfx chips per Pixelfx chip. For integrated chips, the number of TMUs will be returned.*/
			grGet_fill_num(plength, params, GLIDE_MAX_NUM_TMU);
			return FXTRUE;
		case GR_PENDING_BUFFERSWAPS:
			/* 1 4
			The number of buffer swaps pending. */
			break;
		case GR_REVISION_FB:
			/* 1 4
			The revision of the Pixelfx chip(s). */
			break;
		case GR_REVISION_TMU:
			/* 1 4
			The revision of the Texelfx chip(s). */
			break;
		case GR_STATS_LINES:
			/* 1 4
			The number of lines drawn. */
			break;
		case GR_STATS_PIXELS_AFUNC_FAIL:
			/* 1 4
			The number of pixels that failed the alpha function test. */
			break;
		case GR_STATS_PIXELS_CHROMA_FAIL:
			/* 1 4
			The number of pixels that failed the chroma key (or range) test. */
			break;
		case GR_STATS_PIXELS_DEPTHFUNC_FAIL:
			/* 1 4
			The number of pixels that failed the depth buffer test.*/
			break;
		case GR_STATS_PIXELS_IN:
			/*1 4
			The number of pixels that went into the pixel pipe, excluding buffer clears.*/
			break;
		case GR_STATS_PIXELS_OUT:
			/*1 4
			The number of pixels that went out of the pixel pipe, including buffer clears.*/
			break;
		case GR_STATS_POINTS:
			/*1 4
			The number of points drawn.*/
			break;
		case GR_STATS_TRIANGLES_IN:
			/*1 4
			The number of triangles received. */
			break;
		case GR_STATS_TRIANGLES_OUT:
			/* 1 4
			The number of triangles drawn.*/
			break;
		case GR_SWAP_HISTORY:
			/*n 4n
			The swapHistory buffer contents. The ith 4-byte entry counts the number of vertical syncs between the (current frame – i)th frame and its predecessor. If swapHistory is not implemented (e.g. on Voodoo Graphics and Voodoo Rush), grGet will fail, and the params array will be unmodified. Use grGet(GR_NUM_SWAP_HISTORY_BUFFER,…) to determine the number of entries in the buffer.*/
			break;
		case GR_SUPPORTS_PASSTHRU:
			/*1 4
			Returns FXTRUE if pass through mode is supported. See grEnable.*/	
			break;			
		case GR_TEXTURE_ALIGN:
			/*1 4
			The alignment boundary for textures. For example, if textures must be 16-byte aligned, 0x10 would be returned.*/
			grGet_fill_num(plength, params, 256); /*256;*/
			return FXTRUE;
		case GR_VIDEO_POSITION:
			/*2 8
			Vertical and horizontal beam location. Vertical retrace is indicated by y == 0.*/
			break;
		case GR_VIEWPORT:
			/*4 16
			x, y, width, height.*/
			break;
		case GR_WDEPTH_MIN_MAX:
			/*2 8
			The minimum and maximum allowable wbuffer values.*/
			break;
		case GR_ZDEPTH_MIN_MAX:
			/*2 8
			The minimum and maximum allowable zbuffer values.*/
			break;
	}
	
	// manual page .p79 (87)
	
	return FXFALSE;
}

FX_ENTRY void FX_CALL grDepthRange( FxFloat n, FxFloat f )
{
	
}

const char *string_tables[] = {
	" ",
	"Voodoo Graphics",
	"Glide",
	"3Dfx Interactive",
	"3.0",
};

FX_ENTRY const char * FX_CALL grGetString( FxU32 pname )
{
	if(pname >= GR_EXTENSION && pname <= GR_VERSION)
	{
		return string_tables[pname - GR_EXTENSION];
	}
	
	return "";
}

FX_ENTRY void FX_CALL grGlideGetVertexLayout( void *layout ){ }

FX_ENTRY void FX_CALL grGlideSetVertexLayout( const void *layout ){ }

FX_ENTRY void FX_CALL grLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue){ }

typedef struct _GrResolution
{
	GrScreenResolution_t resolution;
	GrScreenRefresh_t refresh;
	int numColorBuffers;
	int numAuxBuffers;
} GrResolution;

FX_ENTRY FxI32 FX_CALL grQueryResolutions( const GrResolution *resTemplate, GrResolution *output )
{
	return 0;
}

FX_ENTRY FxBool FX_CALL grReset( FxU32 what )
{
	switch(what)
	{
		case GR_STATS_PIXELS:
		case GR_STATS_POINTS:
		case GR_STATS_LINES:
		case GR_STATS_TRIANGLES:
//		case GR_VERTEX_PARAMETERS:
			return FXTRUE;
	}
	
	return FXFALSE;
}

typedef FxU32 GrContext_t;

FX_ENTRY FxBool FX_CALL grSelectContext( GrContext_t context )
{
	return FXFALSE;
}

FX_ENTRY void FX_CALL grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
{
	
}

FX_ENTRY void FX_CALL grViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height )
{
	
}

FX_ENTRY void FX_CALL guGammaCorrectionRGB( FxFloat red, FxFloat green, FxFloat blue )
{
	
}

#endif /* GLIDE3 */

FX_ENTRY void FX_CALL
guMovieStart( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieStart( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL
guMovieStop( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieStop( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL
guMovieSetName( const char *name )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guMovieSetName( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL grSstConfigPipeline(long a, long b, long c)
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstConfigPipeline( ) - Not Supported\n" );
#endif
}

FX_ENTRY void FX_CALL grSstVidMode(long a, long b)
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstVidMode( ) - Not Supported\n" );
#endif
}

#ifdef OGL_NOTDONE
# define OGL_STUB(_name, _params) FX_ENTRY void FX_CALL _name _params { GlideMsg( #_name "( ) - Not Supported\n" );  }
#else
# define OGL_STUB(_name, _params) FX_ENTRY void FX_CALL _name _params {  }
#endif

OGL_STUB(guMPDrawTriangle, (long a, long b, long c))
OGL_STUB(guMPInit, (void))
OGL_STUB(guMPTexCombineFunction, (long a))
OGL_STUB(guMPTexSource, (long a, long b))

OGL_STUB(pciClose, (void))
OGL_STUB(pciDeviceExists, (long a))
OGL_STUB(pciFindCard, (long a, long b, long c))
OGL_STUB(pciFindCardMulti, (long a, long b, long c, long d))
OGL_STUB(pciFindFreeMTRR, (long a))
OGL_STUB(pciFindMTRRMatch, (long a, long b, long c, long d))
OGL_STUB(pciGetConfigData, (long a, long b, long c, long d, long e))
OGL_STUB(pciGetErrorCode, (void))
OGL_STUB(pciGetErrorString, (void))
OGL_STUB(pciLinearRangeSetPermission, (long a, long b, long c))
OGL_STUB(pciMapCard, (long a, long b, long c, long d, long e))
OGL_STUB(pciMapCardMulti, (long a, long b, long c, long d, long e, long f))
OGL_STUB(pciMapPhysicalDeviceToLinear, (long a, long b, long c, long d))
OGL_STUB(pciMapPhysicalToLinear, (long a, long b, long c))
OGL_STUB(pciOpen, (void))
OGL_STUB(pciOpenEx, (long a))
OGL_STUB(pciOutputDebugString, (long a))
OGL_STUB(pciSetConfigData, (long a, long b, long c, long d, long e))
OGL_STUB(pciSetMTRR, (long a, long b, long c, long d))
OGL_STUB(pciSetMTRRAmdK6, (long a, long b, long c, long d))
OGL_STUB(pciSetPassThroughBase, (long a, long b))
OGL_STUB(pciUnmapPhysical, (long a, long b))
OGL_STUB(pioInByte, (long a))
OGL_STUB(pioInLong, (long a))
OGL_STUB(pioInWord, (long a))
OGL_STUB(pioOutByte, (long a, long b))
OGL_STUB(pioOutLong, (long a, long b))
OGL_STUB(pioOutWord, (long a, long b))
OGL_STUB(pciFetchRegister, (int a, int b, int c, int d))
OGL_STUB(pciUpdateRegister, (int a, int b, int c, int d, int e))

