#include <precomp.h>
#include "xuithemeslist.h"
#include <api/wnd/popup.h>
#include "../Agave/Language/api_language.h"
#include "resource.h"

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(ColorThemesList_Svc);
DECLARE_SERVICE(XuiObjectCreator<ColorThemesListXuiSvc>);
DECLARE_SERVICE(XuiObjectCreator<NakedColorThemesListXuiSvc>);
DECLARE_SERVICE(ActionCreator<ThemesSlotActionSvc>);
END_SERVICES(ColorThemesList_Svc, _ColorThemesList_Svc);

#ifdef _X86_
extern "C" { int _link_ColorThemesListXuiSvc; }
#else
extern "C" { int __link_ColorThemesListXuiSvc; }
#endif

#endif

int ThemesSlotActionSvc::onActionId(int pvtid, const wchar_t *action, const wchar_t *param, int p1, int p2, void *data, int datalen, ifc_window *source) {
	PopupMenu p_load;
	for (int i=0;i<10;i++) 
	{
		StringW s = ColorThemesList::getSlot(i), no_set = WASABI_API_LNGSTRINGW(IDS_NO_SET);
		p_load.addCommand(StringPrintfW(WASABI_API_LNGSTRINGW(IDS_SLOT_X_X), i+1, s.isempty() ? no_set/*L"no set"*/ : s), 200+i, 0, s.isempty());
	}
	int r = p_load.popAtMouse();
	if (r >= 200 && r < 210)
	{
		StringW set = ColorThemesList::getSlot(r-200);
		if (!set.isempty())
			WASABI_API_SKIN->colortheme_setColorSet(set);
	}
	return 1;
}

// -----------------------------------------------------------------------
const wchar_t ColorThemesListXuiObjectStr[] = L"ColorThemes:List"; // This is the xml tag
char ColorThemesListXuiSvcName[] = "ColorThemes:List xui object";
const wchar_t NakedColorThemesListXuiObjectStr[] = L"ColorThemes:Mgr"; // This is the xml tag
char NakedColorThemesListXuiSvcName[] = "ColorThemes:Mgr xui object";

XMLParamPair ColorThemesList::params[] = {
	{CTLIST_NOHSCROLL, L"NOHSCROLL"},	
};

// -----------------------------------------------------------------------
ColorThemesList::ColorThemesList() {
	setPreventMultipleSelection(1);
	ensure_on_paint = -1;
	setAutoSort(1);
	nohscroll = 0;
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
}

void ColorThemesList::CreateXMLParameters(int master_handle)
{
	//THEMESLIST_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ColorThemesList::~ColorThemesList() {
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
}

// -----------------------------------------------------------------------
NakedColorThemesList::NakedColorThemesList() {
	xuihandle = newXuiHandle();
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
}

// -----------------------------------------------------------------------
NakedColorThemesList::~NakedColorThemesList() {
	items.deleteAll();
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
}

// -----------------------------------------------------------------------
int ColorThemesList::onInit() {
	THEMESLIST_PARENT::onInit();
	addColumn(L"Theme",250);
	//loadThemes();
	return 1;
}

// -----------------------------------------------------------------------
int ColorThemesList::onResize() {
	THEMESLIST_PARENT::onResize();
	if (nohscroll) {
		RECT r;
		getClientRect(&r);
		ListColumn *col = getColumn(0);
		col->setWidth(r.right-r.left-4);
	}
	return 1;
}

// -----------------------------------------------------------------------
void ColorThemesList::onDoubleClick(int itemnum) {
	colorthemes_switch();
}

// -----------------------------------------------------------------------
int ColorThemesList::onRightClick(int itemnum) {
	THEMESLIST_PARENT::onRightClick(itemnum);
	PopupMenu p_save;
	PopupMenu p_load;
	PopupMenu p;
	StringW no_set = WASABI_API_LNGSTRINGW(IDS_NO_SET);

	for (int i=0;i<10;i++) 
	{
		StringW s = getSlot(i);
		p_save.addCommand(StringPrintfW(WASABI_API_LNGSTRINGW(IDS_SLOT_X_X), i+1, s.isempty() ? no_set : s), 100+i);
	}
	for (int i=0;i<10;i++) {
		StringW s = getSlot(i);
		p_load.addCommand(StringPrintfW(WASABI_API_LNGSTRINGW(IDS_SLOT_X_X), i+1, s.isempty() ? no_set : s), 200+i, 0, s.isempty());
	}

	p.addSubMenu(&p_load, WASABI_API_LNGSTRINGW(IDS_XUITHEME_LOAD));
	p.addSubMenu(&p_save, WASABI_API_LNGSTRINGW(IDS_XUITHEME_SAVE));

	int r = p.popAtMouse();
	if (r >= 100 && r < 110) {
		int sel = getFirstItemSelected();
		if (sel >= 0) {
			wchar_t set[256+11]=L"";
			getItemLabel(sel, 0, set, 255); set[255] = 0;
			int p = (getItemData(sel) & 0xFFFF0000) >> 16;
			if (p)
				WCSCPYN(set, StringPrintfW(L"{coloredit}%s", set), 255+11);
			setSlot(r-100, set);
		}
	}
	else if (r >= 200 && r < 210) {
		StringW set = getSlot(r-200);
		if (!set.isempty())
			WASABI_API_SKIN->colortheme_setColorSet(set);
	}
	return 1;
}

void ColorThemesList::setSlot(int i, const wchar_t *set) {
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"%s/slot%d", WASABI_API_SKIN->getSkinName(), i+1), set);
}

const wchar_t *ColorThemesList::getSlot(int i) {
	static wchar_t set[256]=L"";
	WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"%s/slot%d", WASABI_API_SKIN->getSkinName(), i+1), set, 256, L""); 
	return set;
}

// -----------------------------------------------------------------------
int ColorThemesList::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
	if (!_wcsicmp(action, L"colorthemes_switch")) { colorthemes_switch(); return 1; }
	if (!_wcsicmp(action, L"colorthemes_next")) { colorthemes_next(); return 1; }
	if (!_wcsicmp(action, L"colorthemes_previous")) { colorthemes_previous(); return 1; }
	return THEMESLIST_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

// -----------------------------------------------------------------------
int NakedColorThemesList::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
	if (!_wcsicmp(action, L"colorthemes_next")) { colorthemes_next(); return 1; }
	if (!_wcsicmp(action, L"colorthemes_previous")) { colorthemes_previous(); return 1; }
	return THEMESLIST2_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

// -----------------------------------------------------------------------
void ColorThemesList::colorthemes_switch() {
	int sel = getFirstItemSelected();
	if (sel >= 0) {
		wchar_t set[256+11]=L"";
		getItemLabel(sel, 0, set, 255); set[255] = 0;
		int p = (getItemData(sel) & 0xFFFF0000) >> 16;
		if (p)
			WCSCPYN(set, StringPrintfW(L"{coloredit}%s", set), 255+11); 
		WASABI_API_SKIN->colortheme_setColorSet(set);
	}
}

// -----------------------------------------------------------------------
void ColorThemesList::colorthemes_advance(int n) 
{
	StringW curs = WASABI_API_SKIN->colortheme_getColorSet();
	curs.trunc(255);
	wchar_t txt[256+11] = {0};
	int i;
	for (i=0;i<getNumItems();i++) 
	{
		getItemLabel(i, 0, txt, 255);
		int p = (getItemData(i) & 0xFFFF0000) >> 16;
		txt[256] = 0;
		if (p) 
			WCSCPYN(txt, StringPrintfW(L"{coloredit}%s", txt), 255+11); 
		if (!_wcsicmp(txt, curs))
			break;
	}
	if (i >= WASABI_API_SKIN->colortheme_getNumColorSets()) return;
	i += n;
	if (i < 0) i = WASABI_API_SKIN->colortheme_getNumColorSets()-1;
	i %= WASABI_API_SKIN->colortheme_getNumColorSets();
	getItemLabel(i, 0, txt, 255); txt[255] = 0;
	int p = (getItemData(i) & 0xFFFF0000) >> 16;
	if (p) 
		WCSCPYN(txt, StringPrintfW(L"{coloredit}%s", txt), 255+11); 
	ensure_on_paint = i;
	WASABI_API_SKIN->colortheme_setColorSet(txt);
	invalidate();
}

// -----------------------------------------------------------------------
int ColorThemesList::getTextBold(LPARAM lParam) {
	if (WCSCASEEQLSAFE(WASABI_API_SKIN->colortheme_enumColorSet(lParam & 0xFFFF), WASABI_API_SKIN->colortheme_getColorSet())) return 1;
	return THEMESLIST_PARENT::getTextBold(lParam);
}

// -----------------------------------------------------------------------
void ColorThemesList::colorthemes_next() {
	colorthemes_advance(1);
}

// -----------------------------------------------------------------------
void ColorThemesList::colorthemes_previous() {
	colorthemes_advance(-1);
}

// -----------------------------------------------------------------------
void NakedColorThemesList::colorthemes_advance(int n) {
	StringW curs = WASABI_API_SKIN->colortheme_getColorSet();
	curs.trunc(255);
	wchar_t txt[256+11] = {0};
	int i;
	for (i=0;i<items.getNumItems();i++) 
	{
	    WCSCPYN(txt, items.enumItem(i)->getName(), 256); 
		int p = (items.enumItem(i)->getData() & 0xFFFF0000) >> 16;
		txt[256] = 0;
		if (p) 
		{ 
			WCSCPYN(txt, StringPrintfW(L"{coloredit}%s", txt), 255+11); 
		}
		if (!_wcsicmp(txt, curs))
			break;
	}
	if (i >= WASABI_API_SKIN->colortheme_getNumColorSets()) return;
	i += n;
	if (i < 0) i = WASABI_API_SKIN->colortheme_getNumColorSets()-1;
	i %= WASABI_API_SKIN->colortheme_getNumColorSets();
	WCSCPYN(txt, items.enumItem(i)->getName(), 256); 
	int p = (items.enumItem(i)->getData() & 0xFFFF0000) >> 16;
	if (p)
	{ 
		WCSCPYN(txt, StringPrintfW(L"{coloredit}%s", txt), 255+11); 
	}
	WASABI_API_SKIN->colortheme_setColorSet(txt);
}

// -----------------------------------------------------------------------
void NakedColorThemesList::colorthemes_next() {
	colorthemes_advance(1);
}

// -----------------------------------------------------------------------
void NakedColorThemesList::colorthemes_previous() {
	colorthemes_advance(-1);
}

// -----------------------------------------------------------------------
void ColorThemesList::onSetVisible(int show) {
	THEMESLIST_PARENT::onSetVisible(show);
	if (show) loadThemes();
	else getDesktopParent()->setFocus();
}

// -----------------------------------------------------------------------
void NakedColorThemesList::onSetVisible(int show) {
	THEMESLIST2_PARENT::onSetVisible(show);
	if (show) loadThemes();
}

// -----------------------------------------------------------------------
void ColorThemesList::loadThemes() 
{
	setAutoSort(0);
	deleteAllItems();
	const wchar_t *curset = WASABI_API_SKIN->colortheme_getColorSet();
	for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++) {
		const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
		int p = 0;
		if (!_wcsnicmp(set, L"{coloredit}", 11)) {
			set += 11;
			p = 1 << 16;
		}
		addItem(set, p | i);
	}

	setAutoSort(1);
	resort();

	if (curset != NULL) {
		wchar_t set[256+11]=L"";
		for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++) 
		{
			getItemLabel(i, 0, set, 255); set[255]=0;
			int p = (getItemData(i) & 0xFFFF0000) >> 16;
			if (p) 
				WCSCPYN(set, StringPrintfW(L"{coloredit}%s", set), 255+11); 
			if (set && !_wcsicmp(curset,set)) 
			{
				setSelected(i, 1);
				ensureItemVisible(i);
				return;
			}
		}
	}
}

// -----------------------------------------------------------------------
void NakedColorThemesList::loadThemes() 
{
	items.setAutoSort(0);
	items.deleteAll();
	//CUT: const wchar_t *curset = WASABI_API_SKIN->colortheme_getColorSet();
	for (int i=0;i<WASABI_API_SKIN->colortheme_getNumColorSets();i++)
	{
		const wchar_t *set = WASABI_API_SKIN->colortheme_enumColorSet(i);
		int p = 0;
		if (!_wcsnicmp(set, L"{coloredit}", 11)) 
		{
			set += 11;
			p = 1 << 16;
		}
		NakedItem *n = new NakedItem(set, p|i);
		items.addItem(n);
	}
	items.setAutoSort(1);
	items.sort(TRUE);
}

// -----------------------------------------------------------------------
int ColorThemesList::setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value) {
	if (xuihandle == _xuihandle) {
	    switch (xmlattrid) {
			case CTLIST_NOHSCROLL: nohscroll = WTOI(value); return 1;
		}
	}
	return THEMESLIST_PARENT::setXuiParam(_xuihandle, xmlattrid, name, value);
}

int ColorThemesList::onPaint(Canvas *canvas) {
	if (ensure_on_paint > -1) ensureItemVisible(ensure_on_paint);
	ensure_on_paint = -1;
	THEMESLIST_PARENT::onPaint(canvas);
	return 1;
}