#ifndef NULLSOFT_WINAMP_OMSERVICE_LIST_ENUMERATOR_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_LIST_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_omserviceenum.h"
#include <vector>

class ifc_omservice;

class OmServiceList : public ifc_omserviceenum
{
protected:
	OmServiceList();
	~OmServiceList();

public:
	static HRESULT CreateInstance(OmServiceList **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omserviceenum */
	HRESULT Next(unsigned long listSize, ifc_omservice **elementList, unsigned long *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(unsigned long elementCount);

public:
	HRESULT Add(ifc_omservice *service);
	HRESULT Remove(size_t index);
	HRESULT Clear();

protected:
	typedef std::vector<ifc_omservice*> SvcList;

protected:
	size_t ref;
	SvcList list;
	size_t cursor;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_OMSERVICE_LIST_ENUMERATOR_HEADER