//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                PGTexture Class Definition
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#ifndef __PGTEXTURE_H__
#define __PGTEXTURE_H__

#include "sdk2_glide.h"
#include "TexDB.h"

class PGTexture  
{
    struct TexValues
    {
        GrLOD_t lod;
        FxU32 width;
        FxU32 height;
        FxU32 nPixels;
    };

public:
	void NCCTable( GrNCCTable_t tab );

    static FxU32 LodOffset( FxU32 evenOdd, GrTexInfo *info );
    static FxU32 MipMapMemRequired( GrLOD_t lod, GrAspectRatio_t aspectRatio, 
                                    GrTextureFormat_t format );
    void ChromakeyMode( GrChromakeyMode_t mode );
    void ChromakeyValue( GrColor_t value );
    bool GetAspect( int tmu, float *hAspect, float *wAspect );
    void Clear( void );
    static FxU32 TextureMemRequired( FxU32 evenOdd, GrTexInfo *info );
    bool MakeReady( int tmu );
    void DownloadTable( GrTexTable_t type, FxU32 *data, int first, int count );
    FxBool Source(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
    void Invalidate( GrChipID_t tmu);
    void DownloadMipMap(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
    void DownloadMipMapPartial(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info, int start, int end );
    FxU32 GetMemorySize( void );

    PGTexture( int tmus, int mem_size_per_tmu );
    virtual ~PGTexture();

#ifdef OGL_DEBUG
    int Num_565_Tex;
    int Num_1555_Tex;
    int Num_4444_Tex;
    int Num_332_Tex;
    int Num_8332_Tex;
    int Num_Alpha_Tex;
    int Num_AlphaIntensity88_Tex;
    int Num_AlphaIntensity44_Tex;
    int Num_AlphaPalette_Tex;
    int Num_Palette_Tex;
    int Num_Intensity_Tex;
    int Num_YIQ_Tex;
    int Num_AYIQ_Tex;
    int Num_Other_Tex;
#endif

private:
    void ApplyKeyToPalette( void );
    void GetTexValues(int tmu, TexValues *tval );

    FxU32           m_tex_memory_size;
    bool            m_palette_dirty;
    FxU32           m_palette_hash;
    GrChromakeyMode_t m_chromakey_mode;
    FxU32           m_chromakey_value_8888;
    FxU16           m_chromakey_value_565;

    struct TexSingleTMU
    {
      TexDB *         db;
      float           wAspect;
      float           hAspect;
      bool            valid;
      FxU8 *          memory;
      FxU32           startAddress;
      FxU32           evenOdd;
      GrTexInfo       info;
    };
    TexSingleTMU m_tmu[GLIDE_NUM_TMU];
    int m_tmu_cnt;

    FxU32           m_palette[ 256 ];
    GrNCCTable_t    m_ncc_select;
    GuNccTable      m_ncc[2];
};

extern PGTexture *Textures;

#endif
