#include "FormatConversion.h"

#ifndef INSTRUMENTED
void Convert565Kto8888( FxU16 *Buffer1, FxU16 key, FxU32 *Buffer2, FxU32 Pixels )
{
    while ( Pixels )
    {
        *Buffer2++ = ( ( (*Buffer1) == key) ? 0x00000000 : 0xFF000000 ) |   // A
                       ( (*Buffer1)    & 0x001F ) << 19 |                   // B
                       ( (*Buffer1)    & 0x07E0 ) << 5  |                   // G
                       ( (*Buffer1)    & 0xF800 ) >> 8;                     // R
        Buffer1++;
        Pixels--;
    }
}
#endif

#ifndef INSTRUMENTED
void Convert8888to565(FxU32 *Src, FxU16 *Dst, FxU32 Pixels )
{
    while ( Pixels-- )
    {
        *Dst++ = ( FxU16 ) ( ( *Src & 0x00F80000 ) >> 8 |
                ( *Src & 0x0000FC00 ) >> 5 |
                ( *Src & 0x000000F8 ) >> 3 );
        Src++;
    }
}
#endif

#ifndef INSTRUMENTED
// This functions processes 2 pixels at a time, there is no problem in
// passing odd numbers or a number less than 2 for the pixels, but
// the buffers should be large enough
void Convert565to5551( FxU32 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
      *Buffer2++ = ( (*Buffer1) & 0xFFC0FFC0 ) |
                 ( ( (*Buffer1) & 0x001F001F ) << 1 ) |
                     0x00010001;
      Buffer1++;
      Pixels -= 2;
   }
}
#endif

#ifndef INSTRUMENTED
// This functions processes 2 pixels at a time, there is no problem in
// passing odd numbers or a number less than 2 for the pixels, but
// the buffers should be large enough
void Convert5551to565( FxU32 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
      *Buffer2++ = ( (*Buffer1) & 0xFFC0FFC0 ) |
                 ( ( (*Buffer1) & 0x003E003E ) >> 1 );
      Buffer1++;
      Pixels -= 2;
   }
}
#endif

#ifndef INSTRUMENTED
// This functions processes 2 pixels at a time, there is no problem in
// passing odd numbers or a number less than 2 for the pixels, but
// the buffers should be large enough
void Convert4444to4444special( FxU32 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
      *Buffer2++ = ( ( (*Buffer1) & 0x0FFF0FFF ) << 4 )|
                   ( ( (*Buffer1) & 0xF000F000 ) >> 12 );
      Buffer1++;
      Pixels -= 2;
   }
}
#endif

#ifndef INSTRUMENTED
void Convert1555to5551( FxU32 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
      *Buffer2++ = ( ( (*Buffer1) & 0x7FFF7FFF ) << 1 )|
                   ( ( (*Buffer1) & 0x80008000 ) >> 15 );
      Buffer1++;
      Pixels -= 2;
   }
}
#endif

#ifndef INSTRUMENTED
void Convert565to8888( FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels )
{
   while ( Pixels )
   {
      *Buffer2++ = 0xFF000000 |              // A
         ( (*Buffer1) & 0x001F ) << 19 |  // B
         ( (*Buffer1) & 0x07E0 ) << 5  |  // G
         ( (*Buffer1) & 0xF800 ) >> 8;    // R
      Buffer1++;
      Pixels--;
   }
}
#endif

#ifndef INSTRUMENTED
void ConvertA8toAP88( FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels )
{
    while ( Pixels )
    {
#if defined(CONSTANT_LUMINOSITY_ALPHA_TEXTURE)
        *Buffer2 = ( ( ( *Buffer1 ) << 8 ) | 0xFF );
#else
        *Buffer2 = ( ( ( *Buffer1 ) << 8 ) | ( *Buffer1 ) );
#endif
        Buffer1++;
        Buffer2++;
        Pixels--;
    }
}
#endif

#ifndef INSTRUMENTED
void Convert8332to8888( FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels )
{
    static FxU32    R, 
                    G, 
                    B, 
                    A;
    for ( FxU32 i = Pixels; i > 0; i-- )
    {
        A = ( ( ( *Buffer1 ) >> 8 ) & 0xFF );
        R = ( ( ( *Buffer1 ) >> 5 ) & 0x07 ) << 5;
        G = ( ( ( *Buffer1 ) >> 2 ) & 0x07 ) << 5;
        B = (   ( *Buffer1 ) & 0x03 ) << 6;
        *Buffer2 = ( A << 24 ) | ( B << 16 ) | ( G << 8 ) | R;
        Buffer1++;
        Buffer2++;
    }
}
#endif

void ConvertP8to8888( FxU8 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette )
{
    while ( Pixels-- )
    {
        *Buffer2++ = palette[ *Buffer1++ ];
    }
}

#ifndef INSTRUMENTED
void ConvertAI44toAP88( FxU8 *Buffer1, FxU16 *Buffer2, FxU32 Pixels )
{
    for ( FxU32 i = Pixels; i > 0; i-- )
    {
        *Buffer2 = ( ( ( ( *Buffer1 ) & 0xF0 ) << 8 ) | ( ( ( *Buffer1 ) & 0x0F ) << 4 ) );
        Buffer2++;
        Buffer1++;
    }
}
#endif

#ifndef INSTRUMENTED
void ConvertAP88to8888( FxU16 *Buffer1, FxU32 *Buffer2, FxU32 Pixels, FxU32 *palette )
{
    FxU32   RGB, 
            A;
    for ( FxU32 i = Pixels; i > 0; i-- )
    {
        RGB = ( palette[ *Buffer1 & 0x00FF ] & 0x00FFFFFF );
        A = *Buffer1 >> 8;
        *Buffer2 = ( A << 24 ) | RGB;
        Buffer1++;
        Buffer2++;
    }
}
#endif

void ConvertYIQto8888( FxU8 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc )
{
    FxI32  R;
    FxI32  G;
    FxI32  B;

    for ( FxU32 i = Pixels; i > 0; i-- )
    {
        R = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 0 ]
                                  + ncc->qRGB[ ( *in      ) & 0x3 ][ 0 ];

        G = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 1 ]
                                  + ncc->qRGB[ ( *in      ) & 0x3 ][ 1 ];

        B = ncc->yRGB[ *in >> 4 ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 2 ]
                                  + ncc->qRGB[ ( *in      ) & 0x3 ][ 2 ];

        R = ( ( R < 0 ) ? 0 : ( ( R > 255 ) ? 255 : R ) );
        G = ( ( G < 0 ) ? 0 : ( ( G > 255 ) ? 255 : G ) );
        B = ( ( B < 0 ) ? 0 : ( ( B > 255 ) ? 255 : B ) );

        *out = ( R | ( G << 8 ) | ( B << 16 ) | 0xff000000 );

        in++;
        out++;
    }
}

void ConvertAYIQto8888( FxU16 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc)
{
    FxI32  R;
    FxI32  G;
    FxI32  B;

    for ( FxU32 i = Pixels; i > 0; i-- )
    {
        R = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 0 ]
                                            + ncc->qRGB[ ( *in      ) & 0x3 ][ 0 ];

        G = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 1 ]
                                            + ncc->qRGB[ ( *in      ) & 0x3 ][ 1 ];

        B = ncc->yRGB[ ( *in >> 4 ) & 0xf ] + ncc->iRGB[ ( *in >> 2 ) & 0x3 ][ 2 ]
                                            + ncc->qRGB[ ( *in      ) & 0x3 ][ 2 ];

        R = ( ( R < 0 ) ? 0 : ( ( R > 255 ) ? 255 : R ) );
        G = ( ( G < 0 ) ? 0 : ( ( G > 255 ) ? 255 : G ) );
        B = ( ( B < 0 ) ? 0 : ( ( B > 255 ) ? 255 : B ) );

        *out = ( R | ( G << 8 ) | ( B << 16 ) | ( 0xff000000 & ( *in << 16 ) ) );

        in++;
        out++;
    }
}

#ifndef INSTRUMENTED
void SplitAP88( FxU16 *ap88, FxU8 *index, FxU8 *alpha, FxU32 pixels )
{
    for ( FxU32 i = pixels; i > 0; i-- )
    {
        *alpha++ = ( *ap88 >> 8 );
        *index++ = ( *ap88++ & 0xff );
    }
}
#endif

#ifndef INSTRUMENTED
void Convert1555to8888( FxU16 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
   		const FxU32 px = *Buffer1++;
   		FxU32 dst = 0;
   		
   		if(px & 0x8000)
   		{
   			dst = 0xFF000000;
   		}
   		
   		dst |= (px & 0x001F) << (3+16);
   		dst |= (px & 0x03E0) << 6;
   		dst |= (px & 0x7C00) >> 7; // 9-16
   		
      *Buffer2++ = dst;
      Pixels--;
   }
}
#endif

#ifndef INSTRUMENTED
void Convert4444to8888( FxU16 *Buffer1, FxU32 *Buffer2, int Pixels )
{
   while ( Pixels > 0 )
   {
   		const FxU32 px = *Buffer1++;
   		FxU32 dst = 0;
   		
   		dst |= ((px & 0xF000)*17) << 12; /* extend alpha */
   		dst |=  (px & 0x0F00) >> 4;
   		dst |=  (px & 0x00F0) << 8;
   		dst |=  (px & 0x000F) << 20;
   		
      *Buffer2++ = dst;
      Pixels--;
   }
}
#endif
