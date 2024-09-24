#pragma once
#include "ifc_xmlreadercallback.h"

template <class T>
class ifc_xmlreadercallbackT : public ifc_xmlreadercallback
{
protected:
	ifc_xmlreadercallbackT() {}
	~ifc_xmlreadercallbackT() {}

public:
	virtual void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params){}
	virtual void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag){}
	virtual void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str){}
  virtual void xmlReaderOnError( int linenum, int errcode, const wchar_t *errstr) {}

#define CBCLASS T
#define CBCLASST ifc_xmlreadercallbackT<T>
	START_DISPATCH_INLINE;
	VCBT(ONSTARTELEMENT, xmlReaderOnStartElementCallback);
	VCBT(ONENDELEMENT, xmlReaderOnEndElementCallback);
	VCBT(ONCHARDATA, xmlReaderOnCharacterDataCallback);
	VCBT(ONERROR, xmlReaderOnError);
	END_DISPATCH;
#undef CBCLASS
#undef CBCLASST
};
