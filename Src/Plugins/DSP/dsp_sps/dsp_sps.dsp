# Microsoft Developer Studio Project File - Name="dsp_sps" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dsp_sps - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dsp_sps.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dsp_sps.mak" CFG="dsp_sps - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dsp_sps - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dsp_sps - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dsp_sps - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSP_SPS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../Wasabi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSP_SPS_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x8000000" /entry:"DllMain" /dll /map /machine:I386 /out:"C:/progra~1/winamp/plugins/dsp_sps.dll" /opt:nowin98
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "dsp_sps - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSP_SPS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../Wasabi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSP_SPS_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"C:/progra~1/winamp/plugins/dsp_sps.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "dsp_sps - Win32 Release"
# Name "dsp_sps - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "eel"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\ns-eel\megabuf.c"

!IF  "$(CFG)" == "dsp_sps - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "dsp_sps - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\ns-eel\ns-eel-addfuncs.h"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\ns-eel-int.h"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\ns-eel.h"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-caltab.c"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-cfunc.c"

!IF  "$(CFG)" == "dsp_sps - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "dsp_sps - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-compiler.c"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-eval.c"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-lextab.c"
# End Source File
# Begin Source File

SOURCE="..\ns-eel\nseel-yylex.c"
# End Source File
# End Group
# Begin Source File

SOURCE=.\dsp_sps.cpp

!IF  "$(CFG)" == "dsp_sps - Win32 Release"

# SUBTRACT CPP /Z<none>

!ELSEIF  "$(CFG)" == "dsp_sps - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sps_common.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Support Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\Winamp\api_language.h
# End Source File
# Begin Source File

SOURCE=..\Wasabi\api\service\api_service.h
# End Source File
# Begin Source File

SOURCE=..\Winamp\lang.h
# End Source File
# Begin Source File

SOURCE=..\gen_hotkeys\wa_hotkeys.h
# End Source File
# Begin Source File

SOURCE=..\Winamp\wa_ipc.h
# End Source File
# Begin Source File

SOURCE=..\Wasabi\api\service\waservicefactory.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\api.h
# End Source File
# Begin Source File

SOURCE=..\Winamp\DSP.H
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sps_common.h
# End Source File
# Begin Source File

SOURCE=.\sps_configdlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\constant.bin
# End Source File
# Begin Source File

SOURCE=.\function.bin
# End Source File
# Begin Source File

SOURCE=.\general.bin
# End Source File
# Begin Source File

SOURCE=.\operator.bin
# End Source File
# Begin Source File

SOURCE=.\res.rc
# End Source File
# End Group
# End Target
# End Project
