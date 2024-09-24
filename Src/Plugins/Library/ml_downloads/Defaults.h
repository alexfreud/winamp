#ifndef NULLSOFT_DEFAULTSH
#define NULLSOFT_DEFAULTSH
#include <windows.h>

extern wchar_t defaultDownloadPath[MAX_PATH];

#define DOWNLOADSSOURCEWIDTHDEFAULT		200
#define DOWNLOADSTITLEWIDTHDEFAULT		200
#define DOWNLOADSPROGRESSWIDTHDEFAULT	100
#define DOWNLOADSDATEWIDTHDEFAULTS		100
#define DOWNLOADSSIZEWIDTHDEFAULTS		100
#define DOWNLOADSPATHWIDTHDEFAULTS		200

extern int downloadsSourceWidth,
		   downloadsTitleWidth,
		   downloadsProgressWidth,
		   downloadsPathWidth,
		   downloadsSizeWidth,
		   downloadsDateWidth;

extern bool needToMakePodcastsView;

void BuildDefaults(HWND);
#endif