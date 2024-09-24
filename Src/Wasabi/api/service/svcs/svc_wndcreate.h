#ifndef _SVC_WNDCREATE_H
#define _SVC_WNDCREATE_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

#define BUCKETITEM L"buck"
#define PLAYLISTSIDECAR L"plsc"

class ifc_window;

class NOVTABLE svc_windowCreate : public Dispatchable
{
public:
	static FOURCC getServiceType() { return WaSvc::WINDOWCREATE; }

	int testGuid(GUID g);
	ifc_window *createWindowByGuid(GUID g, ifc_window *parent);
	int testType(const wchar_t *windowtype);
	ifc_window *createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n = 0);
	int destroyWindow(ifc_window *w);

	int refcount();	// how many windows created

	enum {
	    TESTGUID = 100,
	    CREATEWINDOWBYGUID = 200,
	    TESTTYPE = 301,
	    CREATEWINDOWOFTYPE = 402,
	    DESTROYWINDOW = 500,
	    REFCOUNT	= 600,
	};
};

inline int svc_windowCreate::testGuid(GUID g)
{
	return _call(TESTGUID, 0, g);
}

inline ifc_window *svc_windowCreate::createWindowByGuid(GUID g, ifc_window *parent)
{
	return _call(CREATEWINDOWBYGUID, (ifc_window*)0, g, parent);
}

inline int svc_windowCreate::destroyWindow(ifc_window *w)
{
	return _call(DESTROYWINDOW, 0, w);
}

inline int svc_windowCreate::testType(const wchar_t *windowtype)
{
	return _call(TESTTYPE, 0, windowtype);
}

inline ifc_window *svc_windowCreate::createWindowOfType(const wchar_t * windowtype, ifc_window *parent, int n)
{
	return _call(CREATEWINDOWOFTYPE, (ifc_window*)0, windowtype, parent, n);
}

inline int svc_windowCreate::refcount()
{
	return _call(REFCOUNT, -1);
}

class NOVTABLE svc_windowCreateI : public svc_windowCreate
{
public:
	virtual ~svc_windowCreateI() { }
	virtual int testGuid(GUID g) { return 0; }
	virtual ifc_window *createWindowByGuid(GUID g, ifc_window *parent) { return NULL; }
	virtual int testType(const wchar_t *windowtype) { return 0; }
	virtual ifc_window *createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n) { return NULL; }
	virtual int destroyWindow(ifc_window *w) = 0;

	virtual int refcount() { return -1; }	// FUCKO: make pure abstract

protected:
	RECVS_DISPATCH;
};

#include <api/service/servicei.h>
template <class T>
class WndCreateCreator : public waServiceFactoryT<svc_windowCreate, T> {};

template <class T>
class WndCreateCreatorSingle : public waServiceFactoryTSingle<svc_windowCreate, T> {};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class WindowCreateByGuidEnum : public SvcEnumT<svc_windowCreate>
{
public:
	WindowCreateByGuidEnum(GUID _g) : targetguid(_g) { }

protected:
	virtual int testService(svc_windowCreate *svc)
	{
		return svc->testGuid(targetguid);
	}

private:
	GUID targetguid;
};

class WindowCreateByTypeEnum : public SvcEnumT<svc_windowCreate>
{
public:
	WindowCreateByTypeEnum(const wchar_t *_wtype) : wtype(_wtype) { }

protected:
	virtual int testService(svc_windowCreate *svc)
	{
		return svc->testType(wtype);
	}

private:
	StringW wtype;
};
#endif
