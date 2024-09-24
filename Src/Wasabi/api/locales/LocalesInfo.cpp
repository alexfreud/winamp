#include <precomp.h>
#include "LocalesInfo.h"
#include "../xml/obj_xml.h"
#include <api/xml/XMLAutoInclude.h>
#include <api/xml/LoadXML.h>
#include <api/service/waservicefactory.h>

LocalesInfosXmlReader::LocalesInfosXmlReader(const wchar_t *localename) : LocaleItem(localename)
{
	parser = 0;

	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		parser = (obj_xml *)parserFactory->getInterface();

		if (parser)
		{
      {
      XMLAutoInclude include(parser, L"Locales");
      parser->xmlreader_registerCallback(L"WinampLocaleDefinition", this);
			parser->xmlreader_open();

			StringPathCombine fn(L"Locales", localename);
			LoadXmlFile(parser, fn);
			parser->xmlreader_unregisterCallback(this);
      }
			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
			parser = 0;
		}

	}
}


void LocalesInfosXmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
		language = params->getItemValue(L"language");
		author = params->getItemValue(L"author");
}
