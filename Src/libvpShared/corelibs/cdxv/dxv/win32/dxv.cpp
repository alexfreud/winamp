#include <stdio.h>
#include <windows.h>
#include <windowsx.h>


// ************************************************************************
// FUNCTION : DllMain( HINSTANCE, DWORD, LPVOID )
// PURPOSE  : DllMain is called by the C run-time library from the
//            _DllMainCRTStartup entry point.  The DLL entry point gets
//            called (entered) on the following events: "Process Attach",
//            "Thread Attach", "Thread Detach" or "Process Detach".
// COMMENTS : No initialization is needed here so this entry point simply
//            returns TRUE.
// ************************************************************************
BOOL WINAPI
DllMain( HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
  UNREFERENCED_PARAMETER( hInstDLL );
  UNREFERENCED_PARAMETER( fdwReason );
  UNREFERENCED_PARAMETER( lpvReserved );
  
  return( TRUE );
}

int DXV_GetVersion()
{
	return (int)0x0365;
}

extern "C" {

char* pannounce;
char *announcestart;
#define ANNBUFSIZE 2048

FILE* hf = NULL;

void Announcement(const char* lpszString)
{
#if _DEBUG
	if (!hf)	{
		hf = fopen("Announce.txt","w");
	}
	if (hf) {
		fprintf(hf,lpszString);
		fflush(hf);
	}
#endif
	if ((2 * strlen(lpszString) + pannounce) > announcestart + ANNBUFSIZE)	{
		pannounce = announcestart + ANNBUFSIZE - 2 * strlen(lpszString);	// lock up at end
	}
	strcpy(pannounce,lpszString);  // copy and bump
	pannounce += strlen(lpszString);
}

void AnnDone()
{
#if _DEBUG
	Announcement("Closing Announcements");
	if(hf) fclose(hf);
//	hf = 0;			// don't reset handle or file will re-open on next call
#endif
}

void ErrorBuffer(char *errorbuf)
{
	pannounce = announcestart = errorbuf;
}

}