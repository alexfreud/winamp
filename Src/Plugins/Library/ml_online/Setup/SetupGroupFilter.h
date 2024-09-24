#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPFILTER_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPFILTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <vector>

class ifc_omservice;

class __declspec(novtable) SetupGroupFilter
{
public:
	typedef enum
	{
		serviceIgnore			= 0x00000000,
		serviceInclude			= 0x00000001,
		serviceForceSubscribe	= 0x00000002,
		serviceForceUnsubscribe = 0x00000004,
	} FilterResult;
protected:
	SetupGroupFilter(const GUID *filterId);
	virtual ~SetupGroupFilter();

public:
	static HRESULT CreateInstance(const GUID *filterId, SetupGroupFilter **instance);

public:
	virtual ULONG AddRef();
	virtual ULONG Release();

	virtual HRESULT GetId(GUID *filterId);

	virtual HRESULT Initialize() = 0;
	virtual HRESULT ProcessService(ifc_omservice *service, UINT *filterResult) = 0;

protected:
	typedef std::vector<UINT> ServiceIdList;
	static BOOL CALLBACK AppendServiceIdCallback(UINT serviceId, void *data);
	
protected:
	ULONG ref;
	GUID id;
};


// {2F45FBDF-4372-4def-B20C-C6F1BAE5AE85}
static const GUID FUID_SetupFeaturedGroupFilter = 
{ 0x2f45fbdf, 0x4372, 0x4def, { 0xb2, 0xc, 0xc6, 0xf1, 0xba, 0xe5, 0xae, 0x85 } };


class SetupFeaturedGroupFilter : public SetupGroupFilter
{
protected:
	SetupFeaturedGroupFilter();
	~SetupFeaturedGroupFilter();
public:
	static HRESULT CreateInstance(SetupFeaturedGroupFilter **instance);

public:
	HRESULT Initialize();
	HRESULT ProcessService(ifc_omservice *service, UINT *filterResult);

protected:
	ServiceIdList filterList;
};

// {7CA8722D-8B11-43a0-8F55-533C9DE3D73E}
static const GUID FUID_SetupKnownGroupFilter = 
{ 0x7ca8722d, 0x8b11, 0x43a0, { 0x8f, 0x55, 0x53, 0x3c, 0x9d, 0xe3, 0xd7, 0x3e } };


class SetupKnownGroupFilter : public SetupGroupFilter
{
protected:
	SetupKnownGroupFilter();
	~SetupKnownGroupFilter();
public:
	static HRESULT CreateInstance(SetupKnownGroupFilter **instance);

public:
	HRESULT Initialize();
	HRESULT ProcessService(ifc_omservice *service, UINT *filterResult);

protected:
	ServiceIdList filterList;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPFILTER_HEADER