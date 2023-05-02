//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*           Windows specific includes and macros
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef WIN32

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef VBOX_GLIDE_WITH_IPRT
# include <windows.h>
# include <io.h>
#else
# include <iprt/win/windows.h>
#endif

#ifdef max
#undef max
#endif

#define max(x,y) ((x) < (y) ? (y) : (x))

#define FASTCALL __fastcall

#define VARARGDECL(t) t _cdecl
typedef int (__stdcall *ExtFn)();

#ifdef _MSC_VER
#  ifdef _WIN64
	typedef long long FxI;
	typedef unsigned long long FxU;
#  else
	typedef int FxI;
	typedef unsigned int FxU;
#  endif
#endif

#endif

extern HINSTANCE glideDLLInt;

#define GLIDE_WND_CLASS_NAME "openglidewin"

BOOL RegisterOGLClass();

void Activate3DWindow();
void Deactivate3DWindow();

void SetGLThread();
void ResetGLThread();

int GetVRetrace();

#endif // WIN32
