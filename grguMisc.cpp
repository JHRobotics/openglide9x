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

//#define GLIDE_MAX_NUM_TMU 1

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
    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.CullMode = mode;

    switch ( Glide.State.CullMode )
    {
    case GR_CULL_DISABLE:
        DGL(glDisable)( GL_CULL_FACE );
        DGL(glCullFace)( GL_BACK );  // This will be called in initialization
        break;

    case GR_CULL_NEGATIVE:
        DGL(glEnable)( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            DGL(glFrontFace)( GL_CCW );
            //GlideMsg( "grCullMode( %d ): GL_CCW\n", mode );
        }
        else
        {
            DGL(glFrontFace)( GL_CW );
            //GlideMsg( "grCullMode( %d ): GL_CW\n", mode );
        }
        break;

    case GR_CULL_POSITIVE:
        DGL(glEnable)( GL_CULL_FACE );
        if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
        {
            DGL(glFrontFace)( GL_CW );
            //GlideMsg( "grCullMode( %d ): GL_CW\n", mode );
        }
        else
        {
            DGL(glFrontFace)( GL_CCW );
            //GlideMsg( "grCullMode( %d ): GL_CCW\n", mode );
        }
        break;
    }

#ifdef OPENGL_DEBUG
    GLErro( "grCullMode" );
#endif

    LeaveGLThread();
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
    EnterGLThread();

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

    DGL(glMatrixMode)( GL_PROJECTION );
    DGL(glLoadIdentity)();

    if ( Glide.State.OriginInformation == GR_ORIGIN_LOWER_LEFT )
    {
        DGL(glOrtho)( Glide.State.ClipMinX, Glide.State.ClipMaxX,
                 Glide.State.ClipMinY, Glide.State.ClipMaxY,
                 OpenGL.ZNear, OpenGL.ZFar );
        DGL(glViewport)( OpenGL.ClipMinX, OpenGL.ClipMinY,
                    OpenGL.ClipMaxX - OpenGL.ClipMinX,
                    OpenGL.ClipMaxY - OpenGL.ClipMinY );
        // Used for the buffer clearing
        DGL(glScissor)( OpenGL.ClipMinX, OpenGL.ClipMinY,
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
        // Used for the buffer clearing
        DGL(glScissor)( OpenGL.ClipMinX, OpenGL.WindowHeight - OpenGL.ClipMaxY,
                   OpenGL.ClipMaxX - OpenGL.ClipMinX,
                   OpenGL.ClipMaxY - OpenGL.ClipMinY );
    }

    DGL(glMatrixMode)( GL_MODELVIEW );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL 
grDisableAllEffects( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grDisableAllEffects( )\n" );
#endif
    EnterGLThread();

    grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    grAlphaTestFunction( GR_CMP_ALWAYS );
    grChromakeyMode( GR_CHROMAKEY_DISABLE );
    grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
    grFogMode( GR_FOG_DISABLE );

    LeaveGLThread();
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
static FxI32 Glide3ViewPort[4] = {0, 0, 640, 480};
static FxFloat Glide3DepthRange[2] = {0.f, 65535.f};
static GrCoordinateSpaceMode_t Glide3CoordinateSpaceMode = GR_WINDOW_COORDS;

/* Glide3.x */
static bool AAOrdered = false;

void FX_CALL grGetGammaTableExt( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue)
{
	
}

FX_ENTRY GrProc FX_CALL grGetProcAddress(char *procName)
{
	if (strcmp(procName, "grGetGammaTableExt") == 0)
	{
		return (GrProc)&grGetGammaTableExt;
	}
	
	return NULL;
}

FX_ENTRY void FX_CALL grEnable( GrEnableMode_t mode)
{
	switch(mode)
	{
		case GR_AA_ORDERED:
			AAOrdered = true;
			break;
		case GR_PASSTHRU:
			Activate3DWindow();
			break;
		case GR_ALLOW_MIPMAP_DITHER:
		case GR_VIDEO_SMOOTHING:
		case GR_SHAMELESS_PLUG:
			break;
	}
}

FX_ENTRY void FX_CALL grDisable ( GrEnableMode_t mode)
{
	switch(mode)
	{
		case GR_AA_ORDERED:
			AAOrdered = false;
			break;
		case GR_PASSTHRU:
			Deactivate3DWindow();
			break;
		case GR_ALLOW_MIPMAP_DITHER:
		case GR_VIDEO_SMOOTHING:
		case GR_SHAMELESS_PLUG:
			break;
	}
}

enum {
	GR_PARAM_POS_XY = 0,
	GR_PARAM_POS_Z,
	GR_PARAM_POS_W,
	GR_PARAM_POS_Q,
	GR_PARAM_POS_FOG_EXT,
	GR_PARAM_POS_ST0,
	GR_PARAM_POS_ST1,
	GR_PARAM_POS_ST2,
	GR_PARAM_POS_Q0,
	GR_PARAM_POS_Q1,
	GR_PARAM_POS_Q2,
	GR_PARAM_POS_A,
	GR_PARAM_POS_RGB,
	GR_PARAM_POS_PARGB,
	GR_PARAM_POS_SIZE
} gr_param_pos_t;

static FxI32 VXLPosition[GR_PARAM_POS_SIZE] = {};

void VXLInit()
{
	for(int i = GR_PARAM_POS_XY; i < GR_PARAM_POS_SIZE; i++)
	{
		VXLPosition[i] = 0;
	}
	
	Glide3CoordinateSpaceMode = GR_WINDOW_COORDS;
};

#define VXLGetOffset(_p) VXLPosition[(_p)]

#define VXLGetFloat(_t, _p, _n) if(VXLGetOffset(_p) != 0){ \
	_t = ((float*)(mem+VXLPosition[_p]))[_n]; \
	}

void Glide3VertexUnpack(GrVertex *v, const void *ptr)
{
	const unsigned char *mem = (const unsigned char *)ptr;
	memset(v, 0, sizeof(GrVertex));
	float fog = 1;
	
	//VXLGetFloat(v->x,   GR_PARAM_POS_XY, 0);
	//VXLGetFloat(v->y,   GR_PARAM_POS_XY, 1);
	/* Glide 3 reference, p. 177: Required. Must be at offset 0. */
	v->x = ((float*)mem)[0];
	v->y = ((float*)mem)[1];
	
	VXLGetFloat(v->ooz, GR_PARAM_POS_Z,  0);
	if(Glide3CoordinateSpaceMode == GR_CLIP_COORDS)
	{
		VXLGetFloat(v->oow, GR_PARAM_POS_W,  0);
		VXLGetFloat(fog,    GR_PARAM_POS_Q,  0);
	}
	else
	{
		VXLGetFloat(v->oow, GR_PARAM_POS_Q,       0);
		VXLGetFloat(fog,    GR_PARAM_POS_FOG_EXT, 0);
	}
	
	if(VXLGetOffset(GR_PARAM_POS_PARGB) == 0)
	{
		VXLGetFloat(v->r,   GR_PARAM_POS_RGB, 0);
		VXLGetFloat(v->g,   GR_PARAM_POS_RGB, 1);
		VXLGetFloat(v->b,   GR_PARAM_POS_RGB, 2);
		VXLGetFloat(v->a,   GR_PARAM_POS_A,   0);
	}
	else
	{
		FxU32 argb = *((FxU32*)(mem+VXLPosition[GR_PARAM_POS_PARGB]));
		ConvertColorF2(argb, v->r, v->g, v->b, v->a);
	}

	VXLGetFloat(v->tmuvtx[0].oow, GR_PARAM_POS_Q0,  0);	
	VXLGetFloat(v->tmuvtx[0].sow, GR_PARAM_POS_ST0, 0);
	VXLGetFloat(v->tmuvtx[0].tow, GR_PARAM_POS_ST0, 1);
	
#if GLIDE_NUM_TMU >= 2
	if(InternalConfig.NumTMU >= 2)
	{
		VXLGetFloat(v->tmuvtx[1].oow, GR_PARAM_POS_Q1,  0);
		VXLGetFloat(v->tmuvtx[1].sow, GR_PARAM_POS_ST1, 0);
		VXLGetFloat(v->tmuvtx[1].tow, GR_PARAM_POS_ST1, 1);
	}
#endif
	
#if GLIDE_NUM_TMU >= 3
	if(InternalConfig.NumTMU >= 3)
	{
		VXLGetFloat(v->tmuvtx[2].oow, GR_PARAM_POS_Q2,  0);
		VXLGetFloat(v->tmuvtx[2].sow, GR_PARAM_POS_ST2, 0);
		VXLGetFloat(v->tmuvtx[2].tow, GR_PARAM_POS_ST2, 1);
	}
#endif

	if(Glide3CoordinateSpaceMode == GR_CLIP_COORDS)
	{
		float q = 65536.0f;
		if(v->oow != 0)
		{
			q = 1.f / v->oow;
		}
		
		v->oow = q;
    v->x = v->x * q * Glide3ViewPort[2] * 0.5f + Glide3ViewPort[2] * 0.5f;
    v->y = v->y * q * Glide3ViewPort[3] * 0.5f + Glide3ViewPort[3] * 0.5f;
    
    float z_range = Glide3DepthRange[1] - Glide3DepthRange[0];
    
    //v->ooz = v->ooz * q * z_range * 0.5f * 65535.f + z_range * 0.5f * 65535.f;
    v->ooz = v->ooz * q * z_range * 0.5f + z_range * 0.5f;

		if(VXLGetOffset(GR_PARAM_POS_PARGB) == 0)
		{
			v->r *= 255.f;
			v->g *= 255.f;
			v->b *= 255.f;
			v->a *= 255.f;
		}
		
		if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU0) != 0)
		{
			//v->tmuvtx[0].oow = v->tmuvtx[0].oow * q; // * 256.0f;
			v->tmuvtx[0].oow = 1.f / v->tmuvtx[0].oow;
		}
		else
		{
			v->tmuvtx[0].oow = q;
		}
		v->tmuvtx[0].sow = v->tmuvtx[0].sow * v->tmuvtx[0].oow * 256.0f;
		v->tmuvtx[0].tow = v->tmuvtx[0].tow * v->tmuvtx[0].oow * 256.0f;

#if GLIDE_NUM_TMU >= 2
		if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU1) != 0)
		{
			//v->tmuvtx[1].oow = v->tmuvtx[1].oow * q; // * 256.0f;
			v->tmuvtx[1].oow = 1.f / v->tmuvtx[1].oow;
		}
		else
		{
			v->tmuvtx[1].oow = v->tmuvtx[0].oow;
		}
		v->tmuvtx[1].sow = v->tmuvtx[1].sow * v->tmuvtx[1].oow * 256.0f;
		v->tmuvtx[1].tow = v->tmuvtx[1].tow * v->tmuvtx[1].oow * 256.0f;
#endif

#if GLIDE_NUM_TMU >= 3
		if((Glide.State.STWHint & GR_STWHINT_W_DIFF_TMU2) != 0)
		{
			//v->tmuvtx[2].oow = v->tmuvtx[2].oow * q; // * 256.0f;
			v->tmuvtx[2].oow = 1.f / v->tmuvtx[2].oow;
		}
		else
		{
			v->tmuvtx[2].oow = v->tmuvtx[1].oow;
		}
		v->tmuvtx[2].sow = v->tmuvtx[2].sow * v->tmuvtx[2].oow * 256.0f;
		v->tmuvtx[2].tow = v->tmuvtx[2].tow * v->tmuvtx[2].oow * 256.0f;
#endif
	}
}

FX_ENTRY void FX_CALL grCoordinateSpace( GrCoordinateSpaceMode_t mode )
{
	Glide3CoordinateSpaceMode = mode;
	GlideMsg( "grCoordinateSpace(%d)\n", mode );
}

FX_ENTRY void FX_CALL grDrawVertexArray ( FxU32 mode, FxU32 count, void **pointers )
{
	static GrVertex fan_pos;
	static GrVertex fan_pos2;
	static GrVertex strip_v[3];
	static FxU32 contindex = 0;
	GrVertex a, b, c;
	
#ifdef OGL_DONE
	GlideMsg( "grDrawVertexArray(%d, %d, --)\n", mode, count );
#endif

  EnterGLThread();
	
	switch(mode)
	{
		case GR_POINTS:
			for(int i = 0; i < count; i++)
			{
				Glide3VertexUnpack(&a, pointers[i]);
				RenderAddPoint(&a, true);
			}
			break;
		case GR_LINE_STRIP:
			for(int i = 1; i < count; i++)
			{
				Glide3VertexUnpack(&a, pointers[i-1]);
				Glide3VertexUnpack(&b, pointers[i]);
				
				RenderDrawTriangles(); /* flush triangles */
    		RenderAddLine(&a, &b, true);
			}
			break;
		case GR_LINES:
			for(int i = 1; i < count; i += 2)
			{
				Glide3VertexUnpack(&a, pointers[i-1]);
				Glide3VertexUnpack(&b, pointers[i]);
				
				RenderDrawTriangles(); /* flush triangles */
    		RenderAddLine(&a, &b, true);
			}
			break;
		case GR_TRIANGLES:
			for(int i = 2; i < count; i += 3)
			{
				Glide3VertexUnpack(&a, pointers[i-2]);
				Glide3VertexUnpack(&b, pointers[i-1]);
				Glide3VertexUnpack(&c, pointers[i]);
				
				//RenderAddTriangle(&a, &b, &c, true);
				RenderAddTriangle(&a, &b, &c, true);
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
		case GR_TRIANGLE_STRIP:
			contindex = 0;
		case GR_TRIANGLE_STRIP_CONTINUE:
			for(int i = 0; i < count; i++, contindex++)
			{
				if(contindex == 0)
				{
					Glide3VertexUnpack(&strip_v[0], pointers[i]);
				}
				else if(contindex == 1)
				{
					Glide3VertexUnpack(&strip_v[1], pointers[i]);
				}
				else
				{
					Glide3VertexUnpack(&strip_v[contindex % 3], pointers[i]);
					/*GrVertex *v1 = &strip_v[(contindex+1) % 3];
					GrVertex *v2 = &strip_v[(contindex+2) % 3];
					GrVertex *v3 = &strip_v[(contindex+0) % 3];
					
					switch(contindex % 2)
					{
						case 0: RenderAddTriangle(v1, v2, v3, true); break;
						//case 1: RenderAddTriangle(v2, v1, v3, true); break;
						case 1: RenderAddTriangle(v1, v3, v2, true); break;
					}*/
					RenderAddTriangle(
						&strip_v[(contindex + 1) % 3],
						&strip_v[(contindex + 2 + (contindex % 2)) % 3],
						&strip_v[(contindex + 2 + ((contindex+1) % 2)) % 3],
						true);
				}
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER && contindex >= 2)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
		case GR_POLYGON:
			/*
			 * "Mode GR_POLYGON is inappropriate with the example vertex list, since
			 * they produce a non-convex polygon. If GR_POLYGON is specified, the
			 * points are drawn as a triangle fan and produce a different polygon
       * than the one that results from connecting the vertices in sequence."
       *
       * Glide 3.0 Reference Manual p. 61
       *
			 */
		case GR_TRIANGLE_FAN:
			contindex = 0;
		case GR_TRIANGLE_FAN_CONTINUE:
			for(int i = 0; i < count; i++,contindex++)
			{
				if(contindex == 0)
				{
					Glide3VertexUnpack(&fan_pos, pointers[i]);
				}
				else if(contindex == 1)
				{
					Glide3VertexUnpack(&fan_pos2, pointers[i]);
				}
				else
				{
					Glide3VertexUnpack(&a, pointers[i]);
					RenderAddTriangle(&fan_pos, &fan_pos2, &a, true);
					fan_pos2 = a;
				}
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER && contindex >= 2)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
	}
	
  LeaveGLThread();
}

#define PVERTEX(_n) ((const void *)(mem+(stride*(_n))))
FX_ENTRY void FX_CALL grDrawVertexArrayContiguous ( FxU32 mode, FxU32 count, void *vertex, FxU32 stride )
{
	const unsigned char *mem = (const unsigned char *)vertex;

	static GrVertex fan_pos;
	static GrVertex fan_pos2;
	static GrVertex strip_v[3];
	static FxU32 contindex = 0;
	GrVertex a, b, c;
	
#ifdef OGL_DONE
	GlideMsg("grDrawVertexArrayContiguous(%d, %d, --, %u)\n", mode, count, stride);
#endif
	
  EnterGLThread();
	
	switch(mode)
	{
		case GR_POINTS:
			for(int i = 0; i < count; i++)
			{
				Glide3VertexUnpack(&a, PVERTEX(i));
				RenderAddPoint(&a, true);
			}
			break;
		case GR_LINE_STRIP:
			for(int i = 1; i < count; i++)
			{
				Glide3VertexUnpack(&a, PVERTEX(i-1));
				Glide3VertexUnpack(&b, PVERTEX(i));
				
				RenderDrawTriangles(); /* flush triangles */
    		RenderAddLine(&a, &b, true);
			}
			break;
		case GR_LINES:
			for(int i = 1; i < count; i += 2)
			{
				Glide3VertexUnpack(&a, PVERTEX(i-1));
				Glide3VertexUnpack(&b, PVERTEX(i));
				
				RenderDrawTriangles(); /* flush triangles */
    		RenderAddLine(&a, &b, true);
			}
			break;
		case GR_TRIANGLES:
			for(int i = 2; i < count; i += 3)
			{
				Glide3VertexUnpack(&a, PVERTEX(i-2));
				Glide3VertexUnpack(&b, PVERTEX(i-1));
				Glide3VertexUnpack(&c, PVERTEX(i));
				
				RenderAddTriangle(&a, &b, &c, true);
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
		case GR_TRIANGLE_STRIP:
			contindex = 0;
		case GR_TRIANGLE_STRIP_CONTINUE:
			for(int i = 0; i < count; i++, contindex++)
			{
				if(contindex == 0)
				{
					Glide3VertexUnpack(&strip_v[0], PVERTEX(i));
				}
				else if(contindex == 1)
				{
					Glide3VertexUnpack(&strip_v[1], PVERTEX(i));
				}
				else
				{
					Glide3VertexUnpack(&strip_v[contindex % 3], PVERTEX(i));
					RenderAddTriangle(
						&strip_v[(contindex + 1) % 3],
						&strip_v[(contindex + 2 + (contindex % 2)) % 3],
						&strip_v[(contindex + 2 + ((contindex+1) % 2)) % 3],
						true);
				}
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
		case GR_POLYGON: /* See: grDrawVertexArray */
		case GR_TRIANGLE_FAN:
			contindex = 0;
		case GR_TRIANGLE_FAN_CONTINUE:
			for(int i = 0; i < count; i++,contindex++)
			{
				if(contindex == 0)
				{
					Glide3VertexUnpack(&fan_pos, PVERTEX(i));
				}
				else if(contindex == 1)
				{
					Glide3VertexUnpack(&fan_pos2, PVERTEX(i));
				}
				else
				{
					Glide3VertexUnpack(&a, PVERTEX(i));
					RenderAddTriangle(&fan_pos, &fan_pos2, &a, true);
					fan_pos2 = a;
				}
			}

			if(Glide.State.RenderBuffer == GR_BUFFER_FRONTBUFFER)
			{
				RenderDrawTriangles();
				DGL(glFlush)();
			}
			break;
	}

	LeaveGLThread();
}
#undef PVERTEX

FX_ENTRY void FX_CALL grFlush ( void )
{
  EnterGLThread();
    
	RenderDrawTriangles();
	DGL(glFlush)();
	DGL(glFinish)();

  LeaveGLThread();
}

FX_ENTRY void FX_CALL grFinish ( void )
{
  EnterGLThread();
	grFlush();

	LeaveGLThread();
}

static inline FxU32 grGet_fill_buffer(void *dst, FxU32 dstlen, const void *src, FxU32 srclen)
{
	FxU32 size = dstlen;
	if(size > srclen)
	{
		size = srclen;
	}
		
	memcpy(dst, src, size);
	
	return size;
}

static inline FxU32 grGet_fill_num(FxU32 dstlen, FxI32 *dst, FxI32 n)
{
	if(dstlen >= 4)
	{
		*dst = n;
		return 4;
	}
	
	return 0;
}

FX_ENTRY FxU32 FX_CALL grGet ( FxU32 pname, FxU32 plength, FxI32 *params )
{
	static const FxU32 rgba_bits[] = {5, 6, 5, 0};
	static const FxU32 empty_fifo[] = {0, 0};
	
#ifdef OGL_DONE
  const char *sname = "---unknown option---";
  
  #define ONAME(_e) case _e: sname = #_e; break;
  
  switch(pname)
  {
  	ONAME(GR_BITS_DEPTH)
  	ONAME(GR_BITS_RGBA)
  	ONAME(GR_BITS_GAMMA)
  	ONAME(GR_FIFO_FULLNESS)
  	ONAME(GR_FOG_TABLE_ENTRIES)
  	ONAME(GR_GLIDE_STATE_SIZE)
  	ONAME(GR_GLIDE_VERTEXLAYOUT_SIZE)
  	ONAME(GR_LFB_PIXEL_PIPE)
  	ONAME(GR_MAX_TEXTURE_SIZE)
  	ONAME(GR_MAX_TEXTURE_ASPECT_RATIO)
  	ONAME(GR_MEMORY_TMU)
  	ONAME(GR_MEMORY_FB)
  	ONAME(GR_MEMORY_UMA)
  	ONAME(GR_NON_POWER_OF_TWO_TEXTURES)
  	ONAME(GR_NUM_BOARDS)
  	ONAME(GR_NUM_FB)
  	ONAME(GR_NUM_SWAP_HISTORY_BUFFER)
  	ONAME(GR_PENDING_BUFFERSWAPS)
  	ONAME(GR_REVISION_FB)
  	ONAME(GR_REVISION_TMU)
  	ONAME(GR_STATS_LINES)
  	ONAME(GR_STATS_PIXELS_AFUNC_FAIL)
  	ONAME(GR_STATS_PIXELS_CHROMA_FAIL)
  	ONAME(GR_STATS_PIXELS_DEPTHFUNC_FAIL)
  	ONAME(GR_STATS_PIXELS_IN)
  	ONAME(GR_STATS_PIXELS_OUT)
  	ONAME(GR_STATS_POINTS)
  	ONAME(GR_STATS_TRIANGLES_IN)
  	ONAME(GR_STATS_TRIANGLES_OUT)
  	ONAME(GR_SWAP_HISTORY)
  	ONAME(GR_SUPPORTS_PASSTHRU)
  	ONAME(GR_TEXTURE_ALIGN)
  	ONAME(GR_VIDEO_POSITION)
  	ONAME(GR_VIEWPORT)
  	ONAME(GR_WDEPTH_MIN_MAX)
  	ONAME(GR_ZDEPTH_MIN_MAX)
  	ONAME(GR_NUM_TMU)
  }
  
  #undef ONAME

	GlideMsg("grGet(%d = %s, %d, ---)\n", pname, sname, plength);
#endif
	
	switch(pname)
	{
		case GR_BITS_DEPTH:
			/* 1 4
			The number of bits of depth (z or w) in the frame buffer. */
			return grGet_fill_num(plength, params, 16);
		case GR_BITS_RGBA:
			/*4 16
			The number of bits each of red, green, blue, alpha in the frame buffer. If there is no separate alpha buffer
			(e.g. on Voodoo2, the depth buffer can be used as an alpha buffer), 0 will be returned for alpha bits.*/
			return grGet_fill_buffer(params, plength, rgba_bits, 16);
		case GR_BITS_GAMMA:
			/*1 4
			The number of bits for each channel in the gamma table. If gamma correction is not available,
			grGet will fail, and the params array will be unmodified.*/
			return grGet_fill_num(plength, params, 8);
		case GR_FIFO_FULLNESS:
			/*2 8
			How full the FIFO is, as a percentage. The value is returned in two forms: 1.24 fixed point and a hardware-specific format. */
			return grGet_fill_buffer(params, plength, &empty_fifo[0], sizeof(empty_fifo));
		case GR_FOG_TABLE_ENTRIES:
			/*1 4
			The number of entries in the hardware fog table.*/
			return grGet_fill_num(plength, params, GR_FOG_TABLE_SIZE);
		case GR_GAMMA_TABLE_ENTRIES:
			/*1 4
			The number of entries in the hardware gamma table. Returns FXFALSE if it is not possible to manipulate gamma
			(e.g. on a Macronix card, or in windowed mode).*/
			return grGet_fill_num(plength, params, 32); /* 32 * 4 */
		case GR_GLIDE_STATE_SIZE:
			/*1 4
			Size of buffer, in bytes, needed to save Glide state. See grGlideGetState.*/
			return grGet_fill_num(plength, params, sizeof(GlideState));
		case GR_GLIDE_VERTEXLAYOUT_SIZE:
			/*1 4
			Size of buffer, in bytes, needed to save the current vertex layout.
			See grGlideGetVertexLayout.*/
			return grGet_fill_num(plength, params, 100); /* value from original driver, more than we need */
		case GR_IS_BUSY:
			/*1 4
			Returns FXFALSE if idle, FXTRUE if busy.*/
			return grGet_fill_num(plength, params, FXFALSE);
		case GR_LFB_PIXEL_PIPE:
			/*1 4
			Returns FXTRUE if LFB writes can go through the 3D 
			pixel pipe, FXFALSE otherwise.*/
			return grGet_fill_num(plength, params, FXTRUE);
		case GR_MAX_TEXTURE_SIZE:
			/* 1 4
			The width of the largest texture supported on this configuration (e.g. Voodoo Graphics returns 256). */
			return grGet_fill_num(plength, params, 256);
		case GR_MAX_TEXTURE_ASPECT_RATIO:
			 /* 1 4
			 The logarithm base 2 of the maximum aspect ratio supported for power-of-two, mipmap-able textures (e.g. Voodoo Graphics returns 3). */
			 return grGet_fill_num(plength, params, 3);
		case GR_MEMORY_FB:
			/* 1 4
			The total number of bytes per Pixelfx chip if a non-UMA configuration is used, else 0. In non-UMA configurations, the total FB memory is GR_MEMORY_FB * GR_NUM_FB.*/
			return grGet_fill_num(plength, params, UserConfig.FrameBufferMemorySize*0x100000);
		case GR_MEMORY_TMU:
			/* 1 4
			The total number of bytes per Texelfx chip if a non-UMA configuration is used, else FXFALSE. In non-UMA configurations, the total usable texture memory is GR_MEMORY_TMU * GR_NUM_TMU.*/
			//return grGet_fill_num(plength, params, UserConfig.TextureMemorySize*0x100000);
			return grGet_fill_num(plength, params, Glide.TexMemoryPerTMU*InternalConfig.NumTMU);
		case GR_MEMORY_UMA:
			/* 1 4
			The total number of bytes if a UMA configuration, else 0. */
			//return grGet_fill_num(plength, params, 4);
			return grGet_fill_num(plength, params, 0);
		case GR_NON_POWER_OF_TWO_TEXTURES:
			/* 1 4
			Returns FXTRUE if this configuration supports textures with arbitrary width and height (up to the maximum). Note that only power-of-two textures may be mipmapped. Not implemented in the initial release of Glide 3.0. */
			return grGet_fill_num(plength, params, FXFALSE);
		case GR_NUM_BOARDS:
			/* 1 4
			The number of installed boards supported by Glide. Valid before a call to grSstWinOpen.*/
			return grGet_fill_num(plength, params, 1);
			//break;
		case GR_NUM_FB:
			/*1 4
			The number of Pixelfx chips present. This number will always be 1 except for SLI configurations.*/
			return grGet_fill_num(plength, params, 1);
		case GR_NUM_SWAP_HISTORY_BUFFER:
			/* 1 4
			Number of entries in the swap history buffer. Each entry is 4 bytes long. */
			return grGet_fill_num(plength, params, 0);
		case GR_NUM_TMU:
			/* 1 4
			The number of Texelfx chips per Pixelfx chip. For integrated chips, the number of TMUs will be returned.*/
			//return grGet_fill_num(plength, params, GLIDE_NUM_TMU);
			return grGet_fill_num(plength, params, InternalConfig.NumTMU);
		case GR_PENDING_BUFFERSWAPS:
			/* 1 4
			The number of buffer swaps pending. */
			return grGet_fill_num(plength, params, 0);
		case GR_REVISION_FB:
			/* 1 4
			The revision of the Pixelfx chip(s). */
			return grGet_fill_num(plength, params, Glide.PixelfxVersion);
		case GR_REVISION_TMU:
			/* 1 4
			The revision of the Texelfx chip(s). */
			return grGet_fill_num(plength, params, Glide.TexelfxVersion);
		case GR_STATS_LINES:
		case GR_STATS_PIXELS_AFUNC_FAIL:
		case GR_STATS_PIXELS_CHROMA_FAIL:
		case GR_STATS_PIXELS_DEPTHFUNC_FAIL:
		case GR_STATS_PIXELS_IN:
		case GR_STATS_PIXELS_OUT:
		case GR_STATS_POINTS:
		case GR_STATS_TRIANGLES_IN:
		case GR_STATS_TRIANGLES_OUT:
			/* we don't count, but return something valid */
			return grGet_fill_num(plength, params, 0);
			break;
		case GR_SWAP_HISTORY:
			/*n 4n
			The swapHistory buffer contents. The ith 4-byte entry counts the number of vertical syncs between the (current frame – i)th frame and its predecessor. If swapHistory is not implemented (e.g. on Voodoo Graphics and Voodoo Rush), grGet will fail, and the params array will be unmodified. Use grGet(GR_NUM_SWAP_HISTORY_BUFFER,…) to determine the number of entries in the buffer.*/
			break;
		case GR_SUPPORTS_PASSTHRU:
			/*1 4
			Returns FXTRUE if pass through mode is supported. See grEnable.*/
			return grGet_fill_num(plength, params, FXFALSE);
		case GR_TEXTURE_ALIGN:
			/*1 4
			The alignment boundary for textures. For example, if textures must be 16-byte aligned, 0x10 would be returned.*/
			return grGet_fill_num(plength, params, GLIDE_TEXTURE_ALIGN);
		case GR_VIDEO_POSITION:
			/*2 8
			Vertical and horizontal beam location. Vertical retrace is indicated by y == 0.*/
			return grGet_fill_buffer(params, plength, &empty_fifo[0], sizeof(empty_fifo));
		case GR_VIEWPORT:
			/*4 16
			x, y, width, height.*/
			return grGet_fill_buffer(params, plength, &Glide3ViewPort[0], 4*4);
		case GR_WDEPTH_MIN_MAX:
			/*2 8
			The minimum and maximum allowable wbuffer values.*/
			{
				FxU32 wDefault[] = {SST1_WDEPTHVALUE_NEAREST, SST1_WDEPTHVALUE_FARTHEST};
				return grGet_fill_buffer(params, plength, &wDefault[0], sizeof(wDefault));
			}
		case GR_ZDEPTH_MIN_MAX:
			/*2 8
			The minimum and maximum allowable zbuffer values.*/
			{
				const FxU32 zDefault[] = {SST1_ZDEPTHVALUE_NEAREST, SST1_ZDEPTHVALUE_FARTHEST};
				return grGet_fill_buffer(params, plength, &zDefault[0], sizeof(zDefault));
			}
	}
	
	// manual page .p79 (87)
	
	return 0;
}

FX_ENTRY void FX_CALL grDepthRange( FxFloat n, FxFloat f )
{
	Glide3DepthRange[0] = n;
	Glide3DepthRange[1] = f;
}

const char *voodoo_names[] = {
	[GR_SSTTYPE_VOODOO]     = "Voodoo Graphics",
	[GR_SSTTYPE_SST96]      = "Voodoo Rush",
	[GR_SSTTYPE_AT3D]       = "AT3D",
	[GR_SSTTYPE_Voodoo2]    = "Voodoo^2",
  [GR_SSTTYPE_Banshee]    = "Banshee"
};

const char *string_tables[] = {
	" GETGAMMA TEXMIRROR ", /* missings: CHROMARANGE, TEXCHROMA, PALETTE6666, FOGCOORD */
	"Voodoo Graphics",
	"Glide",
	"3Dfx Interactive",
	"3.04.00.0455", /* from last original Voodoo driver */
};

FX_ENTRY const char * FX_CALL grGetString( FxU32 pname )
{
	if(pname == GR_HARDWARE && Glide.SSTType >= GR_SSTTYPE_VOODOO && Glide.SSTType <= GR_SSTTYPE_Banshee)
	{
		return voodoo_names[Glide.SSTType];
	}
	
	if(pname >= GR_EXTENSION && pname <= GR_VERSION)
	{
		return string_tables[pname - GR_EXTENSION];
	}
	
	return "";
}

static void dump_layout()
{
	GlideMsg("=== VertexLayout ===\n");
	for(int i = 0; i < GR_PARAM_POS_SIZE; i++)
	{
		GlideMsg("%3d: %08X\n", i, VXLPosition[i]);
	}
	GlideMsg("====================\n");
}

FX_ENTRY void FX_CALL grGlideGetVertexLayout( void *layout )
{
#ifdef OGL_DONE
	GlideMsg( "grGlideGetVertexLayout(%p)\n", layout);
#endif
	
	memcpy(layout, VXLPosition, sizeof(VXLPosition));
}

FX_ENTRY void FX_CALL grGlideSetVertexLayout( const void *layout )
{
#ifdef OGL_DONE
	GlideMsg( "grGlideSetVertexLayout(%p)\n", layout);
#endif
	
	memcpy(VXLPosition, layout, sizeof(VXLPosition));
	
	if(VXLPosition[GR_PARAM_POS_ST1])
		Glide.State.STWHint |= GR_STWHINT_ST_DIFF_TMU1;
	else
		Glide.State.STWHint &= ~GR_STWHINT_ST_DIFF_TMU1;

	if(VXLPosition[GR_PARAM_POS_ST2])
		Glide.State.STWHint |= GR_STWHINT_ST_DIFF_TMU2;
	else
		Glide.State.STWHint &= ~GR_STWHINT_ST_DIFF_TMU2;

	if(VXLPosition[GR_PARAM_POS_Q0])
		Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU0;
	else
		Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU0;

	if(VXLPosition[GR_PARAM_POS_Q1])
		Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU1;
	else
		Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU1;

	if(VXLPosition[GR_PARAM_POS_Q2])
		Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU2;
	else
		Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU2;
	
	//dump_layout();
}

FX_ENTRY void FX_CALL grLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue)
{
#ifdef OGL_NOTDONE
	GlideMsg( "grLoadGammaTable(%u, ---, ---, ---) - not supported\n", nentries);
#endif
}

typedef struct _GrResolution
{
	GrScreenResolution_t resolution;
	GrScreenRefresh_t refresh;
	int numColorBuffers;
	int numAuxBuffers;
} GrResolution;

#define GR_QUERY_ANY  ((FxU32)(~0))

FX_ENTRY FxI32 FX_CALL grQueryResolutions( const GrResolution *resTemplate, GrResolution *output )
{
	FxI32 outsize = 0;
	GrResolution *ptr = output;
	
	for(GrScreenResolution_t r = GR_RESOLUTION_320x200; r <= GR_RESOLUTION_1600x1200; r++)
	{
		GrResolution t = {r, GR_REFRESH_60Hz, 2, 1};
		
		if(resTemplate->resolution == GR_QUERY_ANY || t.resolution == resTemplate->resolution)
		{
			if(resTemplate->refresh == GR_QUERY_ANY || t.refresh == resTemplate->refresh)
			{
				if(resTemplate->numColorBuffers == GR_QUERY_ANY || t.numColorBuffers == resTemplate->numColorBuffers)
				{
					if(resTemplate->numAuxBuffers == GR_QUERY_ANY || t.numAuxBuffers == resTemplate->numAuxBuffers)
					{
						if(ptr != NULL)
						{
							*ptr = t;
							ptr++;
						}
						outsize += sizeof(GrResolution);
					}
				}
			}
		}
	}
	
	return outsize;
}

FX_ENTRY FxBool FX_CALL grReset( FxU32 what )
{
	switch(what)
	{
		case GR_STATS_PIXELS:
		case GR_STATS_POINTS:
		case GR_STATS_LINES:
		case GR_STATS_TRIANGLES:
			return FXTRUE;
		case GR_VERTEX_PARAMETER:
			GlideMsg("grReset(GR_VERTEX_PARAMETER)\n");
			VXLInit();
			return FXTRUE;
	}
	
	return FXFALSE;
}

typedef FxU32 GrContext_t;

FX_ENTRY FxBool FX_CALL grSelectContext( GrContext_t context )
{
  EnterGLThread();
	
	if(context == 1) return FXTRUE; /* only one context */
		
#ifdef OGL_NOTDONE
    GlideMsg( "grSelectContext - only 1 context, required: %d\n", context );
#endif

	return FXFALSE;

	LeaveGLThread();
}

#define VL_CASE(_p) case _p: return #_p;

static const char *vlname(FxU32 param)
{
	switch(param)
	{
		VL_CASE(GR_PARAM_XY)
		VL_CASE(GR_PARAM_Z)
		VL_CASE(GR_PARAM_W)
		VL_CASE(GR_PARAM_Q)
		VL_CASE(GR_PARAM_FOG_EXT)

		VL_CASE(GR_PARAM_A)
		VL_CASE(GR_PARAM_A1)
		VL_CASE(GR_PARAM_A2)
		VL_CASE(GR_PARAM_A3)
		VL_CASE(GR_PARAM_A4)
		VL_CASE(GR_PARAM_A5)
		VL_CASE(GR_PARAM_A6)
		VL_CASE(GR_PARAM_A7)

		VL_CASE(GR_PARAM_RGB)
		VL_CASE(GR_PARAM_RGB1)
		VL_CASE(GR_PARAM_RGB2)
		VL_CASE(GR_PARAM_RGB3)
		VL_CASE(GR_PARAM_RGB4)
		VL_CASE(GR_PARAM_RGB5)
		VL_CASE(GR_PARAM_RGB6)
		VL_CASE(GR_PARAM_RGB7)

		VL_CASE(GR_PARAM_PARGB)
		VL_CASE(GR_PARAM_PARGB1)
		VL_CASE(GR_PARAM_PARGB2)
		VL_CASE(GR_PARAM_PARGB3)
		VL_CASE(GR_PARAM_PARGB4)
		VL_CASE(GR_PARAM_PARGB5)
		VL_CASE(GR_PARAM_PARGB6)
		VL_CASE(GR_PARAM_PARGB7)

		VL_CASE(GR_PARAM_ST0)
		VL_CASE(GR_PARAM_ST1)
		VL_CASE(GR_PARAM_ST2)
		VL_CASE(GR_PARAM_ST3)
		VL_CASE(GR_PARAM_ST4)
		VL_CASE(GR_PARAM_ST5)
		VL_CASE(GR_PARAM_ST6)
		VL_CASE(GR_PARAM_ST7)

		VL_CASE(GR_PARAM_Q0)
		VL_CASE(GR_PARAM_Q1)
		VL_CASE(GR_PARAM_Q2)
		VL_CASE(GR_PARAM_Q3)
		VL_CASE(GR_PARAM_Q4)
		VL_CASE(GR_PARAM_Q5)
		VL_CASE(GR_PARAM_Q6)
		VL_CASE(GR_PARAM_Q7)
	}
	return "UNKNOWN";
}

#undef VL_CASE

FX_ENTRY void FX_CALL grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
{
	if(mode == GR_PARAM_ENABLE)
	{
		GlideMsg( "grVertexLayout(%s=%u, %d, GR_PARAM_ENABLE)\n", vlname(param), param, offset );
		
		switch(param)
		{
			case GR_PARAM_XY:
				VXLPosition[GR_PARAM_POS_XY] = offset;
				break;
			case GR_PARAM_Z:
				VXLPosition[GR_PARAM_POS_Z] = offset;
				break;
			case GR_PARAM_Q:
				VXLPosition[GR_PARAM_POS_Q] = offset;
				break;
			case GR_PARAM_W:
				VXLPosition[GR_PARAM_POS_W] = offset;
				break;
			case GR_PARAM_A:
				VXLPosition[GR_PARAM_POS_A] = offset;
				VXLPosition[GR_PARAM_POS_PARGB] = 0;
				break;
			case GR_PARAM_RGB:
				VXLPosition[GR_PARAM_POS_RGB] = offset;
				VXLPosition[GR_PARAM_POS_PARGB] = 0;
				break;
			case GR_PARAM_PARGB:
				VXLPosition[GR_PARAM_POS_PARGB] = offset;
				VXLPosition[GR_PARAM_POS_RGB] = 0;
				VXLPosition[GR_PARAM_POS_A]   = 0;
				break;
			case GR_PARAM_ST0:
				VXLPosition[GR_PARAM_POS_ST0] = offset;
				break;
			case GR_PARAM_ST1:
				VXLPosition[GR_PARAM_POS_ST1] = offset;
				Glide.State.STWHint |= GR_STWHINT_ST_DIFF_TMU1;
				break;
			case GR_PARAM_ST2:
				VXLPosition[GR_PARAM_POS_ST2] = offset;
				Glide.State.STWHint |= GR_STWHINT_ST_DIFF_TMU2;
				break;
			case GR_PARAM_Q0:
				VXLPosition[GR_PARAM_POS_Q0] = offset;
				Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU0;
				break;
			case GR_PARAM_Q1:
				VXLPosition[GR_PARAM_POS_Q1] = offset;
				Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU1;
				break;
			case GR_PARAM_Q2:
				VXLPosition[GR_PARAM_POS_Q2] = offset;
				Glide.State.STWHint |= GR_STWHINT_W_DIFF_TMU2;
				break;
			case GR_PARAM_FOG_EXT:
				VXLPosition[GR_PARAM_POS_FOG_EXT] = offset;
				break;
		}
	}
	else
	{
		switch(param)
		{
			case GR_PARAM_XY:
				VXLPosition[GR_PARAM_POS_XY] = 0;
				break;
			case GR_PARAM_Z:
				VXLPosition[GR_PARAM_POS_Z] = 0;
				break;
			case GR_PARAM_Q:
				VXLPosition[GR_PARAM_POS_Q] = 0;
				break;
			case GR_PARAM_W:
				VXLPosition[GR_PARAM_POS_W] = 0;
				break;
			case GR_PARAM_A:
				VXLPosition[GR_PARAM_POS_A] = 0;
				break;
			case GR_PARAM_RGB:
				VXLPosition[GR_PARAM_POS_RGB] = 0;
				break;
			case GR_PARAM_PARGB:
				VXLPosition[GR_PARAM_POS_PARGB] = 0;
				break;
			case GR_PARAM_ST0:
				VXLPosition[GR_PARAM_POS_ST0] = 0;
				break;
			case GR_PARAM_ST1:
				VXLPosition[GR_PARAM_POS_ST1] = 0;
				Glide.State.STWHint &= ~GR_STWHINT_ST_DIFF_TMU1;
				break;
			case GR_PARAM_ST2:
				VXLPosition[GR_PARAM_POS_ST2] = 0;
				Glide.State.STWHint &= ~GR_STWHINT_ST_DIFF_TMU2;
				break;
			case GR_PARAM_Q0:
				VXLPosition[GR_PARAM_POS_Q0] = 0;
				Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU0;
				break;
			case GR_PARAM_Q1:
				VXLPosition[GR_PARAM_POS_Q1] = 0;
				Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU1;
				break;
			case GR_PARAM_Q2:
				VXLPosition[GR_PARAM_POS_Q2] = 0;
				Glide.State.STWHint &= ~GR_STWHINT_W_DIFF_TMU2;
				break;
			case GR_PARAM_FOG_EXT:
				VXLPosition[GR_PARAM_POS_FOG_EXT] = 0;
				break;
		}
	}
	// Fog hack by kjliew
	/*if(VXLPosition[GR_PARAM_POS_Q1] != 0 && VXLPosition[GR_PARAM_POS_Q1] == VXLPosition[GR_PARAM_POS_Q2])
	{
		GrFog_t fto[GR_FOG_TABLE_SIZE];
		guFogGenerateExp(fto, 1.f / guFogTableIndexToW(GR_FOG_TABLE_SIZE - 1));
		grFogTable(fto);
	}
	*/
}


FX_ENTRY void FX_CALL grViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height )
{
	if(width != 0 && height != 0)
	{
		Glide3ViewPort[0] = x;
		Glide3ViewPort[1] = y;
		Glide3ViewPort[2] = width;
		Glide3ViewPort[3] = height;
	}
	
	GlideMsg( "grViewport(%d, %d, %d, %d)\n", x, y, width, height);
	
}

FX_ENTRY void FX_CALL guGammaCorrectionRGB( FxFloat red, FxFloat green, FxFloat blue )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guGammaCorrectionRGB(---, ---, ---) - Not Supported\n");
#endif
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

