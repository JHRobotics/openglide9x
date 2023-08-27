//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*     Windows specific functions for library initialisation
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef WIN32

#define OEMRESOURCE

#include "platform.h"
#include "GlOgl.h"

#include "openglide9x.h"

#ifdef HAVE_CRTEX
extern "C" {
# include <lockex.h>
};
#endif

HCURSOR hTransparent = NULL;
HCURSOR hDefault = NULL;

HINSTANCE glideDLLInt = NULL;

extern "C" {
	BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvreserved );
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvreserved )
{
    int Priority;

    switch ( dwReason )
    {
    case DLL_THREAD_ATTACH:
        break;

    case DLL_PROCESS_ATTACH:
#ifdef HAVE_CRTEX
    		crt_locks_init(0);
#endif
        if ( !ClearAndGenerateLogFile( ) )
        {
            return FALSE;
        }
        
        glideDLLInt = hinstDLL;
        RegisterOGLClass();
           
        #ifdef __GNUC__
        __builtin_cpu_init();
        #endif
        
        /* transparent cursor and copy of actual default cursor */
        hTransparent = LoadCursorA(glideDLLInt, MAKEINTRESOURCE(CURSOR_TRANSPARENT));
				if(hTransparent) CopyCursor(hTransparent);
				hDefault = LoadCursorA(NULL, IDC_ARROW);
        if(hDefault) CopyCursor(hDefault);
        
        InitMainVariables( );

        if ( SetPriorityClass( GetCurrentProcess( ), NORMAL_PRIORITY_CLASS ) == 0 )
        {
            Error( "Could not set Class Priority.\n" );
        }
        else
        {
            GlideMsg( OGL_LOG_SEPARATE );
            GlideMsg( "Wrapper Class Priority of %d\n", NORMAL_PRIORITY_CLASS );
        }

        switch ( UserConfig.Priority )
        {
        case 0:     Priority = THREAD_PRIORITY_HIGHEST;         break;
        case 1:     Priority = THREAD_PRIORITY_ABOVE_NORMAL;    break;
        case 2:     Priority = THREAD_PRIORITY_NORMAL;          break;
        case 3:     Priority = THREAD_PRIORITY_BELOW_NORMAL;    break;
        case 4:     Priority = THREAD_PRIORITY_LOWEST;          break;
        case 5:     Priority = THREAD_PRIORITY_IDLE;            break;
        default:    Priority = THREAD_PRIORITY_NORMAL;          break;
        }
        if ( SetThreadPriority( GetCurrentThread(), Priority ) == 0 )
        {
            Error( "Could not set Thread Priority.\n" );
        }
        else
        {
            GlideMsg( "Wrapper Priority of %d\n", UserConfig.Priority );
            GlideMsg( OGL_LOG_SEPARATE );
        }
        break;

    case DLL_THREAD_DETACH:
    	  ResetGLThread();
        break;

    case DLL_PROCESS_DETACH:
        grGlideShutdown( );
        CloseLogFile( );
        
        if(hDefault != NULL) DestroyCursor(hDefault);
        if(hTransparent != NULL) DestroyCursor(hTransparent);
        
        break;
    }
    return TRUE;
}

#endif // WIN32
