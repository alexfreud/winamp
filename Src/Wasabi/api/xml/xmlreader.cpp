#include <precomp.h>
#include "xmlreader.h"
#include <bfc/parse/pathparse.h>
#include <bfc/file/wildcharsenum.h>
#include <bfc/string/url.h>
#include <api/service/svcs/svc_xmlprov.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/util/varmgr.h>
#endif
#include <api/xml/xmlparamsi.h>
#include <api/xml/XMLAutoInclude.h>
#include "../nu/regexp.h"

void LoadXmlFile(obj_xml *parser, const wchar_t *filename);

void XmlReader::registerCallback(const wchar_t *matchstr, XmlReaderCallbackI *callback)
{
  if (matchstr == NULL || callback == NULL) return ;
  xmlreader_cb_struct *s = new xmlreader_cb_struct(matchstr, TYPE_CLASS_CALLBACK, callback);
  callback_list.addItem(s, 0);  // mig: add to FRONT of list... so we don't step on hierarchical toes.
}

void XmlReader::registerCallback(const wchar_t *matchstr, void (* callback)(int, const wchar_t *, skin_xmlreaderparams *))
{
  if (matchstr == NULL || callback == NULL) return ;
  xmlreader_cb_struct *s = new xmlreader_cb_struct(matchstr, TYPE_STATIC_CALLBACK, (XmlReaderCallbackI *)callback);
  callback_list.addItem(s, 0);  // mig: add to FRONT of list... so we don't step on hierarchical toes.
}

void XmlReader::unregisterCallback(void *callback)
{
  for (int i = 0;i < callback_list.getNumItems();i++)
  {
    if (callback_list[i]->callback == callback)
    {
      delete callback_list[i];
      callback_list.delByPos(i);
      i--;
    }
  }
}

int XmlReader::loadFile(const wchar_t *filename, const wchar_t *incpath, int isinclude)
{
  includePath=incpath;
  waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
  if (parserFactory)
  {
    obj_xml *parser = (obj_xml *)parserFactory->getInterface();

    if (parser)
    {
      includer = new XMLAutoInclude(parser, incpath);
      parser->xmlreader_registerCallback(L"*", this);
      parser->xmlreader_open();

      LoadXmlFile(parser, filename);
      parser->xmlreader_unregisterCallback(this);
      delete includer;
      includer=0;

      parser->xmlreader_close();
      parserFactory->releaseInterface(parser);
      parser = 0;
      return 1;
    }

  }

  return 0;
}

const wchar_t *XmlReader::getIncludePath()
{
  //	return include_stack.top()->getValue();
	return (includer != NULL ? includer->path.getValue() : L"");
  //return includePath;
}

void XmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
  size_t numParams = params->getNbItems();
  XmlReaderParamsI p;
  for (size_t i = 0;i!=numParams;i++)
  { //CT> removed api-> for speed
    // BU String* for no exploit
		const wchar_t *paramStr = params->getItemValue(i);
    StringW *str = PublicVarManager::translate_nocontext(paramStr);
		if (str)
		{
			Url::decode(str->getNonConstVal());
			p.addItemSwapValue(params->getItemName(i), *str);
			delete str;
		}
		else
		{

			StringW temp = paramStr;
			Url::decode(temp.getNonConstVal());
			p.addItemSwapValue(params->getItemName(i), temp);
		}
  }

  foreach(callback_list)
    if (Match(callback_list.getfor()->matchstr.v(), xmlpath))
    {
      switch (callback_list.getfor()->type)
      {
      case TYPE_CLASS_CALLBACK:
        callback_list.getfor()->callback->xmlReaderOnStartElementCallback(xmltag, &p);
        break;
      case TYPE_STATIC_CALLBACK:
        ((void (*)(int,  const wchar_t *, skin_xmlreaderparams *))callback_list.getfor()->callback)(1,  xmltag, &p);
        break;
      }
    }

    endfor
}

void XmlReader::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
  foreach(callback_list)
    if (Match(callback_list.getfor()->matchstr.v(), xmlpath))
    {
      switch (callback_list.getfor()->type)
      {
      case TYPE_CLASS_CALLBACK:
        callback_list.getfor()->callback->xmlReaderOnEndElementCallback(xmltag);
        break;
      case TYPE_STATIC_CALLBACK:
        ((void (*)(int,  const wchar_t *, skin_xmlreaderparams *))callback_list.getfor()->callback)(0, xmltag, NULL);
        break;
      }

    }
    endfor

}

void XmlReader::xmlReaderOnError(int linenum, int errcode, const wchar_t *errstr)
{
  int disperr = 1;
  StringPrintfW txt(L"error parsing xml layer\n");
  StringPrintfW err(L"%s at line %d\n", errstr, linenum);

  if (disperr)
  {
#ifdef WASABI_COMPILE_WND
      Wasabi::Std::messageBox(err, txt, 0);
#else
    DebugStringW("%s - %s", err.getValue(), txt.getValue());
#endif

  }
  else
  {
#ifdef _WIN32
    OutputDebugStringW(txt);
    OutputDebugStringW(err);
#endif
  }
}

#if 0 // TODO: benski> need to do onError for obj_xml
int XmlReader::parseBuffer(void *parser, const char *xml, int size, int final, const char *filename, const char *incpath)
{
  if (!XML_Parse(parser, xml, size, final))
  {
    int disperr = 1;
    StringPrintf txt("error parsing xml layer (%s)\n", filename);
    StringPrintf err("%s at line %d\n",
      XML_ErrorString(XML_GetErrorCode(parser)),
      XML_GetCurrentLineNumber(parser));
    if (disperr)
    {
#ifdef WASABI_COMPILE_WND
      Std::messageBox(err, txt, 0);
#else
      DebugString("%s - %s", err.getValue(), txt.getValue());
#endif

    }
    else
    {
      OutputDebugString(txt);
      OutputDebugString(err);
    }
    currentpos = "";
    return 0;
  }
  return 1;
}
#endif


XmlReader skinXML;