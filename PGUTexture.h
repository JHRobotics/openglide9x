//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*               PGUTexture Class Definition
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#ifndef __PGUTEXTURE_H__
#define __PGUTEXTURE_H__

#include "sdk2_glide.h"

class PGUTexture  
{
public:
    FxU32 MemQueryAvail( GrChipID_t tmu );
    GrMipMapId_t GetCurrentMipMap( GrChipID_t tmu );
    FxBool ChangeAttributes( GrMipMapId_t mmid, int width, int height, 
                             GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, 
                             GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                             GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, 
                             GrTextureClampMode_t t_clamp_mode, 
                             GrTextureFilterMode_t minFilterMode, 
                             GrTextureFilterMode_t magFilterMode );
#ifndef GLIDE3
    GrMipMapInfo * GetMipMapInfo( GrMipMapId_t mmid );
#endif
    void Source( GrMipMapId_t id );
    void MemReset( void );
    void DownloadMipMapLevel( GrMipMapId_t mmid, GrLOD_t lod, const void **src );
    void DownloadMipMap( GrMipMapId_t mmid, const void *src, const GuNccTable *table );
    GrMipMapId_t AllocateMemory( GrChipID_t tmu, FxU8 odd_even_mask, int width, int height, 
                                 GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, 
                                 GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                                 GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, 
                                 GrTextureClampMode_t t_clamp_mode, 
                                 GrTextureFilterMode_t minfilter_mode, 
                                 GrTextureFilterMode_t magfilter_mode, 
                                 float lod_bias, FxBool trilinear );

    PGUTexture( void );
    virtual ~PGUTexture( void );


private:

    enum { MAX_MM = 1024*GLIDE_NUM_TMU };

		GrChipID_t      mm_tmu [MAX_MM];
#ifndef GLIDE3

    GrMipMapInfo    mm_info[ MAX_MM ];
#else
GrMipMapInfoInternal mm_info[ MAX_MM ];
#endif

    FxU32           mm_start[ GLIDE_NUM_TMU ][ MAX_MM ];
    FxU32           m_free_mem[ GLIDE_NUM_TMU ];
    GrMipMapId_t    m_free_id;
    GrMipMapId_t    m_current_id[ GLIDE_NUM_TMU ];
    
    FxU32           AbsTMUAddr(GrChipID_t tmu, GrMipMapId_t mmid);
    
};

extern PGUTexture UTextures;

#endif
