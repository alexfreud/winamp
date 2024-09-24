#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLOG_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLOG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <map>

class ifc_omservice;

class SetupLog 
{
public:
	typedef enum
	{
		opUnknown = 0,
		opServiceAdded = 1,
		opServiceRemoved = 2,
	};

protected:
	SetupLog();
	~SetupLog();

public:
	static SetupLog *Open();
	static HRESULT Erase();

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT LogServiceById(UINT serviceUid, UINT operation);
	HRESULT LogService(ifc_omservice *service, UINT operation);
	HRESULT Save();
	HRESULT Send(HANDLE completeEvent);

	BOOL IsOperationSupported(UINT operation);

	

protected:
	typedef std::map<UINT, UINT> ServiceMap;
	friend static size_t SetupLog_GetMaxServiceIdCount(SetupLog::ServiceMap *serviceMap);
	friend static HRESULT SetupLog_FormatServiceId(SetupLog::ServiceMap *serviceMap, INT operation, LPSTR pszBuffer, size_t cchBufferMax);

protected:
	ULONG ref;
	ServiceMap serviceMap;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLOG_HEADER