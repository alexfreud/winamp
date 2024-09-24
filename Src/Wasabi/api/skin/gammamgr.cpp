#include <precomp.h>
#include <api.h>
#include "gammamgr.h"
#include <api/service/servicei.h>
#include <api/skin/skinelem.h>
#include <api/syscb/callbacks/skincb.h>



void GammaMgr::init()
{
	skinXML.registerCallback(L"WinampAbstractionLayer\fgammaset", &xmlreader); //back compat
	skinXML.registerCallback(L"WinampAbstractionLayer\fgammaset\fgammagroup", &xmlreader); //back compat
	skinXML.registerCallback(L"WasabiXML\fgammaset", &xmlreader);
	skinXML.registerCallback(L"WasabiXML\fgammaset\fgammagroup", &xmlreader);	
	
	curset = -1;
}

void GammaMgr::deinit()
{
	WASABI_API_COLORTHEMES->deleteAllGammaSets();
	skinXML.unregisterCallback(&xmlreader);
}

void GammaMgr::onBeforeLoadingGammaGroups()
{
	WASABI_API_COLORTHEMES->deleteAllGammaSets();
	curset = -1;
}

void GammaMgr::onAfterLoadingGammaGroups()
{}

void GammaMgr::loadDefault()
{
	WASABI_API_COLORTHEMES->StartTransaction();
#ifdef ON_LOAD_EXTRA_COLORTHEMES
	ON_LOAD_EXTRA_COLORTHEMES();
#endif
	WASABI_API_COLORTHEMES->EndTransaction();

	if (WASABI_API_COLORTHEMES->getGammaSet() == NULL)
	{
		wchar_t txt[256] = {0};
		WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"Color Themes/%s", WASABI_API_SKIN->getSkinName()), txt, 256, L"Default");
		WASABI_API_COLORTHEMES->setGammaSet(txt);
	}
}

int GammaMgr::gammaEqual(const wchar_t *id1, const wchar_t *id2)
{
	const wchar_t *a = WASABI_API_PALETTE->getGammaGroupFromId(id1);
	const wchar_t *b = WASABI_API_PALETTE->getGammaGroupFromId(id2);
	if (!a || !b) return 1;
	return !WCSICMP(a, b);
}

void GammaMgrXmlReader::xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	GammaMgr::xmlReaderOnStartElementCallback(xmltag, params);
}

void GammaMgr::xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	if (!WCSICMP(xmltag, L"gammaset"))
	{
		const wchar_t *id = params->getItemValue(L"id");
		WASABI_API_COLORTHEMES->resetGammaSet(id);
		curset = WASABI_API_COLORTHEMES->newGammaSet(id);
	}
	else if (!WCSICMP(xmltag, L"gammagroup"))
	{
		const wchar_t *id = params->getItemValue(L"id");
		if (!id) // TODO: should we pop up a skin error msgbox?
			return;
		StringW value = params->getItemValue(L"value");
		int r = 0, g = 0, b = 0;
		wchar_t *p = value.getNonConstVal();
		if (!p) // TODO: should we pop up a skin error msgbox?
			return; 
		wchar_t *q = p;
		while (q && *q && *q != ',') q++;
		if (q && *q == ',') *q++ = 0;
		r = WTOI(p);
		if (*q)
		{
			p = q;
			while (*q && *q != ',') q++;
			if (*q == ',') *q++ = 0;
			g = WTOI(p);
			if (*q) b = WTOI(q);
		}

		int makegray = params->getItemValueInt(L"gray");
		int boost = params->getItemValueInt(L"boost");
		if (!WCSICMP(id, L"General") && curset>=0)
		{
			ColorThemeGroup *generalgroup = WASABI_API_COLORTHEMES->enumColorThemeGroup(curset, -1);
			if (generalgroup)
			{
				generalgroup->setRed(r);
				generalgroup->setGreen(g);
				generalgroup->setBlue(b);
				generalgroup->setGray(makegray);
				generalgroup->setBoost(boost);
			}
		}
		else if (curset != -1)
		{
			ColorThemeGroupI new_group(id, r, g, b, makegray, boost);
			WASABI_API_COLORTHEMES->addGammaGroup(curset, &new_group);
		}
	}

}

GammaMgrXmlReader GammaMgr::xmlreader;
int GammaMgr::curset;

#define CBCLASS ColorThemeGroupI
START_DISPATCH;
CB(COLORTHEMEGROUPGETNAME, getName);
CB(COLORTHEMEGROUPGETRED, getRed);
CB(COLORTHEMEGROUPGETGREEN, getGreen);
CB(COLORTHEMEGROUPGETBLUE, getBlue);
CB(COLORTHEMEGROUPGETGRAY, getGray);
CB(COLORTHEMEGROUPGETBOOST, getBoost);
VCB(COLORTHEMEGROUPSETNAME, setName);
VCB(COLORTHEMEGROUPSETRED, setRed);
VCB(COLORTHEMEGROUPSETGREEN, setGreen);
VCB(COLORTHEMEGROUPSETBLUE, setBlue);
VCB(COLORTHEMEGROUPSETGRAY, setGray);
VCB(COLORTHEMEGROUPSETBOOST, setBoost);
END_DISPATCH;