/****************************************************************************
*
*   Module Title :     VFW_COMP_MAIN.c
*
*   Description  :     Main for video codec demo compression dll
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#define INC_WIN_HEADER      1
#include <windows.h>

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
 
/****************************************************************************
*  Module statics.
*****************************************************************************
*/        

unsigned long cProcessesAttached = 0;         

HINSTANCE hInstance;        /* Application instance handle. */

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Imports
*****************************************************************************
*/
extern void VPEInitLibrary(void);
extern void VPEDeInitLibrary(void);


BOOL WINAPI DllMain(HANDLE hInst, DWORD fdwReason, LPVOID lpReserved)
{
	if ( fdwReason == DLL_PROCESS_ATTACH )
	{
        hInstance = hInst;
		if ( cProcessesAttached++ )
		{	
			return(TRUE);         // Not the first initialization.
    	}
		else
		{
			// initialize all the global variables in the dll
			VPEInitLibrary();

			return TRUE;
		}
	}

	else if ( fdwReason == DLL_PROCESS_DETACH )
	{
		if (--cProcessesAttached)
		{
			return TRUE;
		}
		else
		{
			VPEDeInitLibrary();
			return TRUE;
		}
	}
	else
		return FALSE;
}


