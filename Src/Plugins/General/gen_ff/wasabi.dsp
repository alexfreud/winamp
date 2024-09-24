# Microsoft Developer Studio Project File - Name="wasabi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wasabi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wasabi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wasabi.mak" CFG="wasabi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wasabi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wasabi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wasabi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "_build/Release"
# PROP Intermediate_Dir "_build/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zd /O2 /I "../gen_ff" /I "../wasabi" /I "../wasabi/api/font/freetype/freetype2/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "wasabi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "_build/Debug"
# PROP Intermediate_Dir "_build/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../gen_ff" /I "../wasabi" /I "../wasabi/api/font/freetype/freetype2/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"precomp.h" /FD /GZ /c
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

# Name "wasabi - Win32 Release"
# Name "wasabi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "system"

# PROP Default_Filter ""
# Begin Group "file"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\file\filename.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\file\readdir.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\file\recursedir.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\file\wildcharsenum.cpp
# End Source File
# End Group
# Begin Group "memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\foreach.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\freelist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\loadlib.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\memblock.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\ptrlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\stack.cpp
# End Source File
# End Group
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\util\profiler.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\util\timefmt.cpp
# End Source File
# End Group
# Begin Group "string"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\string\bigstring.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\string\encodedstr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\string\playstring.cpp
# End Source File
# Begin Source File

SOURCE=..\Wasabi\bfc\string\string.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\string\url.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\string\utf8.cpp
# End Source File
# End Group
# Begin Group "parse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\parse\hierarchyparser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\parse\paramparser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\parse\pathparse.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\bfc\assert.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\critsec.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\depend.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\node.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\nsGUID.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\std.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\std_file.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\std_math.cpp
# End Source File
# End Group
# Begin Group "apis"

# PROP Default_Filter ""
# Begin Group "svc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\service\api_service.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\api_servicei.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\api_servicex.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\consolecb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\servicei.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svc_enum.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svccache.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcenum.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcenumbyguid.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcenumt.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactory.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactorybase.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactoryi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactoryt.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactorytsingle.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\waservicefactoryx.cpp
# End Source File
# End Group
# Begin Group "wnd"

# PROP Default_Filter ""
# Begin Group "wndclass"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\abstractwndhold.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\appbarwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\backbufferwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\basewnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\blankwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\bufferpaintwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\buttbar.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\buttwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\clickwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\editwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\editwndstring.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\embeddedxui.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\foreignwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\framewnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\gradientwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\guiobjwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\labelwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\listwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\oswnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\oswndhost.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\paintset.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\qpaintwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\rootwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\rootwndholder.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\scbkgwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\scrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\sepwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\slider.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\status.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\svcwndhold.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\tooltip.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\treewnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\typesheet.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\virtualwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\wndholder.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\wnd\api_wnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\autobitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\blending.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\contextmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\deactivatemgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\dragitemi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\filteredcolor.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\findobjectcb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\util\findopenrect.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\paintcb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\popexitcb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\popexitchecker.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\popup.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\std_wnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_wndcreate.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\timer\timerclient.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\platform\win32\win32_canvas.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\platform\win32\win32_region.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\wndcb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndtrack.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\xmlobject.cpp
# End Source File
# End Group
# Begin Group "timer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\timer\api_timer.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\timer\genwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\timer\timerapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\timer\timermul.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\timer\tmultiplex.cpp
# End Source File
# End Group
# Begin Group "app"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\application\api_application.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\application\api_applicationi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\application\api_applicationx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wac\\compon.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\util\inifile.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\application\pathmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\application\version.cpp
# End Source File
# End Group
# Begin Group "font"

# PROP Default_Filter ""
# Begin Group "freetype"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\font\freetype\freetype.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\font\freetype\freetypefont.cpp
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\font\win32\truetypefont_win32.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\font\api_font.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\font\bitmapfont.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\font\font.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\font\fontapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_font.cpp
# End Source File
# End Group
# Begin Group "syscb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\syscb\api_syscb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\api_syscbi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\api_syscbx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\cbmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\syscb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\syscbi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\syscbx.cpp
# End Source File
# End Group
# Begin Group "imgldr"

# PROP Default_Filter ""
# Begin Group "pngload"

# PROP Default_Filter ""
# Begin Group "pnglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNG.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGERROR.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGGET.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGMEM.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGREAD.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGRIO.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGRTRAN.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGRUTIL.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGSET.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pnglib\PNGTRANS.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\loader.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\pngload\pngload.cpp
# End Source File
# End Group
# Begin Group "jpgload"

# PROP Default_Filter ""
# Begin Group "jpgdlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\H2v2.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\idct.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\jidctfst.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\jpegdecoder.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\jpegdecoder.h
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\jpegdecoder.inl
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgdlib\main.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\jpgload.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\jpgload\loader_jpg.cpp
# End Source File
# End Group
# Begin Group "imggen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\bfc\draw\drawpoly.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imggen\grad.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imggen\osedge.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imggen\poly.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imggen\solid.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_imggen.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\imgldr\api_imgldr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imgldr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\imgldr\imgldrapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_imgload.cpp
# End Source File
# End Group
# Begin Group "ffskin"

# PROP Default_Filter ""
# Begin Group "widgets"

# PROP Default_Filter ""
# Begin Group "tooltips"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\grouptips.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_tooltips.cpp
# End Source File
# End Group
# Begin Group "mb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\Atl.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\iebrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\mbsvc.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_minibrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\xuibrowser.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\animlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\button.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\checkbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\combobox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\compbuck2.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\customobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\dropdownlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\edit.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\feeds\feedwatch.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\feeds\feedwatcherso.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\fx_dmove.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\bfc\draw\gradient.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\group.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\groupclickwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\grouplist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\grouptgbutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\guiradiogroup.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\guistatuscb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\historyeditbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\layer.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\mainminibrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\minibrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mb\minibrowserwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\mouseredir.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\nakedobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\objdirwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\objectactuator.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\pathpicker.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\pslider.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\sa.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\seqband.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\seqpreamp.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\seqvis.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\spanbar.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\sseeker.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\sstatus.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_xuiobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\svolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\tabsheet.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wnd\wndclass\tabsheetbar.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\text.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\tgbutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\title.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\titlebox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiaddparams.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuibookmarklist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuicheckbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuicombobox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuicustomobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuidropdownlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuieditbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiframe.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuigradientwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuigrid.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuigroupxfade.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuihideobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuihistoryedit.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuilist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuimenu.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuimenuso.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiobjdirwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuioswndhost.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuipathpicker.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiprogressgrid.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiradiogroup.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuirect.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuisendparams.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuistatus.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuitabsheet.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuithemeslist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuititlebox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuitree.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\xuiwndholder.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\skin\api_skin.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\cursormgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\gammamgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\groupmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\groupwndcreate.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\guitree.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\regioncache.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skin.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\skincb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinclr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinelem.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinfont.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinitem.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\skinparse.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_skinfilter.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_textfeed.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\feeds\textfeed.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets.cpp
# End Source File
# End Group
# Begin Group "xml"

# PROP Default_Filter ""
# Begin Group "parser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\xml\parser\hashtable.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\parser\simap.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\parser\xmlparser.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\parser\xmlrole.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\parser\xmltok.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\xml\api_xml.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlparams.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlparamsi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlparamsx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlreader.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\xml\xmlwrite.cpp
# End Source File
# End Group
# Begin Group "script"

# PROP Default_Filter ""
# Begin Group "maki"

# PROP Default_Filter ""
# Begin Group "maki classes"

# PROP Default_Filter ""
# Begin Group "encapsulators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjcb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjcbi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjcbx.cpp
# End Source File
# End Group
# Begin Group "c_script"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_browser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_button.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_checkbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_container.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_dropdownlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_edit.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_group.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_guilist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_guiobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_guitree.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_layout.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_menubutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_querylist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_rootobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_slider.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_text.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_togglebutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\c_treeitem.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_browser.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_button.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_checkbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_container.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_dropdownlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_edit.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_group.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_guilist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_guiobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_guitree.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_layout.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_menubutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_querylist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_rootobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_slider.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_text.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_togglebutton.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\h_treeitem.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\c_script\scripthook.cpp
# End Source File
# End Group
# Begin Group "scriptcore"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\script\objects\core\coreadminobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\core\coreobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\core\svc_scriptcore.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\script\objects\guiobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objcontrollert.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjcontroller.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobject.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjecti.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\rootobjectx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\sbitlist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\slist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\smap.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\spopup.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\sregion.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\systemobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\timer.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objects\wacobj.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\script\guru.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objcontroller.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\objecttable.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\script.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\scriptmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\scriptobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\scriptobji.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\scriptobjx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\vcpu.cpp
# End Source File
# End Group
# Begin Group "debugger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\api_makidebug.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\debugapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\debuggerui.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\debugsymbols.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\disasm.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\jitd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\jitdbreak.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\sdebuggerui.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\sourcecodeline.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\debugger\vcpudebug.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\script\api_maki.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\api_makii.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\script\api_makix.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_scriptobj.cpp
# End Source File
# End Group
# Begin Group "wndmgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\alphamgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\animate.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\api_wndmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\appcmds.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\autopopup.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\container.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\gc.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\layout.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\msgbox.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\resize.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\skinembed.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\skinwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\snappnt.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\wndmgr\wndmgrapi.cpp
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\core\api_core.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\syscb\callbacks\corecb.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_action.cpp
# End Source File
# End Group
# Begin Group "filereaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\filereader\api_filereader.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\filereader\local\fileread.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\filereader\\filereaderapi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\filereader\res\resread.h
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\service\svcs\svc_fileread.cpp
# End Source File
# End Group
# Begin Group "cfg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\config\api_config.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\api_configi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\api_configx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\items\attribute.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\items\attrstr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\items\cfgitemi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api/config\cfglist.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\cfgscriptobj.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\config.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\items\intarray.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\options.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\config\uioptions.cpp
# End Source File
# End Group
# Begin Group "locales"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\locales\api_locales.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\locales\api_localesi.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\locales\api_localesx.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\locales\localesmgr.cpp
# End Source File
# End Group
# Begin Group "memmgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\memmgr\api_memmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\memmgr\memmgrapi.cpp
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\util\selectfile.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\util\systray.cpp
# End Source File
# Begin Source File

SOURCE=..\wasabi\api\util\varmgr.cpp
# End Source File
# End Group
# End Group
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\stats\statswnd.cpp

!IF  "$(CFG)" == "wasabi - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "wasabi - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\wasabi\api\skin\widgets\stats\xuistats.cpp

!IF  "$(CFG)" == "wasabi - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "wasabi - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\wasabi\api\apiinit.cpp
# End Source File
# Begin Source File

SOURCE=.\precomp.cpp
# ADD CPP /Yc"precomp.h"
# End Source File
# Begin Source File

SOURCE=.\wasabicfg.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
