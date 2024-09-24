# Microsoft Developer Studio Project File - Name="AudioPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AudioPlugIn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AudioPlugIn.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AudioPlugIn - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "include" /I "..\..\dshow\include" /I "..\..\dshow\dshow" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib DMOGUIDS.LIB winmm.lib comdlg32.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /libpath:"../../dshow" /opt:nowin98
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Custom Build Steps
OutDir=.\Release
TargetName=AudioPlugIn
InputPath=.\Release\AudioPlugIn.dll
SOURCE="$(InputPath)"

"custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "AudioPlugIn - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "include" /I "..\..\dshow\include" /I "..\..\dshow\dshow" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib DMOGUIDS.LIB winmm.lib comdlg32.lib /nologo /entry:"DllEntryPoint@12" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /pdbtype:sept /libpath:"../../dshow"
# Begin Custom Build - Custom Build Steps
OutDir=.\Debug
TargetName=AudioPlugIn
InputPath=.\Debug\AudioPlugIn.dll
SOURCE="$(InputPath)"

"custom.bld" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32.exe /s $(OutDir)\$(TargetName).DLL 
	echo >custom.bld 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "AudioPlugIn - Win32 Release"
# Name "AudioPlugIn - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "dx shit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AudioPlugIn.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioPlugIn.def
# End Source File
# Begin Source File

SOURCE=.\AudioPlugIn.rc
# End Source File
# Begin Source File

SOURCE=.\AudioPlugInPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\Filter.cpp
# End Source File
# Begin Source File

SOURCE=.\MediaParams.cpp
# End Source File
# Begin Source File

SOURCE=.\ParamEnvelope.cpp
# End Source File
# Begin Source File

SOURCE=.\PlugInApp.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "ns-eel"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\ns-eel\megabuf.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\ns-eel-addfuncs.h"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\ns-eel-int.h"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\ns-eel.h"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-caltab.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-cfunc.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-compiler.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-eval.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-lextab.c"
# End Source File
# Begin Source File

SOURCE="..\..\ns-eel\nseel-yylex.c"
# End Source File
# End Group
# Begin Group "sps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\sps_common.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AudioPlugIn.h
# End Source File
# Begin Source File

SOURCE=.\AudioPlugInPropPage.h
# End Source File
# Begin Source File

SOURCE=.\include\DXi.h
# End Source File
# Begin Source File

SOURCE=.\Filter.h
# End Source File
# Begin Source File

SOURCE=.\MediaParams.h
# End Source File
# Begin Source File

SOURCE=.\ParamEnvelope.h
# End Source File
# Begin Source File

SOURCE=.\Parameters.h
# End Source File
# Begin Source File

SOURCE=.\PlugInApp.h
# End Source File
# Begin Source File

SOURCE=.\PlugInGUIDs.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\AudioPlugIn.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
