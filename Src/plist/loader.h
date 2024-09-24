//------------------------------------------------------------------------
//
// iTunes XML Library Reader
// Copyright (C) 2003-2012 Nullsoft, Inc.
//
//------------------------------------------------------------------------

#ifndef NULLSOFT_PLIST_LOADER_H
#define NULLSOFT_PLIST_LOADER_H

#include "types.h"
#include "../xml/ifc_xmlreadercallback.h"
#include <bfc/string/stringw.h>
#include <bfc/stack.h>

//------------------------------------------------------------------------
class plistLoader : public ifc_xmlreadercallback, public plistKey {
public:
	plistLoader();
	virtual ~plistLoader();

	virtual void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	virtual void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
	virtual void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);
private:
	Stack<plistData *> m_context; // either a key or an array, this is where data gets inserted next
	Stack<plistKeyholder *> m_dict;
	StringW m_cdata;

protected:
	RECVS_DISPATCH;
};

#endif

//------------------------------------------------------------------------