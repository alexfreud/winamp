# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Dac32 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Dac32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Dac32 - Win32 Release" && "$(CFG)" != "Dac32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dac32.mak" CFG="Dac32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dac32 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Dac32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Dac32 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Dac32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Dac32.dll" "$(OUTDIR)\Dac32.bsc"

CLEAN : 
	-@erase "$(INTDIR)\aspifunc.obj"
	-@erase "$(INTDIR)\aspifunc.sbr"
	-@erase "$(INTDIR)\dac32.obj"
	-@erase "$(INTDIR)\dac32.res"
	-@erase "$(INTDIR)\dac32.sbr"
	-@erase "$(OUTDIR)\Dac32.bsc"
	-@erase "$(OUTDIR)\Dac32.dll"
	-@erase "$(OUTDIR)\Dac32.exp"
	-@erase "$(OUTDIR)\Dac32.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DLL" /Fr /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DLL"\
 /Fr"$(INTDIR)/" /Fp"$(INTDIR)/Dac32.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/dac32.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Dac32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\aspifunc.sbr" \
	"$(INTDIR)\dac32.sbr"

"$(OUTDIR)\Dac32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /map /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Dac32.pdb" /machine:I386 /out:"$(OUTDIR)/Dac32.dll"\
 /implib:"$(OUTDIR)/Dac32.lib" 
LINK32_OBJS= \
	"$(INTDIR)\aspifunc.obj" \
	"$(INTDIR)\dac32.obj" \
	"$(INTDIR)\dac32.res"

"$(OUTDIR)\Dac32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Dac32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Dac32.dll" "$(OUTDIR)\Dac32.bsc"

CLEAN : 
	-@erase "$(INTDIR)\aspifunc.obj"
	-@erase "$(INTDIR)\aspifunc.sbr"
	-@erase "$(INTDIR)\dac32.obj"
	-@erase "$(INTDIR)\dac32.res"
	-@erase "$(INTDIR)\dac32.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Dac32.bsc"
	-@erase "$(OUTDIR)\Dac32.dll"
	-@erase "$(OUTDIR)\Dac32.exp"
	-@erase "$(OUTDIR)\Dac32.ilk"
	-@erase "$(OUTDIR)\Dac32.lib"
	-@erase "$(OUTDIR)\Dac32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DLL" /Fr /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "DLL" /Fr"$(INTDIR)/" /Fp"$(INTDIR)/Dac32.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/dac32.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Dac32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\aspifunc.sbr" \
	"$(INTDIR)\dac32.sbr"

"$(OUTDIR)\Dac32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Dac32.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Dac32.dll"\
 /implib:"$(OUTDIR)/Dac32.lib" 
LINK32_OBJS= \
	"$(INTDIR)\aspifunc.obj" \
	"$(INTDIR)\dac32.obj" \
	"$(INTDIR)\dac32.res"

"$(OUTDIR)\Dac32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "Dac32 - Win32 Release"
# Name "Dac32 - Win32 Debug"

!IF  "$(CFG)" == "Dac32 - Win32 Release"

!ELSEIF  "$(CFG)" == "Dac32 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\dac32.cpp

!IF  "$(CFG)" == "Dac32 - Win32 Release"

DEP_CPP_DAC32=\
	".\Aspifunc.h"\
	".\Dac32.h"\
	".\Scsidefs.h"\
	".\Winaspi.h"\
	

"$(INTDIR)\dac32.obj" : $(SOURCE) $(DEP_CPP_DAC32) "$(INTDIR)"

"$(INTDIR)\dac32.sbr" : $(SOURCE) $(DEP_CPP_DAC32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Dac32 - Win32 Debug"

DEP_CPP_DAC32=\
	".\Aspifunc.h"\
	".\Dac32.h"\
	".\Scsidefs.h"\
	".\Winaspi.h"\
	

"$(INTDIR)\dac32.obj" : $(SOURCE) $(DEP_CPP_DAC32) "$(INTDIR)"

"$(INTDIR)\dac32.sbr" : $(SOURCE) $(DEP_CPP_DAC32) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\aspifunc.cpp
DEP_CPP_ASPIF=\
	".\Aspifunc.h"\
	".\Scsidefs.h"\
	".\Winaspi.h"\
	

"$(INTDIR)\aspifunc.obj" : $(SOURCE) $(DEP_CPP_ASPIF) "$(INTDIR)"

"$(INTDIR)\aspifunc.sbr" : $(SOURCE) $(DEP_CPP_ASPIF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dac32.rc

"$(INTDIR)\dac32.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
