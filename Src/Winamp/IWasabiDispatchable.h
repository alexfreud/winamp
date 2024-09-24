#pragma once
#include <unknwn.h>
#include <bfc/dispatch.h>
// {6BB64D00-F2E1-4c43-B9AA-A762506B8BB6}
static const GUID IID_IWasabiDispatchable = 
{ 0x6bb64d00, 0xf2e1, 0x4c43, { 0xb9, 0xaa, 0xa7, 0x62, 0x50, 0x6b, 0x8b, 0xb6 } };


class IWasabiDispatchable : public IUnknown
{
public:
	 virtual HRESULT STDMETHODCALLTYPE QueryDispatchable(REFIID riid, Dispatchable **ppDispatchable) = 0;
};