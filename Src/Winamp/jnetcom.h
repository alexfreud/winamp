#pragma once
#include <ocidl.h>
#include "..\Components\wac_network\wac_network_http_receiver_api.h"
#include "api.h"


class JNetCOM : public IDispatch, public ifc_downloadManagerCallback
{
public:
	JNetCOM(IDispatch *_dispatch);
	~JNetCOM();
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	STDMETHOD (Abort)();
	STDMETHOD (AddHeader)(LPCWSTR header);
	STDMETHOD (Connect)(LPCWSTR url, LPCWSTR requestMethod);
	STDMETHOD (GetContent)(VARIANT *variant);
	STDMETHOD (GetContentAsString)(VARIANT *variant);
	STDMETHOD (GetErrorString)(VARIANT *variant);
	STDMETHOD (GetHeader)(LPCWSTR header, VARIANT *variant);
	STDMETHOD (GetReply)(VARIANT *variant);
	STDMETHOD (GetReplyCode)(VARIANT *variant);
	STDMETHOD (GetUrl)(VARIANT *variant);

		/* Dispatchable */
	size_t Dispatchable_AddRef();
	size_t Dispatchable_Release();

	void OnFinish(DownloadToken token);
	void OnTick(DownloadToken token);
	void OnError(DownloadToken token, int error);
	void OnCancel(DownloadToken token);
	void OnConnect(DownloadToken token);
	void OnInit(DownloadToken token);

	void Call(PAPCFUNC func);
	/*
	methods:
	  Abort
	  AddHeader (only call this during OnInit)
	  Connect (here is where you specify the URL)
	  GetContent
	  GetContentAsString
		GetErrorString
	  GetHeader
		GetReply
	  GetReplyCode
	  GetUrl (may not be the same as what you originally connected because of redirection)
	  SetPostString (only call this during OnInit)

	callback methods in YOUR object:
		OnCancel
		OnConnect
		OnError
		OnFinish
		OnTick (called every once in a while)
		OnInit (called immediately after Connect(), you can add headers and shit here

	*/

private:
	LONG refCount;
	DWORD threadId;
	HANDLE threadHandle;
	IDispatch *dispatch;
	DownloadToken token;
	bool retained;
protected:
	RECVS_DISPATCH;
};