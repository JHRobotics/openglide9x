/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision$
** $Date$
*/
#ifndef __3DFX_H__
#define __3DFX_H__

#ifndef VBOX_GLIDE_WITH_IPRT
# include <cstdint>
	typedef uint8_t   FxU8;
	typedef int8_t    FxI8;
	typedef uint16_t  FxU16;
	typedef int16_t   FxI16;
	typedef uint32_t  FxU32;
	typedef int32_t   FxI32;
	typedef uint64_t  FxU64;
	typedef int64_t   FxI64;
	typedef int       FxBool;
	typedef float     FxFloat;
	typedef double    FxDouble;
	typedef uintptr_t FxU;
	typedef intptr_t  FxI;
#else
	typedef unsigned char       FxU8;
	typedef signed char         FxI8;
	typedef unsigned short      FxU16;
	typedef signed short        FxI16;
	typedef unsigned long       FxU32;
	typedef signed long         FxI32;
	typedef unsigned long long  FxU64;
	typedef long long   FxI64;
	typedef int       FxBool;
	typedef float     FxFloat;
	typedef double    FxDouble;
#  ifdef _WIN64
	typedef long long FxI;
	typedef unsigned long long FxU;
#  else
	typedef int FxI;
	typedef unsigned int FxU;
#  endif
#endif

/*
** basic data types
*/


/*
** color types
*/
typedef unsigned long                FxColor_t;
typedef struct { float r, g, b, a; } FxColor4;

/*
** fundamental types
*/
#define FXTRUE    1
#define FXFALSE   0

/*
** helper macros
*/
#define FXUNUSED( a ) ((void)(a))
#define FXBIT( i )    ( 1L << (i) )

/*
** export macros
*/

#if defined(CPPDLL)
#  define DLLEXPORT extern "C" __declspec(dllexport)
#  define FX_ENTRY extern "C"
#  define FX_CALL __stdcall
#elif defined(__MSC__)
#  if defined (MSVC16)
#    define FX_ENTRY
#    define FX_CALL
#  else
#    define FX_ENTRY extern
#    define FX_CALL  __stdcall
#  endif
#elif defined(__MINGW32__)
#  define FX_ENTRY extern "C" 
#  define FX_CALL  __stdcall
/*#elif defined(__WATCOMC__)
#  define FX_ENTRY extern
#  define FX_CALL  __stdcall
#elif defined (__IBMC__) || defined (__IBMCPP__)
   //  IBM Visual Age C/C++: 
#  define FX_ENTRY extern
#  define FX_CALL  __stdcall
#elif defined(__DJGPP__)
#  define FX_ENTRY extern
#  define FX_CALL
#elif defined(__unix__)
#  define FX_ENTRY extern
#  define FX_CALL
#elif defined(__MWERKS__)
#  if macintosh
#    define FX_ENTRY extern
#    define FX_CALL
#  else // !macintosh 
#    error "Unknown MetroWerks target platform"
#  endif // !macintosh */
#else
#  warning define FX_ENTRY & FX_CALL for your compiler
#  define FX_ENTRY extern
#  define FX_CALL
#endif

/*
** x86 compiler specific stuff
*/
#if defined(__BORLANDC_)
#  define REALMODE

#  define REGW( a, b ) ((a).x.b)
#  define REGB( a, b ) ((a).h.b)
#  define INT86( a, b, c ) int86(a,b,c)
#  define INT86X( a, b, c, d ) int86x(a,b,c,d)

#  define RM_SEG( a ) FP_SEG( a )
#  define RM_OFF( a ) FP_OFF( a )
#elif defined(__WATCOMC__)
#  undef FP_SEG
#  undef FP_OFF

#  define REGW( a, b ) ((a).w.b)
#  define REGB( a, b ) ((a).h.b)
#  define INT86( a, b, c ) int386(a,b,c)
#  define INT86X( a, b, c, d ) int386x(a,b,c,d)

#  define RM_SEG( a )  ( ( ( ( FxU32 ) (a) ) & 0x000F0000 ) >> 4 )
#  define RM_OFF( a )  ( ( FxU16 ) (a) )
#endif

#endif /* !__3DFX_H__ */
