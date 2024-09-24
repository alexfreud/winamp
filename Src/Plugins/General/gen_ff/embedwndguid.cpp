#include "precomp__gen_ff.h"
#include "embedwndguid.h"
#include "wa2wndembed.h"
#include "wa2frontend.h"
#include "wa2cfgitems.h"
#include <bfc/string/stringW.h>

extern void initFFApi();

EmbedWndGuidMgr embedWndGuidMgr;

EmbedWndGuid::EmbedWndGuid(embedWindowState *_ws)
{
	hwnd = NULL;
	guid = INVALID_GUID;
	if (_ws == NULL) return ;

	ws = _ws;
	hwnd = ws->me;
	embedWndGuidMgr.getGuid(this);
}

EmbedWndGuid::EmbedWndGuid(EmbedWndGuid *wg)
{
	hwnd = NULL;
	guid = INVALID_GUID;
	if (wg == NULL) return ;

	ws = wg->getEmbedWindowState();
	hwnd = ws->me;
	guid = wg->getGuid();
}

GUID EmbedWndGuidMgr::getGuid(embedWindowState *ws)
{
	foreach(table)
	EmbedWndGuid *ewg = table.getfor();
	if (ewg->getEmbedWindowState() == ws)
	{
		return ewg->getGuid();
	}
	endfor;
	return INVALID_GUID;
}

GUID EmbedWndGuidMgr::getGuid(EmbedWndGuid *wg)
{
	if (wg == NULL) return INVALID_GUID;
	// if we aren't loaded yet, init wasabi, otherwise ignore
	initFFApi();

	int gotit = 0;
	int nowrite = 0;
	wchar_t windowTitle[256] = L"";
	HWND child;

	GUID newGuid = INVALID_GUID;

	if (wg->getEmbedWindowState()->flags & EMBED_FLAGS_GUID)
	{
		newGuid = GET_EMBED_GUID(wg->getEmbedWindowState());
		nowrite=1;
		goto bypass;
	}

	if (wa2.isVis(wg->getEmbedWindowState()->me))
	{
		newGuid = avs_guid;
		nowrite = 1;
		extern int disable_send_visrandom;
		extern _bool visrandom;
		wa2.pollVisRandom();
		goto bypass;
	}

	child = GetWindow(wg->getEmbedWindowState()->me, GW_CHILD);
	if (child == wa2.getMediaLibrary())
	{ newGuid = library_guid; nowrite = 1; goto bypass; }

	if (!gotit)
	{
		foreach(table)
		EmbedWndGuid *ewg = table.getfor();
		if (ewg->getEmbedWindowState() == wg->getEmbedWindowState())
		{
			wg->setGuid(ewg->getGuid());
			gotit = 1;
			break;
		}
		endfor;
	}

	if (!gotit)
	{
		// not found, look for window title in saved table
		GetWindowTextW(wg->getEmbedWindowState()->me, windowTitle, 256);
		if (*windowTitle)
		{
			wchar_t str[256] = L"";
			StringW configString = windowTitle;
			configString += L"_guid";
			WASABI_API_CONFIG->getStringPrivate(configString, str, 256, L"");
			if (*str)
			{
				wg->setGuid(nsGUID::fromCharW(str));
				table.addItem(new EmbedWndGuid(wg));
				gotit = 1;
			}
		}
	}

	if (!gotit)
	{
		// not found in saved table, or no title for the window, assign a new guid
		if (!_wcsicmp(windowTitle, L"AVS"))
		{ newGuid = avs_guid; nowrite = 1; }
		else
			CoCreateGuid(&newGuid);

	bypass:

		wg->setGuid(newGuid);

		// save a copy of the element
		table.addItem(new EmbedWndGuid(wg));

		// write the guid in the saved table
		if (*windowTitle && !nowrite)
		{
			wchar_t str[256] = {0};
			nsGUID::toCharW(newGuid, str);
			StringW configString = windowTitle;
			configString += L"_guid";
			WASABI_API_CONFIG->setStringPrivate(configString, str);
		}
	}
	return wg->getGuid();
}

embedWindowState *EmbedWndGuidMgr::getEmbedWindowState(GUID g)
{
	foreach(table)
	EmbedWndGuid *ewg = table.getfor();
	if (ewg->getGuid() == g)
	{
		embedWindowState *ews = ewg->getEmbedWindowState();
		if (wa2.isValidEmbedWndState(ews))
			return ews;
	}
	endfor;
	return NULL;
}

void EmbedWndGuidMgr::retireEmbedWindowState(embedWindowState *ws)
{
	foreach(table)
	EmbedWndGuid *ewg = table.getfor();
	if (ewg->getEmbedWindowState() == ws)
	{
		delete table.getfor();
		table.removeByPos(foreach_index);
		break;
	}
	endfor;
}

int EmbedWndGuidMgr::testGuid(GUID g)
{
	//  if (g == library_guid) return 1;
	//  if (g == avs_guid) return 1;
	foreach(table)
	EmbedWndGuid *ewg = table.getfor();
	if (ewg->getGuid() == g)
	{
		if (!wa2.isValidEmbedWndState(ewg->getEmbedWindowState())) retireEmbedWindowState(ewg->getEmbedWindowState());
		else return 1;
	}
	endfor;
	return 0;
}

int EmbedWndGuidMgr::getNumWindowStates()
{
	return table.getNumItems();
}

GUID EmbedWndGuidMgr::enumWindowState(int n, embedWindowState **ws)
{
	EmbedWndGuid *ewg = table.enumItem(n);
	if (ewg)
	{
		embedWindowState *ews = ewg->getEmbedWindowState();
		if (wa2.isValidEmbedWndState(ews))
		{
			if (ws) *ws = ews;
			return ewg->getGuid();
		}
	}
	return INVALID_GUID;
}