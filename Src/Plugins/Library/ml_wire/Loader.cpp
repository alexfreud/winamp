#include "main.h"
#include "api.h"
#include "../winamp/wa_ipc.h"
#include "DownloadStatus.h"
using namespace Nullsoft::Utility;
static WNDPROC wa_oldWndProc=0;

/* protocol must be all lower case */
bool ProtocolMatch(const char *file, const char *protocol)
{
	size_t protSize = strlen(protocol);
	for (size_t i=0;i!=protSize;i++)
	{
		if (!file[i] 
		|| tolower(file[i]) != protocol[i])
			return false;
	}
	return true;
}

LRESULT CALLBACK LoaderProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if 0 // not ready to take links yet... too buggy/weird at this point ...
	if (uMsg == WM_COPYDATA)
	{
		COPYDATASTRUCT *copyData  = (COPYDATASTRUCT *)lParam;
		if (copyData->dwData == IPC_ENQUEUEFILE)
		{
			const char *file = (const char *)copyData->lpData;
			if (ProtocolMatch(file, "feed://"))
			{
				Channel newFeed;
				newFeed.url = AutoWide((const char *)copyData->lpData);
				if (DownloadFeedInformation(newFeed)==DOWNLOADRSS_SUCCESS)
				{
				AutoLock lock(channels);
				channels.push_back(newFeed);
				}
				return 0;
			}
			else
				if (ProtocolMatch(file, "http://"))
				{
					// nothing for now, we want to do a head request tho
					JNL_HTTPGet head;
					head.connect(file, 0, "HEAD");
					int ret;
					do
					{
						ret = head.run();
						Sleep(50);
					} while (ret != -1 && ret != 1);

					if (ret!=-1)
					{
						char *contentType = head.getheader("Content-Type");
//						if (contentType)
							//MessageBoxA(NULL, contentType, contentType, MB_OK);
						if (strstr(contentType, "application/rss+xml") == contentType)
						{
							MessageBox(NULL, L"woo!", L"woo!", MB_OK);
							return 0;
						}
						if (strstr(contentType, "application/xml") == contentType)
						{
							MessageBox(NULL, L"regular xml", L"application/xml", MB_OK);
							return 0;
						}
						if (strstr(contentType, "text/xml") == contentType)
						{
							MessageBox(NULL, L"regular xml", L"text/xml", MB_OK);
							return 0;
						}

					}

				}
		}
	}
#endif
	if (wa_oldWndProc)
		return CallWindowProc(wa_oldWndProc, hwnd, uMsg, wParam, lParam);
	else
		return 0;
}

void BuildLoader(HWND winampWindow)
{
	if (IsWindowUnicode(winampWindow))
		wa_oldWndProc=(WNDPROC) SetWindowLongPtrW(winampWindow,GWLP_WNDPROC,(LONG_PTR)LoaderProc);
	else
		wa_oldWndProc=(WNDPROC) SetWindowLongPtrA(winampWindow,GWLP_WNDPROC,(LONG_PTR)LoaderProc);
}

void DestroyLoader(HWND winampWindow)
{
	//if (wa_oldWndProc)
	//	SetWindowLong(winampWindow,GWL_WNDPROC,(LONG)wa_oldWndProc);
	//wa_oldWndProc=0;
}