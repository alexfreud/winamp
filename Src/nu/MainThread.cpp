#include "MainThread.h"


LRESULT CALLBACK MarshallProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_USER:
		{
			Lambda *lambda = (Lambda *)wParam;
			lambda->Run();
			delete lambda;
			return 0;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


MainThread::MainThread()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style	= 0;
	wcex.lpfnWndProc	= (WNDPROC)MarshallProc;
	wcex.cbClsExtra	= 0;
	wcex.cbWndExtra	= 0;
	wcex.hInstance	= GetModuleHandle(0);
	wcex.hIcon	= 0;
	wcex.hCursor	= 0;
	wcex.hbrBackground	= 0;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= "MainWindowMarshaller";
	wcex.hIconSm	= 0;
	RegisterClassEx(&wcex);

	mainWindow = CreateWindow( "MainWindowMarshaller", "MainWindowMarshaller", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(0), NULL);
}

MainThread mainThread;
