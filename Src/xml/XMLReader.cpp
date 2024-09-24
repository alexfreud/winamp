#include "XMLReader.h"
#include "ifc_xmlreadercallback.h"
#include "XMLParameters.h"
#include <memory.h>
#include "../nu/regexp.h"
#include "../nu/strsafe.h"
#include <wctype.h>
#include <vector>
#include <atlconv.h>

/* TODO:
try to remove CharUpper (but towupper doesn't deal with non-english very well)
*/

#ifdef __APPLE__
void CharUpper(wchar_t* src)
{
	while (src && *src)
	{
		*src = (wint_t)towupper(*src);
		src++;
	}
}

wchar_t* _wcsdup(const wchar_t* src)
{
	if (!src)
		return 0;
	size_t len = wcslen(src) + 1;
	if (len) // check for integer wraparound
	{
		wchar_t* newstr = (wchar_t*)malloc(sizeof(wchar_t) * len);
		wcscpy(newstr, src);
		return newstr;
	}
	return 0;
}
#endif

//---------------------------------------------------------------------------------------------------
CallbackStruct::CallbackStruct(ifc_xmlreadercallback* _callback, const wchar_t* _match, bool doUpper)
{
	match = _wcsdup(_match);
	if (doUpper)
		CharUpper(match);
	callback = _callback;
}
//------------------------------------------------------
CallbackStruct::CallbackStruct() 
: callback(0), match(0)
{
}
//-------------------------------
CallbackStruct::~CallbackStruct()
{
	if (match)
	{
		free(match);
		match = 0;
	}
}

/* --- */

void XMLCALL DStartTag(void* data, const XML_Char* name, const XML_Char** atts) { ((XMLReader*)data)->StartTag(name, atts); }
void XMLCALL DEndTag(void* data, const XML_Char* name) { ((XMLReader*)data)->EndTag(name); }
void XMLCALL DTextHandler(void* data, const XML_Char* s, int len) { ((XMLReader*)data)->TextHandler(s, len); }

int XMLCALL UnknownEncoding(void* data, const XML_Char* name, XML_Encoding* info);

//--------------------
XMLReader::XMLReader()
: parser(0)
{
	case_sensitive = false;
}
//---------------------
XMLReader::~XMLReader()
{
	for (size_t i = 0; i != callbacks.size(); i++)
	{
		delete callbacks[i];
		callbacks[i] = 0;
	}
}
//----------------------------------------------------------------------------------------
void XMLReader::RegisterCallback(const wchar_t* matchstr, ifc_xmlreadercallback* callback)
{
	callbacks.push_back(new CallbackStruct(callback, matchstr, !case_sensitive));
}
//-----------------------------------------------------------------
void XMLReader::UnregisterCallback(ifc_xmlreadercallback* callback)
{
	for (size_t i = 0; i != callbacks.size(); i++)
	{
		if (callbacks[i] && callbacks[i]->callback == callback)
		{
			delete callbacks[i];
			callbacks[i] = 0; // we set it to 0 so this can be called during a callback
		}
	}
}
//-------------------
int XMLReader::Open()
{
	parser = XML_ParserCreate(0); // create the expat parser
	if (!parser)
		return OBJ_XML_FAILURE;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return OBJ_XML_SUCCESS;
}
//----------------------------
int XMLReader::OpenNamespace()
{
	parser = XML_ParserCreateNS(0, L'#'); // create the expat parser, using # to separate namespace URI from element name
	if (!parser)
		return OBJ_XML_FAILURE;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return OBJ_XML_SUCCESS;
}
//--------------------------------------------------
void XMLReader::OldFeed(void* data, size_t dataSize)
{
	Feed(data, dataSize);
}
//----------------------------------------------
int XMLReader::Feed(void* data, size_t dataSize)
{
	XML_Status error;
	if (data && dataSize)
	{
		while (dataSize >= 0x7FFFFFFFU) // handle really really big data sizes (hopefully this won't happen)
		{
			XML_Parse(parser, reinterpret_cast<const char*>(data), 0x7FFFFFFF, 0);
			dataSize -= 0x7FFFFFFFU;
		}
		error = XML_Parse(parser, reinterpret_cast<const char*>(data), static_cast<int>(dataSize), 0);
	}
	else
		error = XML_Parse(parser, 0, 0, 1); // passing this sequence tells expat that we're done

	if (error == XML_STATUS_ERROR)
	{
		// TODO: set a flag to prevent further parsing until a Reset occurs
		XML_Error errorCode = XML_GetErrorCode(parser);
		int line = XML_GetCurrentLineNumber(parser);
		// TODO: int column = XML_GetCurrentColumnNumber(parser);
		wa::strings::wa_string szError(XML_ErrorString(errorCode));

		for (CallbackStruct* l_callback : callbacks)
		{
			if (l_callback != NULL)
				l_callback->callback->xmlReaderOnError(line, errorCode, szError.GetW().c_str());
		}

		return OBJ_XML_FAILURE;
	}

	return OBJ_XML_SUCCESS;
}
//---------------------
void XMLReader::Close()
{
	if (parser)
		XML_ParserFree(parser);
	parser = 0;
}
//-----------------------------------
const wchar_t* XMLReader::BuildPath()
{
	return pathString.c_str();
}
//----------------------------------------------------
const wchar_t* XMLReader::AddPath(const wchar_t* node)
{
	currentNode.assign(node);

	if (pathString.length())
	{
		pathString.append(L"\f");
	}

	pathString.append(node);

	if (!case_sensitive)
	{
		std::transform(
			pathString.begin(), pathString.end(),
			pathString.begin(),
			towupper);
	}

	return pathString.c_str();
}
//-------------------------------------------------
const wchar_t* XMLReader::AddPath(const char* node)
{
	wa::strings::wa_string wszNode(node);
	return AddPath(wszNode.GetW().c_str());
}
//-------------------------------------------------------
const wchar_t* XMLReader::RemovePath(const wchar_t* node)
{
	size_t pathSize = pathString.length();
	size_t removeLength = wcslen(node);
	removeLength = pathSize > removeLength ? removeLength + 1 : removeLength;
	pathString = pathString.substr(0, pathSize - removeLength);

	if (pathString.length())
	{
		const wchar_t* last_node = wcsrchr(pathString.c_str(), '\f');
		if (last_node)
		{
			currentNode.assign(last_node + 1);
		}
		else
		{
			currentNode.assign(pathString);
		}
	}
	else
	{
		currentNode = L"";
	}

	return pathString.c_str();
}
//----------------------------------------------------
const wchar_t* XMLReader::RemovePath(const char* node)
{
	wa::strings::wa_string wszNode(node);
	return RemovePath(wszNode.GetW().c_str());
}
//-------------------------------------------------------------------------
void XMLCALL XMLReader::StartTag(const wchar_t* name, const wchar_t** atts)
{
	const wchar_t* xmlpath = AddPath(name);

	XMLParameters xmlParameters(atts);
	for (size_t i = 0; i != callbacks.size(); i++)
	{
		if (callbacks[i] && Match(callbacks[i]->match, xmlpath))
			callbacks[i]->callback->xmlReaderOnStartElementCallback(xmlpath, name, static_cast<ifc_xmlreaderparams*>(&xmlParameters));
	}
}
//-------------------------------------------------------------------
void XMLCALL XMLReader::StartTag(const char* name, const char** atts)
{
	wa::strings::wa_string wszName(name);
	size_t nAttrCount = 0;
	const char** a = atts;

	while (*a)
	{
		nAttrCount++;
		a++;
	}
	wchar_t** wszAtts = new wchar_t* [nAttrCount + 1];

	if (nAttrCount)
	{
		size_t n = 0;
		while (*atts)
		{
			const char* pszAttr = *atts;
			size_t nAttrLen = strlen(pszAttr);
			wchar_t* wc = new wchar_t[nAttrLen + 1];
			mbstowcs_s(NULL, wc, nAttrLen + 1, pszAttr, nAttrLen);
			wszAtts[n++] = wc;
			atts++;
		}

	}
	wszAtts[nAttrCount] = 0;

	StartTag(wszName.GetW().c_str(), const_cast<const wchar_t**>(wszAtts));
}
//-------------------------------------------------
void XMLCALL XMLReader::EndTag(const wchar_t* name)
{
	endPathString = BuildPath();

	RemovePath(name);

	for (size_t i = 0; i != callbacks.size(); i++)
	{
		if (callbacks[i] && Match(callbacks[i]->match, endPathString.c_str()))
			callbacks[i]->callback->xmlReaderOnEndElementCallback(endPathString.c_str(), name);
	}
}
//----------------------------------------------
void XMLCALL XMLReader::EndTag(const char* name)
{
	wa::strings::wa_string wszName(name);
	return EndTag(wszName.GetW().c_str());
}
//------------------------------------------------------------
void XMLCALL XMLReader::TextHandler(const wchar_t* s, int len)
{
	if (len)
	{
		textCache.assign(s, len);

		const wchar_t* xmlpath = BuildPath();

		for (size_t i = 0; i != callbacks.size(); i++)
		{
			if (callbacks[i] && Match(callbacks[i]->match, xmlpath))
				callbacks[i]->callback->xmlReaderOnCharacterDataCallback(xmlpath, currentNode.c_str(), textCache.c_str());
		}
	}
}
//---------------------------------------------------------
void XMLCALL XMLReader::TextHandler(const char* s, int len)
{
	wa::strings::wa_string wszText(s);
	return TextHandler(wszText.GetW().c_str(), len);
}
//---------------------------
void XMLReader::PushContext()
{
	context.push_back(parser);
	parser = XML_ExternalEntityParserCreate(parser, L"\0", NULL);
}
//--------------------------
void XMLReader::PopContext()
{
	if (parser)
		XML_ParserFree(parser);
	parser = context.back();
	context.pop_back();
}
//---------------------
void XMLReader::Reset()
{
	if (parser)
	{
		XML_ParserReset(parser, 0);
		XML_SetUserData(parser, this); // give our object pointer as context
		XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
		XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	}
}
//--------------------------------------------------
void XMLReader::SetEncoding(const wchar_t* encoding)
{
	wa::strings::wa_string szEncoding(encoding);
	XML_SetEncoding(parser, szEncoding.GetW().c_str());
}
//-------------------------------
int XMLReader::SetCaseSensitive()
{
	case_sensitive = true;
	return OBJ_XML_SUCCESS;
}

#define CBCLASS XMLReader
START_DISPATCH;
VCB(OBJ_XML_REGISTERCALLBACK, RegisterCallback)
VCB(OBJ_XML_UNREGISTERCALLBACK, UnregisterCallback)
CB(OBJ_XML_OPEN, Open)
CB(OBJ_XML_OPEN2, OpenNamespace)
VCB(OBJ_XML_OLDFEED, OldFeed)
CB(OBJ_XML_FEED, Feed)
VCB(OBJ_XML_CLOSE, Close)
VCB(OBJ_XML_INTERRUPT, PushContext)
VCB(OBJ_XML_RESUME, PopContext)
VCB(OBJ_XML_RESET, Reset)
VCB(OBJ_XML_SETENCODING, SetEncoding)
CB(OBJ_XML_SETCASESENSITIVE, SetCaseSensitive)
END_DISPATCH;
#undef CBCLASS
