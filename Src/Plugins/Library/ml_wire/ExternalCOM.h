#ifndef NULLSOFT_PODCAST_PLUGIN_EXTERNAL_HEADER
#define NULLSOFT_PODCAST_PLUGIN_EXTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <atomic>

#include "../nu/dispatchTable.h"

class ExternalCOM : public IDispatch
{
public:
	typedef enum
	{
		DISPATCH_PODCAST = 777,
	} DispatchCodes;

protected:
	ExternalCOM();
	~ExternalCOM();

public:
	static HRESULT CreateInstance(ExternalCOM **instance);

	/* IUnknown*/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

protected:
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnPodcast);

	std::atomic<std::size_t> _ref = 1;
};

#endif //NULLSOFT_PODCAST_PLUGIN_EXTERNAL_HEADER