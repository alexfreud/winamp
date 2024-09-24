#ifndef __WASABI_API_TEXTFEED_H
#define __WASABI_API_TEXTFEED_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <api/service/services.h>

class ifc_dependent;

// {A04E3420-CBA1-4ae1-B3C0-8DE127D2B861}
static const GUID textfeed_dependent_GUID = { 0xa04e3420, 0xcba1, 0x4ae1, { 0xb3, 0xc0, 0x8d, 0xe1, 0x27, 0xd2, 0xb8, 0x61 } };

class NOVTABLE svc_textFeed : public Dispatchable
{
public:
	static FOURCC getServiceType() { return WaSvc::TEXTFEED; }
	static const GUID *depend_getClassGuid()
	{
		return &textfeed_dependent_GUID;
	}
	static const char *getServiceName() { return "Untitled Textfeed"; }

public:
	int hasFeed(const wchar_t *name);
	const wchar_t *getFeedText(const wchar_t *name);
	const wchar_t *getFeedDescription(const wchar_t *name);
	ifc_dependent *getDependencyPtr();

public:
	enum 
	{
	    Event_TEXTCHANGE = 100,  // param is const char* to id, ptr points to new text
	};

	DISPATCH_CODES
	{
	    SVCTEXTFEED_HASFEED          = 10,
	    //20,30 retired
	    SVCTEXTFEED_GETFEEDTEXT      = 40,
	    SVCTEXTFEED_GETFEEDDESC      = 45,
	    SVCTEXTFEED_GETDEPENDENCYPTR = 100,
	};
};

inline int svc_textFeed::hasFeed(const wchar_t *name)
{
	return _call(SVCTEXTFEED_HASFEED, 0, name);
}

inline const wchar_t *svc_textFeed::getFeedText(const wchar_t *name)
{
	return _call(SVCTEXTFEED_GETFEEDTEXT, (const wchar_t *)NULL, name);
}

inline const wchar_t *svc_textFeed::getFeedDescription(const wchar_t *name)
{
	return _call(SVCTEXTFEED_GETFEEDDESC, (const wchar_t *)NULL, name);
}

inline ifc_dependent *svc_textFeed::getDependencyPtr()
{
	return _call(SVCTEXTFEED_GETDEPENDENCYPTR, (ifc_dependent*)NULL);
}


#endif
