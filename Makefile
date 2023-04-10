include config.mk

DEPS = config.mk Makefile

# only usefull with gcc/mingw
CSTD=c99
CXXSTD=gnu++11

# base address 3DFX drivers
BASE_glide2x.dll   := 0x10000000
BASE_glide3x.dll   := 0x10000000

TARGETS = glide2x.dll glide3x.dll

all: $(TARGETS)
.PHONY: all clean

ifdef MSC
#
# MSC configuration
#
  OBJ := .obj
  LIBSUFFIX := .lib
  LIBPREFIX := 

  CL_INCLUDE = 

  # /DNDEBUG
	CL_DEFS =  /D_WIN32 /DWIN32
  
  ifdef DEBUG
    DD_DEFS = /DDEBUG
  else
    DD_DEFS = /DNDEBUG
  endif
    
  ifdef SPEED
    CFLAGS   = /nologo $(MSC_RUNTIME) /O2 /Oi /Zc:inline $(CL_INCLUDE) $(DD_DEFS) $(CL_DEFS)
    CXXFLAGS = /nologo $(MSC_RUNTIME) /O2 /Oi /Zc:inline $(CL_INCLUDE) $(DD_DEFS) $(CL_DEFS)
    LDFLAGS  = /nologo $(MSC_RUNTIME) /O2 /Oi /Zc:inline
  else
    CFLAGS   = /nologo $(MSC_RUNTIME) /Z7 /Od $(CL_INCLUDE) $(DD_DEFS) $(CL_DEFS)
    CXXFLAGS = /nologo $(MSC_RUNTIME) /Z7 /Od $(CL_INCLUDE) $(DD_DEFS) $(CL_DEFS)
    LDFLAGS  = /nologo $(MSC_RUNTIME) /Z7 /Od
  endif
  
  GLIDE_LIBS = kernel32.lib user32.lib gdi32.lib
    
  %.c.obj: %.c $(DEPS)
		$(CC) $(CFLAGS) /Fo"$@" /c $<
		
  %.cpp.obj: %.cpp $(DEPS)
		$(CXX) $(CXXFLAGS) /Fo"$@" /c $<
	
  %.res: %.rc $(DEPS)
		$(WINDRES) /nologo /fo $@ $<
  
  LDLAGS = CXXFLAGS
  DLLFLAGS = /link /DLL /MACHINE:X86 /IMPLIB:$(@:dll=lib) /OUT:$@ /PDB:$(@:dll=pdb) /BASE:$(BASE_$@) /DEF:$(DEF_$@)
  
  DEF_glide2x.dll = Glide2x.def
  DEF_glide3x.dll = Glide3x.def

  LIBSTATIC = LIB.EXE /nologo /OUT:$@ 
	
else
#
# MinGW configurationn
#
  OBJ := .o
  LIBSUFFIX := .a
  LIBPREFIX := lib
  
  DLLFLAGS = -o $@ -shared -Wl,--dll,--out-implib,lib$(@:dll=a),--enable-stdcall-fixup,--image-base,$(BASE_$@)$(TUNE_LD)
  
  INCLUDE = -Iplatform\windows -I.

  DEFS = -DWIN32 -DCPPDLL
  
  ifdef DEBUG
    DD_DEFS = -DDEBUG
  else
    DD_DEFS = -DNDEBUG
  endif
  
  ifdef SPEED
    CFLAGS = -std=$(CSTD) -O3 -fno-exceptions $(TUNE) $(INCLUDE) -DNDEBUG $(DEFS)
    CXXFLAGS = -std=$(CXXSTD) -O3 -fno-exceptions -fno-rtti $(TUNE) $(INCLUDE) -DNDEBUG $(DEFS)
    LDFLAGS = -std=$(CXXSTD) -O3
  else
    CFLAGS = -std=$(CSTD) -O0 -g $(TUNE) $(INCLUDE) $(DD_DEFS) $(DEFS)
    CXXFLAGS = -std=$(CXXSTD) -O0 -g  $(TUNE) $(INCLUDE) $(DD_DEFS) $(DEFS)
    LDFLAGS = -std=$(CXXSTD)
  endif
  
  GLIDE_LIBS  = -static -Lpthread9x pthread9x/crtfix.o -lpthread -lopengl32 -lgdi32
  GLIDE2_DEF = Glide2x.def
  GLIDE3_DEF = Glide3x.def
  
  DEPS_EXTRA = pthread9x/libpthread.a
  		
  %.c.g2.o: %.c $(DEPS)
		$(CC) $(CFLAGS) -DGLIDE2 -c -o $@ $<
		
  %.cpp.g2.o: %.cpp $(DEPS)
		$(CXX) $(CXXFLAGS) -DGLIDE2 -c -o $@ $<
		
  %.c.g3.o: %.c $(DEPS)
		$(CC) $(CFLAGS) -DGLIDE3 -c -o $@ $<
		
  %.cpp.g3.o: %.cpp $(DEPS)
		$(CXX) $(CXXFLAGS) -DGLIDE3 -c -o $@ $<
	
  %.res: %.rc $(DEPS)
		$(WINDRES) -DWINDRES --input $< --output $@ --output-format=coff
	
  LIBSTATIC = ar rcs -o $@ 
endif

pthread9x/libpthread.a: $(DEPS) pthread9x/Makefile
	cd pthread9x && $(MAKE)

#
# Glide2x.dll + Glide3x.dll
#
glide_SRC  := \
	grguDepth.cpp \
	grguMisc.cpp \
	grgu3df.cpp \
	grguDraw.cpp \
	grguSstGlide.cpp \
	grguFog.cpp \
	grguTex.cpp \
	grguLfb.cpp \
	GLRender.cpp \
	OGLFogTables.cpp \
	OGLTextureTables.cpp \
	OGLColorAlphaTables.cpp \
	TexDB.cpp \
	PGUTexture.cpp \
	Glide.cpp \
	GLExtensions.cpp \
	PGTexture.cpp \
	FormatConversion.cpp \
	grguBuffer.cpp \
	grguColorAlpha.cpp \
	GLutil.cpp \
	gsplash.cpp \
	platform/windows/clock.cpp \
	platform/windows/error.cpp \
	platform/windows/library.cpp \
	platform/windows/openglext.cpp \
	platform/windows/window.cpp

glide_OBJS := $(glide_SRC:.c=.c.g2$(OBJ))
glide_OBJS := $(glide_OBJS:.cpp=.cpp.g2$(OBJ))

glide3_OBJS := $(glide_SRC:.c=.c.g3$(OBJ))
glide3_OBJS := $(glide3_OBJS:.cpp=.cpp.g3$(OBJ))

glide2x.dll: $(glide_OBJS) glide2x.res $(DEPS) $(DEPS_EXTRA)
	$(LD) $(LDFLAGS) $(glide_OBJS) glide2x.res $(GLIDE_LIBS) $(DLLFLAGS) $(GLIDE2_DEF)
	
glide3x.dll: $(glide3_OBJS) glide3x.res $(DEPS) $(DEPS_EXTRA)
	$(LD) $(LDFLAGS) $(glide3_OBJS) glide3x.res $(GLIDE_LIBS) $(DLLFLAGS) $(GLIDE3_DEF)

ifdef OBJ
clean:
	-$(RM) $(glide_OBJS)
	-$(RM) $(glide3_OBJS)
	-$(RM) glide2x.res glide2x.dll
	-$(RM) glide3x.res glide3x.dll
	-cd pthread9x && $(MAKE) clean
endif
