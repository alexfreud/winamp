#pragma once
#include <ocidl.h>
#include "JSAPI_Info.h"
#include <vector>
#include "JSAPI_CallbackParameters.h"

namespace JSAPI2
{
	class AsyncDownloaderAPI : public IDispatch
	{
	public:
		AsyncDownloaderAPI(const wchar_t *_key, JSAPI::ifc_info *info);
		~AsyncDownloaderAPI();
		STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);
		// *** IDispatch Methods ***
		STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
		STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
		STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
		STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
	public:
		/** Callbacks
		 ** JSAPI2::CallbackManager will have been nice enough to marshall onto our thread
		 ** so we don't have to worry about that
		 */
		void OnInit(const wchar_t *url);
		void OnConnect(const wchar_t *url);
		void OnData(const wchar_t *url, size_t downloadedlen, size_t totallen);
		void OnCancel(const wchar_t *url);
		void OnError(const wchar_t *url, int error);
		void OnFinish(const wchar_t *url, const wchar_t *destfilename);
		void InvokeEvent(const wchar_t *eventName, JSAPI::CallbackParameters::PropertyTemplate *parameters, size_t parametersCount);
		//Getter for key
		const wchar_t *GetKey();
	private:
		const wchar_t *key;
		volatile LONG refCount;
		JSAPI::ifc_info *info;
		typedef std::vector<IDispatch*> EventsList;
		EventsList events;

		STDMETHOD (DownloadMedia)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (RegisterForEvents)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
		STDMETHOD (UnregisterFromEvents)(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr);
	};
}