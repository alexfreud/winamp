# Microsoft Developer Studio Project File - Name="pfc_unicode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pfc_unicode - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pfc_unicode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pfc_unicode.mak" CFG="pfc_unicode - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pfc_unicode - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pfc_unicode - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pfc_unicode - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_unicode"
# PROP Intermediate_Dir "Release_unicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PFC_UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pfc_unicode - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_unicode"
# PROP Intermediate_Dir "Debug_unicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "PFC_UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pfc_unicode - Win32 Release"
# Name "pfc_unicode - Win32 Debug"
# Begin Source File

SOURCE=.\cfg_var.cpp
# End Source File
# Begin Source File

SOURCE=.\cfg_var.h
# End Source File
# Begin Source File

SOURCE=.\critsec.h
# End Source File
# Begin Source File

SOURCE=.\grow_buf.cpp
# End Source File
# Begin Source File

SOURCE=.\grow_buf.h
# End Source File
# Begin Source File

SOURCE=.\mem_block.h
# End Source File
# Begin Source File

SOURCE=.\pfc.h
# End Source File
# Begin Source File

SOURCE=.\ptr_list.h
# End Source File
# Begin Source File

SOURCE=.\string.cpp
# End Source File
# Begin Source File

SOURCE=.\string.h
# End Source File
# Begin Source File

SOURCE=.\string_unicode.cpp
# End Source File
# Begin Source File

SOURCE=.\string_unicode.h
# End Source File
# End Target
# End Project
