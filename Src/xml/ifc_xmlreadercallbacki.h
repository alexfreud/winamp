#ifndef __WASABI_API_XMLREADERCALLBACKI_H
#define __WASABI_API_XMLREADERCALLBACKI_H

#include "ifc_xmlreadercallback.h"

class ifc_xmlreadercallbackI : public ifc_xmlreadercallback
{
protected:
	ifc_xmlreadercallbackI() {}
	~ifc_xmlreadercallbackI() {}

public:
	virtual void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params){}
	virtual void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag){}
	virtual void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str){}
  virtual void xmlReaderOnError( int linenum, int errcode, const wchar_t *errstr) {}
  /*
	const wchar_t *xmlGetFileName();
	int xmlGetFileLine();
	void *xmlGetParserHandle();
	*/
	#undef CBCLASS
#define CBCLASS ifc_xmlreadercallbackI
	START_DISPATCH_INLINE;
	VCB(ONSTARTELEMENT, xmlReaderOnStartElementCallback);
	VCB(ONENDELEMENT, xmlReaderOnEndElementCallback);
	VCB(ONCHARDATA, xmlReaderOnCharacterDataCallback);
	VCB(ONERROR, xmlReaderOnError);
  /*
	CB(GETFILENAME, xmlGetFileName);
	CB(GETFILELINE, xmlGetFileLine);
	CB(GETHANDLE, xmlGetParserHandle);
	*/
	END_DISPATCH;
#undef CBCLASS
};


#endif
