#include <precomp.h>
#include <api.h>
#include "skinelem.h"
#include <api/skin/skin.h>
#include <api/skin/skinparse.h>
#include <api/wac/compon.h>
#include <api/wac/wac.h>
#include <api/font/font.h>
#include <bfc/parse/pathparse.h>
#include <api/service/svcs/svc_collection.h>
#include <tataki/canvas/bltcanvas.h>

xml_elementtag elementtaglist[] =
    {
        {L"bitmap", XML_ELEMENTTAG_BITMAP, 0},
        {L"bitmapfont", XML_ELEMENTTAG_BITMAPFONT, 0},
        {L"color", XML_ELEMENTTAG_COLOR, 0},
        {L"cursor", XML_ELEMENTTAG_CURSOR, 0},
        {L"elements", XML_ELEMENTTAG_ELEMENTS, 1},
        {L"elementalias", XML_ELEMENTTAG_ELEMENTALIAS, 0},
        {L"truetypefont", XML_ELEMENTTAG_TRUETYPEFONT, 0},
    };

//-------------------------







//-------------------------

void SkinElementsMgr::init()
{
	if (!quickxmltaglist.getNumItems())
	{
		for (int i = 0;i < sizeof(elementtaglist) / sizeof(xml_elementtag);i++)
			quickxmltaglist.addItem(&elementtaglist[i]);
	}
	skinXML.registerCallback(L"WinampAbstractionLayer\felements\f*", &xmlreader); //back compat
	skinXML.registerCallback(L"WasabiXML\felements\f*", &xmlreader);
}

void SkinElementsMgr::deinit()
{
	resetSkinElements();
	skinXML.unregisterCallback(&xmlreader);
}

void SkinElementsMgr::onBeforeLoadingSkinElements(const wchar_t *_rootpath)
{
	Skin::sendBeforeLoadingElementsCallback();
	elementScriptId = WASABI_API_PALETTE->newSkinPart();
	
	WASABI_API_PALETTE->StartTransaction();

	rootpath = _rootpath;
	original_rootpath = rootpath;
	last_includepath = L"";
}

void SkinElementsMgr::onAfterLoadingSkinElements()
{
	WASABI_API_PALETTE->EndTransaction();

#ifdef WASABI_COMPILE_COMPONENTS
	ComponentManager::broadcastNotify(WAC_NOTIFY_SKINELEMENTSLOADED, Skin::getSkinPartIterator());
#endif
}

void SkinElementsXmlReader::xmlReaderOnEndElementCallback(const wchar_t *xmltag)
{
	SkinElementsMgr::xmlReaderOnEndElementCallback(xmltag);
}

void SkinElementsMgr::xmlReaderOnEndElementCallback(const wchar_t *xmltag)
{
	xml_elementtag *i = quickxmltaglist.findItem(xmltag);
	if (!i) return ;
	if (i->id == XML_ELEMENTTAG_ELEMENTS)
	{
		if (inelements)
			inelements = 0;
	}
}

void SkinElementsXmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	SkinElementsMgr::xmlReaderOnStartElementCallback(xmltag, params);
}

void SkinElementsMgr::xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	xml_elementtag *i = quickxmltaglist.findItem(xmltag);
	if (i) 
		_xmlReaderOnStartElementCallback( i->id, xmltag, params);
	else 
		_xmlReaderOnStartElementCallback( XML_ELEMENTTAG_UNKNOWN, xmltag, params);
}

void SkinElementsMgr::_xmlReaderOnStartElementCallback(int tagid, const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	const wchar_t *ic = skinXML.getIncludePath();
	if (WCSICMP(ic, last_includepath))
	{
		last_includepath = skinXML.getIncludePath();
		rootpath = getSkinRootpathFromIncludePath(last_includepath, original_rootpath);
	}
	// If we're loading from a buffer, there should be no rootpath prefix.
	if (!WCSNICMP(rootpath, L"buf:", 4))
	{
		rootpath = NULL;
	}
	if (tagid == XML_ELEMENTTAG_ELEMENTALIAS)
	{
		WASABI_API_PALETTE->AddAlias(params->getItemValue(L"id"), params->getItemValue(L"target"));
	}
	else if (tagid == XML_ELEMENTTAG_BITMAP)
	{
		StringW id;
		const wchar_t *fn;
		id = params->getItemValue(L"id");
		fn = params->getItemValue(L"file");
		int x = params->getItemValueInt(L"x", -1);
		int y = params->getItemValueInt(L"y", -1);
		int w = params->getItemValueInt(L"w", -1);
		int h = params->getItemValueInt(L"h", -1);

		const wchar_t *aliastarget = WASABI_API_PALETTE->getElementAlias(id);
		if (aliastarget)
			id = aliastarget;

		const wchar_t *colorgroup = params->getItemValue(L"colorgroup");
		if (!colorgroup || !*colorgroup)
			colorgroup = params->getItemValue(L"gammagroup");

		WASABI_API_PALETTE->AddBitmap(id, fn, rootpath, x, y, w, h, params, colorgroup);

	}
	else if (tagid == XML_ELEMENTTAG_COLOR)
	{
		const wchar_t *colorstr = params->getItemValue(L"value");
		StringW id = params->getItemValue(L"id");
		const wchar_t *aliastarget = WASABI_API_PALETTE->getElementAlias(id);
		if (aliastarget)
			id = aliastarget;
		const wchar_t *colorgroup = params->getItemValue(L"colorgroup");
		if (!colorgroup || !*colorgroup)
			colorgroup = params->getItemValue(L"gammagroup");
		if (!wcschr(colorstr, ','))
		{
			ARGB32 c = WASABI_API_PALETTE->getColorElement((colorstr));
			WASABI_API_PALETTE->AddColor(id, c, colorgroup, rootpath, params);
		}
		else
    {
				WASABI_API_PALETTE->AddColor((id), SkinParser::parseColor(colorstr), colorgroup, rootpath, params);
    }
	}
	else if (tagid == XML_ELEMENTTAG_BITMAPFONT)
	{
		Font::installBitmapFont(params->getItemValue(L"file"), rootpath, params->getItemValue(L"id"), params->getItemValueInt(L"charwidth", 0), params->getItemValueInt(L"charheight", 0), params->getItemValueInt(L"hspacing", 0), params->getItemValueInt(L"vspacing", 0), elementScriptId, params->getItemValueInt(L"allowmapping", 1));
	}
	else if (tagid == XML_ELEMENTTAG_TRUETYPEFONT)
	{
		Font::installTrueTypeFont(params->getItemValue(L"file"), rootpath, params->getItemValue(L"id"), elementScriptId, params->getItemValueInt(L"allowmapping", 1), 0);
	}
	else if (tagid == XML_ELEMENTTAG_CURSOR)
	{
		const wchar_t *bitmap = params->getItemValue(L"bitmap");
		StringW id = params->getItemValue(L"id");
		const wchar_t *aliastarget = WASABI_API_PALETTE->getElementAlias(id);
		int x = params->getItemValueInt(L"hotspot_x", 0);
		int y = params->getItemValueInt(L"hotspot_y", 0);
		if (aliastarget)
			id = aliastarget;
		WASABI_API_PALETTE->AddCursor(id, bitmap, x, y, rootpath, params);
	}
	else if (tagid == XML_ELEMENTTAG_UNKNOWN)
	{
		CollectionSvcEnum cse(xmltag);
		svc_collection *svc;
		if (svc = cse.getFirst())
		{ // got one!
			svc->addElement(params->getItemValue(L"id"), rootpath, elementScriptId, params);
			WASABI_API_SVC->service_release(svc);
		}
	}
	else
	{
		DebugStringW(L"SkinElementsMgr: tag %s was recognized but not handled!\n", xmltag);
	}
}

void SkinElementsMgr::resetSkinElements()
{
	WASABI_API_PALETTE->Reset();

	Font::uninstallAll();
	// remove any element inserted into a hierarchical collection
	for (int i = 0;i < (int)WASABI_API_SVC->service_getNumServices(WaSvc::COLLECTION);i++)
	{
		waServiceFactory *f = WASABI_API_SVC->service_enumService(WaSvc::COLLECTION, i);
		if (f != NULL)
		{
			svc_collection *svc = static_cast<svc_collection*>(f->getInterface(FALSE));
			svc->removeAllElements();
			f->releaseInterface(svc);
		}
	}
}

void SkinElementsMgr::onBeforeLoadingScriptElements(const wchar_t *name, int script_id)
{
	SkinElementsMgr::rootpathstack.addItem(new StringW(rootpath));
	oldid = elementScriptId;
	oldinel = inelements;

	WASABI_API_PALETTE->StartTransaction();
	wchar_t buf[WA_MAX_PATH] = {0};
	WCSCPYN(buf, name, WA_MAX_PATH);

	wchar_t *ptr = const_cast<wchar_t *>(Wasabi::Std::filename(buf));
	if (ptr != NULL) *ptr = '\0';
	rootpath = buf;
	rootpath.AddBackslash();

	original_rootpath = rootpath;

	last_includepath = L"";

	inelements = 0;
	elementScriptId = script_id;
}

void SkinElementsMgr::onAfterLoadingScriptElements()
{
	WASABI_API_PALETTE->EndTransaction();
	elementScriptId = oldid;
	inelements = oldinel;
	rootpath = SkinElementsMgr::rootpathstack.getLast();
	delete SkinElementsMgr::rootpathstack.getLast();
	SkinElementsMgr::rootpathstack.removeLastItem();
}

void SkinElementsMgr::unloadScriptElements(int scriptid)
{
	int i;


	WASABI_API_PALETTE->UnloadElements(scriptid);
	
	Font::uninstallByScriptId(scriptid);
	// remove any element inserted into a hierarchical collection
	for (i = 0;i < (int)WASABI_API_SVC->service_getNumServices(WaSvc::COLLECTION);i++)
	{
		waServiceFactory *f = WASABI_API_SVC->service_enumService(WaSvc::COLLECTION, i);
		if (f != NULL)
		{
			svc_collection *svc = static_cast<svc_collection*>(f->getInterface(FALSE));
			svc->removeElement(scriptid);
			f->releaseInterface(svc);
		}
	}
}

int SkinElementsMgr::elementEqual(const wchar_t *file1, const wchar_t *rootpath1,
                                  const wchar_t *file2, const wchar_t *rootpath2)
{

	StringPathCombine a(rootpath1, file1);
	StringPathCombine b(rootpath2, file2);

	return PATHEQL(a, b);
}

const wchar_t *SkinElementsMgr::getSkinRootpathFromIncludePath(const wchar_t *includepath, const wchar_t *def)
{
	if (!wcsstr(includepath, L"..")) return def;

	PathParserW pp(includepath);
	if (pp.getNumStrings() < 2 || !WCSCASEEQLSAFE(pp.enumString(0), L"skins"))  // UNSAFE if the skinpath isn't "skins"
		return def;

	StringW baseskin = pp.enumString(1);

	if (wcsstr(includepath, L".."))
	{
		int x = 0;
		for (int i = 0;i < pp.getNumStrings();i++)
		{
			const wchar_t *p = pp.enumString(i);
			if (WCSICMP(p, L".."))
			{
				if (x == 1)
					baseskin = pp.enumString(i);
				x++;
			}
			else
				x--;
		}
	}

	t_rootpath = pp.enumString(0);
	t_rootpath.AppendFolder(baseskin);
	return t_rootpath;
}

SkinElementsXmlReader SkinElementsMgr::xmlreader;
int SkinElementsMgr::inelements = 0;
int SkinElementsMgr::elementScriptId = -1;
int SkinElementsMgr::oldid, SkinElementsMgr::oldinel;
StringW SkinElementsMgr::rootpath, SkinElementsMgr::original_rootpath, SkinElementsMgr::t_rootpath, SkinElementsMgr::last_includepath;
PtrList<StringW> SkinElementsMgr::rootpathstack;
PtrListQuickSorted<xml_elementtag, XmlElementTagComp> SkinElementsMgr::quickxmltaglist;
