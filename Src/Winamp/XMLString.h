#ifndef NULLSOFT_WINAMP_XMLSTRING_H
#define NULLSOFT_WINAMP_XMLSTRING_H


#include "../xml/ifc_xmlreadercallback.h"
/*
this one is an xml callback that just saves the last encountered string
*/

#define XMLSTRING_SIZE MAX_URL
class XMLString : public ifc_xmlreadercallback
{
public:
	XMLString();
	void Reset();
	const wchar_t *GetString();
	void ManualSet(const wchar_t *string);
private:
		/* XML callbacks */
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void EndTag(const wchar_t *xmlpath, const wchar_t *xmltag);
	void TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);

	wchar_t data[XMLSTRING_SIZE]; // for now, we'll make it dynamic later

	RECVS_DISPATCH;
};

#endif