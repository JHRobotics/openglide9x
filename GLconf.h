
OGL_CFG_BOOL(FogEnable,                false, "Enable fog (Warning: fog is broken now)")

#ifndef DEBUG
OGL_CFG_BOOL(InitFullScreen,           true, "Fullscreen mode")
#else
OGL_CFG_BOOL(InitFullScreen,           false, "Fullscreen mode")
#endif

OGL_CFG_BOOL(PrecisionFix,             false, "Round values for integer depth buffer")
OGL_CFG_BOOL(CreateWindow,             true, "Create Window if NULL is passed to grSstWinOpen")
OGL_CFG_INT(DepthBits,                 0, 0, 64, "Sets required depth buffer bits, 0 auto choose for best performance")
OGL_CFG_BOOL(EnableMipMaps,            true, "")
OGL_CFG_BOOL(BuildMipMaps,            false, "")
OGL_CFG_BOOL(IgnorePaletteChange,     false, "")
OGL_CFG_BOOL(ARB_multitexture,         true, "")
OGL_CFG_BOOL(EXT_paletted_texture,     true, "")
OGL_CFG_BOOL(EXT_texture_env_add,     false, "")
OGL_CFG_BOOL(EXT_texture_env_combine, false, "")
OGL_CFG_BOOL(EXT_vertex_array,        false, "")
OGL_CFG_BOOL(EXT_fog_coord,            true, "")
OGL_CFG_BOOL(EXT_blend_func_separate,  true, "")
OGL_CFG_BOOL(Wrap565to5551,           false, "")
OGL_CFG_BOOL(Textures32bit,           false, "Convert textures to ARGB8888 instead of equivalent 16b format")
OGL_CFG_BOOL(MMXEnable,                true, "Always enabled and SSE too!")
OGL_CFG_INT(SwapInterval,          1,   -1,  1, "Vertical synchronization, 0 = disabled, 1 = enabled, -1 adaptive (if supported)")

OGL_CFG_FLOAT(Resolution,          0,   0,   16, "Scale screen, 0 = 1 = disabled, 2 = 2x scale (800x600 => 1600x1200)")
OGL_CFG_INT(TextureMemorySize,     8,   2,   32, "Texture memory size in MB, from 2 to 32")
OGL_CFG_INT(FrameBufferMemorySize, 8,   2,   16, "Frame buffer size in MB, from 2 to 16")
OGL_CFG_INT(Priority,              2,   0,    5, "0 = highest, 5 = lowest")
OGL_CFG_INT(HideCursor,            1,   0,    2, "Try to suppress flashing cursor, 0 = disable, 1 = hide by thread, 2 = disable in whole system, when GLIDE active")
#ifndef DEBUG
OGL_CFG_INT(NoSplash,              2,   0,    2, "3DFX splash screen is 0 = enable, 1 = disabled, 2 = orignal controled by FX_GLIDE_NO_SPLASH or disabled")
#else
OGL_CFG_INT(NoSplash,              1,   0,    2, "")
#endif
OGL_CFG_BOOL(ShamelessPlug,               false, "")

#ifndef DEBUG
OGL_CFG_BOOL(Logging,                     false, "")
#else
OGL_CFG_BOOL(Logging,                     true, "")
#endif

#if GLIDE_NUM_TMU > 1
OGL_CFG_INT(NumTMU,                2,   1,    GLIDE_NUM_TMU, "Number of texture mapping units (TMU), 1 = most compatible")
#else
OGL_CFG_INT(NumTMU,                1,   1,    GLIDE_NUM_TMU, "Number of texture mapping units (TMU), 1 = most compatible")
#endif
OGL_CFG_INT(SSTType,              -1,  -1,    4, "Type of Voodoo board: -1 = auto, 0 = Voodoo, 1 = Rush, 3 = Voodoo2, 4 = Banshee")
OGL_CFG_BOOL(Disabled,                    false, "Deny load, ideal, if you want ship 3dfx OpenGL and continue to normal OpenGL")
