//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*           implementation of the PGTexture class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include "GlOgl.h"
#include "PGTexture.h"
#include "Glextensions.h"
#include "FormatConversion.h"
#include "OGLTables.h"

#define TEX_TEMP_PIXELS (256 * 256 * 2)

static FxU32 m_tex_temp[ TEX_TEMP_PIXELS ];

#define OGL_LOAD_CREATE_TEXTURE( compnum, compformat, comptype, texdata )   \
    {                                                                       \
        DGL(glTexImage2D)( GL_TEXTURE_2D, texVals.lod, compnum,                  \
                    texVals.width, texVals.height,                          \
                    0, compformat, comptype, texdata );                     \
        if ( InternalConfig.BuildMipMaps )                                  \
        {                                                                   \
            /*gluBuild2DMipmaps( GL_TEXTURE_2D, compnum, texVals.width, texVals.height, compformat, comptype, texdata );*/ \
        }                                                                   \
    }


void genPaletteMipmaps( FxU32 width, FxU32 height, FxU8 *data )
{
    FxU8    buf[ 128 * 128 ];
    FxU32   mmwidth;
    FxU32   mmheight;
    FxU32   lod;
    FxU32   skip;

    mmwidth = width;
    mmheight = height;
    lod = 0;
    skip = 1;

    while ( ( mmwidth > 1 ) || ( mmheight > 1 ) )
    {
        FxU32   x, 
                y;

        mmwidth = mmwidth > 1 ? mmwidth / 2 : 1;
        mmheight = mmheight > 1 ? mmheight / 2 : 1;
        lod += 1;
        skip *= 2;

        for ( y = 0; y < mmheight; y++ )
        {
            FxU8    * in, 
                    * out;

            in = data + width * y * skip;
            out = buf + mmwidth * y;
            for ( x = 0; x < mmwidth; x++ )
            {
                out[ x ] = in[ x * skip ];
            }
        }

        DGL(glTexImage2D)( GL_TEXTURE_2D, lod, GL_COLOR_INDEX8_EXT, mmwidth, mmheight, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buf );
    }
}

PGTexture::PGTexture( int tmus, int mem_size_per_tmu )
{
	m_tex_memory_size = mem_size_per_tmu;

	m_tmu_cnt = tmus;

	for(int tmu = 0; tmu < m_tmu_cnt; tmu++)
	{
		m_tmu[tmu].db = new TexDB( mem_size_per_tmu );
		m_tmu[tmu].valid = false;
		m_tmu[tmu].memory = new FxU8[ mem_size_per_tmu ];
	}
	m_chromakey_mode = GR_CHROMAKEY_DISABLE;
	m_palette_dirty = true;
	
	m_ncc_select = GR_NCCTABLE_NCC0;

#ifdef OGL_DEBUG
    Num_565_Tex = 0;
    Num_1555_Tex = 0;
    Num_4444_Tex = 0;
    Num_332_Tex = 0;
    Num_8332_Tex = 0;
    Num_Alpha_Tex = 0;
    Num_AlphaIntensity88_Tex = 0;
    Num_AlphaIntensity44_Tex = 0;
    Num_AlphaPalette_Tex = 0;
    Num_Palette_Tex = 0;
    Num_Intensity_Tex = 0;
    Num_YIQ_Tex = 0;
    Num_AYIQ_Tex = 0;
    Num_Other_Tex = 0;
#endif
}

PGTexture::~PGTexture( void )
{
	for(int tmu = 0; tmu < m_tmu_cnt; tmu++)
	{
		delete[] m_tmu[tmu].memory;
		delete m_tmu[tmu].db;
	}
}

void PGTexture::DownloadMipMap( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info )
{
#ifndef GLIDE3
    FxU32 mip_size = MipMapMemRequired( info->smallLod, 
                                        info->aspectRatio, 
                                        info->format );
#else
    FxU32 mip_size = MipMapMemRequired( info->smallLodLog2,  info->aspectRatioLog2, info->format );
#endif

    FxU32 mip_offset = startAddress + TextureMemRequired( evenOdd, info );
    
    if ( info->format == GR_TEXFMT_BGRA_8888 )
    {
        // Glidos never defines an extension texture unless it intends to use
        // it, so we can create the OpenGL texture here and not bother to
        // copy anything to the virtual glide texture memory
        GLuint  texNum;
        TexValues  texVals;

#ifndef GLIDE3
        texVals.width = texInfo[ info->aspectRatio ][ info->largeLod ].width;
        texVals.height = texInfo[ info->aspectRatio ][ info->largeLod ].height;
        texVals.nPixels = texInfo[ info->aspectRatio ][ info->largeLod ].numPixels;
#else
        texVals.width = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].width;
        texVals.height = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].height;
        texVals.nPixels = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].numPixels;
#endif
        texVals.lod = 0;

        m_tmu[tmu].db->WipeRange( startAddress, mip_offset, 0 );
        m_tmu[tmu].db->Add( startAddress, mip_offset, info, 0, &texNum, NULL);

				p_glActiveTextureARB( OGLUnit(tmu) );

        DGL(glBindTexture)( GL_TEXTURE_2D, texNum );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.tmu[tmu].SClampMode );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.tmu[tmu].TClampMode );
    
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.tmu[tmu].MinFilterMode );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.tmu[tmu].MagFilterMode );

        if ( InternalConfig.EnableMipMaps && !InternalConfig.BuildMipMaps )
            DGL(glTexParameteri)( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true );

        OGL_LOAD_CREATE_TEXTURE( GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, info->data );
    }
    else
    {
        if ( mip_offset <= m_tex_memory_size )
        {
            memcpy( m_tmu[tmu].memory + mip_offset - mip_size, info->data, mip_size );
        }
        
        // Any texture based on memory crossing this range
        // is now out of date
        m_tmu[tmu].db->WipeRange( startAddress, mip_offset, 0 );
    }

#ifdef OGL_DEBUG
# ifndef GLIDE3
    if ( info->smallLod == info->largeLod )
# else
    if ( info->smallLodLog2 == info->largeLodLog2 )
#endif
    {
        switch ( info->format )
        {
        case GR_TEXFMT_RGB_332:             Num_332_Tex++;                  break;
        case GR_TEXFMT_YIQ_422:             Num_YIQ_Tex++;                  break;
        case GR_TEXFMT_ALPHA_8:             Num_Alpha_Tex++;                break;
        case GR_TEXFMT_INTENSITY_8:         Num_Intensity_Tex++;            break;
        case GR_TEXFMT_ALPHA_INTENSITY_44:  Num_AlphaIntensity44_Tex++;     break;
        case GR_TEXFMT_P_8:                 Num_565_Tex++;                  break;
        case GR_TEXFMT_ARGB_8332:           Num_8332_Tex++;                 break;
        case GR_TEXFMT_AYIQ_8422:           Num_AYIQ_Tex++;                 break;
        case GR_TEXFMT_RGB_565:             Num_565_Tex++;                  break;
        case GR_TEXFMT_ARGB_1555:           Num_1555_Tex++;                 break;
        case GR_TEXFMT_ARGB_4444:           Num_4444_Tex++;                 break;
        case GR_TEXFMT_ALPHA_INTENSITY_88:  Num_AlphaIntensity88_Tex++;     break;
        case GR_TEXFMT_AP_88:               Num_AlphaPalette_Tex++;         break;
        case GR_TEXFMT_RSVD0:
        case GR_TEXFMT_RSVD1:
        case GR_TEXFMT_RSVD2:               Num_Other_Tex++;                break;
        }
    }
#endif
}

void PGTexture::DownloadMipMapPartial( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info, int start, int end )
{
    // Glidos has different handling of grTexDownload, so keep it
    if ( info->format == GR_TEXFMT_BGRA_8888 )
    {
        DownloadMipMap( tmu, startAddress, evenOdd, info );
        return;
    }
#ifndef GLIDE3
    FxU32 mip_size = MipMapMemRequired( info->smallLod, 
                                        info->aspectRatio, 
                                        info->format );
#else
    FxU32 mip_size = MipMapMemRequired( info->smallLodLog2, info->aspectRatioLog2, info->format );
#endif
    FxU32 mip_offset = startAddress + TextureMemRequired( evenOdd, info );
 
    if ( mip_offset <= m_tex_memory_size )
    {
#ifndef GLIDE3
        int stride = texInfo[ info->aspectRatio ][ info->smallLod ].width;
#else
				int stride = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->smallLodLog2 ].width;
#endif
        
        if (info->format >= GR_TEXFMT_16BIT)
            stride *= 2;
        FxU8 *mip_memory = m_tmu[tmu].memory + mip_offset - mip_size;
        memcpy( mip_memory + start * stride, info->data, (end - start + 1) * stride );

        m_tmu[tmu].db->WipeRange( startAddress, mip_offset, 0 );
    }
}

FxBool PGTexture::Source( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info )
{	
    m_tmu[tmu].startAddress = startAddress;
    m_tmu[tmu].evenOdd = evenOdd;
    m_tmu[tmu].info = *info;

#ifndef GLIDE3
    m_tmu[tmu].wAspect = texAspects[ info->aspectRatio ].w;
    m_tmu[tmu].hAspect = texAspects[ info->aspectRatio ].h;
    m_tmu[tmu].rAspect = texAspects[ info->aspectRatio ].r;
#else
    m_tmu[tmu].wAspect = texAspects[ 0x3 - info->aspectRatioLog2 ].w;
    m_tmu[tmu].hAspect = texAspects[ 0x3 - info->aspectRatioLog2 ].h;
    m_tmu[tmu].rAspect = texAspects[ 0x3 - info->aspectRatioLog2 ].r;
#endif

    m_tmu[tmu].valid = ( ( startAddress + TextureMemRequired( evenOdd, info ) ) <= m_tex_memory_size );

    return m_tmu[tmu].valid ? FXTRUE : FXFALSE;
}

void PGTexture::Invalidate( GrChipID_t tmu)
{
	m_tmu[tmu].valid = false;
}

void PGTexture::DownloadTable( GrTexTable_t type, FxU32 *data, int first, int count )
{
    if ( type == GR_TEXTABLE_PALETTE )
    {
        for ( int i = count - 1; i >= 0; i-- )
        {
              m_palette[ first + i ] = data[ i ]; 
        }
        
        m_palette_dirty = true;
    }
    else
    {
        // GR_TEXTABLE_NCC0 or GR_TEXTABLE_NCC1
        int         i;
        GuNccTable *ncc = &(m_ncc[ type ]);

        memcpy( ncc, data, sizeof( GuNccTable ) );

        for ( i = 0; i < 4; i++ )
        {
            if ( ncc->iRGB[ i ][ 0 ] & 0x100 )
                ncc->iRGB[ i ][ 0 ] |= 0xff00;
            if ( ncc->iRGB[ i ][ 1 ] & 0x100 )
                ncc->iRGB[ i ][ 1 ] |= 0xff00;
            if ( ncc->iRGB[ i ][ 2 ] & 0x100 )
                ncc->iRGB[ i ][ 2 ] |= 0xff00;

            if ( ncc->qRGB[ i ][ 0 ] & 0x100 )
                ncc->qRGB[ i ][ 0 ] |= 0xff00;
            if ( ncc->qRGB[ i ][ 1 ] & 0x100 )
                ncc->qRGB[ i ][ 1 ] |= 0xff00;
            if ( ncc->qRGB[ i ][ 2 ] & 0x100 )
                ncc->qRGB[ i ][ 2 ] |= 0xff00;
        }
    }
}

bool PGTexture::MakeReady( int tmu )
{
    FxU8        * data;
    TexValues   texVals;
    GLuint      texNum;
    GLuint      tex2Num;
    FxU32       size;
    FxU32       test_hash;
    FxU32       wipe_hash;
    bool        palette_changed;
    bool        * pal_change_ptr;
    bool        use_mipmap_ext;
    bool        use_mipmap_ext2;
    
    p_glActiveTextureARB(OGLUnit(tmu));

    DGL(glDisable)(GL_TEXTURE_2D);
    if(!OpenGL.Texture[tmu])
    {
    	return false;
    }

#if 0
    if(tmu == 1)
    {
    	return false;
    }
#endif

    if(!m_tmu[tmu].valid)
    {
        return false;
    }

    DGL(glEnable)(GL_TEXTURE_2D);

    test_hash        = 0;
    wipe_hash        = 0;
    palette_changed  = false;
    use_mipmap_ext   = ( InternalConfig.EnableMipMaps && !InternalConfig.BuildMipMaps );
    use_mipmap_ext2  = use_mipmap_ext;
    pal_change_ptr   = NULL;
    
    data             = m_tmu[tmu].memory + m_tmu[tmu].startAddress;

    size             = TextureMemRequired( m_tmu[tmu].evenOdd, &m_tmu[tmu].info );

    GetTexValues(tmu, &texVals );
    
    switch ( m_tmu[tmu].info.format )
    {
    case GR_TEXFMT_P_8:
        ApplyKeyToPalette( );
        if ( InternalConfig.EXT_paletted_texture )
        {
            //
            // OpenGL's mipmap generation doesn't seem
            // to handle paletted textures.
            //
            use_mipmap_ext = false;
            pal_change_ptr = &palette_changed;
        }
        else
        {
            wipe_hash = m_palette_hash;
        }

        test_hash = m_palette_hash;
        break;

    case GR_TEXFMT_AP_88:
        ApplyKeyToPalette( );
#if 0
        if ( InternalConfig.EXT_paletted_texture && InternalConfig.ARB_multitexture )
        {
            use_mipmap_ext   = false;
            pal_change_ptr   = &palette_changed;
            use_two_textures = true;
        }
        else
#endif
        {
            wipe_hash = m_palette_hash;
        }

        test_hash = m_palette_hash;
        break;
    }

    // See if we already have an OpenGL texture to match this
    if ( m_tmu[tmu].db->Find( m_tmu[tmu].startAddress, &m_tmu[tmu].info, test_hash,
                     &texNum, NULL,
                     pal_change_ptr ) )
    {
        DGL(glBindTexture)( GL_TEXTURE_2D, texNum );

        if ( palette_changed )
        {
            p_glColorTableEXT( GL_TEXTURE_2D, GL_RGBA, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_palette );
        }
    }
    else
    {
        // Any existing textures crossing this memory range
        // is unlikely to be used, so remove the OpenGL version
        // of them
        m_tmu[tmu].db->WipeRange( m_tmu[tmu].startAddress, m_tmu[tmu].startAddress + size, wipe_hash );

        // Add this new texture to the data base
        m_tmu[tmu].db->Add( m_tmu[tmu].startAddress, m_tmu[tmu].startAddress + size, &m_tmu[tmu].info, test_hash,
                   &texNum, NULL );

        DGL(glBindTexture)( GL_TEXTURE_2D, texNum );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.tmu[tmu].SClampMode );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.tmu[tmu].TClampMode );
    
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.tmu[tmu].MinFilterMode );
        DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.tmu[tmu].MagFilterMode );
    
        if( use_mipmap_ext )
        {
            DGL(glTexParameteri)( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true );
        }

        switch ( m_tmu[tmu].info.format )
        {
        case GR_TEXFMT_RGB_565:
            if ( m_chromakey_mode )
            {
                Convert565Kto8888( (FxU16*)data, m_chromakey_value_565, m_tex_temp, texVals.nPixels );
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
            }
            else if ( InternalConfig.OGLVersion > OGL_VER_1_1 && !InternalConfig.Textures32bit)
            {
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data );
            }
            else
            {
                if ( InternalConfig.Wrap565to5551 && !InternalConfig.Textures32bit)
                {
                    Convert565to5551( (FxU32*)data, m_tex_temp, texVals.nPixels );
                    OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT, m_tex_temp );
                }
                else
                {
                    Convert565to8888( (FxU16*)data, m_tex_temp, texVals.nPixels );
                    OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
                }
            }
            break;
            
        case GR_TEXFMT_ARGB_4444:
        	  if(InternalConfig.Textures32bit)
        	  {
            	Convert4444to8888( (FxU16*)data, m_tex_temp, texVals.nPixels );
            	OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
        	  }
        	  else if ( InternalConfig.OGLVersion > OGL_VER_1_1 )
            {
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_BGRA_EXT, GL_UNSIGNED_SHORT_4_4_4_4_REV, data );
            }
            else
            {
                Convert4444to4444special( (FxU32*)data, m_tex_temp, texVals.nPixels );
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_EXT, m_tex_temp );
            }

            break;
            
        case GR_TEXFMT_ARGB_1555:
        	  if(InternalConfig.Textures32bit)
        	  {
        	  	Convert1555to8888((FxU16*)data, m_tex_temp, texVals.nPixels);
            	OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp);
        	  }
            else if ( InternalConfig.OGLVersion > OGL_VER_1_1 )
            {
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, data );
            }
            else
            {
                Convert1555to5551( (FxU32*)data, m_tex_temp, texVals.nPixels );
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT, m_tex_temp );
            }
            break;
            
        case GR_TEXFMT_P_8:
            if ( InternalConfig.EXT_paletted_texture )
            {
                p_glColorTableEXT( GL_TEXTURE_2D, GL_RGBA, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_palette );
                
                DGL(glTexImage2D)( GL_TEXTURE_2D, texVals.lod, GL_COLOR_INDEX8_EXT, 
                              texVals.width, texVals.height, 0, 
                              GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data );
                if ( InternalConfig.EnableMipMaps )
                {
                    genPaletteMipmaps( texVals.width, texVals.height, data );
                }
            }
            else
            {
                ConvertP8to8888( data, m_tex_temp, texVals.nPixels, m_palette );
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_tex_temp );
            }
            break;
            
        case GR_TEXFMT_AP_88:
#if 0
            if ( use_two_textures )
            {
                FxU32 *tex_temp2 = m_tex_temp + 256 * 128;

                p_glColorTableEXT( GL_TEXTURE_2D, GL_RGBA, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_palette );

                SplitAP88( (FxU16 *)data, (FxU8 *)m_tex_temp, (FxU8 *)tex_temp2, texVals.nPixels );
                
                DGL(glTexImage2D)( GL_TEXTURE_2D, texVals.lod, GL_COLOR_INDEX8_EXT, 
                              texVals.width, texVals.height, 0, 
                              GL_COLOR_INDEX, GL_UNSIGNED_BYTE, m_tex_temp );
                if ( InternalConfig.EnableMipMaps )
                {
                    genPaletteMipmaps( texVals.width, texVals.height, (FxU8 *)m_tex_temp );
                }

                p_glActiveTextureARB( GL_TEXTURE1_ARB );

                OGL_LOAD_CREATE_TEXTURE( GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, tex_temp2 );

                p_glActiveTextureARB( GL_TEXTURE0_ARB );
            }
            else
#endif
            {
            	// JHFIX: buffer overflow here!
            	if(texVals.nPixels <= TEX_TEMP_PIXELS)
            	{
                ConvertAP88to8888( (FxU16*)data, m_tex_temp, texVals.nPixels, m_palette );
                OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_tex_temp );
              }
              else
              {
              	GlideMsg("Too large texture: texVals.nPixels %d\n", texVals.nPixels);
              }
              
            }
            break;
            
        case GR_TEXFMT_ALPHA_8:
            ConvertA8toAP88( (FxU8*)data, (FxU16*)m_tex_temp, texVals.nPixels );
            OGL_LOAD_CREATE_TEXTURE( 2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_tex_temp );
            //OGL_LOAD_CREATE_TEXTURE(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, data );
            // ^ we cannot load A8 native way because in Glide is channel layout for
            //   Alpha8 texture folowing A = R = G = B, but in OpenGL is A and R = G = B = 0
            break;
            
        case GR_TEXFMT_ALPHA_INTENSITY_88:
            OGL_LOAD_CREATE_TEXTURE(GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
            break;
            
        case GR_TEXFMT_INTENSITY_8:
            OGL_LOAD_CREATE_TEXTURE(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE, data );
            break;
            
        case GR_TEXFMT_ALPHA_INTENSITY_44:
#if 0
            OGL_LOAD_CREATE_TEXTURE( GL_LUMINANCE4_ALPHA4, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
#else
            ConvertAI44toAP88( (FxU8*)data, (FxU16*)m_tex_temp, texVals.nPixels );
            DGL(glTexImage2D)( GL_TEXTURE_2D, texVals.lod, 2, texVals.width, texVals.height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_tex_temp );
            if ( InternalConfig.BuildMipMaps )
            {
                /*gluBuild2DMipmaps( GL_TEXTURE_2D, 2, texVals.width, texVals.height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_tex_temp );*/
            }
#endif
            break;
            
        case GR_TEXFMT_8BIT: //GR_TEXFMT_RGB_332
            OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE_3_3_2_EXT, data );
            break;
            
        case GR_TEXFMT_16BIT: //GR_TEXFMT_ARGB_8332:
            Convert8332to8888( (FxU16*)data, m_tex_temp, texVals.nPixels );
            OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
            break;
            
        case GR_TEXFMT_YIQ_422:
            ConvertYIQto8888( (FxU8*)data, m_tex_temp, texVals.nPixels, &(m_ncc[m_ncc_select]) );
            OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
            break;
            
        case GR_TEXFMT_AYIQ_8422:
            ConvertAYIQto8888( (FxU16*)data, m_tex_temp, texVals.nPixels, &(m_ncc[m_ncc_select]) );
            OGL_LOAD_CREATE_TEXTURE(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_tex_temp );
            break;
            
        case GR_TEXFMT_RSVD0:
        case GR_TEXFMT_RSVD1:
        case GR_TEXFMT_RSVD2:
            Error( "grTexDownloadMipMapLevel - Unsupported format(%d)\n", m_tmu[tmu].info.format );
            memset( m_tex_temp, 255, texVals.nPixels * 2 );
            OGL_LOAD_CREATE_TEXTURE(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_tex_temp );
            break;
        }
    }

    DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL.tmu[tmu].SClampMode );
    DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL.tmu[tmu].TClampMode );
    
    DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL.tmu[tmu].MinFilterMode );
    DGL(glTexParameteri)( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL.tmu[tmu].MagFilterMode );
    
    return true;
}

FxU32 PGTexture::LodOffset( FxU32 evenOdd, GrTexInfo *info )
{
    FxU32   total = 0;
    GrLOD_t i;
#ifndef GLIDE3
    for( i = info->largeLod; i < info->smallLod; i++ )
    {
        total += MipMapMemRequired( i, info->aspectRatio, info->format );
    }
#else
    for( i = info->smallLodLog2; i < info->largeLodLog2; i++ )
    {
				total += MipMapMemRequired( i, info->aspectRatioLog2, info->format );
    }
#endif

    total = ( total + 7 ) & ~7;

    return total;
}

FxU32 PGTexture::TextureMemRequired( FxU32 evenOdd, GrTexInfo *info )
{
    //
    // If the format is one of these:
    // GR_TEXFMT_RGB_332, GR_TEXFMT_YIQ_422, GR_TEXFMT_ALPHA_8
    // GR_TEXFMT_INTENSITY_8, GR_TEXFMT_ALPHA_INTENSITY_44, GR_TEXFMT_P_8
    // Reduces the size by 2
    //
    
    // The extension format doesn't use Glide texture memory, but its
    // convenient to pretend it does because of the way TexDB works.
    if( info->format == GR_TEXFMT_BGRA_8888)
        return 2048;

#ifndef GLIDE3
    return nSquareTexLod[ info->format <= GR_TEXFMT_RSVD1 ][ info->aspectRatio ][ info->largeLod ][ info->smallLod ];
#else
    return nSquareTexLod[ info->format <= GR_TEXFMT_RSVD1 ][ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ][ 8 - info->smallLodLog2 ];
#endif
}

FxU32 PGTexture::MipMapMemRequired( GrLOD_t lod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format )
{
    //
    // If the format is one of these:
    // GR_TEXFMT_RGB_332, GR_TEXFMT_YIQ_422, GR_TEXFMT_ALPHA_8
    // GR_TEXFMT_INTENSITY_8, GR_TEXFMT_ALPHA_INTENSITY_44, GR_TEXFMT_P_8
    // Reduces the size by 2
    //

    // The extension format doesn't use Glide texture memory, but its
    // convenient to pretend it does because of the way TexDB works.
    if( format == GR_TEXFMT_BGRA_8888)
        return 2048;

#ifndef GLIDE3
    return nSquareLod[ format > GR_TEXFMT_RSVD1 ? 1 : 0 ][ aspectRatio ][ lod ];
#else
    return nSquareLod[ format > GR_TEXFMT_RSVD1 ? 1 : 0 ][ 3 - aspectRatio ][ 8 - lod ];
#endif
}

void PGTexture::GetTexValues(int tmu, TexValues * tval )
{
	GrTexInfo *info = &m_tmu[tmu].info;
	
#ifndef GLIDE3
    tval->width = texInfo[ info->aspectRatio ][ info->largeLod ].width;
    tval->height = texInfo[ info->aspectRatio ][ info->largeLod ].height;
    tval->nPixels = texInfo[ info->aspectRatio ][ info->largeLod ].numPixels;
#else
    tval->width = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].width;
    tval->height = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].height;
    tval->nPixels = texInfo[ 3 - info->aspectRatioLog2 ][ 8 - info->largeLodLog2 ].numPixels;
#endif
    tval->lod = 0;
}

void PGTexture::Clear( void )
{
	for(int tmu = 0; tmu < m_tmu_cnt; tmu++)
	{
		m_tmu[tmu].db->Clear( );
	}
}

bool PGTexture::GetAspect( int tmu, float *hAspect, float *wAspect )
{
	if(tmu > m_tmu_cnt)
	{
		return false;
	}

	*hAspect = m_tmu[tmu].hAspect;
  *wAspect = m_tmu[tmu].wAspect;
  
  return true;
}

bool PGTexture::GetAspect( int tmu, float *rAspect )
{
	if(tmu > m_tmu_cnt)
	{
		return false;
	}

	*rAspect = m_tmu[tmu].rAspect;
  
  return true;
}

void PGTexture::ChromakeyValue( GrColor_t value )
{
    m_chromakey_value_8888 = value & 0x00ffffff;
    m_chromakey_value_565  = (FxU16)( ( value & 0x00F80000 ) >> 8 |
                                      ( value & 0x0000FC00 ) >> 5 |
                                      ( value & 0x000000F8 ) >> 3 );
    m_palette_dirty = true;
}

void PGTexture::ChromakeyMode( GrChromakeyMode_t mode )
{
    m_chromakey_mode = mode;
    m_palette_dirty = true;
}

void PGTexture::ApplyKeyToPalette( void )
{
    FxU32   hash;
    int     i;
    
    if ( m_palette_dirty )
    {
        hash = 0;
        for ( i = 0; i < 256; i++ )
        {
            if ( ( m_chromakey_mode ) && 
                 ( ( m_palette[i] & 0x00ffffff ) == m_chromakey_value_8888 ) )
            {
                m_palette[i] &= 0x00ffffff;
            }
            else
            {
                m_palette[i] |= 0xff000000;
            }
            
            hash = ( ( hash << 5 ) | ( hash >> 27 ) );
            hash += ( InternalConfig.IgnorePaletteChange
                      ? ( m_palette[ i ] & 0xff000000  )
                      : m_palette[ i ]);
        }
        
        m_palette_hash = hash;
        m_palette_dirty = false;
    }
}

void PGTexture::NCCTable( GrNCCTable_t tab )
{
    switch ( tab )
    {
    case GR_NCCTABLE_NCC0:
    case GR_NCCTABLE_NCC1:
        m_ncc_select = tab;
    }
}

FxU32 PGTexture::GetMemorySize( void )
{
    return m_tex_memory_size;
}
