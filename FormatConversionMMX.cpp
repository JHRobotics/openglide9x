#include "FormatConversion.h"

#ifdef INSTRUMENTED_MMX
typedef unsigned int   v2ui __attribute__ ((vector_size(8)));
typedef unsigned short v2us __attribute__ ((vector_size(4)));
typedef unsigned short v4us __attribute__ ((vector_size(8)));
typedef unsigned char  v4uc __attribute__ ((vector_size(4)));

void Convert565Kto8888(FxU16 *in, FxU16 key, FxU32 *out, FxU32 pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2us vs = *((v2us*)in);
		v2ui v = __builtin_convertvector(vs, v2ui);
		v2ui r, g, b;
		
		r = (v & 0x0000F800) >> 8;
		g = (v & 0x000007E0) << 5;
		b = (v & 0x0000001F) << 19;
		v = r | g | b;
		
		if(vs[0] != key) v[0] |= 0xFF000000;
		if(vs[1] != key) v[1] |= 0xFF000000;
		
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
		*out =  ( ( (*in) == key) ? 0x00000000 : 0xFF000000 ) |   // A
						( (*in)    & 0x001F ) << 19 |                   // B
						( (*in)    & 0x07E0 ) << 5  |                   // G
						( (*in)    & 0xF800 ) >> 8;                     // R
		in++;
		out++;
	}
}

void Convert8888to565(FxU32 *in, FxU16 *out, FxU32 pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2ui v = *((v2ui*)in);
		v2ui r, g, b;
		v2us vo;
		
		r = (v & 0x00F80000) >> 8;
		g = (v & 0x0000FC00) >> 5;
		b = (v & 0x000000F8) >> 3;
		v = r | g | b;
		
		vo = __builtin_convertvector(v, v2us);
		*((v2us*)out) = vo;
		
		in += 2;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
		*out = (FxU16)(((*in) & 0x00F80000 ) >> 8 | ((*in) & 0x0000FC00 ) >> 5 | ((*in) & 0x000000F8 ) >> 3 );
  	out++;
  	in++;
	}
	
}

void Convert565to5551(FxU32 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v2ui v = *((v2ui*)in);
		v = (v & 0xFFC0FFC0) | ((v & 0x001F001F) << 1) | 0x00010001;
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	FxU16 *s_in  = (FxU16 *)in;
	FxU16 *s_out = (FxU16 *)out;
	for(; pixels > 0; pixels--)
	{
		*out = ( (*s_in) & 0xFFC0 ) | (((*s_in) & 0x001F) << 1) | 0x0001;
		s_in++;
		s_out++;
	}
}

void Convert5551to565(FxU32 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v2ui v = *((v2ui*)in);
		v = (v & 0xFFC0FFC0) | ((v & 0x003E003E) >> 1);
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	FxU16 *s_in  = (FxU16 *)in;
	FxU16 *s_out = (FxU16 *)out;
	for(; pixels > 0; pixels--)
	{
		*out = ( (*s_in) & 0xFFC0 ) | (((*s_in) & 0x003E) >> 1);
		s_in++;
		s_out++;
	}
}

void Convert4444to4444special(FxU32 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v2ui v = *((v2ui*)in);
		v = ((v & 0x0FFF0FFF) << 4) | ((v & 0xF000F000) >> 12);
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	FxU16 *s_in  = (FxU16 *)in;
	FxU16 *s_out = (FxU16 *)out;
	for(; pixels > 0; pixels--)
	{
		*out = (((*s_in) & 0x0FFF) << 4) | (((*s_in) & 0xF000) >> 12);
		s_in++;
		s_out++;
	}
}

void Convert1555to5551(FxU32 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v2ui v = *((v2ui*)in);
		v = ((v & 0x7FFF7FFF) << 1) | ((v & 0x80008000) >> 15);
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	FxU16 *s_in  = (FxU16 *)in;
	FxU16 *s_out = (FxU16 *)out;
	for(; pixels > 0; pixels--)
	{
		*out = (((*s_in) & 0x7FFF) << 1) | (((*s_in) & 0x8000) >> 15);
		s_in++;
		s_out++;
	}
}

void Convert565to8888(FxU16 *in, FxU32 *out, FxU32 pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2us vs = *((v2us*)in);
		v2ui v = __builtin_convertvector(vs, v2ui);
		v2ui r, g, b;
		
		r = (v & 0x0000F800) >> 8;
		g = (v & 0x000007E0) << 5;
		b = (v & 0x0000001F) << 19;
		v = r | g | b | 0xFF000000;
		
		*((v2ui*)out) = v;
		
		in += 2;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
		*out = 0xFF000000 |               // A
					(((*in) & 0x001F) << 19) |  // B
					(((*in) & 0x07E0) << 5)  |  // G
					(((*in) & 0xF800) >> 8);    // R
		in++;
		out++;
	}
}

void ConvertA8toAP88(FxU8 *in, FxU16 *out, FxU32 pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v4uc c = *((v4uc*)in);
		v4us v = __builtin_convertvector(c, v4us);
#if defined(CONSTANT_LUMINOSITY_ALPHA_TEXTURE)
		v = (v << 8) | 0xFF;
#else
		v = (v << 8) | v;
#endif
		*((v4us*)out) = v;
		
		in  += 4;
		out += 4;
	}
	
	for(; pixels > 0; pixels--)
	{
#if defined(CONSTANT_LUMINOSITY_ALPHA_TEXTURE)
		*out = (((*in) << 8 ) | 0xFF);
#else
		*out = (((*in) << 8 ) | (*in));
#endif
		in++;
		out++;
		pixels--;
	}
}

void Convert8332to8888(FxU16 *in, FxU32 *out, FxU32 pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2us vs = *((v2us*)in);
		v2ui v = __builtin_convertvector(vs, v2ui);
		v2ui a, r, g, b;
		
		a = (v & 0xFF00) << 16;
		r = (v & 0x00E0);
		g = (v & 0x001C) << 11;
		b = (v & 0x0003) << 22;
		v = a | b | g | r;
		
		*((v2ui*)out) = v;
		
		in += 4;
		out += 4;
	}
	
	for(; pixels > 0; pixels--)
	{
		FxU32 px = *in;
				
		*out = ((px & 0xFF00) << 16) | // A
			 (px & 0x00E0) |             // R
			((px & 0x001C) << 11) |      // G
			((px & 0x0003) << 22);       // B
		
		in++;
		out++;
	}
}

//void ConvertP8to8888( FxU8 *in, FxU32 *out, FxU32 Pixels, FxU32 *palette );
void ConvertAI44toAP88(FxU8 *in, FxU16 *out, FxU32 pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v4uc c = *((v4uc*)in);
		v4us v = __builtin_convertvector(c, v4us);
		v = ((v & 0xF0) << 8) | ((v & 0x0F) << 4);
		*((v4us*)out) = v;
		in += 4;
		out += 4;
	}
	
	for(; pixels > 0; pixels--)
	{
		*out = ((((FxU16)*in) & 0xF0 ) << 8) | ((((FxU16)*in) & 0x0F ) << 4);
		in++;
		out++;
	}
}

void ConvertAP88to8888(FxU16 *in, FxU32 *out, FxU32 pixels, FxU32 *palette)
{
	FxU8 *cin = (FxU8*)in;
	for(; pixels >= 2; pixels -= 2)
	{
		v2ui rgb, a;
		rgb[0] = palette[*cin++];
		  a[0] = *cin++;
		rgb[1] = palette[*cin++];
		  a[1] = *cin++;
		
		a  <<= 24;
		rgb &= 0x00FFFFFF;
		
		rgb |= a;
		
		*((v2ui*)out) = rgb;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
		FxU32 rgb, a;
		rgb = palette[*cin++] & 0x00FFFFFF;
		a = (*cin++) << 24;
		
		*out = a | rgb;
		out++;
  }
}

//void ConvertYIQto8888( FxU8 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc);
//void ConvertAYIQto8888( FxU16 *in, FxU32 *out, FxU32 Pixels, GuNccTable *ncc);

void SplitAP88(FxU16 *ap88, FxU8 *index, FxU8 *alpha, FxU32 pixels)
{
	for(; pixels >= 4; pixels -= 4)
	{
		v4uc i4, a4;
		v4us v;
		
		v = *((v4us*)ap88);
		i4 = __builtin_convertvector(v & 0xFF, v4uc);
		a4 = __builtin_convertvector(v >> 8, v4uc);
		
		*((v4uc*)index) = i4;
		*((v4uc*)alpha) = a4;
		
		ap88 += 4;
		index += 4;
		alpha += 4;
	}
	
	for(; pixels > 0; pixels--)
	{
		*alpha = *ap88 >> 8;
		*index = *ap88 & 0xff;
		
		ap88++;
		index++;
		alpha++;
  }
}

void Convert1555to8888(FxU16 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2us vs = *((v2us*)in);
		v2ui v = __builtin_convertvector(vs, v2ui);
		v2ui vo;
		
		vo = (v & 0x8000) << 9;
		vo *= 255; // extend alpha
		
   	vo |= (v & 0x001F) << (3+16);
   	vo |= (v & 0x03E0) << 6;
   	vo |= (v & 0x7C00) >> 7; // 9-16

		*((v2ui*)out) = vo;
		
		in += 2;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
   	const FxU32 px = *in;
   	FxU32 dst = (px & 0x8000) << 9;
   	dst *= 255; // extend alpha
   	dst |= (px & 0x001F) << (3+16);
   	dst |= (px & 0x03E0) << 6;
   	dst |= (px & 0x7C00) >> 7; // 9-16
   	
   	*out = dst;
   	
		in++;
		out++;
	}
}


void Convert4444to8888(FxU16 *in, FxU32 *out, int pixels)
{
	for(; pixels >= 2; pixels -= 2)
	{
		v2us vs = *((v2us*)in);
		v2ui v = __builtin_convertvector(vs, v2ui);
		v2ui vo;
		
		vo = (v & 0xF000) << 12;
		vo *= 17; // extend alpha
		
   	vo |= (v & 0x0F00) >> 4;
   	vo |= (v & 0x00F0) << 8;
   	vo |= (v & 0x000F) << 20;

		*((v2ui*)out) = vo;
		
		in += 2;
		out += 2;
	}
	
	for(; pixels > 0; pixels--)
	{
   	const FxU32 px = *in;
   	FxU32 dst = (px & 0xF000) << 12;
   	dst *= 17; // extend alpha
   	dst |= (px & 0x0F00) >> 4;
   	dst |= (px & 0x00F0) << 8;
   	dst |= (px & 0x000F) << 20;
   	
   	*out = dst;
   	
		in++;
		out++;
	}
}

#endif /* MMX */
