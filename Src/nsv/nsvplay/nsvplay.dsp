# Microsoft Developer Studio Project File - Name="nsvplay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nsvplay - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nsvplay.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nsvplay.mak" CFG="nsvplay - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nsvplay - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nsvplay - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nsvplay - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "USE_ASM" /D "NO_LAYER12" /D "DLL_DECODER_SUPPORT" /D "BUILTIN_MP3_SUPPORT" /D "BUILTIN_VP3_SUPPORT" /D "SUBTITLES_READER" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib ddraw.lib wsock32.lib vfw32.lib /nologo /subsystem:windows /machine:I386 /out:"c:\nsv\nsvplay.exe"

!ELSEIF  "$(CFG)" == "nsvplay - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "BUILTIN_VFW_SUPPORT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "USE_ASM" /D "NO_LAYER12" /D "DLL_DECODER_SUPPORT" /D "BUILTIN_MP3_SUPPORT" /D "BUILTIN_VP3_SUPPORT" /D "SUBTITLES_READER" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib winmm.lib wsock32.lib vfw32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\nsv\nsvplay.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "nsvplay - Win32 Release"
# Name "nsvplay - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\audiostub.cpp
# End Source File
# Begin Source File

SOURCE=.\audiostub.h
# End Source File
# End Group
# Begin Group "vp3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\vp32\include\duck_dxl.h
# End Source File
# Begin Source File

SOURCE=.\vp3stub.cpp
# End Source File
# Begin Source File

SOURCE=.\vp3stub.h
# End Source File
# Begin Source File

SOURCE=..\..\vp32\lib\win32\Release\s_cpuid.lib
# End Source File
# Begin Source File

SOURCE=..\..\vp32\lib\win32\Release\s_dxv.lib
# End Source File
# Begin Source File

SOURCE=..\..\vp32\lib\win32\Release\s_sal.lib
# End Source File
# Begin Source File

SOURCE=..\..\vp32\lib\win32\Release\s_vp31d.lib
# End Source File
# End Group
# Begin Group "mp3dec"

# PROP Default_Filter ""
# Begin Group "fhg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\mp3dec\bitsequence.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\bitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\bitstream.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\conceal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\conceal.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\crc16.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\crc16.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\giobase.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffdec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffdec.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffmanbitobj.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffmandecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffmandecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffmantable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\huffmantable.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\l3table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\l3table.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mdct.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mdct.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp2decode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp2decode.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3decode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3decode.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3quant.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3quant.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3read.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3read.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3ssc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3ssc.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3sscdef.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3streaminfo.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3tools.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mp3tools.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpeg.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpegbitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpegbitstream.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpegheader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpegheader.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpgadecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\mpgadecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\polyphase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mp3dec\polyphase.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\mp3stub.cpp
# End Source File
# Begin Source File

SOURCE=.\mp3stub.h
# End Source File
# End Group
# Begin Group "nsv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\nsvbs.h
# End Source File
# Begin Source File

SOURCE=..\nsvlib.cpp
# End Source File
# Begin Source File

SOURCE=..\nsvlib.h
# End Source File
# End Group
# Begin Group "jnetlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\jnetlib\asyncdns.cpp
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\asyncdns.h
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\connection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\connection.h
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\httpget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\httpget.h
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\jnetlib.h
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\netinc.h
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\jnetlib\util.h
# End Source File
# End Group
# Begin Group "video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vid_ddraw.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_ddraw.h
# End Source File
# Begin Source File

SOURCE=.\vid_overlay.cpp
# End Source File
# Begin Source File

SOURCE=.\vid_overlay.h
# End Source File
# Begin Source File

SOURCE=.\video.cpp
# End Source File
# Begin Source File

SOURCE=.\video.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\about.h
# End Source File
# Begin Source File

SOURCE=.\decoders.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\nsvdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\nsvplay.rc
# End Source File
# Begin Source File

SOURCE=.\readers.cpp
# End Source File
# Begin Source File

SOURCE=.\subtitles.cpp
# End Source File
# Begin Source File

SOURCE=.\wndmenu.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\subtitles.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\nsv_logo.bmp
# End Source File
# End Group
# End Target
# End Project
