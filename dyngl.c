#include <Windows.h>
#include <stdio.h>
#include "dyngl.h"

dyngl_t dyngl;

static HGLRC WINAPI wglCreateContext_wrapper(HDC hDc)
{
	if(!dyngl_load())
	{
		return NULL;
	}
	
	return dyngl.wglCreateContext(hDc);
}

void dyngl_init(HMODULE thislib)
{
	memset(&dyngl, 0, sizeof(dyngl_t));
	dyngl.wglCreateContext = wglCreateContext_wrapper;
	dyngl.thislib = thislib;
}

void dyngl_destroy()
{
	if(dyngl.lib)
	{
		FreeLibrary(dyngl.lib);
	}
	memset(&dyngl, 0, sizeof(dyngl_t));
}

#define DGL_ITEM(_ret, _call, _name, _args) \
	dyngl._name = (dyngl_f_ ## _name)GetProcAddress(dyngl.lib, #_name); \
	if(dyngl._name == NULL){return FALSE;}

BOOL dyngl_load()
{
	char path[PATH_MAX];
	if(dyngl.lib != NULL)
	{
		return TRUE;
	}
	
	/*
		OK, this is a bit thought, some game used wrapper named "opengl32.dll" which
		call this (glide2x.dll/glide3x.dll) and we cannot load "opengl32.dll",
		because this creates loop. This is usually also using load-time dynamic
		linking so we cannot determine call path. Conclusion: try load openg32.dll
		in same directory as this DLL, when not exists, load openg32 from system dir.
	 */
	if(GetModuleFileNameA(dyngl.thislib, path, PATH_MAX) != 0)
	{
		char *pstop;
		char *ptr1 = strrchr(path, '\\');
		char *ptr2 = strrchr(path, '/');
		
		if(ptr1 != NULL && ptr2 != NULL)
		{
			pstop = ptr1 > ptr2 ? ptr1 : ptr2;
			if(strlen(pstop + sizeof("\\opengl32.dll")) < PATH_MAX)
			{
				strcpy(pstop+1, "opengl32.dll");
				
				dyngl.lib = LoadLibraryA(path);
				if(dyngl.lib == dyngl.thislib) /* we load self */
				{
					FreeLibrary(dyngl.lib);
					dyngl.lib = NULL;
				}
			}
		}
	}
	
	if(dyngl.lib == NULL)
	{
		if(GetSystemDirectoryA(path, PATH_MAX))
		{
			strcat(path, "\\opengl32.dll");
			dyngl.lib = LoadLibraryA(path);
			if(dyngl.lib == dyngl.thislib) /* OK, this is bad... */
			{
				FreeLibrary(dyngl.lib);
				dyngl.lib = NULL;
				return FALSE;
			}
		}
	}
	
	if(dyngl.lib == NULL)
		return FALSE;
	
	#include "dyngl_list.h"

	return TRUE;
}

#undef DGL_ITEM

