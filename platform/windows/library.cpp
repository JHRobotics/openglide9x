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

#include "platform.h"
#include "GlOgl.h"

LRESULT CALLBACK DrawWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
		  //PostQuitMessage(0);
			break;
		case WM_SETCURSOR:
			SetCursor(NULL);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}

HINSTANCE glideDLLInt = NULL;

BOOL registerWinClass()
{
	WNDCLASS wc      = {0};
	wc.lpfnWndProc   = DrawWndProc;
	wc.hInstance     = glideDLLInt;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = GLIDE_WND_CLASS_NAME;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	
	printf("RegisterClass!\n");
	if( !RegisterClass(&wc) )
	{
		return TRUE;
	}
	
	return FALSE;
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvreserved )
{
    int Priority;

    switch ( dwReason )
    {
    case DLL_THREAD_ATTACH:
        break;

    case DLL_PROCESS_ATTACH:
        if ( !ClearAndGenerateLogFile( ) )
        {
            return false;
        }
        glideDLLInt = hinstDLL;
        registerWinClass();
        
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
        break;

    case DLL_PROCESS_DETACH:
        grGlideShutdown( );
        CloseLogFile( );
        break;
    }
    return TRUE;
}

#endif // WIN32
