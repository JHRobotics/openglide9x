//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*           implementation of the PGUexture class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************


#include "GlOgl.h"
#include "PGTexture.h"
#include "PGUTexture.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PGUTexture::PGUTexture( void )
{
    MemReset();
}

PGUTexture::~PGUTexture( void )
{
}

GrMipMapId_t PGUTexture::AllocateMemory( GrChipID_t tmu, FxU8 odd_even_mask, 
                                         int width, int height,
                                         GrTextureFormat_t fmt, GrMipMapMode_t mm_mode,
                                         GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                                         GrAspectRatio_t aspect,
                                         GrTextureClampMode_t s_clamp_mode, 
                                         GrTextureClampMode_t t_clamp_mode,
                                         GrTextureFilterMode_t minfilter_mode, 
                                         GrTextureFilterMode_t magfilter_mode,
                                         float lod_bias, FxBool trilinear )
{
    FxU32   size = 0;
    GrLOD_t lod;

#ifndef GLIDE3
    for ( lod = largest_lod; lod <= smallest_lod; lod++ )
#else
    for ( lod = smallest_lod; lod <= largest_lod; lod++ )
#endif
    {
        size += PGTexture::MipMapMemRequired( lod, aspect, fmt );
    }

    size = ((size + 7) & ~7);

#ifdef OGL_UTEX
    GlideMsg( "Allocate id = %d size = %d\n", m_free_id, size );
#endif

    if ( ( m_free_id >= MAX_MM ) || 
         ( ( m_free_mem[ tmu ] + size ) >= Textures->GetMemorySize( ) ) )
    {
#ifdef OGL_UTEX
        GlideMsg("Allocation failed\n");
#endif
        return GR_NULL_MIPMAP_HANDLE;
    }

    mm_info[ m_free_id ].odd_even_mask  = odd_even_mask;
    mm_info[ m_free_id ].width          = width;
    mm_info[ m_free_id ].height         = height;
    mm_info[ m_free_id ].format         = fmt;
    mm_info[ m_free_id ].mipmap_mode    = mm_mode;
#ifndef GLIDE3
    mm_info[ m_free_id ].lod_min        = smallest_lod;
    mm_info[ m_free_id ].lod_max        = largest_lod;
    mm_info[ m_free_id ].aspect_ratio   = aspect;
#else
    mm_info[ m_free_id ].lod_min_log2   = smallest_lod;
    mm_info[ m_free_id ].lod_max_log2   = largest_lod;
    mm_info[ m_free_id ].aspect_ratio_log2 = aspect;
#endif
    mm_info[ m_free_id ].s_clamp_mode   = s_clamp_mode;
    mm_info[ m_free_id ].t_clamp_mode   = t_clamp_mode;
    mm_info[ m_free_id ].minfilter_mode = minfilter_mode;
    mm_info[ m_free_id ].magfilter_mode = magfilter_mode;
    //mm_info[ m_free_id ].lod_bias       = lod_bias;
    mm_info[ m_free_id ].trilinear      = trilinear;
    mm_info[ m_free_id ].valid          = FXTRUE;

    mm_start[ tmu ][ m_free_id ] = m_free_mem[ tmu ];

    m_free_mem[ tmu ] += size;
    
    mm_tmu[ m_free_id ] = tmu;

    return m_free_id++;
}

FxU32 PGUTexture::AbsTMUAddr(GrChipID_t tmu, GrMipMapId_t mmid)
{
	return mm_start[ tmu ][ mmid ] + grTexMinAddress(tmu);
}

void PGUTexture::DownloadMipMap( GrMipMapId_t mmid, const void *src, const GuNccTable *table )
{
#ifdef OGL_UTEX
    GlideMsg("Download id = %d ", mmid);
#endif

    if ( ( mmid >=0 ) && ( mmid < MAX_MM ) && ( mm_info[ mmid ].valid ) )
    {
        GrTexInfo info;

        info.format      = mm_info[ mmid ].format;
#ifndef GLIDE3
        info.aspectRatio = mm_info[ mmid ].aspect_ratio;
        info.largeLod    = mm_info[ mmid ].lod_max;
        info.smallLod    = mm_info[ mmid ].lod_min;
#else
        info.aspectRatioLog2 = mm_info[ mmid ].aspect_ratio_log2;
        info.largeLodLog2    = mm_info[ mmid ].lod_max_log2;
        info.smallLodLog2    = mm_info[ mmid ].lod_min_log2;
#endif
        info.data        = (void *)src;

#ifdef OGL_UTEX
        {
            FxU32 size = 0;

#ifndef GLIDE3
            for ( GrLOD_t lod = info.largeLod; lod <= info.smallLod; lod++ )
            {
                size += PGTexture::MipMapMemRequired( lod, info.aspectRatio, info.format );
            }
#else
            for ( GrLOD_t lod = info.smallLodLog2; lod <= info.largeLodLog2; lod++ )
            {
                size += PGTexture::MipMapMemRequired( lod, info.aspectRatioLog2, info.format );
            }
#endif
            GlideMsg( "size = %d\n", size );
        }
#endif

        GrChipID_t tmu = mm_tmu[ mmid ];
        grTexDownloadMipMap(tmu, AbsTMUAddr(tmu, mmid), mm_info[ mmid ].odd_even_mask, &info );
    }
#ifdef OGL_UTEX
    else
    {
        GlideMsg( "failed\n" );
    }
#endif
}

void PGUTexture::DownloadMipMapLevel( GrMipMapId_t mmid, GrLOD_t lod, const void **src )
{
    if ( ( mmid >=0 ) && ( mmid < MAX_MM ) && ( mm_info[ mmid ].valid ) )
    {
        GrTexInfo info;
        FxU32 size;
        GrChipID_t tmu = mm_tmu[ mmid ];


        info.format      = mm_info[ mmid ].format;
#ifndef GLIDE3
        info.aspectRatio = mm_info[ mmid ].aspect_ratio;
        info.largeLod    = mm_info[ mmid ].lod_max;
        info.smallLod    = mm_info[ mmid ].lod_min;
        
        size = PGTexture::MipMapMemRequired( lod, info.aspectRatio, info.format );

        grTexDownloadMipMapLevel( tmu, AbsTMUAddr(tmu, mmid), lod, lod, 
                mm_info[ mmid ].aspect_ratio,
                mm_info[ mmid ].format, 
                mm_info[ mmid ].odd_even_mask, (void *)*src );
#else
        info.aspectRatioLog2 = mm_info[ mmid ].aspect_ratio_log2;
        info.largeLodLog2    = mm_info[ mmid ].lod_max_log2;
        info.smallLodLog2    = mm_info[ mmid ].lod_min_log2;
        
        size = PGTexture::MipMapMemRequired( lod, info.aspectRatioLog2, info.format );

        grTexDownloadMipMapLevel( tmu, AbsTMUAddr(tmu, mmid), lod, lod, 
                mm_info[ mmid ].aspect_ratio_log2,
                mm_info[ mmid ].format, 
                mm_info[ mmid ].odd_even_mask, (void *)*src );
#endif


        *src = (void *)(((intptr_t)*src) + size);
    }
}

void PGUTexture::MemReset( void )
{
#ifdef OGL_UTEX
    GlideMsg("Reset\n");
#endif
    for ( int i = 0; i < MAX_MM; i++ )
    {
        mm_info[ i ].valid = false;
        mm_tmu[ i ] = GR_TMU0;
    }

    for(int tmu = 0; tmu < GLIDE_NUM_TMU; tmu++)
    {
        m_free_mem[tmu] = 0;
        m_current_id[tmu] = GR_NULL_MIPMAP_HANDLE;
    }
    m_free_id = 0;
}

void PGUTexture::Source( GrMipMapId_t id )
{
    if ( ( id >=0 ) && ( id < MAX_MM ) && ( mm_info[ id ].valid ) )
    {
        GrTexInfo info;
        GrChipID_t tmu = mm_tmu[ id ];

        info.format      = mm_info[ id ].format;
#ifndef GLIDE3
        info.aspectRatio = mm_info[ id ].aspect_ratio;
        info.largeLod    = mm_info[ id ].lod_max;
        info.smallLod    = mm_info[ id ].lod_min;
#else
        info.aspectRatioLog2 = mm_info[ id ].aspect_ratio_log2;
        info.largeLodLog2    = mm_info[ id ].lod_max_log2;
        info.smallLodLog2    = mm_info[ id ].lod_min_log2;
#endif
        grTexSource(     tmu, AbsTMUAddr(tmu, id), mm_info[ id ].odd_even_mask, &info );
        grTexFilterMode( tmu, mm_info[ id ].minfilter_mode, mm_info[ id ].magfilter_mode );
        grTexMipMapMode( tmu, mm_info[ id ].mipmap_mode, mm_info[ id ].trilinear );
        grTexClampMode(  tmu, mm_info[ id ].s_clamp_mode, mm_info[ id ].t_clamp_mode );
        //grTexLodBiasValue( 0, mm_info[ id ].lod_bias );

        m_current_id[ tmu ] = id;
    }
#ifdef OGL_UTEX
    else
    {
        GlideMsg( "TexSourcefailed\n" );
    }
#endif
}

#ifndef GLIDE3
GrMipMapInfo *PGUTexture::GetMipMapInfo( GrMipMapId_t mmid )
{
    return ( ( mmid >= 0 ) && ( mmid < MAX_MM ) && 
             ( mm_info[mmid].valid ) ? &( mm_info[ mmid ] ) : NULL );
}
#endif

FxBool PGUTexture::ChangeAttributes( GrMipMapId_t mmid, int width, int height, 
                                     GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, 
                                     GrLOD_t smallest_lod, GrLOD_t largest_lod, 
                                     GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, 
                                     GrTextureClampMode_t t_clamp_mode, 
                                     GrTextureFilterMode_t minFilterMode, 
                                     GrTextureFilterMode_t magFilterMode )
{
    return FXFALSE;
}

GrMipMapId_t PGUTexture::GetCurrentMipMap( GrChipID_t tmu )
{
    return m_current_id[ tmu ];
}

FxU32 PGUTexture::MemQueryAvail( GrChipID_t tmu )
{
    return Textures->GetMemorySize( ) - m_free_mem[ tmu ];
}
