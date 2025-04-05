//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                      Utility File
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originally made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*               Linux version by Simon White
//**************************************************************

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "wrapper_config.h"
#include "GlOgl.h"
#include "Glextensions.h"
#include "OGLTables.h"

#include "platform.h"
#include "platform/window.h"
#include "platform/clock.h"

// Configuration Variables
ConfigStruct    UserConfig;
ConfigStruct    InternalConfig;

// Extern prototypes
extern unsigned long    NumberOfErrors;

const char *reg_config_path = "SOFTWARE\\OpenGlide";
HKEY        reg_config_key  = HKEY_LOCAL_MACHINE;

static CRITICAL_SECTION glide_cs;

// Functions

VARARGDECL(void) GlideMsg( const char *szString, ... )
{
    EnterCriticalSection(&glide_cs);
    
    va_list Arg;
    va_start( Arg, szString );

    if(InternalConfig.Logging)
    {
      FILE *fHandle = fopen( GLIDEFILE, "at" );
      if ( !fHandle )
      {
          return;
   	  }
   	  fprintf(fHandle, "[%X]", GetCurrentThreadId());
   	  
   	  vfprintf( fHandle, szString, Arg );
   	  fflush( fHandle );
   	  fclose( fHandle );
    }
    va_end( Arg );
   
   LeaveCriticalSection(&glide_cs);
}

VARARGDECL(void) Error( const char *szString, ... )
{
    EnterCriticalSection(&glide_cs);
    
    va_list Arg;
    va_start( Arg, szString );

    if(InternalConfig.Logging)
    {
      if ( NumberOfErrors == 0 )
      {
          GenerateErrorFile( );
      }

      FILE *fHandle = fopen( ERRORFILE, "at" );
      if ( ! fHandle )
      {
          return;
      }
      vfprintf( fHandle, szString, Arg );
      
      fflush( fHandle );
      fclose( fHandle );
    }

    va_end( Arg );
    NumberOfErrors++;
    
    LeaveCriticalSection(&glide_cs);
}

void GLErro( const char *Funcao )
{
    GLenum Erro = DGL(glGetError)( );

    if ( Erro != GL_NO_ERROR )
    {
        Error( "%s: OpenGLError = %d\n", Funcao, Erro );
        //asm volatile (".byte 0xCC" );
    }    
}

void ConvertColorB( GrColor_t GlideColor, FxU8 &R, FxU8 &G, FxU8 &B, FxU8 &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        R = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        B = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        B = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        R = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        B = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (FxU8)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (FxU8)( ( GlideColor & 0x00FF0000 ) >> 16 );
        R = (FxU8)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (FxU8)( ( GlideColor & 0x000000FF )       );
        break;
    }
}

void ConvertColor4B( GrColor_t GlideColor, FxU32 &C )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        C = GlideColor;
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        C = ( ( GlideColor & 0xFF00FF00 ) |
              ( ( GlideColor & 0x00FF0000 ) >> 16 ) |
              ( ( GlideColor & 0x000000FF ) <<  16 ) );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        C = ( ( ( GlideColor & 0x00FFFFFF ) << 8 ) |
              ( ( GlideColor & 0xFF000000 ) >> 24 ) );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        C = ( ( ( GlideColor & 0xFF000000 ) >> 24 ) |
              ( ( GlideColor & 0x00FF0000 ) >>  8 ) |
              ( ( GlideColor & 0x0000FF00 ) <<  8 ) |
              ( ( GlideColor & 0x000000FF ) << 24 ) );
        break;
    }
}

GrColor_t ConvertConstantColor( float R, float G, float B, float A )
{
    GrColor_t r = (GrColor_t) R;
    GrColor_t g = (GrColor_t) G;
    GrColor_t b = (GrColor_t) B;
    GrColor_t a = (GrColor_t) A;

    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        return ( r << 24 ) | ( g << 16 ) | ( b << 8 ) | a;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        return ( b << 24 ) | ( g << 16 ) | ( r << 8 ) | a;
    }

    return 0;
}

void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        B = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (float)( ( GlideColor & 0xFF000000 ) >> 24 ) * D1OVER255;
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 ) * D1OVER255;
        R = (float)( ( GlideColor & 0x0000FF00 ) >>  8 ) * D1OVER255;
        A = (float)( ( GlideColor & 0x000000FF )       ) * D1OVER255;
        break;
    }
}

void ConvertColorF2( GrColor_t GlideColor, float &R, float &G, float &B, float &A )
{
    switch ( Glide.State.ColorFormat )
    {
    case GR_COLORFORMAT_ARGB:   //0xAARRGGBB
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 );
        R = (float)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 );
        B = (float)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_ABGR:   //0xAABBGGRR
        A = (float)( ( GlideColor & 0xFF000000 ) >> 24 );
        B = (float)( ( GlideColor & 0x00FF0000 ) >> 16 );
        G = (float)( ( GlideColor & 0x0000FF00 ) >>  8 );
        R = (float)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_RGBA:   //0xRRGGBBAA
        R = (float)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 );
        B = (float)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (float)( ( GlideColor & 0x000000FF )       );
        break;

    case GR_COLORFORMAT_BGRA:   //0xBBGGRRAA
        B = (float)( ( GlideColor & 0xFF000000 ) >> 24 );
        G = (float)( ( GlideColor & 0x00FF0000 ) >> 16 );
        R = (float)( ( GlideColor & 0x0000FF00 ) >>  8 );
        A = (float)( ( GlideColor & 0x000000FF )       );
        break;
    }
}

//*************************************************
FxU32 GetTexSize( const int Lod, const int aspectRatio, const int format )
{
    /*
    ** If the format is one of these:
    ** GR_TEXFMT_RGB_332
    ** GR_TEXFMT_YIQ_422
    ** GR_TEXFMT_ALPHA_8
    ** GR_TEXFMT_INTENSITY_8
    ** GR_TEXFMT_ALPHA_INTENSITY_44
    ** GR_TEXFMT_P_8
    ** Reduces the size by 2
    */
#ifndef GLIDE3
    return nSquareLod[ format > GR_TEXFMT_RSVD1 ? 1 : 0 ][ aspectRatio ][ Lod ];
#else
		return nSquareLod[ format > GR_TEXFMT_RSVD1 ? 1 : 0 ][ 3 - aspectRatio ][ 8 - Lod ];
#endif
}

static char * FindConfig( const char *IniFile, const char *IniConfig )
{
    // Cannot return pointer to local buffer, unless
    // static.
    static char Buffer1[ 256 ];
    char    * EqLocation, 
            * Find;
    FILE    * file;

    Find = NULL;
    file = fopen( IniFile, "r" );

    while ( fgets( Buffer1, 255, file ) != NULL )
    {
        if ( ( EqLocation = strchr( Buffer1, '=' ) ) != NULL )
        {       
            if ( !strncmp( Buffer1, IniConfig, EqLocation - Buffer1 ) )
            {
                Find = EqLocation + 1;
                if ( Find[ strlen( Find ) - 1 ] == '\n' )
                {
                    Find[ strlen( Find ) - 1 ] = '\0';
                }
                break;
            }
        }
    }

    fclose( file );

    return Find;
}

static char *FindConfigReg(const char *exe, const char *IniConfig)
{
	char keybuf[MAX_PATH];
	static char retbuf[MAX_PATH];
	HKEY hKey;
	DWORD type;
	DWORD size;
	bool rc = false;
	
	strcpy(keybuf, reg_config_path);
	strcat(keybuf, "\\");
	strcat(keybuf, exe);
	
	LSTATUS lResult = RegOpenKeyEx(reg_config_key, keybuf, 0, KEY_READ, &hKey);
	if(lResult == ERROR_SUCCESS)
	{
		size = MAX_PATH;
		lResult = RegQueryValueExA(hKey, IniConfig, NULL, &type, (LPBYTE)keybuf, &size);
		if(lResult == ERROR_SUCCESS)
	  {
	  	switch(type)
	   	{
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
				{
					memcpy(retbuf, keybuf, size-1);
					keybuf[size] = '\0';
					rc = true;
					break;
				}
				case REG_DWORD:
				{
					DWORD temp_dw = *((LPDWORD)keybuf);
					sprintf(retbuf, "%lu", temp_dw);
					rc = true;
					break;
				}
			}
		}
			
		RegCloseKey(hKey);
	}
	
	if(!rc)
	{
		strcpy(keybuf, reg_config_path);
		strcat(keybuf, "\\global");
		
		LSTATUS lResult = RegOpenKeyEx(reg_config_key, keybuf, 0, KEY_READ, &hKey);
		if(lResult == ERROR_SUCCESS)
		{
			size = MAX_PATH;
			lResult = RegQueryValueExA(hKey, IniConfig, NULL, &type, (LPBYTE)keybuf, &size);
			if(lResult == ERROR_SUCCESS)
		  {
		  	switch(type)
		   	{
					case REG_SZ:
					case REG_MULTI_SZ:
					case REG_EXPAND_SZ:
					{
						memcpy(retbuf, keybuf, size-1);
						keybuf[size] = '\0';
						rc = true;
						break;
					}
					case REG_DWORD:
					{
						DWORD temp_dw = *((LPDWORD)keybuf);
						sprintf(retbuf, "%lu", temp_dw);
						rc = true;
						break;
					}
				}
			}
				
			RegCloseKey(hKey);
		}
	}
	
	if(rc) return retbuf;
	
	return NULL;
}

#ifdef VBOX_GLIDE_WITH_IPRT 
int vbox_access(char const* FileName, int AccessMode)
{
	FILE *fp = fopen(FileName, "r");
	if(fp != NULL)
	{
		fclose(fp);
		return 0;
	}
	return -1;
}
#else
#  define vbox_access access
#endif

void GetOptions( void )
{
    FILE        * IniFile;
    char        * Pointer;
    char        * ExeName;
    char        Path[MAX_PATH];
    
    /* config default */
    memset(&UserConfig, 0, sizeof(UserConfig));

    #define OGL_CFG_BOOL(_name, _def, _des)              UserConfig._name = _def;
    #define OGL_CFG_FLOAT(_name, _def, _min, _max, _dec) UserConfig._name = _def;
    #define OGL_CFG_INT(_name, _def, _min, _max, _dec)   UserConfig._name = _def;

		#include "GLconf.h"
		
		#undef OGL_CFG_BOOL
		#undef OGL_CFG_FLOAT
		#undef OGL_CFG_INT
		
		/* read config from registry */
    GetModuleFileNameA(NULL, Path, MAX_PATH-1);
    Path[MAX_PATH-1] = '\0';
    ExeName = strrchr(Path, '\\');
    if(strrchr(Path, '/') > ExeName)
    {
    	ExeName = strrchr(Path, '/');
    }
    
    if(ExeName != NULL)
    {
    	ExeName++; /* strip leading '\' or '/' */
    	if(strlen(ExeName))
    	{
    		#define OGL_CFG_BOOL(_name, _def, _des) \
    			if((Pointer = FindConfigReg(ExeName, #_name)) != NULL) UserConfig._name = atoi(Pointer) ? true : false;
    	
    		#define OGL_CFG_INT(_name, _def, _min, _max, _des) \
    			if((Pointer = FindConfigReg(ExeName, #_name)) != NULL) UserConfig._name = atoi(Pointer);
    	
    		#define OGL_CFG_FLOAT(_name, _def, _min, _max, _des) \
    			if((Pointer = FindConfigReg(ExeName, #_name)) != NULL) UserConfig._name = atof(Pointer);
    				
				#include "GLconf.h"
		
				#undef OGL_CFG_BOOL
				#undef OGL_CFG_FLOAT
				#undef OGL_CFG_INT
    	}
    }

    /* read config name from registry */
    strcpy( Path, INIFILE ); 

    GlideMsg( "Configuration file is %s\n", Path );
    
    if ( vbox_access( Path, 0 ) == -1 )
    {
    	const char * createConfigStr = getenv("OGL_CREATE_CONFIG");
    	bool createConfig = false;
    	if(createConfigStr != NULL && strlen(createConfigStr) > 0)
    	{
    		if(strcmp(createConfigStr, "0") != 0 && stricmp(createConfigStr, "off"))
    		{
    			createConfig = true;
    		}
    	}
    	
    	if(createConfig)
    	{
        IniFile = fopen( Path, "w" );
        fprintf( IniFile, "Configuration File for OpenGLide\n\n" );
        fprintf( IniFile, "Version=%s\n\n", OpenGLideVersion );
        fprintf( IniFile, "[Options]\n" );

				#define OGL_CFG_BOOL(_name, _def, _des) \
					if(sizeof(_des) > 1){fprintf(IniFile, "; BOOL: %s\n", _des);} \
					else{fprintf(IniFile, "; BOOL, default: %d\n", _def ? 1 : 0);} \
					fprintf(IniFile, "%s=%d\n\n", #_name, _def ? 1 : 0);
				
				#define OGL_CFG_FLOAT(_name, _def, _min, _max, _des) \
					if(sizeof(_des) > 1){fprintf(IniFile, "; FLOAT: %s\n", _des);} \
					else{fprintf(IniFile, "; FLOAT, default: %.2f, min: %.2f, max: %.2f\n", (double)_def, (double)_min, (double)_max);} \
					fprintf(IniFile, "%s=%.2f\n\n", #_name, (double)_def);
				
				#define OGL_CFG_INT(_name, _def, _min, _max, _des) \
					if(sizeof(_des) > 1){fprintf(IniFile, "; INT: %s\n", _des);} \
					else{fprintf(IniFile, "; INT, default: %d, min: %d, max: %d\n", _def, _min, _max);} \
					fprintf(IniFile, "%s=%d\n\n", #_name, _def);
				
				#include "GLconf.h"
		
				#undef OGL_CFG_BOOL
				#undef OGL_CFG_FLOAT
				#undef OGL_CFG_INT
					
        fclose(IniFile);
      }
    } // !access
    else
    {
    	#define OGL_CFG_BOOL(_name, _def, _des) \
    		if((Pointer = FindConfig(Path, #_name))) UserConfig._name = atoi(Pointer) ? true : false;
    	
    	#define OGL_CFG_INT(_name, _def, _min, _max, _des) \
    		if((Pointer = FindConfig(Path, #_name))) UserConfig._name = atoi(Pointer);
    	
    	#define OGL_CFG_FLOAT(_name, _def, _min, _max, _des) \
    		if((Pointer = FindConfig(Path, #_name))) UserConfig._name = atof(Pointer);
    	
			#include "GLconf.h"
		
			#undef OGL_CFG_BOOL
			#undef OGL_CFG_FLOAT
			#undef OGL_CFG_INT
    }
}


FX_ENTRY void FX_CALL setConfig(FxU32 flags)
{
    UserConfig.EnableMipMaps = ((flags & WRAPPER_FLAG_MIPMAPS) != 0);
}


bool ClearAndGenerateLogFile( void )
{
    FILE    * GlideFile;
    char    tmpbuf[ 128 ];
    
    InitializeCriticalSection(&glide_cs);

    if(!InternalConfig.Logging)
    {
    	return true;
    }
    	
    remove( ERRORFILE );
    GlideFile = fopen( GLIDEFILE, "w" );
    if ( ! GlideFile )
    {
        return false;
    }
    fclose( GlideFile );

    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "OpenGLide Log File\n" );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "***** OpenGLide %s *****\n", OpenGLideVersion );
    GlideMsg( OGL_LOG_SEPARATE );
    _strdate( tmpbuf );
    GlideMsg( "Date: %s\n", tmpbuf );
    _strtime( tmpbuf );
    GlideMsg( "Time: %s\n", tmpbuf );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( OGL_LOG_SEPARATE );
    ClockFreq = ClockFrequency( );
    GlideMsg( "Clock Frequency: %-4.2f Mhz\n", ClockFreq / 1000000.0f );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( OGL_LOG_SEPARATE );

    return true;
}

void CloseLogFile( void )
{
    char tmpbuf[ 128 ];
    GlideMsg( OGL_LOG_SEPARATE );
    _strtime( tmpbuf );
    GlideMsg( "Time: %s\n", tmpbuf );
    GlideMsg( OGL_LOG_SEPARATE );

#ifdef OGL_DEBUG
    Fps = (float) Frame * ClockFreq / FpsAux;
    GlideMsg( "FPS = %f\n", Fps );
    GlideMsg( OGL_LOG_SEPARATE );
#endif
}

bool GenerateErrorFile( void )
{
    char    tmpbuf[ 128 ];
    FILE    * ErrorFile;
    
    ErrorFile = fopen( ERRORFILE, "w");
    if( !ErrorFile )
    {
        return false;
    }
    fclose( ErrorFile );

    NumberOfErrors++;
    Error(  OGL_LOG_SEPARATE );
    Error(  "OpenGLide Error File\n");
    Error(  OGL_LOG_SEPARATE );
    _strdate( tmpbuf );
    Error(  "Date: %s\n", tmpbuf );
    _strtime( tmpbuf );
    Error(  "Time: %s\n", tmpbuf );
    Error(  OGL_LOG_SEPARATE );
    Error(  OGL_LOG_SEPARATE );

    return true;
}

// Detect if Processor has MMX Instructions
int DetectMMX( void )
{
#ifdef HAVE_MMX
    FxU32 Result;

#ifdef _MSC_VER
    __asm
    {
        push EAX
        push EDX
        mov EAX, 1
        CPUID
        mov Result, EDX
        pop EDX
        pop EAX
    }
    return Result & 0x00800000;
#endif

#ifdef __GNUC__
	return __builtin_cpu_supports("mmx");

#endif

#else
    return 0;
#endif
}
