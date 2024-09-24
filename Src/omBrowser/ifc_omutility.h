#ifndef NULLSOFT_WINAMP_OMUTILITY_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMUTILITY_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {14E8C9B6-1BA4-4e8d-AD26-FA848813CC5B}
static const GUID IFC_OmUtility = 
{ 0x14e8c9b6, 0x1ba4, 0x4e8d, { 0xad, 0x26, 0xfa, 0x84, 0x88, 0x13, 0xcc, 0x5b } };

#define RESPATH_TARGETIE		0x0001		// IE safe path
#define RESPATH_COMPACT			0x0002		// compact path relative to winamp location if possible

class ifc_omcachemanager;
class ifc_mlnavigationhelper;
class ifc_omimageloader;
class ifc_omgraphics;
class ifc_omstoragehelper;

class __declspec(novtable) ifc_omutility : public Dispatchable
{
public:
	typedef void ( CALLBACK *ThreadCallback )( ULONG_PTR /*param*/ );
	typedef void ( CALLBACK *ThreadCallback2 )( Dispatchable *object, ULONG_PTR /*param1*/, ULONG_PTR /*param2*/ );

protected:
	ifc_omutility() {}
	~ifc_omutility() {}

public:
	HRESULT EnsurePathExist( const wchar_t *directory );
	HRESULT MakeResourcePath( wchar_t *buffer, unsigned int bufferMax, HINSTANCE instance, const wchar_t *type, const wchar_t *name, unsigned int flags );

	HRESULT GetCacheManager( ifc_omcachemanager **cacheManager );
	HRESULT GetMlNavigationHelper( HWND hLibrary, ifc_mlnavigationhelper **helper );
	HRESULT QueryImageLoader( HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader );
	HRESULT GetGraphics( ifc_omgraphics **graphics );
	HRESULT PostMainThreadCallback( ThreadCallback callback, ULONG_PTR param );
	HRESULT PostMainThreadCallback2( ThreadCallback2 callback, Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2 );
	HRESULT GetStorageHelper( ifc_omstoragehelper **helper );

public:
	DISPATCH_CODES
	{	
		API_ENSUREPATHEXIST         = 10,
		API_MAKERESPATH             = 20,
		API_GETCACHEMANAGER         = 30,
		API_GETMLNAVIGATIONHELPER   = 40,
		API_QUERYIMAGELOADER        = 50,
		API_GETGRAPHICS             = 60,
		API_POSTMAINTHREADCALLBACK  = 70,
		API_POSTMAINTHREADCALLBACK2 = 80,
		API_GETSTORAGEHELPER        = 90,
	};
};

inline HRESULT ifc_omutility::EnsurePathExist(const wchar_t *directory)
{
	return _call(API_ENSUREPATHEXIST, (HRESULT)E_NOTIMPL, directory);
}

inline HRESULT ifc_omutility::MakeResourcePath(wchar_t *buffer, unsigned int bufferMax, HINSTANCE instance, const wchar_t *type, const wchar_t *name, unsigned int flags)
{
	return _call(API_MAKERESPATH, (HRESULT)E_NOTIMPL, buffer, bufferMax, instance, type, name, flags);
}

inline HRESULT ifc_omutility::GetCacheManager(ifc_omcachemanager **cacheManager)
{
	return _call(API_GETCACHEMANAGER, (HRESULT)E_NOTIMPL, cacheManager);
}

inline HRESULT ifc_omutility::GetMlNavigationHelper(HWND hLibrary, ifc_mlnavigationhelper **helper)
{
	return _call(API_GETMLNAVIGATIONHELPER, (HRESULT)E_NOTIMPL, hLibrary, helper);
}

inline HRESULT ifc_omutility::QueryImageLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader)
{
	return _call(API_QUERYIMAGELOADER, (HRESULT)E_NOTIMPL, hInstance, pszName, fPremultiply, imageLoader);
}

inline HRESULT ifc_omutility::GetGraphics(ifc_omgraphics **graphics)
{
	return _call(API_GETGRAPHICS, (HRESULT)E_NOTIMPL, graphics);
}

inline HRESULT ifc_omutility::PostMainThreadCallback(ThreadCallback callback, ULONG_PTR param)
{
	return _call(API_POSTMAINTHREADCALLBACK, (HRESULT)E_NOTIMPL, callback, param);
}

inline HRESULT ifc_omutility::PostMainThreadCallback2(ThreadCallback2 callback, Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2)
{
	return _call(API_POSTMAINTHREADCALLBACK2, (HRESULT)E_NOTIMPL, callback, object, param1, param2);
}

inline HRESULT ifc_omutility::GetStorageHelper(ifc_omstoragehelper **helper)
{
	return _call(API_GETSTORAGEHELPER, (HRESULT)E_NOTIMPL, helper);
}

#endif //NULLSOFT_WINAMP_OMUTILITY_INTERFACE_HEADER