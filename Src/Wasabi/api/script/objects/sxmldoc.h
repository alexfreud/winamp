#ifndef __SXMLDOC_H
#define __SXMLDOC_H

class SXmlDoc;

#include <api/script/objects/sfile.h>

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>

#include <api/xml/XMLAutoInclude.h>

#define SXMLDOC_SCRIPTPARENT SFile

class XmlDocScriptController : public fileScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController();
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

class SXmlDocParserCallback : public ifc_xmlreadercallbackI
{
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
	void xmlReaderOnError(int linenum, int errcode, const wchar_t *errstr);

public: SXmlDoc *parent;	// reference to the xmldoc that triggered this callback
};

extern XmlDocScriptController *xmlDocController;

class SXmlDoc : public SXMLDOC_SCRIPTPARENT
{
public:
	SXmlDoc();
	virtual ~SXmlDoc();

	void addParserCallback(const wchar_t *b);
	void startParsing();
	void destroyParser();
	void elementCallback(const wchar_t *xmltag);

private:
	obj_xml *myXmlParser;
	SXmlDocParserCallback myXmlParserCallback;
	waServiceFactory *myXmlParserFactory;

	void createParser();

public:
	static scriptVar script_vcpu_addParserCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar fn);
	static scriptVar script_vcpu_parse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_destroyParser(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_onXmlParserCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar xmlpath, scriptVar xmltag, scriptVar param, scriptVar paramvalue);
	static scriptVar script_vcpu_onXmlParserError(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar filename, scriptVar linenum, scriptVar incpath, scriptVar errcode, scriptVar errstr);
	static scriptVar script_vcpu_onXmlParserEndCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar xmlpath, scriptVar xmltag);

};

#endif
