#include <precomp.h>
#include "sxmldoc.h"

#include "slist.h"

#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objecttable.h>


// {417FFB69-987F-4be8-8D87-D9965EEEC868}
static const GUID xmlDocGuid = 
{ 0x417ffb69, 0x987f, 0x4be8, { 0x8d, 0x87, 0xd9, 0x96, 0x5e, 0xee, 0xc8, 0x68 } };


XmlDocScriptController _xmlDocController;
XmlDocScriptController *xmlDocController=&_xmlDocController;

// -- Functions table -------------------------------------
function_descriptor_struct XmlDocScriptController::exportedFunction[] = {
	{L"parser_addCallback",		1, (void*)SXmlDoc::script_vcpu_addParserCallback },
	{L"parser_start",			0, (void*)SXmlDoc::script_vcpu_parse },
	{L"parser_destroy",			0, (void*)SXmlDoc::script_vcpu_destroyParser },
	{L"parser_onCallback",		4, (void*)SXmlDoc::script_vcpu_onXmlParserCallback},
	{L"parser_onCloseCallback",	2, (void*)SXmlDoc::script_vcpu_onXmlParserEndCallback},
	{L"parser_onError",			5, (void*)SXmlDoc::script_vcpu_onXmlParserError},
};
// --------------------------------------------------------

const wchar_t *XmlDocScriptController::getClassName() {
	return L"XmlDoc";
}

const wchar_t *XmlDocScriptController::getAncestorClassName() {
  return L"File";
}

ScriptObjectController *XmlDocScriptController::getAncestorController() { return rootScriptObjectController; }

ScriptObject *XmlDocScriptController::instantiate() {
  SXmlDoc *xd = new SXmlDoc;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void XmlDocScriptController::destroy(ScriptObject *o) {
  SXmlDoc *xd = static_cast<SXmlDoc *>(o->vcpu_getInterface(xmlDocGuid));
  ASSERT(xd != NULL);
  delete xd;
}

void *XmlDocScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for XmlDocs for now
}

void XmlDocScriptController::deencapsulate(void *o) {
}

int XmlDocScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *XmlDocScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID XmlDocScriptController::getClassGuid() {
	return xmlDocGuid;
}


SXmlDoc::SXmlDoc() {
  getScriptObject()->vcpu_setInterface(xmlDocGuid, (void *)static_cast<SXmlDoc *>(this));
  getScriptObject()->vcpu_setClassName(L"XmlDoc");
  getScriptObject()->vcpu_setController(xmlDocController);
  filename = NULL;
  myXmlParser = NULL;
}

SXmlDoc::~SXmlDoc() {
	destroyParser();
}

void SXmlDoc::addParserCallback(const wchar_t *name)
{
	createParser();
	StringW sw_name = name;
	sw_name.replace(L"/", L"\f"); // We call subsections in Maki with a single /. in Wasabi \f is used
	//debug: Std::messageBox(sw_name,name,0);
	myXmlParser->xmlreader_registerCallback(sw_name, &myXmlParserCallback);
	//debug: myXmlParser->xmlreader_registerCallback(L"WasabiXML\fbla", &myXmlParserCallback);
}

void LoadXmlFile(obj_xml *parser, const wchar_t *filename);

void SXmlDoc::startParsing()
{
/*	debug: createParser();
myXmlParser->xmlreader_registerCallback(L"WasabiXML\fbla", &myXmlParserCallback);
myXmlParser->xmlreader_registerCallback(L"WasabiXML\fbrowserQuickLinks", &myXmlParserCallback);*/

	if (!myXmlParser) return;
	myXmlParser->xmlreader_open();
	LoadXmlFile(myXmlParser, filename);
}

void SXmlDoc::createParser()
{
	if (myXmlParser != NULL) return;

	myXmlParserCallback.parent = this;

	myXmlParserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (myXmlParserFactory)
	{
		myXmlParser = (obj_xml *)myXmlParserFactory->getInterface();

		if (myXmlParser)
		{
			const wchar_t *file = Wasabi::Std::filename(filename);
			int fnlen = wcslen(file);
			StringW path = filename;
			path.trunc( -fnlen);
			XMLAutoInclude include(myXmlParser, path);
		}
	}
}

void SXmlDoc::destroyParser()
{
	if (!myXmlParser) return;
	myXmlParser->xmlreader_unregisterCallback(&myXmlParserCallback);
	myXmlParser->xmlreader_close();
	myXmlParserFactory->releaseInterface(myXmlParser);
	myXmlParser = NULL;
}

// ParserCallbacks

void SXmlDocParserCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	//debug: Std::messageBox(xmlpath,xmltag,0);
	StringW sw_xmlpath = xmlpath;
	sw_xmlpath.replace(L"\f", L"/");

	// Store the params and paramvalues in a SList
	SList param;
	SList paramvalue;

	for (size_t i = 0; i != params->getNbItems(); i++)
	{
		SList::script_vcpu_addItem(SCRIPT_CALL, param.getScriptObject(), MAKE_SCRIPT_STRING(params->getItemName(i)));
		SList::script_vcpu_addItem(SCRIPT_CALL, paramvalue.getScriptObject(), MAKE_SCRIPT_STRING(params->getItemValue(i)));
	}
	
	// and now the monster call ;)
	SXmlDoc::script_vcpu_onXmlParserCallback(
		SCRIPT_CALL, parent->getScriptObject(),
		MAKE_SCRIPT_STRING(sw_xmlpath),
		MAKE_SCRIPT_STRING(xmltag),
		MAKE_SCRIPT_OBJECT(param.getScriptObject()),
		MAKE_SCRIPT_OBJECT(paramvalue.getScriptObject()) );
}

void SXmlDocParserCallback::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	StringW sw_xmlpath = xmlpath;
	sw_xmlpath.replace(L"\f", L"/");
	
	SXmlDoc::script_vcpu_onXmlParserEndCallback(SCRIPT_CALL, parent->getScriptObject(), MAKE_SCRIPT_STRING(sw_xmlpath), MAKE_SCRIPT_STRING(xmltag));
}

void SXmlDocParserCallback::xmlReaderOnError(int linenum, int errcode, const wchar_t *errstr)
{
	SXmlDoc::script_vcpu_onXmlParserError(
		SCRIPT_CALL, parent->getScriptObject(),
		MAKE_SCRIPT_STRING(L""), // xml api changed, but we should keep the same maki function!
		MAKE_SCRIPT_INT(linenum),
		MAKE_SCRIPT_STRING(L""), // xml api changed, but we should keep the same maki function!
		MAKE_SCRIPT_INT(errcode),
		MAKE_SCRIPT_STRING(errstr) );
}

// VCPU

scriptVar SXmlDoc::script_vcpu_addParserCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar fn) {
	SCRIPT_FUNCTION_INIT; 
	ASSERT(fn.type == SCRIPT_STRING);
	SXmlDoc *m = static_cast<SXmlDoc *>(o->vcpu_getInterface(xmlDocGuid));
	if (m) m->addParserCallback(fn.data.sdata);

	RETURN_SCRIPT_VOID;  
}

scriptVar SXmlDoc::script_vcpu_parse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SXmlDoc *m = static_cast<SXmlDoc *>(o->vcpu_getInterface(xmlDocGuid));
	if (m) m->startParsing();

	RETURN_SCRIPT_VOID;
}

scriptVar SXmlDoc::script_vcpu_destroyParser(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT;
	SXmlDoc *m = static_cast<SXmlDoc *>(o->vcpu_getInterface(xmlDocGuid));
	if (m) m->destroyParser();

	RETURN_SCRIPT_VOID;
}

scriptVar SXmlDoc::script_vcpu_onXmlParserCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar xmlpath, scriptVar xmltag, scriptVar param, scriptVar paramvalue)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS4(o, xmlDocController, xmlpath, xmltag, param, paramvalue);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, xmlpath, xmltag, param, paramvalue);
}
scriptVar SXmlDoc::script_vcpu_onXmlParserEndCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar xmlpath, scriptVar xmltag)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS2(o, xmlDocController, xmlpath, xmltag);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT2(o, xmlpath, xmltag);
}
scriptVar SXmlDoc::script_vcpu_onXmlParserError(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar filename, scriptVar linenum, scriptVar incpath, scriptVar errcode, scriptVar errstr)
{
	SCRIPT_FUNCTION_INIT
	PROCESS_HOOKS5(o, xmlDocController, filename, linenum, incpath, errcode, errstr);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT5(o, filename, linenum, incpath, errcode, errstr);
}