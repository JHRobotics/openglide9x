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

static DEVMODEA savedMode;
static BOOL saveModeUsable = FALSE;

/*
 * EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, ...) not working on W95,
 * so we use this replacement
 */
static BOOL EnumDisplayCurrent(DEVMODEA *lpDevMode)
{
	HWND hDesktop = GetDesktopWindow();
	HDC hdc = GetDC(hDesktop);
	if(hdc)
	{
		lpDevMode->dmBitsPerPel = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
		lpDevMode->dmPelsWidth  = GetDeviceCaps(hdc, HORZRES);
		lpDevMode->dmPelsHeight = GetDeviceCaps(hdc, VERTRES);
		lpDevMode->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		return TRUE;
	}

	return FALSE;
}

static void desktopSave()
{
	saveModeUsable = FALSE;
	
	memset(&savedMode, 0, sizeof(DEVMODEA));
	savedMode.dmSize = sizeof(DEVMODEA);
	
	DEVMODEA curmode;
	memset(&curmode, 0, sizeof(DEVMODEA));
	curmode.dmSize = sizeof(DEVMODEA);
	
	if(EnumDisplaySettingsA(NULL, ENUM_REGISTRY_SETTINGS, &savedMode))
	{
		if(EnumDisplayCurrent(&curmode))
		{
			if(savedMode.dmBitsPerPel == curmode.dmBitsPerPel &&
			   savedMode.dmPelsWidth  == curmode.dmPelsWidth &&
			   savedMode.dmPelsHeight	== curmode.dmPelsHeight)
			{
				saveModeUsable = TRUE;
			}
		}
	}
}

static void desktopRestore()
{
	if(saveModeUsable)
	{
		savedMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettingsA(&savedMode, CDS_FULLSCREEN);
	}
}


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
        desktopSave();
        dyngl_init(hinstDLL);
        
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
        
        if(UserConfig.Disabled)
        {
        	return FALSE; /* deny load */
        }

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
    		//if(lpvreserved != NULL) break;
    			
        grGlideShutdown( );
        CloseLogFile( );
        
        if(hDefault != NULL) DestroyCursor(hDefault);
        if(hTransparent != NULL) DestroyCursor(hTransparent);
        	
        dyngl_destroy();
        
        desktopRestore();
        break;
    }
    return TRUE;
}

#endif // WIN32
