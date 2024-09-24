#pragma once
#include <ocidl.h>
#include <vector>
#include <string>
#include "../nu/AutoLock.h"
#include "JSAPI_Info.h"

namespace JSAPI2
{
	class MediaCoreAPI : public IDispatch
	{
	public:
		MediaCoreAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		~MediaCoreAPI();
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

		/* For CallbackManager */
		bool OverrideMetadata(const wchar_t *filename, const wchar_t *tag, wchar_t *out, size_t outCch);
	private:
		const wchar_t *key;
		volatile LONG refCount;
		JSAPI::ifc_info *info;
		
struct metadata_info
{
	std::wstring url;
	std::wstring tag;
	std::wstring metadata;
};
typedef std::vector<metadata_info> MetadataMap;
MetadataMap metadataMap;
Nullsoft::Utility::LockGuard metadataGuard;

		STDMETHOD (IsRegisteredExtension)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (GetMetadata)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (AddMetadataHook)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (RemoveMetadataHook)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		void RemoveMetadataHook(const wchar_t *filename);
		void RemoveMetadataHook(const wchar_t *filename, const wchar_t *tag);
	};
}