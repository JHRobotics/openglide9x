//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*   Windows specific functions for handling display window
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined C_USE_SDL && defined WIN32
/*
#include <windows.h>
#include <io.h>
#include <math.h>
*/
#define OEMRESOURCE

#include "platform.h"
#include <math.h>
#include "GlOgl.h"

#include "platform/window.h"

static HDC   hDC;
static HGLRC hRC;
static HWND  hWND;
static bool  hWND_created;
static struct
{
    FxU16 red[ 256 ];
    FxU16 green[ 256 ];
    FxU16 blue[ 256 ];
} old_ramp;

static BOOL ramp_stored  = false;
static BOOL mode_changed = false;

static HHOOK win_hook = NULL;

LRESULT CALLBACK DrawWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}

BOOL RegisterWinClass()
{
	WNDCLASSA wc      = {0};
	wc.lpfnWndProc   = DrawWndProc;
	wc.hInstance     = glideDLLInt;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = GLIDE_WND_CLASS_NAME;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	
	if( !RegisterClass(&wc) )
	{
		return TRUE;
	}
	
	return FALSE;
}

/* Hook proc for glide window to hide annoing cursor pointer */
LRESULT WINAPI GlideHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	/* If nCode is greater than or equal to HC_ACTION, we should process the message. */
	if (code >= HC_ACTION)
	{
		const LPCWPRETSTRUCT lpcwprs = (LPCWPRETSTRUCT)lParam;
		switch (lpcwprs->message)
		{
			case WM_SETCURSOR:
				SetCursor(NULL);
				break;
		}
	}
	return CallNextHookEx(win_hook, code, wParam, lParam); 
}

typedef BOOL (WINAPI * GLIDE_PFNWGLSWAPINTERVALEXTPROC) (int interval);

BOOL setVSync()
{
	GLIDE_PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (GLIDE_PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
	if(wglSwapIntervalEXT)
	{
		return wglSwapIntervalEXT(1);
	}
	
	return FALSE;		
}

extern HCURSOR hTransparent;
extern HCURSOR hDefault;

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    PIXELFORMATDESCRIPTOR   pfd;
    int                     PixFormat;
    unsigned int            BitsPerPixel;
    HWND                    hwnd = (HWND) wnd;

    if(UserConfig.CreateWindow)
    {
    	hwnd = CreateWindowA(GLIDE_WND_CLASS_NAME, "Glide fake window", WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, width, height, NULL, NULL, glideDLLInt, NULL);
    	hWND_created = true;
    }
    else
    {
    	hWND_created = false;
    }

    if( hwnd == NULL )
    {
        hwnd = GetActiveWindow();
    }

    if ( hwnd == NULL)
    {
        MessageBox( NULL, "NULL window specified", "Error", MB_OK );
        exit( 1 );
    }
    // if we haven window we will create some

    mode_changed = false;
    
    hWND = hwnd;
    
		if (UserConfig.InitFullScreen)
	  {
	      SetWindowLong(hwnd, GWL_STYLE,  WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	      MoveWindow(hwnd, 0, 0, width, height, false);
	      mode_changed = SetScreenMode( width, height );
	  }
	  else
	  {
	     RECT rect;
	     rect.left = 0;
	     rect.right = width;
	     rect.top = 0;
	     rect.bottom = height;
	
	     AdjustWindowRectEx(&rect, GetWindowLong(hwnd, GWL_STYLE), GetMenu(hwnd) != NULL, GetWindowLong(hwnd, GWL_EXSTYLE));
	     MoveWindow(hwnd, x, y, x + ( rect.right - rect.left ), y + ( rect.bottom - rect.top ), true);
	  }
    
    switch(UserConfig.HideCursor)
    {
    	case 1:
    		win_hook = SetWindowsHookExA(WH_CALLWNDPROCRET /*WH_CALLWNDPROC*/, GlideHookProc, NULL, GetCurrentThreadId());
    		break;
    	case 2:
    		SetSystemCursor(hTransparent, OCR_NORMAL);
    		break;
    }
    
    //win_hook = SetWindowsHookExA(WH_CALLWNDPROCRET /*WH_CALLWNDPROC*/, GlideHookProc, NULL, GetCurrentThreadId());
    //win_hook = SetWindowsHookExA(WH_CALLWNDPROC /*WH_CALLWNDPROC*/, GlideHookProc, glideDLLInt, 0);



    hDC = GetDC( hWND );
    BitsPerPixel = GetDeviceCaps( hDC, BITSPIXEL );

    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize        = sizeof( pfd );
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = BitsPerPixel;
    pfd.cDepthBits   = BitsPerPixel;

    if ( !( PixFormat = ChoosePixelFormat( hDC, &pfd ) ) )
    {
        MessageBox( NULL, "ChoosePixelFormat() failed:  "
                    "Cannot find a suitable pixel format.", "Error", MB_OK );
        exit( 1 );
    } 

    // the window must have WS_CLIPCHILDREN and WS_CLIPSIBLINGS for this call to
    // work correctly, so we SHOULD set this attributes, not doing that yet
    if ( !SetPixelFormat( hDC, PixFormat, &pfd ) )
    {
        MessageBox( NULL, "SetPixelFormat() failed:  "
                    "Cannot set format specified.", "Error", MB_OK );
        exit( 1 );
    } 

    DescribePixelFormat( hDC, PixFormat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd );
    GlideMsg( "ColorBits	= %d\n", pfd.cColorBits );
    GlideMsg( "DepthBits	= %d\n", pfd.cDepthBits );

    if ( pfd.cDepthBits > 16 )
    {
        UserConfig.PrecisionFix = false;
    }

    hRC = wglCreateContext( hDC );
    wglMakeCurrent( hDC, hRC );
    
    setVSync();

    HDC pDC = GetDC( NULL );

    ramp_stored = GetDeviceGammaRamp( pDC, &old_ramp );

    ReleaseDC( NULL, pDC );
    return true;
}

void FinaliseOpenGLWindow( void)
{
    if ( ramp_stored )
    {
        HDC pDC = GetDC( NULL );

        BOOL res = SetDeviceGammaRamp( pDC, &old_ramp );

        ReleaseDC( NULL, pDC );
    }

    if(win_hook)
    {
        UnhookWindowsHookEx(win_hook);
        win_hook = NULL;
    }
   
    if(UserConfig.HideCursor == 2)
    {
    	SetSystemCursor(hDefault, OCR_NORMAL);
    }

    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWND, hDC );
    
    if(hWND_created)
    {
    	DestroyWindow(hWND);
    }
    hWND_created = false;
    
    hWND = NULL;
    
    if( mode_changed )
    {
        ResetScreenMode( );
    }
}

void SetGamma(float value)
{
    struct
    {
        WORD red[256];
        WORD green[256];
        WORD blue[256];
    } ramp;
    int i;
    HDC pDC = GetDC( NULL );

    for ( i = 0; i < 256; i++ )
    {
        WORD v = (WORD)( 0xffff * pow( i / 255.0, 1.0 / value ) );

        ramp.red[ i ] = ramp.green[ i ] = ramp.blue[ i ] = ( v & 0xff00 );
    }

    BOOL res = SetDeviceGammaRamp( pDC, &ramp );

    ReleaseDC( NULL, pDC );
}

bool SetScreenMode(int &xsize, int &ysize)
{
    HDC     hdc;
    FxU32   bits_per_pixel;
    bool    found;
    DEVMODE DevMode;

    hdc = GetDC( hWND );
    bits_per_pixel = GetDeviceCaps( hdc, BITSPIXEL );
    ReleaseDC( hWND, hdc );
    
    found = false;
    DevMode.dmSize = sizeof( DEVMODE );
    
    for ( int i = 0; 
          !found && EnumDisplaySettings( NULL, i, &DevMode ) != false; 
          i++ )
    {
        if ( ( DevMode.dmPelsWidth == (FxU32)xsize ) && 
             ( DevMode.dmPelsHeight == (FxU32)ysize ) && 
             ( DevMode.dmBitsPerPel == bits_per_pixel ) )
        {
            found = true;
        }
    }
    
    return ( found && ChangeDisplaySettings( &DevMode, CDS_RESET|CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL );
}

void ResetScreenMode()
{
    ChangeDisplaySettings( NULL, 0 );
}

void SwapBuffers()
{
  SwapBuffers(hDC);
}

#endif // !C_USE_SDL && WIN32
