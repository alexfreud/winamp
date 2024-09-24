#include <precomp.h>
#include "skininfo.h"
#include "../xml/obj_xml.h"
#include <api/xml/XMLAutoInclude.h>
#include <api/xml/LoadXML.h>
#include "../nu/AutoChar.h"
#include "gen.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"

SkinInfosXmlReader::SkinInfosXmlReader(const wchar_t *skinname) : SkinInfoBlock(skinname)
{
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
	{
		obj_xml *parser = (obj_xml *)parserFactory->getInterface();
		if (parser)
		{
			{
				StringPathCombine skinPath(WASABI_API_SKIN->getSkinsPath(), skinname);

				XMLAutoInclude include(parser, skinPath);
				parser->xmlreader_registerCallback(L"WinampAbstractionLayer",this);
				//parser->xmlreader_registerCallback(L"WinampAbstractionLayer\fSkinInfo", this);
				parser->xmlreader_registerCallback(L"WinampAbstractionLayer\fSkinInfo\f*", this);
				parser->xmlreader_registerCallback(L"WasabiXML",this);
				//parser->xmlreader_registerCallback(L"WasabiXML\fSkinInfo", this);
				parser->xmlreader_registerCallback(L"WasabiXML\fSkinInfo\f*", this);
				parser->xmlreader_open();

				skinPath.AppendPath(L"skin.xml");
				LoadXmlFile(parser, skinPath);
				parser->xmlreader_unregisterCallback(this);
			}
			parser->xmlreader_close();
			parserFactory->releaseInterface(parser);
			parser = 0;
		}
	}
}

void SkinInfosXmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (!_wcsicmp(xmltag, L"WinampAbstractionLayer") || !_wcsicmp(xmltag, L"WasabiXML"))
	{
		const wchar_t *version = params->getItemValue(L"version");
		if (version)
			walversion = version;
	}
}

void SkinInfosXmlReader::xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *s)
{
	if (!wcscmp(xmltag, L"name")) { fullname = s;}
	else if (!wcscmp(xmltag, L"version")) { version = s;}
	else if (!wcscmp(xmltag, L"comment")) { comment = s;}
	else if (!wcscmp(xmltag, L"author")) { author = s;}
	else if (!wcscmp(xmltag, L"email")) { email = s;}
	else if (!wcscmp(xmltag, L"homepage")) { homepage = s;}
	else if (!wcscmp(xmltag, L"parentskin")) { parentskin = s;}
	else if (!wcscmp(xmltag, L"screenshot")) { screenshot = s;}
}

extern int m_are_we_loaded;
static StringW skin;

const wchar_t *getSkinInfoW()
{
	static StringW skininfoW;

	// these checks is repeated in getSkinInfo, also
	if (!m_are_we_loaded) return NULL;
	if (skin.iscaseequal(WASABI_API_SKIN->getSkinName()) && !skininfoW.isempty()) return skininfoW;

	skininfoW = L"";
	SkinInfosXmlReader r(WASABI_API_SKIN->getSkinName());

	/* skin name */
	const wchar_t *name = r.getFullName();
	if (name && *name)
		skininfoW += name;
	else
		skininfoW += WASABI_API_SKIN->getSkinName();

	/* skin version */
	const wchar_t *ver = r.getVersion();
	if (ver && *ver)
	{
		skininfoW += L" v";
		skininfoW += ver;
	}
	skininfoW += L"\r\n";

	/* skin author & e-mail */
	const wchar_t *aut = r.getAuthor();
	if (aut && *aut)
	{
		skininfoW += WASABI_API_LNGSTRINGW(IDS_BY_SPACE);
		skininfoW += aut;

		const wchar_t *email = r.getEmail();
		if (email && *email)
		{
			skininfoW += L" (";
			skininfoW += email;
			skininfoW += L")";
		}
		skininfoW += L"\r\n";
	}

	/* skin homepage */
	const wchar_t *web = r.getHomepage();
	if (web && *web)
	{
		skininfoW += web;
		skininfoW += L"\r\n";
	}
	skininfoW += L"\r\n";
	const wchar_t *comment = r.getComment();
	if (comment && *comment)
		skininfoW += comment;

	skin = WASABI_API_SKIN->getSkinName();
	return skininfoW;
}

const char *getSkinInfo()
{
	static String skininfo;

	// these checks is repeated in getSkinInfoW, also
	if (!m_are_we_loaded) return NULL;
	if (skin.iscaseequal(WASABI_API_SKIN->getSkinName()) && !skininfo.isempty()) return skininfo;

	// get the wide character version
	const wchar_t *wideSkinInfo = getSkinInfoW();

	// and convert
	skininfo = AutoChar(wideSkinInfo);
	return skininfo;
}