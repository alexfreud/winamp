#ifndef NULLSOFT_AUTH_LOGIN_PROVIDER_LOADER_HEADER
#define NULLSOFT_AUTH_LOGIN_PROVIDER_LOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "../../nu/ptrlist.h"
#include "./providerParser.h"

class obj_xml;
class LoginProviderEnumerator;

class LoginProviderLoader : public ifc_xmlreadercallback
{

public:
	LoginProviderLoader();
	~LoginProviderLoader();

public:
	HRESULT ReadXml(LPCWSTR pszPath, LoginProviderEnumerator **enumerator, INT *prefVisible);

private:
	HRESULT FeedFile(obj_xml *reader, HANDLE hFile, DWORD bufferSize);

	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

private:
	typedef nu::PtrList<LoginProvider> ProviderList;

private:
	ProviderList		providerList;
	LoginProviderParser	parser;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_PROVIDER_LOADER_HEADER