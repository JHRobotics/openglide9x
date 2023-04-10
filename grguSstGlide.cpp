
#include "platform.h"
#include "GlOgl.h"
#include "GLRender.h"
#include "PGTexture.h"
#include "OGLTables.h"

static bool InterpretScreenResolution(GrScreenResolution_t eResolution, FxU32 &width, FxU32 &height)
{
    if ( eResolution > GR_RESOLUTION_400x300 )
        return false;

    static const FxU32 windowDimensions[16][2] =
    {
        {  320,  200 }, // GR_RESOLUTION_320x200
        {  320,  240 }, // GR_RESOLUTION_320x240
        {  400,  256 }, // GR_RESOLUTION_400x256
        {  512,  384 }, // GR_RESOLUTION_512x384
        {  640,  200 }, // GR_RESOLUTION_640x200
        {  640,  350 }, // GR_RESOLUTION_640x350
        {  640,  400 }, // GR_RESOLUTION_640x400
        {  640,  480 }, // GR_RESOLUTION_640x480
        {  800,  600 }, // GR_RESOLUTION_800x600
        {  960,  720 }, // GR_RESOLUTION_960x720
        {  856,  480 }, // GR_RESOLUTION_856x480:
        {  512,  256 }, // GR_RESOLUTION_512x256:
        { 1024,  768 }, // GR_RESOLUTION_1024x768:
        { 1280, 1024 }, // GR_RESOLUTION_1280x1024:
        { 1600, 1200 }, // GR_RESOLUTION_1600x1200:
        {  400,  300 }  // GR_RESOLUTION_400x300:
    };

    width  = windowDimensions[eResolution][0];
    height = windowDimensions[eResolution][1];
    return true;
}

static bool InterpretScreenRefresh(GrScreenRefresh_t eRefresh, GLuint &refresh)
{
    if ( eRefresh > GR_REFRESH_120Hz )
        return false;

    static const GLuint windowRefresh[9] =
    {
         60, // GR_REFRESH_60Hz
         70, // GR_REFRESH_70Hz
         72, // GR_REFRESH_72Hz
         75, // GR_REFRESH_75Hz
         80, // GR_REFRESH_80Hz
         90, // GR_REFRESH_90Hz
        100, // GR_REFRESH_100Hz
         85, // GR_REFRESH_85Hz
        120  // GR_REFRESH_120Hz
    };
    
    refresh = windowRefresh[eRefresh];
    return true;
}

static FxU32 int_log2 (FxU32 val) {
    FxU32 log = 0;
    while ((val >>= 1) != 0)
        log++;
    return log;
}


//*************************************************
//* Returns the current Glide Version
//*************************************************
FX_ENTRY void FX_CALL
grGlideGetVersion( char version[80] )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideGetVersion( --- )\n" );
#endif
    sprintf( version, "Glide 2.45 - OpenGLide %s", OpenGLideVersion );
}

//*************************************************
//* Initializes what is needed
//*************************************************
FX_ENTRY void FX_CALL
grGlideInit( void )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideInit( )\n" );
#endif

    if ( OpenGL.GlideInit )
    {
        grGlideShutdown( );
    }

    ZeroMemory( &Glide, sizeof( GlideStruct ) );
    ZeroMemory( &OpenGL, sizeof( OpenGLStruct ) );

    Glide.ActiveVoodoo      = 0;
    Glide.State.VRetrace    = FXTRUE;

    ExternErrorFunction = NULL;

#ifdef OGL_DEBUG
    RDTSC( FinalTick );
    RDTSC( InitialTick );
    Fps = FpsAux = Frame = 0;
#endif

    GetOptions ();
    OpenGL.GlideInit = true;

    RenderInitialize( );

    Glide.TextureMemory = UserConfig.TextureMemorySize * 1024 * 1024;

    Textures = new PGTexture( Glide.TextureMemory );
    if ( Textures == NULL )
    {
        Error( "Cannot allocate enough memory for Texture Buffer in User setting, using default" );
    }

    Glide.TexMemoryMaxPosition   = (FxU32)Glide.TextureMemory;
    InternalConfig.NoSplash      = UserConfig.NoSplash;
    InternalConfig.ShamelessPlug = UserConfig.ShamelessPlug;
}

//*************************************************
//* Finishes everything
//*************************************************
FX_ENTRY void FX_CALL
grGlideShutdown( void )
{
    if ( !OpenGL.GlideInit )
    {
        return;
    }

    OpenGL.GlideInit = false;

#ifdef OGL_DEBUG
    RDTSC( FinalTick );
#endif
#ifdef OGL_DONE
    GlideMsg( "grGlideShutdown()\n" );
#endif

    grSstWinClose( );

    RenderFree( );
    delete Textures;
}

//*************************************************
//* Sets all Glide State Variables
//*************************************************
FX_ENTRY void FX_CALL
grGlideSetState( const GrState *state )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grGlideSetState( --- )\n" );
#endif

    GlideState StateTemp;

    CopyMemory( &StateTemp, state, sizeof( GlideState ) );

    Glide.State.ColorFormat = StateTemp.ColorFormat;

    grRenderBuffer( StateTemp.RenderBuffer );
    grDepthBufferMode( StateTemp.DepthBufferMode );
    grDepthBufferFunction( StateTemp.DepthFunction );
    grDepthMask( StateTemp.DepthBufferWritting );
    grDepthBiasLevel( StateTemp.DepthBiasLevel );
    grDitherMode( StateTemp.DitherMode );
    grChromakeyValue( StateTemp.ChromakeyValue );
    grChromakeyMode( StateTemp.ChromaKeyMode );
    grAlphaTestReferenceValue( StateTemp.AlphaReferenceValue );
    grAlphaTestFunction( StateTemp.AlphaTestFunction );
    grColorMask( StateTemp.ColorMask, StateTemp.AlphaMask );
    grConstantColorValue( StateTemp.ConstantColorValue );
    grFogColorValue( StateTemp.FogColorValue );
    grFogMode( StateTemp.FogMode );
    grCullMode( StateTemp.CullMode );
    grTexClampMode( GR_TMU0, StateTemp.SClampMode, StateTemp.TClampMode );
    grTexFilterMode( GR_TMU0, StateTemp.MinFilterMode, StateTemp.MagFilterMode );
    grTexMipMapMode( GR_TMU0, StateTemp.MipMapMode, StateTemp.LodBlend );
    grColorCombine( StateTemp.ColorCombineFunction, StateTemp.ColorCombineFactor, 
                    StateTemp.ColorCombineLocal, StateTemp.ColorCombineOther, StateTemp.ColorCombineInvert );
    grAlphaCombine( StateTemp.AlphaFunction, StateTemp.AlphaFactor, StateTemp.AlphaLocal, StateTemp.AlphaOther, StateTemp.AlphaInvert );
    grTexCombine( GR_TMU0, StateTemp.TextureCombineCFunction, StateTemp.TextureCombineCFactor,
                  StateTemp.TextureCombineAFunction, StateTemp.TextureCombineAFactor,
                  StateTemp.TextureCombineRGBInvert, StateTemp.TextureCombineAInvert );
    grAlphaBlendFunction( StateTemp.AlphaBlendRgbSf, StateTemp.AlphaBlendRgbDf, StateTemp.AlphaBlendAlphaSf, StateTemp.AlphaBlendAlphaDf );
    grClipWindow( StateTemp.ClipMinX, StateTemp.ClipMinY, StateTemp.ClipMaxX, StateTemp.ClipMaxY );
//  grSstOrigin( StateTemp.OriginInformation );
//  grTexSource( GR_TMU0, StateTemp.TexSource.StartAddress, StateTemp.TexSource.EvenOdd, &StateTemp.TexSource.Info );
}

//*************************************************
//* Gets all Glide State Variables
//*************************************************
FX_ENTRY void FX_CALL
grGlideGetState( GrState *state )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grGlideGetState( --- )\n" );
#endif

    CopyMemory( state, &Glide.State, sizeof( GlideState ) );
}

//*************************************************
FX_ENTRY void FX_CALL
grGlideShamelessPlug( const FxBool on )
{
#ifdef OGL_DONE
    GlideMsg( "grGlideShamelessPlug( %d )\n", on );
#endif
    if (!UserConfig.ShamelessPlug)
        InternalConfig.ShamelessPlug = on;
}

//*************************************************
//* Returns the number of Voodoo Boards Installed
//*************************************************
FX_ENTRY FxBool FX_CALL
grSstQueryBoards( GrHwConfiguration *hwConfig )
{
#ifdef OGL_DONE
    GlideMsg( "grSstQueryBoards( --- )\n" );
#endif

    ZeroMemory( hwConfig, sizeof(GrHwConfiguration) );
    hwConfig->num_sst = 1;

    return FXTRUE;
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstWinOpen(   FxU hwnd,
                GrScreenResolution_t res,
                GrScreenRefresh_t ref,
                GrColorFormat_t cformat,
                GrOriginLocation_t org_loc,
                int num_buffers,
                int num_aux_buffers )
{
    if ( OpenGL.WinOpen )
    {
        grSstWinClose( );
    }

#ifdef OGL_DONE
    GlideMsg( "grSstWinOpen( %d, %d, %d, %d, %d, %d, %d )\n",
        hwnd, res, ref, cformat, org_loc, num_buffers, num_aux_buffers );
#endif

    if (!InterpretScreenResolution(res, Glide.WindowWidth, Glide.WindowHeight))
    {
#ifdef OGL_DEBUG
        Error( "grSstWinOpen: res = GR_RESOLUTION_NONE\n" );
#endif
        return FXFALSE;
    }

    if (!InterpretScreenRefresh(ref, OpenGL.Refresh))
    {
#ifdef OGL_DEBUG
        Error( "grSstWinOpen: Refresh Incorrect\n" );
#endif
        return FXFALSE;
    }

    // Set the size of the OpenGL window (might be different from Glide window size)
    if (UserConfig.Resolution == 0)
    {
        // Use the resolution requested by the game
        OpenGL.WindowWidth = Glide.WindowWidth;
        OpenGL.WindowHeight = Glide.WindowHeight;
    }
    else if (UserConfig.Resolution <= 16)
    {
        // multiply the original size by the resolution factor
        OpenGL.WindowWidth = Glide.WindowWidth * UserConfig.Resolution;
        OpenGL.WindowHeight = Glide.WindowHeight * UserConfig.Resolution;
    }
    else
    {
        // override the resolution
        OpenGL.WindowWidth = UserConfig.Resolution;
        // Glide games have a fixed 4/3 aspect ratio
        OpenGL.WindowHeight = UserConfig.Resolution * 3 / 4;
    }

    Glide.WindowTotalPixels = Glide.WindowWidth * Glide.WindowHeight;
    OpenGL.WaitSignal = (int)( 1000 / OpenGL.Refresh );

    // Initing OpenGL Window
    if ( !InitWindow( hwnd ) )
    {
        return FXFALSE;
    }

    OpenGL.ClipMinX = 0;
    OpenGL.ClipMinY = 0;
    OpenGL.ClipMaxX = OpenGL.WindowWidth;
    OpenGL.ClipMaxY = OpenGL.WindowHeight;
    OpenGL.WindowTotalPixels = (FxU32)( OpenGL.WindowWidth * OpenGL.WindowHeight );

    Glide.State.ColorFormat = cformat;
    Glide.NumBuffers        = num_buffers;
    Glide.AuxBuffers        = num_aux_buffers;

    // Initializing Glide and OpenGL
    InitOpenGL( );

    OpenGL.tmpBuf = new FxU32[ OpenGL.WindowTotalPixels ];
    Glide.SrcBuffer.Address = new FxU16[ Glide.WindowTotalPixels ];
    Glide.DstBuffer.Address = new FxU16[ Glide.WindowTotalPixels ];
    Glide.LFBTextureSize = 2 << int_log2(Glide.WindowWidth > Glide.WindowHeight ? (Glide.WindowWidth-1) : (Glide.WindowHeight-1));

    glGenTextures( 1, &Glide.LFBTexture );
    glBindTexture( GL_TEXTURE_2D, Glide.LFBTexture );
    if ( OpenGL.WindowTotalPixels != Glide.WindowTotalPixels ) {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    } else {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    }
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, Glide.LFBTextureSize, Glide.LFBTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Just checking
    if ( ( !Glide.SrcBuffer.Address ) || ( !Glide.DstBuffer.Address ) || ( !OpenGL.tmpBuf ) )
    {
        Error( "Could NOT allocate sufficient memory for Buffers... Sorry\n" );
        exit( -1 );
    }

    ZeroMemory( Glide.SrcBuffer.Address, Glide.WindowTotalPixels * 2 );

#define BLUE_SCREEN     (0x07FF)
    for( FxU32 i = 0; i < Glide.WindowTotalPixels; i++ )
    {
        Glide.DstBuffer.Address[i] = BLUE_SCREEN;
    }

#ifdef OGL_DONE
    GlideMsg( "----Start of grSstWinOpen()\n" );
#endif
    // All should be disabled
    //depth buffering, fog, chroma-key, alpha blending, alpha testing
    grSstOrigin( org_loc );
    grTexClampMode( 0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP );
    grTexMipMapMode( 0, GR_MIPMAP_DISABLE, FXFALSE );
    grTexFilterMode( 0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR );
    grChromakeyMode( GR_CHROMAKEY_DISABLE );
    grFogMode( GR_FOG_DISABLE );
    grCullMode( GR_CULL_DISABLE );
    grRenderBuffer( GR_BUFFER_BACKBUFFER );
    grAlphaTestFunction( GR_CMP_ALWAYS );
    grDitherMode( GR_DITHER_4x4 );
    grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                    GR_COMBINE_FACTOR_ONE,
                    GR_COMBINE_LOCAL_ITERATED,
                    GR_COMBINE_OTHER_ITERATED,
                    FXFALSE );
    grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                    GR_COMBINE_FACTOR_ONE,
                    GR_COMBINE_LOCAL_NONE,
                    GR_COMBINE_OTHER_CONSTANT,
                    FXFALSE );
    grTexCombine( GR_TMU0,GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
    grAlphaControlsITRGBLighting( FXFALSE );
    grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    grColorMask( FXTRUE, FXFALSE );
    grDepthMask( FXFALSE );
    grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
    grDepthBufferFunction( GR_CMP_LESS );

    //(chroma-key value, alpha test reference, constant depth value,
    //constant alpha value, etc.) and pixel rendering statistic counters 
    //are initialized to 0x00.
    grChromakeyValue( 0x00 );
    grAlphaTestReferenceValue( 0x00 );
    grDepthBiasLevel( 0x00 );
    grFogColorValue( 0x00 );
    grConstantColorValue( 0xFFFFFFFF );
    grClipWindow( 0, 0, Glide.WindowWidth, Glide.WindowHeight );
//  grGammaCorrectionValue( 1.6f );
    grHints( GR_HINT_STWHINT, 0 );

#ifdef OGL_DONE
    GlideMsg( "----End of grSstWinOpen()\n" );
#endif

    OpenGL.WinOpen = true;

#ifdef OPENGL_DEBUG
    GLErro( "grSstWinOpen" );
#endif

    glFinish( );

    // Show the splash screen? (code copied from the linux driver src)
    if (InternalConfig.NoSplash == false)
    {
        grSplash(0.0f, 0.0f, 
            static_cast<float>(Glide.WindowWidth),
            static_cast<float>(Glide.WindowHeight),
            0);
        // The splash screen is displayed once, and because the
        // internal config is reinitialised each time grWinOpen()
        // is called, the value must be reset in the user config
        InternalConfig.NoSplash = true;
    }

    return FXTRUE;
}

//*************************************************
//* Close the graphics display device
//*************************************************
FX_ENTRY void FX_CALL
grSstWinClose( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstWinClose()\n" );
#endif
    if ( ! OpenGL.WinOpen )
    {
        return;
    }

    OpenGL.WinOpen = false;

#ifdef OGL_DEBUG
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "** Debug Information **\n" );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "MaxTriangles in Frame = %d\n", OGLRender.MaxTriangles );
    GlideMsg( "MaxTriangles in Sequence = %d\n", OGLRender.MaxSequencedTriangles );
    GlideMsg( "MaxZ = %f\nMinZ = %f\n", OGLRender.MaxZ, OGLRender.MinZ );
    GlideMsg( "MaxX = %f\nMinX = %f\nMaxY = %f\nMinY = %f\n", 
              OGLRender.MaxX, OGLRender.MinX, OGLRender.MaxY, OGLRender.MinY );
    GlideMsg( "MaxS = %f\nMinS = %f\nMaxT = %f\nMinT = %f\n", 
              OGLRender.MaxS, OGLRender.MinS, OGLRender.MaxT, OGLRender.MinT );
    GlideMsg( "MaxF = %f\nMinF = %f\n", OGLRender.MaxF, OGLRender.MinF );
    GlideMsg( "MaxR = %f\nMinR = %f\n", OGLRender.MaxR, OGLRender.MinR );
    GlideMsg( "MaxG = %f\nMinG = %f\n", OGLRender.MaxG, OGLRender.MinG );
    GlideMsg( "MaxB = %f\nMinB = %f\n", OGLRender.MaxB, OGLRender.MinR );
    GlideMsg( "MaxA = %f\nMinA = %f\n", OGLRender.MaxA, OGLRender.MinA );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( "Texture Information:\n" );
    GlideMsg( "  565 = %d\n", Textures->Num_565_Tex );
    GlideMsg( " 1555 = %d\n", Textures->Num_1555_Tex );
    GlideMsg( " 4444 = %d\n", Textures->Num_4444_Tex );
    GlideMsg( "  332 = %d\n", Textures->Num_332_Tex );
    GlideMsg( " 8332 = %d\n", Textures->Num_8332_Tex );
    GlideMsg( "Alpha = %d\n", Textures->Num_Alpha_Tex );
    GlideMsg( " AI88 = %d\n", Textures->Num_AlphaIntensity88_Tex );
    GlideMsg( " AI44 = %d\n", Textures->Num_AlphaIntensity44_Tex );
    GlideMsg( " AP88 = %d\n", Textures->Num_AlphaPalette_Tex );
    GlideMsg( "   P8 = %d\n", Textures->Num_Palette_Tex );
    GlideMsg( "Inten = %d\n", Textures->Num_Intensity_Tex );
    GlideMsg( "  YIQ = %d\n", Textures->Num_YIQ_Tex );
    GlideMsg( " AYIQ = %d\n", Textures->Num_AYIQ_Tex );
    GlideMsg( "Other = %d\n", Textures->Num_Other_Tex );
    GlideMsg( OGL_LOG_SEPARATE );
    GlideMsg( OGL_LOG_SEPARATE );
#endif

    Textures->Clear( );

    FinaliseOpenGLWindow( );

    glDeleteTextures(1, &Glide.LFBTexture);
    delete[] Glide.SrcBuffer.Address;
    delete[] Glide.DstBuffer.Address;
    delete[] OpenGL.tmpBuf;
}

//*************************************************
//* Returns the Hardware Configuration
//*************************************************
FX_ENTRY FxBool FX_CALL
grSstQueryHardware( GrHwConfiguration *hwconfig )
{
#ifdef OGL_DONE
    GlideMsg( "grSstQueryHardware( --- )\n" );
#endif

    hwconfig->num_sst = 1;
    hwconfig->SSTs[0].type = GR_SSTTYPE_VOODOO;
//  hwconfig->SSTs[0].type = GR_SSTTYPE_Voodoo2;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.fbRam = UserConfig.FrameBufferMemorySize;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.fbiRev = 2;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.nTexelfx = 1;
//  hwconfig->SSTs[0].sstBoard.VoodooConfig.nTexelfx = 2;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.sliDetect = FXFALSE;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRev = 1;
    hwconfig->SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRam = UserConfig.TextureMemorySize;

    return FXTRUE;
}

//*************************************************
//* Selects which Voodoo Board is Active
//*************************************************
FX_ENTRY void FX_CALL
grSstSelect( int which_sst )
{
#ifdef OGL_DONE
    GlideMsg( "grSstSelect( %d )\n", which_sst );
#endif
    // Nothing Needed Here but...
    Glide.ActiveVoodoo = which_sst;
}

//*************************************************
//* Returns the Screen Height
//*************************************************
FX_ENTRY FxU32 FX_CALL
grSstScreenHeight( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstScreenHeight()\n" );
#endif

    return Glide.WindowHeight;
}

//*************************************************
//* Returns the Screen Width
//*************************************************
FX_ENTRY FxU32 FX_CALL
grSstScreenWidth( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSstScreenWidth()\n" );
#endif

    return Glide.WindowWidth;
}

//*************************************************
//* Sets the Y Origin
//*************************************************
FX_ENTRY void FX_CALL
grSstOrigin( GrOriginLocation_t  origin )
{
#ifdef OGL_DONE
    GlideMsg( "grSstSetOrigin( %d )\n", origin );
#endif

    RenderDrawTriangles( );

    Glide.State.OriginInformation = origin;

    switch ( origin )
    {
    case GR_ORIGIN_LOWER_LEFT:
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        glOrtho( 0, Glide.WindowWidth, 0, Glide.WindowHeight, OpenGL.ZNear, OpenGL.ZFar );
        glViewport( 0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight );
        glMatrixMode( GL_MODELVIEW );
        break;

    case GR_ORIGIN_UPPER_LEFT:
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        glOrtho( 0, Glide.WindowWidth, Glide.WindowHeight, 0, OpenGL.ZNear, OpenGL.ZFar );
        glViewport( 0, 0, OpenGL.WindowWidth, OpenGL.WindowHeight );
        glMatrixMode( GL_MODELVIEW );
        break;
    }
    grCullMode( Glide.State.CullMode );

#ifdef OPENGL_DEBUG
    GLErro( "grSstOrigin" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grSstPerfStats( GrSstPerfStats_t * pStats )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstPerfStats\n" );
#endif
}

//*************************************************
FX_ENTRY void FX_CALL
grSstResetPerfStats( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstResetPerfStats( )\n" );
#endif
}

//*************************************************
FX_ENTRY FxU32 FX_CALL 
grSstVideoLine( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstVideoLine( )\n" );
#endif

    return 0;
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
grSstVRetraceOn( void )
{
#ifdef OGL_NOTDONE
    GlideMsg( "grSstVRetraceOn( )\n" );
#endif

    return Glide.State.VRetrace;
}

//*************************************************
FX_ENTRY FxBool FX_CALL 
grSstIsBusy( void )
{ 
#ifdef OGL_NOTDONE
    GlideMsg( "grSstIsBusy( )\n" ); 
#endif

    return FXFALSE; 
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstControl( FxU32 code )
{ 
#ifdef OGL_NOTDONE
    GlideMsg( "grSstControl( %lu )\n", code ); 
#endif

    return GR_CONTROL_ACTIVATE; 
}

//*************************************************
FX_ENTRY FxBool FX_CALL
grSstControlMode( GrControl_t mode )
{ 
#ifdef OGL_NOTDONE
    GlideMsg( "grSstControlMode( %d )\n", mode );
#endif

    switch ( mode )
    {
    case GR_CONTROL_ACTIVATE:
        break;
    case GR_CONTROL_DEACTIVATE:
        break;
    case GR_CONTROL_RESIZE:
    case GR_CONTROL_MOVE:
        break;
    }

    return FXTRUE; 
}

//*************************************************
//* Return the Value of the graphics status register
//*************************************************
FX_ENTRY FxU32 FX_CALL 
grSstStatus( void )
{
#ifdef OGL_PARTDONE
    GlideMsg( "grSstStatus( )\n" );
#endif

//    FxU32 Status = 0x0FFFF43F;
    FxU32 Status = 0x0FFFF03F;
    
    // Vertical Retrace
    Status      |= ( ! Glide.State.VRetrace ) << 6;

    return Status;
// Bits
// 5:0      PCI FIFO free space (0x3F = free)
// 6        Vertical Retrace ( 0 = active, 1 = inactive )
// 7        PixelFx engine busy ( 0 = engine idle )
// 8        TMU busy ( 0 = engine idle )
// 9        Voodoo Graphics busy ( 0 = idle )
// 11:10    Displayed buffer ( 0 = buffer 0, 1 = buffer 1, 2 = auxiliary buffer, 3 = reserved )
// 27:12    Memory FIFO ( 0xFFFF = FIFO empty )
// 30:28    Number of swap buffers commands pending
// 31       PCI interrupt generated ( not implemented )
}

//*************************************************
//* Returns when Glides is Idle
//*************************************************
FX_ENTRY void FX_CALL
grSstIdle( void )
{
#ifdef OGL_DONE
    GlideMsg( "grSetIdle( )\n" );
#endif

    RenderDrawTriangles( );
    glFlush( );
    glFinish( );

#ifdef OPENGL_DEBUG
    GLErro( "grSstIdle" );
#endif
}

