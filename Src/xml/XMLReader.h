#ifndef NULLSOFT_XML_XMLREADER_H
#define NULLSOFT_XML_XMLREADER_H

#include "obj_xml.h"
#include <vector>
#include "expat.h"
#include "../WAT/WAT.h"

struct CallbackStruct
{
	CallbackStruct(ifc_xmlreadercallback* _callback, const wchar_t* _match, bool doUpper);
	CallbackStruct();
	~CallbackStruct();
	ifc_xmlreadercallback* callback;
	wchar_t* match;
};

class XMLReader : public obj_xml
{
public:
	XMLReader();
	~XMLReader();
	void RegisterCallback(const wchar_t* matchstr, ifc_xmlreadercallback* callback);
	void UnregisterCallback(ifc_xmlreadercallback* callback);
	int Open();
	int OpenNamespace();
	void OldFeed(void* data, size_t dataSize);
	int Feed(void* data, size_t dataSize);
	void Close();
	void PushContext();
	void PopContext();
	void Reset();
	void SetEncoding(const wchar_t* encoding);
	int SetCaseSensitive();

protected:
	RECVS_DISPATCH;

public:
	void XMLCALL StartTag(const wchar_t* name, const wchar_t** atts);
	void XMLCALL EndTag(const wchar_t* name);
	void XMLCALL TextHandler(const wchar_t* s, int len);

	void XMLCALL StartTag(const char* name, const char** atts);
	void XMLCALL EndTag(const char* name);
	void XMLCALL TextHandler(const char* s, int len);

private:
	const wchar_t* BuildPath();
	const wchar_t* AddPath(const wchar_t* node);
	const wchar_t* AddPath(const char* node);
	const wchar_t* RemovePath(const wchar_t* node);
	const wchar_t* RemovePath(const char* node);
	std::wstring pathString;//, pathUpper;
	std::wstring endPathString;//, endPathUpper;
	std::wstring currentNode;

private:
	std::vector<CallbackStruct*> callbacks;
	std::vector<XML_Parser> context;
	XML_Parser parser;
	bool case_sensitive;
	std::wstring textCache;

};
#endif