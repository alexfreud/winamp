#ifndef _XMLREADER_H
#define _XMLREADER_H

#include <bfc/ptrlist.h>
#include <bfc/string/bfcstring.h>
#include <bfc/stack.h>
#include <bfc/dispatch.h>
#include <api/xml/xmlparams.h>
#include "../xml/ifc_xmlreadercallbackI.h"
#include <api/xml/XMLAutoInclude.h>
class svc_xmlProvider;

typedef enum {
    TYPE_CLASS_CALLBACK = 1,
    TYPE_STATIC_CALLBACK,
} xmlreader_callbackType;


class XmlReaderCallbackI 
{
public:
	XmlReaderCallbackI() : handle(NULL) {}

	virtual void xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params) { }
	virtual void xmlReaderOnEndElementCallback(const wchar_t *xmltag) { }

private:
	void *handle;
};


class xmlreader_cb_struct
{
public:
	xmlreader_cb_struct(const wchar_t *m, xmlreader_callbackType t, XmlReaderCallbackI *cb) : matchstr(m), type(t), callback(cb) 
	{
		matchstr.toupper();
	}

	StringW matchstr;
	xmlreader_callbackType type;
	XmlReaderCallbackI * callback;
};

class XmlReader : public ifc_xmlreadercallbackI
{
public:
	// matchstr is a regexp string such as "WinampAbstractionLayer/Layer[a-z]"
	// or "Winamp*Layer/*/Layout"
	 void registerCallback(const wchar_t *matchstr, XmlReaderCallbackI *callback);
	 void registerCallback(const wchar_t *matchstr, void (*static_callback)(int start, const wchar_t *xmltag, skin_xmlreaderparams *params));

	 void unregisterCallback(void *callback);

	// if only_this_class param is specified, only this class will be called back
	// returns 1 on success, 0 on error
	 int loadFile(const wchar_t *filename, const wchar_t *incpath = NULL, int isinclude = 0);
	 const wchar_t *getIncludePath();

	 int getNumCallbacks() { return callback_list.getNumItems(); }

private:
  void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
  void xmlReaderOnError(int linenum, int errcode, const wchar_t *errstr);
//  int doLoadFile(FILE *fp, svc_xmlProvider *svc, const wchar_t *filename, const wchar_t *incpath);

	 PtrList<xmlreader_cb_struct> callback_list;
   StringW includePath;
XMLAutoInclude *includer;
};

extern XmlReader skinXML;

#endif
