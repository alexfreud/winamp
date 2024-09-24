#include <precomp.h>
#include "xuidownloadslist.h"
#include "wa2frontend.h"
#include <api/wnd/popup.h>
#include <bfc/parse/pathparse.h>

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(DownloadsList_Svc);
DECLARE_SERVICE(XuiObjectCreator<DownloadsListXuiSvc>);
END_SERVICES(DownloadsList_Svc, _DownloadsList_Svc);

#ifdef _X86_
extern "C" { int _link_DownloadsListXuiSvc; }
#else
extern "C" { int __link_DownloadsListXuiSvc; }
#endif

#endif

// -----------------------------------------------------------------------
const wchar_t DownloadsListXuiObjectStr[] = L"DownloadsList"; // This is the xml tag
char DownloadsListXuiSvcName[] = "DownloadsList xui object";

XMLParamPair DownloadsList::params[] = {
	{CTLIST_NOHSCROLL, L"NOHSCROLL"},	
	};

// -----------------------------------------------------------------------
DownloadsList::DownloadsList ()
{
	setPreventMultipleSelection(1);
	ensure_on_paint = -1;
	nohscroll = 1;

	setAutoSort(false);
	setVirtual(0);

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	

	DownloadsList::skinObjects.addItem(this);
}

void DownloadsList::CreateXMLParameters(int master_handle)
{
	//DOWNLOADSLIST_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
DownloadsList::~DownloadsList() {
	for (int i = 0; i != this->getNumItems(); i++)
	{
		const wchar_t * w = (const wchar_t *) this->getItemData(i);
		delete w;
	}
	DownloadsList::skinObjects.removeItem(this);
}

// -----------------------------------------------------------------------
int DownloadsList::onInit() 
{
	DOWNLOADSLIST_PARENT::onInit();

	addColumn(LocalesManager::GetString(L"nullsoft.browser", 19),100);
	addColumn(LocalesManager::GetString(L"nullsoft.browser", 18), 95);

	ListColumn *urlCol = new ListColumn(LocalesManager::GetString(L"nullsoft.browser", 17), 0);
	insertColumn(urlCol);

	ListColumn *tCol = new ListColumn(LocalesManager::GetString(L"nullsoft.browser", 20), 0);
	insertColumn(tCol);

	// Load all previous downloads - we must begin from the end of our list
	for (int i = activeDownloads.getNumItems()-1; i != -1; i--)
	{
		if (activeDownloads.enumItem(i) == NULL)
		{
			activeDownloads.removeByPos(i);
			continue;	// Next loop
		}
		newItem(STATUS_WAITING, activeDownloads.enumItem(i));
	}

	return 1;
}

// Prohibit list sorting ;)
int DownloadsList::onColumnLabelClick (int col, int x, int y)
{
	//do nothing
	return 1;
}
int DownloadsList::onColumnDblClick (int col, int x, int y)
{
	//do nothing
	return 1;
}

// -----------------------------------------------------------------------
int DownloadsList::onResize() {
	DOWNLOADSLIST_PARENT::onResize();
	if (nohscroll) {
		RECT r;
		getClientRect(&r);
		int nw = r.right-r.left - getColumn(0)->getWidth() - getColumn(1)->getWidth();
		ListColumn *cLoc = getColumn(2);
		ListColumn *cTit = getColumn(3);
		cLoc->setWidth((int)(nw*0.65));
		cTit->setWidth((int)(nw*0.35));
	}
	return 1;
}

// -----------------------------------------------------------------------
void DownloadsList::onDoubleClick(int itemnum) 
{
	if (getItemData(itemnum) != NULL) wa2.playFile(this->getSubitemText(itemnum, 2));
}

// -----------------------------------------------------------------------
int DownloadsList::onRightClick(int itemnum) 
{
	DOWNLOADSLIST_PARENT::onRightClick(itemnum);

	PopupMenu p;
	if (this->getItemData(itemnum) == NULL)
		p.addCommand(L"Wait for file to be transferred", 666, 0 , 1);
	else
	{
		p.addCommand(L"Play", 1, 0 , 0);
		p.addCommand(L"Enqueue", 2, 0 , 0);
	}

	int result = p.popAtMouse();

	switch (result)
	{
		case 1: wa2.playFile(this->getSubitemText(itemnum, 2)); break;
		case 2: wa2.enqueueFile(this->getSubitemText(itemnum, 2)); break;
	}

	return 1;
}

// -----------------------------------------------------------------------
int DownloadsList::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
	if (!_wcsicmp(action, L"play_selected"))
	{
		int sel = getFirstItemSelected();
		
		if (sel > -1)
		{
			if (getItemData(sel) != NULL) wa2.playFile(this->getSubitemText(sel, 2));
		}
		else
		{
			boolean enq = false;
			for (int i = 0; i != getNumItems(); i++)
			{
				if (getItemData(i) != NULL)
				{
					if (!enq)
					{
						wa2.playFile(this->getSubitemText(i, 2));
						enq = true;
					}
					else
						wa2.enqueueFile(this->getSubitemText(i, 2));
				}
			}
		}
		return 1;
	}

	return DOWNLOADSLIST_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

// -----------------------------------------------------------------------
/*int DownloadsList::getTextBold(LPARAM lParam) {
	if (WCSCASEEQLSAFE(WASABI_API_SKIN->colortheme_enumColorSet(lParam & 0xFFFF), WASABI_API_SKIN->colortheme_getColorSet())) return 1;
	return DOWNLOADSLIST_PARENT::getTextBold(lParam);
}*/

// -----------------------------------------------------------------------
void DownloadsList::onSetVisible(int show) {
	DOWNLOADSLIST_PARENT::onSetVisible(show);
	/*if (show) loadThemes();
	else getDesktopParent()->setFocus();*/
}

// -----------------------------------------------------------------------
int DownloadsList::setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value) {
	if (xuihandle == _xuihandle) {
		switch (xmlattrid) {
			case CTLIST_NOHSCROLL: nohscroll = WTOI(value); return 1;
		}
	}
	return DOWNLOADSLIST_PARENT::setXuiParam(_xuihandle, xmlattrid, name, value);
}

int DownloadsList::onPaint(Canvas *canvas) {
	if (ensure_on_paint > -1) ensureItemVisible(ensure_on_paint);
	ensure_on_paint = -1;
	DOWNLOADSLIST_PARENT::onPaint(canvas);
	return 1;
}

void DownloadsList::newItem(int status,  DownloadToken token)
{
	uint64_t total=0;
	const char *url = 0;
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (http)
	{
		total = http->content_length();
		if (url == NULL)
		{
			url = http->get_url();//WAC_API_DOWNLOADMANAGER->GetUrl(token);
			if (!url)
			{
				url = "";
			}
		}
	}
	else
	{
		url = "";
	}

	uint64_t downloaded = WAC_API_DOWNLOADMANAGER->GetBytesDownloaded(token);
	wchar_t text[256] = {0};

	if (total)
	{
		if (total == downloaded)
			return;		// Delete from list on skin-reload	
	}

	StringCchPrintfW(text, 256, L"%I64u / %d %s", downloaded, total, _(L"bytes"));

	switch (status)
	{
		case STATUS_WAITING:
			insertItem(0, _(L"Waiting"), 0);
			break;
		case STATUS_TRANSFERRING:
			insertItem(0, _(L"Transferring"), 0);
			break;
		case STATUS_FINISHED:
			insertItem(0, _(L"Finished"), 0);
			break;
		case STATUS_ERROR:
			insertItem(0, _(L"Error"), 0);
			break;
		default:
			insertItem(0, L"", 0);
			break;
	}

	setSubItem(0, 1, text);
	setSubItem(0, 2, AutoWide(url));
}

/**
 *	Static Managing of downloadlist
 *  handled via callbacks from MediaDownloader
 */
void DownloadsList::onDownloadStart (const char *url, DownloadToken token)
{
	for(int i=0; i<skinObjects.getNumItems(); i++)
	{
		DownloadsList *tmp = skinObjects.enumItem(i);
		tmp->newItem(STATUS_WAITING, token);
		activeDownloads.addItem(token, 0); // dunno if we will need this later on...
	}
}

void DownloadsList::onDownloadTick (DownloadToken token)
{
	int n = activeDownloads.searchItem(token);
	if (n < 0) return;

	uint64_t total=0;
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (http)
	{
		total = http->content_length();
	}

	uint64_t downloaded = WAC_API_DOWNLOADMANAGER->GetBytesDownloaded(token);
	uint64_t percent = 0;

	wchar_t text[256] = {0};
	wchar_t text2[64] = {0};

	if (total)
	{
		percent=downloaded*100;
		if (total)
			percent/=total;
		else
			percent=0;
		StringCchPrintfW(text, 256, L"%I64u / %d %s", downloaded/1024, total/1024, _(L"KB"));
		StringCchPrintfW(text2, 64, L"%s %d%%", _(L"Transferring"), percent);
	}
	else
	{
		StringCchPrintfW(text, 256, L"%I64u / %d %s", downloaded, total, _(L"KB"));
		StringCchPrintfW(text2, 64, _(L"Transferring"));
	}

	for(int i=0; i!=skinObjects.getNumItems(); i++)
	{
		DownloadsList *tmp = skinObjects.enumItem(i);

		tmp->setSubItem(n, 1, text);
		tmp->setItemLabel(n, text2);
	}
}

void DownloadsList::onDownloadEnd (DownloadToken token, const wchar_t * filename)
{
	int n = activeDownloads.searchItem(token);
	if (n < 0) return;

	activeDownloads.setItem(NULL, n);	// So we know the download is done!

	wchar_t artist[256] = {0};
	wchar_t title[256] = {0};
	wa2.getMetaData(filename, L"artist", artist, 256);
	wa2.getMetaData(filename, L"title", title, 256);
	
	StringW display;
	bool hasArtist = !!WCSICMP(artist, L"");
	bool hasTitle= !!WCSICMP(title, L"");

	StringW url = L"";

	for(int i=0; i!=skinObjects.getNumItems(); i++)
	{
		DownloadsList *tmp = skinObjects.enumItem(i);

		if (!WCSICMP(url, L"")) 
			url = tmp->getSubitemText(n, 2);

		tmp->setItemLabel(n,  _(L"Finished"));
		tmp->setSubItem(n, 2, filename);

		if (!hasArtist && !hasTitle)
		{
			PathParserW pp(filename);
			display = pp.getLastString();
		}
		else if (!hasArtist)
			display = title;
		else if (!hasTitle)
			display = artist;
		else
			display = StringPrintfW(L"%s - %s",artist,title); 
		
		tmp->setSubItem(n, 3, display);
		tmp->setItemParam(n, STATUS_FINISHED);
	}
}

void DownloadsList::onDownloadCancel(DownloadToken token)
{
	int n = activeDownloads.searchItem(token);
	if (n < 0) return;

	activeDownloads.setItem(NULL, n);	// So we know the download is done/cancelled!
	const wchar_t * url = 0;

	for(int i=0; i!=skinObjects.getNumItems(); i++)
	{
		DownloadsList *tmp = skinObjects.enumItem(i);
		tmp->setItemLabel(n, _(L"Cancelled"));
		if (!url) url = tmp->getSubitemText(n, 2);
	}

	SystemObject::onDownloadFinished((!url ? L"" : url), false, L"");
}

void DownloadsList::onDownloadError(DownloadToken token, int code)
{
	int n = activeDownloads.searchItem(token);
	if (n < 0) return;

	activeDownloads.setItem(NULL, n);	// So we know the download is done/cancelled!

	const wchar_t * url = 0;

	for(int i=0; i!=skinObjects.getNumItems(); i++)
	{
		DownloadsList *tmp = skinObjects.enumItem(i);
		wchar_t buf[64] = {0};
		StringCchPrintfW(buf, 64, L"%s: %i", _(L"Error"), code);
		tmp->setItemLabel(n, buf);
		if (!url) url = tmp->getSubitemText(n, 2);
	}

	SystemObject::onDownloadFinished((!url ? L"" : url), false, L"");
}

PtrList<void> DownloadsList::activeDownloads;
PtrList<DownloadsList> DownloadsList::skinObjects;