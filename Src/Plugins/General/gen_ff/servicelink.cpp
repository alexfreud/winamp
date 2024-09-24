#include <precomp.h>

// these are pragmas to force a reference to objects that otherwise are entirely decoupled from the rest of the
// program except for their static constructor code -- in this case, if the code is in a lib, the object gets
// optimized out, and we definitly do not want that
//
// generally you want to add more of these pragmas for services declared through the BEGIN_SERVICES/END_SERVICES
// macros which you want to link with

// color themes list xui object
#ifdef WASABI_COMPILE_COLORTHEMES
#pragma comment(linker, "/include:__link_ColorThemesListXuiSvc")
#endif

// config script objects
#ifdef WASABI_COMPILE_CONFIG
#pragma comment(linker, "/include:__link_ConfigObjectSvc")
#endif

// minibrowser service
#ifdef WASABI_WIDGETS_BROWSER
#pragma comment(linker, "/include:__link_MbSvc")
#endif

// skinned tooltips
#ifdef WASABI_WIDGETS_TOOLTIPS
#pragma comment(linker, "/include:__link_GroupTipsSvc")
#endif

// freetype font renderer
#ifdef WASABI_FONT_RENDERER_USE_FREETYPE
//#pragma comment(linker, "/include:__link_FreeTypeFontRenderer_Svc")
#endif

// pldir svc
#pragma comment(linker, "/include:__link_wa2PlDirObj_Svcs")

// pleditor xuiobject
#pragma comment(linker, "/include:__link_Wa2PleditXuiSvc")

// song ticker xui object
#pragma comment(linker, "/include:__link_wa2SongTicker_Svcs")

// Winamp Config script object
#pragma comment(linker, "/include:__link_WinampConfig_svcs")

// progress grid xui object 
#ifdef WASABI_WIDGETS_MEDIASLIDERS
#pragma comment(linker, "/include:__link_ProgressGridXuiSvc")
#endif

// gradient xui object 
#ifdef WASABI_WIDGETS_MEDIASLIDERS
#pragma comment(linker, "/include:__link_GradientXuiSvc")
#endif

#pragma comment(linker, "/include:__link_GroupXFadeXuiSvc")

#pragma comment(linker, "/include:__link_GradientGen_Svc")

#pragma comment(linker, "/include:__link_OsEdgeGen_Svc")

#pragma comment(linker, "/include:__link_PolyGen_Svc")

#pragma comment(linker, "/include:__link_SolidGen_Svc")

#pragma comment(linker, "/include:__link_ScriptCore_Svc")



//#pragma comment(linker, "/include:__link_ColorEditor_Svc")

