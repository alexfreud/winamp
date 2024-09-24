#include "main.h"
#include "resource.h"
#include "../winamp/in2.h"
#include "api__in_mp4.h"

int infoDlg(const wchar_t *fn, HWND hwnd)
{ // this has been made obsolete by the below.
	return 0;
}

extern "C"
{
	// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
	// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
	__declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
	{
		return 1;
	}
	// no advanced pane in this plugin :)
};
