#include <Windows.h>
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
	
	dyngl.lib = LoadLibraryA("opengl32.dll");
	
	if(dyngl.lib == dyngl.thislib) /* we load self */
	{
		FreeLibrary(dyngl.lib);
		dyngl.lib = NULL;
		
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
		
		return FALSE;
	}
	
	if(dyngl.lib == NULL)
		return FALSE;
	
	#include "dyngl_list.h"

	return TRUE;
}

#undef DGL_ITEM

