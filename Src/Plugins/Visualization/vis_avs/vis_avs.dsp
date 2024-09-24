# Microsoft Developer Studio Project File - Name="vis_avs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vis_avs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vis_avs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vis_avs.mak" CFG="vis_avs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vis_avs - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vis_avs - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vis_avs - Win32 Laser Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vis_avs - Win32 NoMMX Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/vis_avs", RDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vis_avs - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "evallib/" /I "../Wasabi" /D "NDEBUG" /D "WA2_EMBED" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /D "AVS_MEGABUF_SUPPORT" /FD /c
# SUBTRACT CPP /Z<none> /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /map /machine:I386 /out:"c:\progra~1\winamp\plugins\vis_avs.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "evallib/" /D "_DEBUG" /D "WA2_EMBED" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /D "AVS_MEGABUF_SUPPORT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /debug /machine:I386 /out:"c:\progra~1\winamp\plugins\vis_avs.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Laser Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vis_avs___Win32_Laser_Release"
# PROP BASE Intermediate_Dir "vis_avs___Win32_Laser_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vis_avs___Win32_Laser_Release"
# PROP Intermediate_Dir "vis_avs___Win32_Laser_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /I "evallib/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /I "evallib/" /D "NDEBUG" /D "LASER" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /D "AVS_MEGABUF_SUPPORT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "LASER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /machine:I386 /out:"c:\program files\winamp\plugins\vis_avs.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /machine:I386 /out:"c:\program files\winamp\plugins\vis_avs_laser.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "vis_avs - Win32 NoMMX Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vis_avs___Win32_NoMMX_Release"
# PROP BASE Intermediate_Dir "vis_avs___Win32_NoMMX_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vis_avs___Win32_NoMMX_Release"
# PROP Intermediate_Dir "vis_avs___Win32_NoMMX_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /I "evallib/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /Ob2 /I "evallib/" /D "NDEBUG" /D "NO_MMX" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VIS_PL_EXPORTS" /D "NSEEL_LOOPFUNC_SUPPORT" /D "AVS_MEGABUF_SUPPORT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /map /machine:I386 /out:"c:\progra~1\winamp\plugins\vis_avs.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib vfw32.lib /nologo /dll /map /machine:I386 /out:"c:\progra~1\winamp\plugins\vis_avs.dll"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "vis_avs - Win32 Release"
# Name "vis_avs - Win32 Debug"
# Name "vis_avs - Win32 Laser Release"
# Name "vis_avs - Win32 NoMMX Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Renders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\r_avi.cpp
# End Source File
# Begin Source File

SOURCE=.\r_blit.cpp
# End Source File
# Begin Source File

SOURCE=.\r_blur.cpp
# End Source File
# Begin Source File

SOURCE=.\r_bpm.cpp
# End Source File
# Begin Source File

SOURCE=.\r_bright.cpp
# End Source File
# Begin Source File

SOURCE=.\r_bspin.cpp
# End Source File
# Begin Source File

SOURCE=.\r_bump.cpp
# End Source File
# Begin Source File

SOURCE=.\r_chanshift.cpp
# End Source File
# Begin Source File

SOURCE=.\r_clear.cpp
# End Source File
# Begin Source File

SOURCE=.\r_colorfade.cpp
# End Source File
# Begin Source File

SOURCE=.\r_colorreduction.cpp
# End Source File
# Begin Source File

SOURCE=.\r_comment.cpp
# End Source File
# Begin Source File

SOURCE=.\r_contrast.cpp
# End Source File
# Begin Source File

SOURCE=.\r_dcolormod.cpp
# End Source File
# Begin Source File

SOURCE=.\r_ddm.cpp
# End Source File
# Begin Source File

SOURCE=.\r_dmove.cpp
# End Source File
# Begin Source File

SOURCE=.\r_dotfnt.cpp
# End Source File
# Begin Source File

SOURCE=.\r_dotgrid.cpp
# End Source File
# Begin Source File

SOURCE=.\r_dotpln.cpp
# End Source File
# Begin Source File

SOURCE=.\r_fadeout.cpp
# End Source File
# Begin Source File

SOURCE=.\r_fastbright.cpp
# End Source File
# Begin Source File

SOURCE=.\r_grain.cpp
# End Source File
# Begin Source File

SOURCE=.\r_interf.cpp
# End Source File
# Begin Source File

SOURCE=.\r_interleave.cpp
# End Source File
# Begin Source File

SOURCE=.\r_invert.cpp
# End Source File
# Begin Source File

SOURCE=.\r_linemode.cpp
# End Source File
# Begin Source File

SOURCE=.\r_mirror.cpp
# End Source File
# Begin Source File

SOURCE=.\r_mosaic.cpp
# End Source File
# Begin Source File

SOURCE=.\r_multidelay.cpp
# End Source File
# Begin Source File

SOURCE=.\r_multiplier.cpp
# End Source File
# Begin Source File

SOURCE=.\r_nfclr.cpp
# End Source File
# Begin Source File

SOURCE=.\r_onetone.cpp
# End Source File
# Begin Source File

SOURCE=.\r_oscring.cpp
# End Source File
# Begin Source File

SOURCE=.\r_oscstar.cpp
# End Source File
# Begin Source File

SOURCE=.\r_parts.cpp
# End Source File
# Begin Source File

SOURCE=.\r_picture.cpp
# End Source File
# Begin Source File

SOURCE=.\r_rotblit.cpp
# End Source File
# Begin Source File

SOURCE=.\r_rotstar.cpp
# End Source File
# Begin Source File

SOURCE=.\r_scat.cpp
# End Source File
# Begin Source File

SOURCE=.\r_shift.cpp
# End Source File
# Begin Source File

SOURCE=.\r_simple.cpp
# End Source File
# Begin Source File

SOURCE=.\r_sscope.cpp
# End Source File
# Begin Source File

SOURCE=.\r_stack.cpp
# End Source File
# Begin Source File

SOURCE=.\r_stars.cpp
# End Source File
# Begin Source File

SOURCE=.\r_svp.cpp
# End Source File
# Begin Source File

SOURCE=.\r_text.cpp
# End Source File
# Begin Source File

SOURCE=.\r_timescope.cpp
# End Source File
# Begin Source File

SOURCE=.\r_trans.cpp
# End Source File
# Begin Source File

SOURCE=.\r_videodelay.cpp
# End Source File
# Begin Source File

SOURCE=.\r_water.cpp
# End Source File
# Begin Source File

SOURCE=.\r_waterbump.cpp
# End Source File
# End Group
# Begin Group "EvalLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\avs_eelif.cpp

!IF  "$(CFG)" == "vis_avs - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Debug"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Laser Release"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 NoMMX Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\avs_eelif.h
# End Source File
# Begin Source File

SOURCE="..\ns-eel\megabuf.c"

!IF  "$(CFG)" == "vis_avs - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Debug"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Laser Release"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 NoMMX Release"

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

!IF  "$(CFG)" == "vis_avs - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Debug"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 Laser Release"

!ELSEIF  "$(CFG)" == "vis_avs - Win32 NoMMX Release"

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
# Begin Group "Render utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\linedraw.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\r_defs.h
# End Source File
# Begin Source File

SOURCE=.\r_list.cpp
# End Source File
# Begin Source File

SOURCE=.\r_list.h
# End Source File
# Begin Source File

SOURCE=.\r_transition.cpp
# End Source File
# Begin Source File

SOURCE=.\r_transition.h
# End Source File
# Begin Source File

SOURCE=.\r_unkn.cpp
# End Source File
# Begin Source File

SOURCE=.\r_unkn.h
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# End Group
# Begin Group "laser"

# PROP Default_Filter ""
# Begin Group "laser renders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\laser\rl_beathold.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\rl_bren.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\rl_cones.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\rl_line.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\rl_trans.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\laser\laser.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\laserline.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\laserline.h
# End Source File
# Begin Source File

SOURCE=.\laser\ld32.c
# End Source File
# Begin Source File

SOURCE=.\laser\Ld32.h
# End Source File
# Begin Source File

SOURCE=.\laser\linelist.cpp
# End Source File
# Begin Source File

SOURCE=.\laser\linelist.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bpm.cpp
# End Source File
# Begin Source File

SOURCE=.\cfgwin.cpp
# End Source File
# Begin Source File

SOURCE=.\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\res.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rlib.cpp
# End Source File
# Begin Source File

SOURCE=.\TIMING.C
# End Source File
# Begin Source File

SOURCE=.\TIMING.H
# End Source File
# Begin Source File

SOURCE=.\undo.cpp
# End Source File
# Begin Source File

SOURCE=.\wnd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ape.h
# End Source File
# Begin Source File

SOURCE=.\bpm.h
# End Source File
# Begin Source File

SOURCE=.\cfgwnd.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\rlib.h
# End Source File
# Begin Source File

SOURCE=.\undo.h
# End Source File
# Begin Source File

SOURCE=.\wnd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\avs-hilited.png"
# End Source File
# Begin Source File

SOURCE=".\avs-normal.png"
# End Source File
# Begin Source File

SOURCE=".\avs-selected.png"
# End Source File
# Begin Source File

SOURCE=.\bump_lig.bin
# End Source File
# Begin Source File

SOURCE=.\color_mo.bin
# End Source File
# Begin Source File

SOURCE=.\dyn_dist.bin
# End Source File
# Begin Source File

SOURCE=.\dyn_move.bin
# End Source File
# Begin Source File

SOURCE=.\dyn_shift.bin
# End Source File
# Begin Source File

SOURCE=.\effect_l.bin
# End Source File
# Begin Source File

SOURCE=.\help_1.bin
# End Source File
# Begin Source File

SOURCE=.\help_2.bin
# End Source File
# Begin Source File

SOURCE=.\help_3.bin
# End Source File
# Begin Source File

SOURCE=.\help_4.bin
# End Source File
# Begin Source File

SOURCE=.\movement.bin
# End Source File
# Begin Source File

SOURCE=.\supersco.bin
# End Source File
# End Group
# Begin Group "Text files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vis_avs.txt
# End Source File
# Begin Source File

SOURCE=.\whatsnew.txt
# End Source File
# End Group
# End Target
# End Project
