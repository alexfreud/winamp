#pragma once
#include "../xml/ifc_xmlreadercallback.h"
#include "XMLNode.h"
#include "../nu/Alias.h"

class XMLDOM : public ifc_xmlreadercallback
{
public:
	XMLDOM();
	~XMLDOM();

	const XMLNode *GetRoot() const;

private:
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void EndTag(const wchar_t *xmlpath, const wchar_t *xmltag);
	void TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);

private:
	wchar_t *curtext;
	size_t curtext_len; // number of characters in curtext, not including null terminator
	XMLNode *xmlNode;
	XMLNode *curNode;

protected:
	RECVS_DISPATCH;
};