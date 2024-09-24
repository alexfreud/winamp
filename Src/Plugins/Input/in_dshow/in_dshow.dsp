# Microsoft Developer Studio Project File - Name="in_dshow" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=in_dshow - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "in_dshow.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "in_dshow.mak" CFG="in_dshow - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "in_dshow - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "in_dshow - Win32 Debug Winamp" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "in_dshow - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "in_dshow_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../dshow/dshow" /I "../dshow/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "in_dshow_EXPORTS" /D "USE_ASM" /D "NO_LAYER12" /D "DLL_DECODER_SUPPORT" /D "BUILTIN_MP3_SUPPORT" /D "BUILTIN_VP3_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ../dshow/strmbase.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib ../dshow/strmiids.lib /nologo /dll /machine:I386 /out:"$(ProgramFiles)\winamp\plugins\in_dshow.dll" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "in_dshow - Win32 Debug Winamp"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "in_dshow___Win32_Debug_Winamp"
# PROP BASE Intermediate_Dir "in_dshow___Win32_Debug_Winamp"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "in_dshow___Win32_Debug_Winamp"
# PROP Intermediate_Dir "in_dshow___Win32_Debug_Winamp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../dshow/dshow" /I "../dshow/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "in_dshow_EXPORTS" /D "USE_ASM" /D "NO_LAYER12" /D "DLL_DECODER_SUPPORT" /D "BUILTIN_MP3_SUPPORT" /D "BUILTIN_VP3_SUPPORT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../dshow/dshow" /I "../dshow/include" /I "../WinAmpX" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "in_dshow_EXPORTS" /D "USE_ASM" /D "NO_LAYER12" /D "DLL_DECODER_SUPPORT" /D "BUILTIN_MP3_SUPPORT" /D "BUILTIN_VP3_SUPPORT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../dshow/strmbasd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib ../dshow/strmiids.lib /nologo /dll /debug /machine:I386 /out:"c:\progra~1\winamp\plugins\in_dshow.dll" /pdbtype:sept
# ADD LINK32 ../dshow/strmbasd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib ../dshow/strmiids.lib /nologo /dll /debug /machine:I386 /out:"C:/Program Files/Winamp/Plugins/in_dshow.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "in_dshow - Win32 Release"
# Name "in_dshow - Win32 Debug Winamp"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\audioswitch.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowrender.cpp
# End Source File
# Begin Source File

SOURCE=.\header_asf.cpp
# End Source File
# Begin Source File

SOURCE=.\header_avi.cpp
# End Source File
# Begin Source File

SOURCE=.\header_mpg.cpp
# End Source File
# Begin Source File

SOURCE=.\info.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\grabber.h
# End Source File
# Begin Source File

SOURCE=.\header_asf.h
# End Source File
# Begin Source File

SOURCE=.\header_avi.h
# End Source File
# Begin Source File

SOURCE=.\header_mpg.h
# End Source File
# Begin Source File

SOURCE=.\IN2.H
# End Source File
# Begin Source File

SOURCE=.\OUT.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\in_dshow.rc
# End Source File
# End Group
# Begin Group "jnetlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\jnetlib\asyncdns.cpp
# End Source File
# Begin Source File

SOURCE=..\jnetlib\asyncdns.h
# End Source File
# Begin Source File

SOURCE=..\jnetlib\connection.cpp
# End Source File
# Begin Source File

SOURCE=..\jnetlib\connection.h
# End Source File
# Begin Source File

SOURCE=..\jnetlib\httpget.cpp
# End Source File
# Begin Source File

SOURCE=..\jnetlib\httpget.h
# End Source File
# Begin Source File

SOURCE=..\jnetlib\util.cpp
# End Source File
# Begin Source File

SOURCE=..\jnetlib\util.h
# End Source File
# End Group
# End Target
# End Project
