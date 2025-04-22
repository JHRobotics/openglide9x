//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  Glide Texture Functions
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
#include "PGUTexture.h"


// Functions

//*************************************************
//* Return the lowest start address for texture downloads
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexMinAddress( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "grTexMinAddress( %d )", tmu);
#endif
    if(tmu >= InternalConfig.NumTMU) return 0;

#if 0
    return Glide.TexMemoryPerTMU * tmu;
#else
		return 0;
#endif
}

//*************************************************
//* Return the highest start address for texture downloads
//*************************************************
/*
  From manual:
  grTexMaxAddress() returns the last possible appropriately aligned address that can be used as a
  starting address; only the smallest possible texture can be loaded there: the 1x1 texture
  GR_LOD_LOG2_1.
*/
FX_ENTRY FxU32 FX_CALL
grTexMaxAddress( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "grTexMaxAddress( %d )", tmu );
#endif
    if(tmu >= InternalConfig.NumTMU) return 0;

#if 0
    return (Glide.TexMemoryPerTMU * (tmu+1) - GLIDE_TEXTURE_ALIGN);
#else
		return Glide.TexMemoryPerTMU - GLIDE_TEXTURE_ALIGN;
#endif
}

static FxU32 relativeTMUAddress(GrChipID_t tmu, FxU32 addr)
{
	FxU32 start = grTexMinAddress(tmu);
#if DEBUG
	if(start > addr)
	{
		Error("unchecked address!");
		return 0;
	}
#endif
	
	return addr - start;
}


//*************************************************
//* Specify the current texture source for rendering
//*************************************************
FX_ENTRY void FX_CALL
grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info )
{
#ifdef OGL_DONE
    GlideMsg( "grTexSource( %d, %d, %d, --- )\n", tmu, startAddress, evenOdd );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    RenderDrawTriangles( );
    
		if(!(startAddress >= grTexMinAddress(tmu) && startAddress <= grTexMaxAddress(tmu)))
		{
    	GlideMsg("%s, TMU: %d: WRONG memory address (%x)!\n", __FUNCTION__, tmu, startAddress);
    	GlideMsg("Call from 0x%X\n", __builtin_extract_return_addr (__builtin_return_address(0)));
    	LeaveGLThread();
			return;
		}
		FxU32 relative = relativeTMUAddress(tmu, startAddress);

    Glide.State.TexSource[tmu].StartAddress = startAddress;
    Glide.State.TexSource[tmu].EvenOdd = evenOdd;
    Glide.State.TexSource[tmu].Info.format = info->format;
#ifdef GLIDE3
    Glide.State.TexSource[tmu].Info.aspectRatioLog2 = info->aspectRatioLog2;
    Glide.State.TexSource[tmu].Info.largeLodLog2 = info->largeLodLog2;
    Glide.State.TexSource[tmu].Info.smallLodLog2 = info->smallLodLog2;
#else
    Glide.State.TexSource[tmu].Info.aspectRatio = info->aspectRatio;
    Glide.State.TexSource[tmu].Info.largeLod = info->largeLod;
    Glide.State.TexSource[tmu].Info.smallLod = info->smallLod;
#endif

    Glide.State.TexSource[tmu].Valid = Textures->Source(tmu, relative, evenOdd, info);

    LeaveGLThread();
}

//*************************************************
//* Return the texture memory consumed by a texture
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexTextureMemRequired( FxU32 dwEvenOdd, GrTexInfo *texInfo )
{
#ifdef OGL_DONE
    GlideMsg( "grTexTextureMemRequired( %u, --- )\n", dwEvenOdd );
#endif

    return Textures->TextureMemRequired( dwEvenOdd, texInfo );
}

//*************************************************
//* Return the texture memory consumed by a texture
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadMipMap( %d, %u, %u, --- )\n", tmu, 
        startAddress, evenOdd );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    RenderDrawTriangles( );
#ifndef GLIDE3
    info->smallLod = info->largeLod;
#else
    info->smallLodLog2 = info->largeLodLog2;
#endif

		if(!(startAddress >= grTexMinAddress(tmu) && startAddress <= grTexMaxAddress(tmu)))
		{
    	GlideMsg("%s, TMU: %d: WRONG memory address (%x)!\n", __FUNCTION__, tmu, startAddress);
    	GlideMsg("Min %x, Max %x\n", grTexMinAddress(tmu), grTexMaxAddress(tmu));
    	LeaveGLThread();
			return;
		}

    Textures->DownloadMipMap( tmu, relativeTMUAddress(tmu, startAddress), evenOdd, info );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadMipMapLevel( %d, %lu, %d, %d, %d, %d, %d, %lu )\n",
        tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    static GrTexInfo info;
#ifndef GLIDE3
    info.smallLod       = thisLod;
    info.largeLod       = largeLod;
    info.aspectRatio    = aspectRatio;
#else
    info.smallLodLog2   = thisLod;
    info.largeLodLog2   = largeLod;
    info.aspectRatioLog2= aspectRatio;
#endif
    info.format         = format;
    info.data           = data;

		if(!(startAddress >= grTexMinAddress(tmu) && startAddress <= grTexMaxAddress(tmu)))
		{
    	GlideMsg("%s, TMU: %d: WRONG memory address (%x)!\n", __FUNCTION__, tmu, startAddress);
    	LeaveGLThread();
			return;
		}

    Textures->DownloadMipMap( tmu, relativeTMUAddress(tmu, startAddress), evenOdd, &info );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadMipMapLevelPartial( %d, %lu, %d, %d, %d, %d, %d, ---, %d, %d )\n",
        tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, start, end );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    GrTexInfo info;
#ifndef GLIDE3
    info.smallLod    = thisLod;
    info.largeLod    = largeLod;
    info.aspectRatio = aspectRatio;
#else
    info.smallLodLog2 = thisLod;
    info.largeLodLog2 = largeLod;
    info.aspectRatioLog2 = aspectRatio;
#endif
    info.format      = format;
    info.data        = data;

		if(!(startAddress >= grTexMinAddress(tmu) && startAddress <= grTexMaxAddress(tmu)))
		{
    	GlideMsg("%s, TMU: %d: WRONG memory address (%x)!\n", __FUNCTION__, tmu, startAddress);
    	LeaveGLThread();
			return;
		}
		
    Textures->DownloadMipMapPartial( tmu, relativeTMUAddress(tmu, startAddress), evenOdd, &info, start, end );

    LeaveGLThread();
}

//*************************************************
//* Set the texture map clamping/wrapping mode
//*************************************************
FX_ENTRY void FX_CALL
grTexClampMode( GrChipID_t tmu,
                GrTextureClampMode_t s_clampmode,
                GrTextureClampMode_t t_clampmode )
{
#ifdef OGL_DONE
    GlideMsg( "grTexClampMode( %d, %d, %d )\n",
        tmu, s_clampmode, t_clampmode );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.tmu[tmu].SClampMode = s_clampmode;
    Glide.State.tmu[tmu].TClampMode = t_clampmode;

    switch ( s_clampmode )
    {
    case GR_TEXTURECLAMP_CLAMP:       OpenGL.tmu[tmu].SClampMode = GL_CLAMP_TO_EDGE;   break;
    case GR_TEXTURECLAMP_WRAP:        OpenGL.tmu[tmu].SClampMode = GL_REPEAT;  break;
    case GR_TEXTURECLAMP_MIRROR_EXT:  OpenGL.tmu[tmu].SClampMode = GL_MIRRORED_REPEAT; break;
    }
    switch ( t_clampmode )
    {
    case GR_TEXTURECLAMP_CLAMP:       OpenGL.tmu[tmu].TClampMode = GL_CLAMP_TO_EDGE;   break;
    case GR_TEXTURECLAMP_WRAP:        OpenGL.tmu[tmu].TClampMode = GL_REPEAT;  break;
    case GR_TEXTURECLAMP_MIRROR_EXT:  OpenGL.tmu[tmu].TClampMode = GL_MIRRORED_REPEAT; break;
    }

#ifdef OPENGL_DEBUG
    GLErro( "grTexClampMode" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Set the texture Min/Mag Filter
//*************************************************
FX_ENTRY void FX_CALL
grTexFilterMode( GrChipID_t tmu,
                 GrTextureFilterMode_t minfilter_mode,
                 GrTextureFilterMode_t magfilter_mode )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexFilterMode( %d, %d, %d )\n",
        tmu, minfilter_mode, magfilter_mode );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    RenderDrawTriangles( );

    Glide.State.tmu[tmu].MinFilterMode = minfilter_mode;
    Glide.State.tmu[tmu].MagFilterMode = magfilter_mode;

    switch ( minfilter_mode )
    {
    case GR_TEXTUREFILTER_POINT_SAMPLED:
        if ( ( Glide.State.tmu[tmu].MipMapMode != GR_MIPMAP_DISABLE ) && 
             ( InternalConfig.EnableMipMaps ) )
        {
            if ( Glide.State.tmu[tmu].LodBlend )
            {
                OpenGL.tmu[tmu].MinFilterMode = GL_NEAREST_MIPMAP_LINEAR;
            }
            else
            {
                OpenGL.tmu[tmu].MinFilterMode = GL_NEAREST_MIPMAP_NEAREST;
            }
        }
        else
        {
            OpenGL.tmu[tmu].MinFilterMode = GL_NEAREST;
        }
        break;

    case GR_TEXTUREFILTER_BILINEAR:
        if ( InternalConfig.EnableMipMaps )
        {
            if ( Glide.State.tmu[tmu].LodBlend )
            {
                OpenGL.tmu[tmu].MinFilterMode = GL_LINEAR_MIPMAP_LINEAR;
            }
            else
            {
                OpenGL.tmu[tmu].MinFilterMode = GL_LINEAR_MIPMAP_NEAREST;
            }
        }
        else
        {
            OpenGL.tmu[tmu].MinFilterMode = GL_LINEAR;
        }
        break;
    }
    switch ( magfilter_mode )
    {
    case GR_TEXTUREFILTER_POINT_SAMPLED:    OpenGL.tmu[tmu].MagFilterMode = GL_NEAREST;      break;
    case GR_TEXTUREFILTER_BILINEAR:         OpenGL.tmu[tmu].MagFilterMode = GL_LINEAR;       break;
    }

#ifdef OPENGL_DEBUG
    GLErro( "grTexFilterMode" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Set the texture MipMap Mode
//*************************************************
FX_ENTRY void FX_CALL
grTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grTexMipMapMode( %d, %d, %d )\n",
        tmu, mode, lodBlend );
#endif
    if(tmu >= InternalConfig.NumTMU) return;
    EnterGLThread();

    Glide.State.tmu[tmu].MipMapMode = mode;
    Glide.State.tmu[tmu].LodBlend = lodBlend;

    grTexFilterMode( tmu,
                Glide.State.tmu[tmu].MinFilterMode,
                Glide.State.tmu[tmu].MagFilterMode );

#ifdef OPENGL_DEBUG
    GLErro( "grTexMipMapMode" );
#endif
    LeaveGLThread();
}

//*************************************************
//* Returns the memory occupied by a texture
//*************************************************
FX_ENTRY FxU32 FX_CALL
grTexCalcMemRequired( GrLOD_t lodmin, GrLOD_t lodmax,
                      GrAspectRatio_t aspect, GrTextureFormat_t fmt )
{
#ifdef OGL_DONE
    GlideMsg( "grTexCalcMemRequired( %d, %d, %d, %d )\n",
        lodmin, lodmax, aspect, fmt );
#endif
    EnterGLThread();

    static GrTexInfo texInfo;
#ifndef GLIDE3
    texInfo.aspectRatio = aspect;
    texInfo.largeLod    = lodmax;
    texInfo.smallLod    = lodmin;
#else
    texInfo.aspectRatioLog2 = aspect;
    texInfo.largeLodLog2    = lodmax;
    texInfo.smallLodLog2    = lodmin;
#endif
    texInfo.format      = fmt;

    FxU32 s = Textures->TextureMemRequired( 0, &texInfo );

    LeaveGLThread();
    
    return s;
}

//*************************************************
//* Download a subset of an NCC table or color palette
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadTablePartial(
#ifndef GLIDE3
                           GrChipID_t   tmu,
#endif
                           GrTexTable_t type, 
                           void        *data,
                           int          start,
                           int          end )
{
#ifndef GLIDE3
# ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTablePartial( %d, %d, ---, %d, %d )\n",
        tmu, type, start, end );
# endif


    if(tmu >= InternalConfig.NumTMU) return;
    
#else
# ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTablePartial(%d, ---, %d, %d )\n", type, start, end );
# endif
#endif
    EnterGLThread();
    
    RenderDrawTriangles( );
    // FIXME: tmu
    //Textures->DownloadTable( type, (FxU32*)data, start, end + 1 - start );
    Textures->DownloadTable( type, ((FxU32*)data) + start, start, end + 1 - start );

    LeaveGLThread();
}

//*************************************************
//* download an NCC table or color palette
//*************************************************
FX_ENTRY void FX_CALL
grTexDownloadTable(
#ifndef GLIDE3
                    GrChipID_t   tmu,
#endif
                    GrTexTable_t type, 
                    void         *data )
{
#ifndef GLIDE3
# ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTable( %d, %d, --- )\n", tmu, type );
# endif

	if(tmu >= InternalConfig.NumTMU) return;

#else
# ifdef OGL_PARTDONE
    GlideMsg( "grTexDownloadTable( %d, --- )\n", type );
# endif
#endif
    EnterGLThread();
    // FIXME: tmu

    RenderDrawTriangles( );

    Textures->DownloadTable( type, (FxU32*)data, 0, 256 );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL
grTexLodBiasValue( GrChipID_t tmu, float bias )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexLodBiasValue( %d, %d )\n",
        tmu, bias );
#endif
   if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

    RenderDrawTriangles();

    if ( InternalConfig.EXT_texture_lod_bias )
    {
    	  p_glActiveTextureARB( OGLUnit(tmu) );
        DGL(glTexEnvf)( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias );
    }

    LeaveGLThread();
}

#define CASE_STR(_c) case _c: return #_c;

static const char *str_GRFunction(GrCombineFunction_t f)
{
	switch(f)
	{
		CASE_STR(GR_COMBINE_FUNCTION_ZERO)
		CASE_STR(GR_COMBINE_FUNCTION_LOCAL)
		CASE_STR(GR_COMBINE_FUNCTION_LOCAL_ALPHA)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL)
		CASE_STR(GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA)
	}
	return "Uncased function";
}

static const char *str_GRFactor(GrCombineFactor_t f)
{
	switch(f)
	{
		CASE_STR(GR_COMBINE_FACTOR_ZERO)
		CASE_STR(GR_COMBINE_FACTOR_LOCAL)
		CASE_STR(GR_COMBINE_FACTOR_OTHER_ALPHA)
		CASE_STR(GR_COMBINE_FACTOR_LOCAL_ALPHA)
		CASE_STR(GR_COMBINE_FACTOR_TEXTURE_ALPHA)
//		CASE_STR(GR_COMBINE_FACTOR_TEXTURE_RGB)
		CASE_STR(GR_COMBINE_FACTOR_LOD_FRACTION)
		CASE_STR(GR_COMBINE_FACTOR_ONE)
		CASE_STR(GR_COMBINE_FACTOR_ONE_MINUS_LOCAL)
		CASE_STR(GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA)
		CASE_STR(GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA)
		CASE_STR(GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA)
		CASE_STR(GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION)
	}
	return "Uncased factor";
}

static const char *str_GL(GLenum e)
{
	switch(e)
	{
		CASE_STR(GL_REPLACE)
		CASE_STR(GL_ZERO)
		CASE_STR(GL_ONE)
		CASE_STR(GL_COLOR)
		CASE_STR(GL_CONSTANT)
		CASE_STR(GL_TEXTURE)
		CASE_STR(GL_MODULATE)
		CASE_STR(GL_ADD)
		CASE_STR(GL_DECAL)
		CASE_STR(GL_PREVIOUS)
		CASE_STR(GL_SRC_ALPHA)
		CASE_STR(GL_SRC_COLOR)
		CASE_STR(GL_ONE_MINUS_SRC_COLOR)
		CASE_STR(GL_ONE_MINUS_SRC_ALPHA)
		CASE_STR(GL_COMBINE)
		CASE_STR(GL_COMBINE4_NV)
	}
	return "Uncased GLenum";
}

#undef CASE_STR

static void GLCheck_(const char *file, int line)
{
	GLenum err;
	while((err = DGL(glGetError)()) != GL_NO_ERROR)
	{
		GlideMsg("glGetError() = %d on %s:%d\n", err, file, line);
	}
}

#define GLCheck() GLCheck_(__FILE__, __LINE__);

/* NOTE: using this - https://registry.khronos.org/OpenGL/extensions/NV/NV_texture_env_combine4.txt */
#include "OGLCombine.h"

//*************************************************
FX_ENTRY void FX_CALL
grTexCombine( GrChipID_t tmu,
              GrCombineFunction_t rgb_function,
              GrCombineFactor_t rgb_factor, 
              GrCombineFunction_t alpha_function,
              GrCombineFactor_t alpha_factor,
              FxBool rgb_invert,
              FxBool alpha_invert )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "grTexCombine( %d, %d, %d, %d, %d, %d, %d )\n",
        tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, 
        rgb_invert, alpha_invert );
#endif

		if(tmu >= InternalConfig.NumTMU) return;

    RenderDrawTriangles( );

    Glide.State.tmu[tmu].TextureCombineCFunction = rgb_function;
    Glide.State.tmu[tmu].TextureCombineCFactor   = rgb_factor;
    Glide.State.tmu[tmu].TextureCombineAFunction = alpha_function;
    Glide.State.tmu[tmu].TextureCombineAFactor   = alpha_factor;
    Glide.State.tmu[tmu].TextureCombineRGBInvert = rgb_invert;
    Glide.State.tmu[tmu].TextureCombineAInvert   = alpha_invert;

    if ( ( rgb_function != GR_COMBINE_FUNCTION_ZERO ) ||
         ( alpha_function != GR_COMBINE_FUNCTION_ZERO ) )
    {
        OpenGL.Texture[tmu] = true;
    }
    else
    {
        OpenGL.Texture[tmu] = false;
    }
    
     GlideMsg("grTexCombine( %d, %s, %s, %s, %s, %d, %d )\n",
        tmu,
        str_GRFunction(rgb_function), str_GRFactor(rgb_factor),
        str_GRFunction(alpha_function), str_GRFactor(alpha_factor),
        rgb_invert, alpha_invert);

    OGLTMUCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor);
    //OGLCombine(tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ONE, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ONE);
    /*
    p_glActiveTextureARB(GL_TEXTURE2);
    GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE); GLCheck();

		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_REPLACE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE0_RGB,      GL_TEXTURE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE1_RGB,      GL_TEXTURE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND0_RGB,     GL_SRC_COLOR); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND1_RGB,     GL_SRC_COLOR); GLCheck();

		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_REPLACE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,    GL_TEXTURE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,    GL_TEXTURE); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA,   GL_SRC_ALPHA); GLCheck();
		DGL(glTexEnvi)(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA,   GL_SRC_ALPHA); GLCheck();
*/
}

//*************************************************
FX_ENTRY void FX_CALL
grTexNCCTable(
#ifndef GLIDE3
    GrChipID_t tmu,
#endif
    GrNCCTable_t NCCTable )
{
#ifdef OGL_DONE
    GlideMsg( "grTexNCCTable( -, %u )\n", NCCTable );
#endif

#ifndef GLIDE3
    if(tmu >= InternalConfig.NumTMU) return;
    // FIXME: TMU
#endif

    EnterGLThread();

	  Textures->NCCTable( NCCTable );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL 
grTexDetailControl( GrChipID_t tmu,
                    int lod_bias,
                    FxU8 detail_scale,
                    float detail_max )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexDetailControl( %d, %d, %d, %-4.2f )\n",
        tmu, lod_bias, detail_scale, detail_max );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL 
grTexMultibase( GrChipID_t tmu,
                FxBool     enable )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexMultibase( %d, %d )\n", tmu, enable );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grTexMultibaseAddress( %d, %d, %lu, %lu, --- )\n",
        tmu, range, startAddress, evenOdd );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grTexCombineFunction( GrChipID_t tmu, GrTextureCombineFnc_t func )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "grTexCombineFunction( %d, %d )\n", tmu, func );
#endif
    if(tmu >= InternalConfig.NumTMU) return;

    EnterGLThread();

	switch ( func )
    {
    case GR_TEXTURECOMBINE_ZERO:            // 0x00 per component
        grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_DECAL:           // Clocal decal texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_OTHER:           // Cother pass through
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_ADD:             // Cother + Clocal additive texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_MULTIPLY:        // Cother * Clocal modulated texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_SUBTRACT:        // Cother – Clocal subtractive texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_ONE:             // 255 0xFF per component
        grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXTRUE, FXTRUE );
        break;

    default:
        GlideMsg( "grTexCombineFunction: not handled func=%d, tmu=%d\n", func, tmu);
        break;

//  case GR_TEXTURECOMBINE_DETAIL:          // blend (Cother, Clocal) detail textures with detail on selected TMU
//  case GR_TEXTURECOMBINE_DETAIL_OTHER:    // blend (Cother, Clocal) detail textures with detail on neighboring TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_ODD:   // blend (Cother, Clocal) LOD blended textures with odd levels on selected TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_EVEN:  // blend (Cother, Clocal) LOD blended textures with even levels on selected TMU
//      break;
    }

    LeaveGLThread();
}

//*************************************************
//* Return the amount of unallocated texture memory on a Texture Mapping Unit
//*************************************************
FX_ENTRY FxU32 FX_CALL 
guTexMemQueryAvail( GrChipID_t tmu )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guTexMemQueryAvail( %d ) = %u\n", tmu, UTextures.MemQueryAvail( tmu ) );
#endif
   if(tmu >= InternalConfig.NumTMU) return 0;

    return UTextures.MemQueryAvail( tmu );
}

//*************************************************
FX_ENTRY void FX_CALL
guTexCombineFunction( GrChipID_t tmu, GrTextureCombineFnc_t func )
{
#if defined( OGL_PARTDONE ) || defined( OGL_COMBINE )
    GlideMsg( "guTexCombineFunction( %d, %d )\n", tmu, func );
#endif
    EnterGLThread();

    if(tmu >= InternalConfig.NumTMU) return;

    switch ( func )
    {
    case GR_TEXTURECOMBINE_ZERO:            // 0x00 per component
        grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_DECAL:           // Clocal decal texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_OTHER:           // Cother pass through
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_ADD:             // Cother + Clocal additive texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_MULTIPLY:        // Cother * Clocal modulated texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_SUBTRACT:        // Cother – Clocal subtractive texture
        grTexCombine( tmu, GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, GR_COMBINE_FACTOR_ONE, FXFALSE, FXFALSE );
        break;

    case GR_TEXTURECOMBINE_ONE:             // 255 0xFF per component
        grTexCombine( tmu, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO,
            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_ZERO, FXTRUE, FXTRUE );
        break;

    default:
        GlideMsg( "guTexCombineFunction: not handled func=%d, tmu=%d\n", func, tmu);
        break;

//  case GR_TEXTURECOMBINE_DETAIL:          // blend (Cother, Clocal) detail textures with detail on selected TMU
//  case GR_TEXTURECOMBINE_DETAIL_OTHER:    // blend (Cother, Clocal) detail textures with detail on neighboring TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_ODD:   // blend (Cother, Clocal) LOD blended textures with odd levels on selected TMU
//  case GR_TEXTURECOMBINE_TRILINEAR_EVEN:  // blend (Cother, Clocal) LOD blended textures with even levels on selected TMU
//      break;
    }

    LeaveGLThread();
}

//*************************************************
FX_ENTRY GrMipMapId_t FX_CALL 
guTexGetCurrentMipMap( GrChipID_t tmu )
{
#ifdef OGL_DONE
    GlideMsg( "guTexGetCurrentMipMap( %d ) = %d\n", tmu, UTextures.GetCurrentMipMap( tmu ) );
#endif
    EnterGLThread();

		if(tmu >= InternalConfig.NumTMU) GR_NULL_MIPMAP_HANDLE;

    GrMipMapId_t id = UTextures.GetCurrentMipMap( tmu );
    
    LeaveGLThread();
    
    return id;
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
guTexChangeAttributes( GrMipMapId_t mmid,
                       int width, int height,
                       GrTextureFormat_t fmt,
                       GrMipMapMode_t mm_mode,
                       GrLOD_t smallest_lod, GrLOD_t largest_lod,
                       GrAspectRatio_t aspect,
                       GrTextureClampMode_t s_clamp_mode,
                       GrTextureClampMode_t t_clamp_mode,
                       GrTextureFilterMode_t minFilterMode,
                       GrTextureFilterMode_t magFilterMode )
{
#ifdef OGL_DONE
    GlideMsg( "guTexChangeAttributes( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )\n",
        mmid, width, height, fmt, mm_mode, smallest_lod, largest_lod, aspect,
        s_clamp_mode, t_clamp_mode, minFilterMode, magFilterMode );
#endif
    EnterGLThread();
    
    FxBool b = UTextures.ChangeAttributes( mmid, width, height, fmt, mm_mode,
        smallest_lod, largest_lod, aspect, s_clamp_mode, t_clamp_mode, 
        minFilterMode, magFilterMode );
        
    LeaveGLThread();
    
    return b;
}

//*************************************************
#ifndef GLIDE3
FX_ENTRY GrMipMapInfo * FX_CALL 
guTexGetMipMapInfo( GrMipMapId_t mmid )
{
#ifdef OGL_DONE
    GlideMsg( "guTexGetMipMapInfo( ) = 0x%p\n" );
#endif
    EnterGLThread();
    
    GrMipMapInfo *info = UTextures.GetMipMapInfo( mmid );
    
    LeaveGLThread();
    
    return info;
}
#endif
//*************************************************
FX_ENTRY void FX_CALL 
guTexMemReset( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "guTexMemReset( )\n" );
#endif
    EnterGLThread();

    UTextures.MemReset( );
    Textures->Clear( );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexDownloadMipMapLevel( GrMipMapId_t mmid, GrLOD_t lod, const void **src )
{
#ifdef OGL_DONE
    GlideMsg( "guTexDownloadMipMapLevel( %d, %d, 0x%p )\n", mmid, lod, src );
#endif
    EnterGLThread();

    UTextures.DownloadMipMapLevel( mmid, lod, src );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexDownloadMipMap( GrMipMapId_t mmid, const void *src, const GuNccTable *table )
{
#ifdef OGL_DONE
    GlideMsg( "guTexDownloadMipMap( %d, 0x%p, 0x%p )\n", mmid, src, table );
#endif
    EnterGLThread();

    UTextures.DownloadMipMap( mmid, src, table );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY GrMipMapId_t FX_CALL 
guTexAllocateMemory( GrChipID_t tmu,
                     FxU8 odd_even_mask,
                     int width, int height,
                     GrTextureFormat_t fmt,
                     GrMipMapMode_t mm_mode,
                     GrLOD_t smallest_lod, GrLOD_t largest_lod,
                     GrAspectRatio_t aspect,
                     GrTextureClampMode_t s_clamp_mode,
                     GrTextureClampMode_t t_clamp_mode,
                     GrTextureFilterMode_t minfilter_mode,
                     GrTextureFilterMode_t magfilter_mode,
                     float lod_bias,
                     FxBool trilinear )
{
#ifdef OGL_DONE
    GlideMsg( "guTexAllocateMemory( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )\n",
        tmu, odd_even_mask, width, height, fmt, mm_mode, smallest_lod, largest_lod, aspect,
        s_clamp_mode, t_clamp_mode, minfilter_mode, magfilter_mode, lod_bias, trilinear );
#endif
    if(tmu >= InternalConfig.NumTMU) return GR_NULL_MIPMAP_HANDLE;

    EnterGLThread();

    GrMipMapId_t id = UTextures.AllocateMemory( tmu, odd_even_mask, width, height, fmt, mm_mode,
        smallest_lod, largest_lod, aspect, s_clamp_mode, t_clamp_mode,
        minfilter_mode, magfilter_mode, lod_bias, trilinear );

    LeaveGLThread();
    
   	return id;
}

//*************************************************
FX_ENTRY void FX_CALL 
guTexSource( GrMipMapId_t id )
{
#ifdef OGL_DONE
    GlideMsg( "guTexSource( %d )\n", id );
#endif
    EnterGLThread();

    RenderDrawTriangles( );

    UTextures.Source( id );

    LeaveGLThread();
}

//*************************************************
FX_ENTRY FxU16 * FX_CALL
guTexCreateColorMipMap( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "guTexCreateColorMipMap( ) = NULL\n" );
#endif

    return NULL;
}

