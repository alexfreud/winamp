#include "Main.h"

void WaitForEvent(HANDLE hEvent, DWORD msMaxWaitTime)
{
	//  DWORD   i;
	MSG msg;
	const unsigned long eachWait = 10;
	unsigned long totalWait = 0;

	while (WaitForSingleObject(hEvent, eachWait) == WAIT_TIMEOUT)
	{
		while (PeekMessage(&msg, (HWND) NULL, 0, 0, PM_REMOVE))
		{
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		totalWait += eachWait;
		if (totalWait >= msMaxWaitTime)
			break;

	}
}