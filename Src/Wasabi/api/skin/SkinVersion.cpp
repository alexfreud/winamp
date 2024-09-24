#include <precomp.h>
#include "SkinVersion.h"
#include "../xml/obj_xml.h"
#include <api/service/waservicefactory.h>
#include <bfc/string/StringW.h>

void LoadXmlFile(obj_xml *parser, const wchar_t *filename);

SkinVersionXmlReader::SkinVersionXmlReader(const wchar_t *skinname)
{
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		obj_xml *parser = (obj_xml *)parserFactory->getInterface();

		if (parser)
		{
			parser->xmlreader_registerCallback(L"WinampAbstractionLayer", this);
			parser->xmlreader_registerCallback(L"WasabiXML", this);
			parser->xmlreader_open();

			StringPathCombine fn(WASABI_API_SKIN->getSkinPath(), L"skin.xml");
			LoadXmlFile(parser, fn);
			parser->xmlreader_unregisterCallback(this);

			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
			parser = 0;
		}
	}
}

void SkinVersionXmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *version = params->getItemValue(L"version");
	if (version)
		walversion = version;
}
const wchar_t *SkinVersionXmlReader::getWalVersion()
{
	return walversion;
}
