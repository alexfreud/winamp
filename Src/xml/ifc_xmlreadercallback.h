#ifndef NULLSOFT_XML_IFC_XMLREADERCALLBACK_H
#define NULLSOFT_XML_IFC_XMLREADERCALLBACK_H

#include <bfc/dispatch.h>
#include "ifc_xmlreaderparams.h"

class NOVTABLE ifc_xmlreadercallback : public Dispatchable
{
protected:
	ifc_xmlreadercallback()                                           {}
	~ifc_xmlreadercallback()                                          {}

public:
	void xmlReaderOnStartElementCallback( const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params );
	void xmlReaderOnEndElementCallback( const wchar_t *xmlpath, const wchar_t *xmltag );
	void xmlReaderOnCharacterDataCallback( const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str );
	void xmlReaderOnError( int linenum, int errcode, const wchar_t *errstr );

	DISPATCH_CODES
	{
		ONSTARTELEMENT =  100,
		ONENDELEMENT   =  200,
		ONCHARDATA     =  300,
		ONERROR        = 1200,
	};
};

inline void ifc_xmlreadercallback::xmlReaderOnStartElementCallback( const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params )
{
	_voidcall( ONSTARTELEMENT, xmlpath, xmltag, params );
}

inline void ifc_xmlreadercallback::xmlReaderOnEndElementCallback( const wchar_t *xmlpath, const wchar_t *xmltag )
{
	_voidcall( ONENDELEMENT, xmlpath, xmltag );
}

inline void ifc_xmlreadercallback::xmlReaderOnCharacterDataCallback( const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str )
{
	_voidcall( ONCHARDATA, xmlpath, xmltag, str );
}

inline void ifc_xmlreadercallback::xmlReaderOnError( int linenum, int errcode, const wchar_t *errstr )
{
	_voidcall( ONERROR, linenum, errcode, errstr );
}

#endif
