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

#include <map>

#define OEMRESOURCE

#include "platform.h"
#include <math.h>
#include "GlOgl.h"

#include "platform/window.h"

#include <GL/gl.h>
#include "GL/glext.h"

static HDC   hDC = NULL;
static HGLRC hRC = NULL;
static HWND  hWND;
static HWND  hWND_parent;
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

static CRITICAL_SECTION draw_cs;
static DWORD draw_thread = 0;

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

BOOL RegisterOGLClass()
{
	WNDCLASSA wc      = {0};
	wc.lpfnWndProc   = DrawWndProc;
	wc.hInstance     = glideDLLInt;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = GLIDE_WND_CLASS_NAME;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	
	if(RegisterClassA(&wc) == 0)
	{
		if(GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			char buf[256];
			sprintf(buf, "RegisterClassA failure %u", GetLastError());		
    	MessageBox( NULL, buf, "Error", MB_OK );
			return FALSE;
		}
	}
	
	return TRUE;
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
	wglSwapIntervalEXT = (GLIDE_PFNWGLSWAPINTERVALEXTPROC) DGL(wglGetProcAddress)("wglSwapIntervalEXT");
	if(wglSwapIntervalEXT)
	{
		return wglSwapIntervalEXT(UserConfig.SwapInterval);
	}
	
	return FALSE;		
}

extern HCURSOR hTransparent;
extern HCURSOR hDefault;

#ifdef OGL_DEBUG_HEAVY
void APIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity,
                 GLsizei length, const GLchar* message, const void* userParam )
{
	GlideMsg( "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", 
	(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message );
}
#endif

#define WND_CLASS_TEST_NAME "gldepthtest"

static bool HasFloatDepth_set = false;
static bool HasFloatDepth_rc  = false;

bool HasFloatDepth()
{
	if(HasFloatDepth_set)
	{
		return HasFloatDepth_rc;
	}
	
	WNDCLASS wc      = {0};
	HWND win;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC dc;
	HGLRC ctx;
	int pixel_format;
	bool rc = false;

	wc.lpfnWndProc   = DefWindowProc;
	wc.hInstance     = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = WND_CLASS_TEST_NAME;
	wc.style         = CS_OWNDC;
	wc.hInstance     = GetModuleHandle(NULL);

	if( !RegisterClass(&wc) )
	{
		return false;
	}

	win = CreateWindowA(WND_CLASS_TEST_NAME, "WGL Check 1",
		WS_OVERLAPPEDWINDOW|WS_VISIBLE, 0,0,64,64,0,0, NULL, 0);

	if(!win)
	{
		return false;
	}

	dc = GetDC(win);
	pixel_format = DescribePixelFormat(dc, 0, 0, NULL);
	pixel_format = ChoosePixelFormat(dc, &pfd);
	if(pixel_format)
	{
		SetPixelFormat(dc, pixel_format, &pfd);

		ctx = DGL(wglCreateContext)(dc);
		if(ctx)
		{
			DGL(wglMakeCurrent)(dc, ctx);
			const char *exts = (const char*)DGL(glGetString)(GL_EXTENSIONS);
			if(exts)
			{
				const char *support = strstr(exts, "GL_ARB_depth_buffer_float");
				if(support != NULL)
				{
					rc = true;
				}
			}

			DGL(wglDeleteContext)(ctx);

			HasFloatDepth_set = true;
			HasFloatDepth_rc = rc;
		}
	}
	DestroyWindow(win);

	return rc;
}

bool InitialiseOpenGLWindow(FxU wnd, int x, int y, int width, int height)
{
    PIXELFORMATDESCRIPTOR   pfd;
    int                     PixFormat;
    unsigned int            BitsPerPixel;
    unsigned int            DepthBits;
    HWND                    phwnd = (HWND) wnd;
    HWND                    hwnd = NULL;
    bool                    depth64;
    
    if(!dyngl_load())
    {
    	MessageBox( NULL, "Cannot load opengl32.dll", "Error", MB_OK );
    	exit(1);
    }

    depth64 = HasFloatDepth();

    if( phwnd == NULL )
    {
      phwnd = GetActiveWindow();
    }
    
    if(phwnd == NULL && UserConfig.CreateWindow == false)
    {
    	MessageBox( NULL, "NULL window specified", "Error", MB_OK );
    	exit(1);
    }
    
    if(phwnd == NULL)
    {
    	hwnd = CreateWindowA(GLIDE_WND_CLASS_NAME, "Glide window", WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    		0, 0, width, height,
    		NULL, NULL, glideDLLInt, NULL);
    	
    	hWND_created = true;
    }
    else
    {
    	hwnd = phwnd;
    	hWND_created = false;
    }
    
    if(hwnd != NULL)
    {
			if(UserConfig.InitFullScreen)
		  {
		      SetWindowLong(hwnd, GWL_STYLE,  WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		      MoveWindow(hwnd, 0, 0, width, height, false);
		      mode_changed = SetScreenMode(width, height);
		  }
		  else if(hWND_created == false)
		  {
		     RECT rect;
		     rect.left = 0;
		     rect.right = width;
		     rect.top = 0;
		     rect.bottom = height;
		     
		     //LONG s = GetWindowLong(phwnd, GWL_STYLE) | WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		     LONG s = GetWindowLong(hwnd, GWL_STYLE) | WS_VISIBLE;
		     SetWindowLong(hwnd, GWL_STYLE,  s);
		
		     AdjustWindowRectEx(&rect, s, GetMenu(hwnd) != NULL, GetWindowLong(hwnd, GWL_EXSTYLE));
		     MoveWindow(hwnd, x, y, x + ( rect.right - rect.left ), y + ( rect.bottom - rect.top ), true);
		  }
    }
    else
    {
        MessageBox( NULL, "Windows cannot be created", "Error", MB_OK );
        exit( 1 );
    }
    
    hWND_parent = phwnd;
    hWND = hwnd;
    
    switch(UserConfig.HideCursor)
    {
    	case 1:
    		win_hook = SetWindowsHookExA(WH_CALLWNDPROCRET /*WH_CALLWNDPROC*/, GlideHookProc, NULL, GetCurrentThreadId());
    		break;
    	case 2:
    		SetSystemCursor(hTransparent, OCR_NORMAL);
    		break;
    }

    hDC = GetDC( hWND );
    BitsPerPixel = GetDeviceCaps( hDC, BITSPIXEL );

    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize        = sizeof( pfd );
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = BitsPerPixel;
    pfd.cDepthBits   = 0;

    /* depth buffer choice */
    if(UserConfig.DepthBits != 0)
    {
    	DepthBits = UserConfig.DepthBits;
    }
    else
    {
			if(depth64)
			{
				DepthBits = 64;
			}
			else
			{
				DepthBits = 24;
			}
    }

    if(DepthBits > 32)
    {
    	/* when GL_DEPTH32F_STENCIL8 is supported, it is usually fastest choice */
    	pfd.cDepthBits   = 32;
    	pfd.cStencilBits = 8;
    }
    else
    {
    	pfd.cDepthBits   = DepthBits;
    }
/*
    char buf[128];
    sprintf(buf, "OpenGL version: %d, cDepthBits: %d, %d",
    	GLVer, pfd.cDepthBits, pfd.cStencilBits);
    MessageBoxA(NULL, buf, "GL version", MB_OK);
*/
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

    if ( pfd.cDepthBits > 24 ) /* JH: 24 bit int need fixing too */
    {
        UserConfig.PrecisionFix = false;
    }

		InitializeCriticalSection(&draw_cs);
    hRC = DGL(wglCreateContext)( hDC );
    DGL(wglMakeCurrent)( hDC, hRC );
    draw_thread = GetCurrentThreadId();
    
#ifdef OGL_DEBUG_HEAVY
    PFNGLDEBUGMESSAGECALLBACKPROC p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)DGL(wglGetProcAddress)("glDebugMessageCallback");
    
    if(p_glDebugMessageCallback != NULL)
    {
    	DGL(glEnable)(GL_DEBUG_OUTPUT);
			p_glDebugMessageCallback(MessageCallback, 0);
		}
		else
		{
			MessageBox( NULL, "get glDebugMessageCallback failed!", "wglGetProcAddress", MB_OK );
		}
#endif
    
    setVSync();

    HDC pDC = GetDC( NULL );

    ramp_stored = GetDeviceGammaRamp( pDC, &old_ramp );

    ReleaseDC( NULL, pDC );
    
    if(hWND_parent)
    {
    	UpdateWindow(hWND_parent);
    }
    
    return true;
}

void ResetGLThread()
{
/*	DWORD tid = GetCurrentThreadId();
	if(tid == hRC2_thread)
	{
		wglMakeCurrent(NULL, NULL);
		hRC2_thread = 0;
	}

	if(tid == hRC3_thread)
	{
		wglMakeCurrent(NULL, NULL);
		hRC3_thread = 0;
	}*/
}

static void CloneContext()
{
	GrState state;
	grGlideGetState(&state);
	InitOpenGL();
	grGlideSetState(&state);
}

void EnterGLThread()
{
	if(hDC == NULL || hRC == NULL)
	{
		return;
	}
	
	EnterCriticalSection(&draw_cs);
	
	DWORD tid = GetCurrentThreadId();
	
	if(tid == draw_thread)
	{
		LeaveCriticalSection(&draw_cs);
		return; /* all OK */
	}
#if 0
	/* this is usualy works in other implementation (mesa os, mesa icd),
	 * but on WGL is context thread specific.
	 */
	DGL(wglMakeCurrent)(hDC, hRC);
	draw_thread = tid;
	LeaveCriticalSection(&draw_cs);	
#else
	/* flush old context, if there were any */
	DGL(wglMakeCurrent)(NULL, NULL);
		
	/* NOT OK, create new CTX and copy */
	HGLRC new_hRC = DGL(wglCreateContext)(hDC);
	DGL(wglCopyContext)(hRC, new_hRC, GL_ALL_ATTRIB_BITS);
	
	DGL(wglMakeCurrent)(hDC, new_hRC);
	
	HGLRC old_hRc = hRC;
	hRC = new_hRC;
	draw_thread = tid;
	LeaveCriticalSection(&draw_cs);	
	
	CloneContext();
	
	DGL(wglDeleteContext)(old_hRc);
#endif
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

    DGL(wglMakeCurrent)( NULL, NULL );

    DGL(wglDeleteContext)( hRC );
    ReleaseDC( hWND, hDC );

    hRC = NULL;
    hDC = NULL;
    
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

void Activate3DWindow()
{
#ifdef OGL_NOTDONE
    GlideMsg( "Activate3DWindow()\n" );
#endif
}

void Deactivate3DWindow()
{
#ifdef OGL_NOTDONE
    GlideMsg( "Deactivate3DWindow()\n" );
#endif
}

void SwapBuffers()
{
  SwapBuffers(hDC);
}

/*
 * JH: this doesn't get real vretrace period, only simulating it's time
 * with system timer 
 * Edit: GetTickCount has very low resolution, using solution from Wine
 */
int GetVRetrace()
{
	LONGLONG freq_per_frame, freq_per_line;
	LARGE_INTEGER counter, freq_per_sec;
	unsigned refresh_rate = 60, height = 640;
	int scanline;

	if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&freq_per_sec))
		return -1;

	freq_per_frame = freq_per_sec.QuadPart / refresh_rate;

	/* Assume 20 scan lines in the vertical blank. */
	freq_per_line = freq_per_frame / (height + 20);
	scanline = (counter.QuadPart % freq_per_frame) / freq_per_line;

	if (scanline < height)
	{
		// visible
		return 0;
	}
	else
	{
		// vblank
		return 1;
	}
}

typedef FxBool (FX_CALL *fxSplashInit_f)(FxU32 hWnd, FxU32 screenWidth,
	FxU32 screenHeight, FxU32 numColBuf, FxU32 numAuxBuf, GrColorFormat_t colorFormat);

typedef void (FX_CALL *fxSplash_f)(float x, float y, float w, float h, FxU32 frameNumber);
typedef void (FX_CALL *fxSplashShutdown_f)(void);

#ifdef GLIDE3
# define SPLASH_DLL "3dfxSpl3.dll"
#else
# define SPLASH_DLL "3dfxSpl2.dll"
#endif

BOOL ExternalSplash()
{
	HMODULE splashdll = LoadLibraryA(SPLASH_DLL);
	BOOL rc = FALSE;
	
	if(splashdll != NULL)
	{
		fxSplashInit_f fxSplashInit_p = (fxSplashInit_f)GetProcAddress(splashdll, "_fxSplashInit@24");
		fxSplash_f fxSplash_p = (fxSplash_f)GetProcAddress(splashdll, "_fxSplash@20");
		fxSplashShutdown_f fxSplashShutdown_p = (fxSplashShutdown_f)GetProcAddress(splashdll, "_fxSplashShutdown@0");
		
		if(fxSplashInit_p != NULL && fxSplash_p != NULL)
		{
			const char *ev = getenv("FX_GLIDE_NO_SPLASH");
			if(ev != NULL && atoi(ev) > 0)
			{
				/* skip play */
				rc = TRUE;
			}
			else
			{
				if(fxSplashInit_p(0, (float)Glide.WindowWidth, (float)Glide.WindowHeight, 2, 1, GR_COLORFORMAT_ABGR))
				{
					fxSplash_p(0, 0, (float)Glide.WindowWidth, (float)Glide.WindowHeight, 0);
				
					if(fxSplashShutdown_p != NULL)
					{
						fxSplashShutdown_p();
					}
					rc = TRUE;
				}
			}
		}
		
		FreeLibrary(splashdll);
	}
	
	return rc;
}

#endif // !C_USE_SDL && WIN32
