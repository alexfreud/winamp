everything : all 
###### dxv standard project specs
     
PROJECT = dxv
# project name

# standard modules for this library

OBJS = \
    $(OBJDIR)dxlcsize.$(OBJ) \
    $(OBJDIR)dxv_init.$(OBJ) \
    $(OBJDIR)dxv_initex.$(OBJ) \
    $(OBJDIR)dxl_main.$(OBJ) \
    $(OBJDIR)vscreen.$(OBJ) \
    $(OBJDIR)ximage.$(OBJ) \
    $(OBJDIR)ximgblit.$(OBJ) \
    $(OBJDIR)getprefs.$(OBJ) \
    $(OBJDIR)dxl_clip.$(OBJ) \
	$(OBJDIR)dxl_bkgr.$(OBJ) \
    $(OBJDIR)dxl_view.$(OBJ) \
	$(OBJDIR)vgetview.$(OBJ) \
	$(OBJDIR)vscerase.$(OBJ) \
    $(OBJDIR)dxl_attr.$(OBJ) \
    $(OBJDIR)dxl_reg.$(OBJ) \
    $(OBJDIR)dxl_feat.$(OBJ) \
	$(OBJDIR)genximg.$(OBJ) \
    $(OBJDIR)svsbitdp.$(OBJ) \
	$(OBJDIR)vscraddr.$(OBJ) \
    $(OBJDIR)blitall.$(OBJ) \
    $(OBJDIR)dxlvinfd.$(OBJ) \
    $(OBJDIR)cpuid.$(OBJ) \
	$(OBJDIR)count_tm.$(OBJ)

!ifndef NOCLUT

OBJS = $(OBJS) \
	$(OBJDIR)dxl_clut.$(OBJ)
    
!endif
	
!ifndef NOLOGO

OBJS = $(OBJS) \
	$(OBJDIR)on2logo.$(OBJ) \
	$(OBJDIR)tmlogo55.$(OBJ) \
	$(OBJDIR)tmlogo56.$(OBJ) \
	$(OBJDIR)tmlogo24.$(OBJ) \
	$(OBJDIR)ducklogo.$(OBJ) 

!endif
	

MAJREV = 6		# major library revision
MINREV = 0		# minor library revision
PVER = 1		# minor.minor library revision
BVER = ".b1"	# build version

# libraries on which this should be built

!if 0
    $(LIBDIR)$(LIBPREFIX)tmrt$(LIBSUFFIX) \
	$(LIBDIR)$(LIBPREFIX)tm1$(LIBSUFFIX) \
	$(LIBDIR)$(LIBPREFIX)tm20$(LIBSUFFIX) \

	$(LIBDIR)$(LIBPREFIX)vp3$(LIBSUFFIX) \
	$(LIBDIR)$(LIBPREFIX)vp31d$(LIBSUFFIX)


!endif

!if 0
USER_LIBS = \
	$(LIBDIR)$(LIBPREFIX)torq$(LIBSUFFIX)
!endif

#
# project sub options sent to compilers
#
!ifndef CENV
CENV = -DTORQB -DTM1B -DTM2B -DTMRTB
!endif

# add sub-options to c options (allows for environment override)
C_FLAGS = $(CENV)
c_flags = $(C_FLAGS)

# this proj library is to be built as a shared or master library
SHAREDLIB = 0

M_INCLUDES = -I$(INCDIR)/$(OS) -I$(INCDIR) -I. -I../generic $(M_INCLUDES)
# win95 specific modules

OBJS = $(OBJS) \
	$(OBJDIR)dxv.$(OBJ) \
    $(OBJDIR)dkprof.$(OBJ) \
	$(OBJDIR)perf.$(OBJ) \
    $(OBJDIR)pentium.$(OBJ) \
	$(OBJDIR)$(PROJECT).res

# modules specifically sent to linker,
# others are provided in libraries

DLLOBJS = $(OBJDIR)dxl_main.obj \
	$(OBJDIR)dxv_mem.$(OBJ) \
	$(OBJDIR)dxv_mems.$(OBJ) \
	$(OBJDIR)$(PROJECT).res \
	$(OBJDIR)$(PROJECT).$(OBJ)

PROJLIBS = $(PROJLIBS) 

#$(OBJDIR)sc_$(PROJECT).lib

THISFILE = makefile

SRCDIR    = .
GENDIR    = ..\generic
OBJDIR    = obj\  
LIBDIR    = obj\ 
INCDIR    = $(CORELIBS_INC)

ASMDIR    = ..\X86\ 
MMXDIR    = ..\mmx\ 
WASMDIR   = ..\wX86\ 

OBJRELDIR = $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)\Release\ 
OBJDEBDIR = $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)\Debug\ 
LIBRELDIR = $(ROOT)Private\CoreLibs\Lib\$(OS)\Release 
LIBDEBDIR = $(ROOT)Private\CoreLibs\Lib\$(OS)\Debug 

OBJ = obj


MKDIR = 

# tools 
CC = cl
ASM = tasm /ml /m8 /dWINP /zd /I$(ASMDIR) /dWIN95
WASM = wasm /dWINP /dWIN95 /I=$(ASMDIR) /I=$(MMXDIR) -q
RC = rc

LIBNAME = $(OBJDIR)$(PROJECT).lib
SLIBNAME = $(OBJDIR)s_$(PROJECT).lib
EXENAME = $(OBJDIR)$(PROJECT).exe
###DEFNAME = $(SRCDIR)$(PROJECT).DEF
DEFNAME = $(PROJECT).DEF

LIBPREFIX = \s_
LIBSUFFIX = .lib

# Include Path Variable Extensible
include = $(SDK)/inc;$(include)


############################################################
# Global Flags (compiler / linker)
C_FLAGS = $(C_FLAGS) -Gy -W3 -nologo -DWIN32 -D_MBCS -D_AFXDLL -D_WINDOWS -DHIRES\
 -DDOS32 -c  $(M_INCLUDES:/=\) 
#-WX

L_FLAGS = $(L_FLAGS) /SUBSYSTEM:windows /INCREMENTAL:no /MACHINE:I386 

############################################################
# Locally Overridable Flags

!ifndef DLLNAME
DLLNAME = $(OBJDIR)$(PROJECT).dll
!endif

!ifndef c_thread

## SJL - NEED TO USE LIBCMT NOW
## c_thread = -MD
c_thread = -MT
!endif 

!ifndef c_ndebugflags
c_ndebugflags = -DNDEBUG -Ob2 -Ot -O2 -G5
!endif 

!ifndef c_debugflags
c_debugflags = -D_DEBUG -GX -Od -Z7 
!endif 

!ifndef l_ndebugflags
l_ndebugflags = 
!endif 

!ifndef l_debugflags
l_debugflags = -DEBUG -DEBUGTYPE:BOTH
!endif 

# exe flags / objects
!ifndef l_exe_flags
l_exe_flags = 
!endif


# DLL flags / objects
!ifndef l_dll_flags
l_dll_flags = -DLL -DEF:$(DEFNAME)
!endif

!ifndef DLLOBJS
DLLOBJS = $(OBJS)
!endif

# targets we will build
!ifndef targets
targets = $(SLIBNAME) 
!ifdef SHAREDLIB
!if $(SHAREDLIB) == 1
targets = $(TARGETS) $(DLLNAME) 
!endif
!endif 
!endif 


# versions we will build
!ifndef VERSIONS
VERSIONS = release.ver debug.ver
!endif 

############################################################
# inference rules
#    -mkdir $(OBJDIR:\= )
#    -mkdir $(MKOBJDIR:\= ) 

# Target Directories
#$(OBJDIR:\= ) :
!if 0
$(OBJDIR:\= ) :
	mkdir $*
	cd $* 
!endif

$(LIBDIR) :
#    -mkdir $(LIBDIR)\..
	-mkdir $(MKLIBDIR)

# Object files 

{$(GENDIR)}.c{$(OBJDIR)}.obj:
	$(CC)  $(C_FLAGS)  $<  -Fo$@

{$(GENDIR)}.cpp{$(OBJDIR)}.obj:
	$(CC)  $(C_FLAGS)  $<  -Fo$@

{$(SRCDIR)}.c{$(OBJDIR)}.obj:
	$(CC)  $(C_FLAGS)  $<  -Fo$@

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	$(CC)  $(C_FLAGS)  $<  -Fo$@

{$(WASMDIR)}.asm{$(OBJDIR)}.obj:
    $(WASM) $< -fo=$@ 
    editbin /NOLOGO /RELEASE $@

{$(ASMDIR)}.asm{$(OBJDIR)}.obj:
    $(ASM) $< $@ 
    editbin /NOLOGO /RELEASE $@

{$(SRCDIR)}.rc{$(OBJDIR)}.res:
    $(RC) /fo$@ /dALG="$(CENV:-D= )"  /dPROJ=$(PROJECT) /dMAJ=$(MAJREV) /dMIN=$(MINREV) /dPVER=$(PVER) /dBVER=$(BVER) $< 

#
# building of targets 
#

###$(SLIBNAME): $(OBJDIR:\= ) $(LIBDIR) $(OBJS) 
$(SLIBNAME): $(OBJS) 
	LIB $(OBJS) /OUT:$@
!ifdef USER_LIBS
	for %i in ($(USER_LIBS)) do LIB $(SLIBNAME) %i /OUT:$@
!endif
	copy $(SLIBNAME) $(LIBDIR)

##$(DLLNAME): $(SLIBNAME) $(OBJDIR:\= ) $(LIBDIR) $(DEFNAME) $(OBJDIR)$(PROJECT).res $(SLIBNAME) $(DLLOBJS)
$(DLLNAME): $(SLIBNAME) $(LIBDIR) $(DEFNAME) $(OBJDIR)$(PROJECT).res $(SLIBNAME) $(DLLOBJS)
	link $(L_FLAGS) $(l_dll_flags) /OUT:$(DLLNAME) $(DLLOBJS) $(SLIBNAME) /MAP:$(OBJDIR)$(PROJECT).map 
	copy $@ $(LIBDIR)
	copy $(LIBNAME) $(LIBDIR)

$(EXENAME): $(OBJDIR:\= ) $(DEFNAME) $(OBJDIR)$(PROJECT).res $(OBJS) 
	link $(L_FLAGS) $(l_exe_flags) /OUT:$(EXENAME) $(OBJS) /MAP:$(OBJDIR)$(PROJECT).map 

#
# Goals
#
build_me: $(targets) $(PROJLIBS)
    @echo Build Completed

#
# there should be a more elegant way to build the following directories
#
build_dirs:
	@echo BUILDING DIRECTORIES
	mkdir $(ROOT)Object
	mkdir $(ROOT)Object\CoreLibs
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)\Dx
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)\Release
	mkdir $(ROOT)Object\CoreLibs\$(PROJECT)\Dx\$(OS)\Debug

release.ver:
    set C_FLAGS=$(C_FLAGS) $(c_ndebugflags) $(c_thread) $(c_local)
    set L_FLAGS=$(L_FLAGS) $(l_ndebugflags)
    set OBJDIR=$(OBJRELDIR) 
    set LIBDIR=$(LIBRELDIR) 
	nmake /NOLOGO /C /S build_dirs
    nmake /NOLOGO -f $(THISFILE) build_me

debug.ver:
	@echo NOW DOING DEBUG VERSION
    set C_FLAGS=$(C_FLAGS) $(c_debugflags) $(c_thread) $(c_local)
    set L_FLAGS=$(L_FLAGS) $(l_debugflags)
    set OBJDIR=$(OBJDEBDIR) 
    set LIBDIR=$(LIBDEBDIR) 
	nmake /NOLOGO /C /S build_dirs
    nmake /NOLOGO -f $(THISFILE) build_me

all:: $(VERSIONS)

clean:
    deltree /Y obj
    deltree /Y obj.*




$(OBJDIR)sc_$(PROJECT).lib: $(LIBDIR)\sc_tm20.lib $(LIBDIR)\sc_torq.lib
    LIB $(OBJS) /OUT:$@
    LIB $@ $(LIBDIR)\s_tm1.lib /OUT:$@
    LIB $@ $(LIBDIR)\sc_tm20.lib /OUT:$@
    LIB $@ $(LIBDIR)\s_tmrt.lib /OUT:$@
    LIB $@ $(LIBDIR)\sc_torq.lib /OUT:$@
    copy $@ $(LIBDIR)

