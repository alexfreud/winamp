#include "main.h"
#include "directdraw.h"

HRESULT (WINAPI *_DirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter) = 0;

HRESULT DDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
		static int a = 0;
	if (!_DirectDrawCreate && !a)
	{
		a++;
		HINSTANCE h = LoadLibrary(L"ddraw.dll");
		if (h)
		{
			*(void**)&_DirectDrawCreate = (void*)GetProcAddress(h, "DirectDrawCreate");
		}
	}

	if (_DirectDrawCreate)
		return _DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
	else
		return S_OK; // TODO: uhhh no this should be an error :)
}