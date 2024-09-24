#ifndef _EVNTSINK_H
#define _EVNTSINK_H

class CEventSink : public IDispatch
{
    private:
        ULONG       m_cRefs;        // ref count

    public:
        CEventSink();

    public:
        // *** IUnknown Methods ***
        STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        // *** IDispatch Methods ***
        STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames,	unsigned int cNames, LCID lcid,	DISPID FAR* rgdispid);
        STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
        STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
        STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
};

#endif