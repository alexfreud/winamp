# Microsoft Developer Studio Generated NMAKE File, Based on AudioPlugIn.dsp
!IF "$(CFG)" == ""
CFG=AudioPlugIn - Win32 Debug
!MESSAGE No configuration specified. Defaulting to AudioPlugIn - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "AudioPlugIn - Win32 Release" && "$(CFG)" != "AudioPlugIn - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AudioPlugIn.mak" CFG="AudioPlugIn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AudioPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AudioPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "AudioPlugIn - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\AudioPlugIn.dll" ".\custom.bld"


CLEAN :
	-@erase "$(INTDIR)\AudioPlugIn.obj"
	-@erase "$(INTDIR)\AudioPlugIn.pch"
	-@erase "$(INTDIR)\AudioPlugIn.res"
	-@erase "$(INTDIR)\AudioPlugInPropPage.obj"
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\MediaParams.obj"
	-@erase "$(INTDIR)\ParamEnvelope.obj"
	-@erase "$(INTDIR)\PlugInApp.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AudioPlugIn.dll"
	-@erase "$(OUTDIR)\AudioPlugIn.exp"
	-@erase "$(OUTDIR)\AudioPlugIn.lib"
	-@erase "custom.bld"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\AudioPlugIn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AudioPlugIn.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AudioPlugIn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib \build\sdks\dxmedia\lib\DMOGUIDS.LIB winmm.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\AudioPlugIn.pdb" /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /def:".\AudioPlugIn.def" /out:"$(OUTDIR)\AudioPlugIn.dll" /implib:"$(OUTDIR)\AudioPlugIn.lib" 
DEF_FILE= \
	".\AudioPlugIn.def"
LINK32_OBJS= \
	"$(INTDIR)\AudioPlugIn.obj" \
	"$(INTDIR)\AudioPlugInPropPage.obj" \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\MediaParams.obj" \
	"$(INTDIR)\ParamEnvelope.obj" \
	"$(INTDIR)\PlugInApp.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\AudioPlugIn.res"

"$(OUTDIR)\AudioPlugIn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
TargetName=AudioPlugIn
InputPath=.\Release\AudioPlugIn.dll
SOURCE="$(InputPath)"

".\custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
<< 
	

!ELSEIF  "$(CFG)" == "AudioPlugIn - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\AudioPlugIn.dll" ".\custom.bld"


CLEAN :
	-@erase "$(INTDIR)\AudioPlugIn.obj"
	-@erase "$(INTDIR)\AudioPlugIn.pch"
	-@erase "$(INTDIR)\AudioPlugIn.res"
	-@erase "$(INTDIR)\AudioPlugInPropPage.obj"
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\MediaParams.obj"
	-@erase "$(INTDIR)\ParamEnvelope.obj"
	-@erase "$(INTDIR)\PlugInApp.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AudioPlugIn.dll"
	-@erase "$(OUTDIR)\AudioPlugIn.exp"
	-@erase "$(OUTDIR)\AudioPlugIn.ilk"
	-@erase "$(OUTDIR)\AudioPlugIn.lib"
	-@erase "$(OUTDIR)\AudioPlugIn.pdb"
	-@erase "custom.bld"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\AudioPlugIn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AudioPlugIn.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AudioPlugIn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib \build\sdks\dxmedia\lib\DMOGUIDS.LIB winmm.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\AudioPlugIn.pdb" /debug /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /def:".\AudioPlugIn.def" /out:"$(OUTDIR)\AudioPlugIn.dll" /implib:"$(OUTDIR)\AudioPlugIn.lib" /pdbtype:sept 
DEF_FILE= \
	".\AudioPlugIn.def"
LINK32_OBJS= \
	"$(INTDIR)\AudioPlugIn.obj" \
	"$(INTDIR)\AudioPlugInPropPage.obj" \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\MediaParams.obj" \
	"$(INTDIR)\ParamEnvelope.obj" \
	"$(INTDIR)\PlugInApp.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\AudioPlugIn.res"

"$(OUTDIR)\AudioPlugIn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetName=AudioPlugIn
InputPath=.\Debug\AudioPlugIn.dll
SOURCE="$(InputPath)"

".\custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("AudioPlugIn.dep")
!INCLUDE "AudioPlugIn.dep"
!ELSE 
!MESSAGE Warning: cannot find "AudioPlugIn.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AudioPlugIn - Win32 Release" || "$(CFG)" == "AudioPlugIn - Win32 Debug"
SOURCE=.\AudioPlugIn.cpp

"$(INTDIR)\AudioPlugIn.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\AudioPlugIn.rc

"$(INTDIR)\AudioPlugIn.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\AudioPlugInPropPage.cpp

"$(INTDIR)\AudioPlugInPropPage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\Filter.cpp

"$(INTDIR)\Filter.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\MediaParams.cpp

"$(INTDIR)\MediaParams.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\ParamEnvelope.cpp

"$(INTDIR)\ParamEnvelope.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\PlugInApp.cpp

"$(INTDIR)\PlugInApp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioPlugIn.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "AudioPlugIn - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\AudioPlugIn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\AudioPlugIn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "AudioPlugIn - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "include" /I "$(MSSDK)\include" /I "$(MSSDK)\Samples\Multimedia\DirectShow\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\AudioPlugIn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\AudioPlugIn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

