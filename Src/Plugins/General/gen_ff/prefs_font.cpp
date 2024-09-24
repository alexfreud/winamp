#include "precomp__gen_ff.h"
#include <commctrl.h>
#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include <api/font/font.h>
#include <bfc/parse/paramparser.h>
#include "../nu/ListView.h"
#include "prefs.h"
#include "gen.h"
#include "wa2cfgitems.h"
#include <api/font/win32/truetypefont_win32.h>
#include "../Agave/Language/api_language.h"

extern HWND subWnd;
#define ComboBox_SetItemDataW(hwndCtl, index, data)  ((int)(DWORD)SendMessageW((hwndCtl), CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))
#define ComboBox_GetLBTextLenW(hwndCtl, index)           ((int)(DWORD)SendMessageW((hwndCtl), CB_GETLBTEXTLEN, (WPARAM)(int)(index), 0L))
#define ComboBox_GetLBTextW(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessageW((hwndCtl), CB_GETLBTEXT, (WPARAM)(int)(index), (LPARAM)(LPCWSTR)(lpszBuffer)))
#define ComboBox_SelectStringW(hwndCtl, indexStart, lpszSelect)  ((int)(DWORD)SendMessageW((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCWSTR)(lpszSelect)))

static W_ListView mappingList;
int fonts_loaded = 0;
int ComboBox_AddStringW(HWND list, const wchar_t *string);

class font_entry
{
public:
	StringW filename;
	StringW face;
};

class font_entry_comparator1
{
public:
	// comparator for sorting
	static int compareItem(font_entry *p1, font_entry* p2)
	{
		return wcscmp(p1->face, p2->face);
	}
	// comparator for searching
	static int compareAttrib(const wchar_t *attrib, font_entry *item)
	{
		return wcscmp(attrib, item->face);
	}
};
class font_entry_comparator2
{
public:
	// comparator for sorting
	static int compareItem(font_entry *p1, font_entry* p2)
	{
		return _wcsicmp(p1->filename, p2->filename);
	}
	// comparator for searching
	static int compareAttrib(const wchar_t *attrib, font_entry *item)
	{
		return _wcsicmp(attrib, item->filename);
	}
};

int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const wchar_t * str, int width);

PtrListQuickSorted<font_entry, font_entry_comparator1> fontlist;
PtrListQuickSorted<font_entry, font_entry_comparator2> fontlist_byfilename;

PtrListQuickSorted<font_entry, font_entry_comparator1> skin_fontlist;
PtrListQuickSorted<font_entry, font_entry_comparator2> skin_fontlist_byfilename;

void fillFontLists(HWND list, HWND list2, int selectdefault = 1)
{
	fontlist.deleteAll();
	fontlist_byfilename.removeAll();

	ComboBox_ResetContent(list);
	ComboBox_ResetContent(list2);
	SendMessageW(list, CB_INITSTORAGE, 400, 32);
	SendMessageW(list2, CB_INITSTORAGE, 400, 32);

	wchar_t *txt = WMALLOC(WA_MAX_PATH);
	Wasabi::Std::getFontPath(WA_MAX_PATH, txt);
	StringW path = txt;
	StringW deffont = cfg_options_ttfoverridefont.getValue();
	FREE(txt);
	StringPathCombine mask(path, L"*.ttf");
	WIN32_FIND_DATAW fd = {0};
	HANDLE fh;
	int width = 0, width2 = 0;
	if ((fh = FindFirstFileW(mask, &fd)) != INVALID_HANDLE_VALUE)
	{
		while (1)
		{
			StringW fullpath = fd.cFileName;
			if (_wcsicmp(fullpath, L".") && _wcsicmp(fullpath, L".."))
			{
				fullpath = StringPathCombine(path, fullpath);
				StringW fontname = TrueTypeFont_Win32::filenameToFontFace(fullpath);
				if (!fontname.isempty())
				{
					if (fontlist_byfilename.findItem(fd.cFileName)) continue;
					font_entry *fe = new font_entry;
					fe->face = fontname;
					fe->filename = fd.cFileName;
					fontlist.addItem(fe);
					fontlist_byfilename.addItem(fe);
					int idx = ComboBox_AddStringW(list, fontname);
					ComboBox_SetItemData(list, idx, fe);
					width = ResizeComboBoxDropDown(list, 0, fontname, width);

					idx = ComboBox_AddStringW(list2, fontname);
					ComboBox_SetItemData(list2, idx, fe);
					width2 = ResizeComboBoxDropDown(list2, 0, fontname, width2);
				}
			}
			if (!FindNextFileW(fh, &fd)) break;
		}
	}
	if (fh != INVALID_HANDLE_VALUE) FindClose(fh);
	int pos = -1;
	font_entry *fe = fontlist_byfilename.findItem(deffont.v(), NULL);
	if (fe)
	{
		fontlist.sort(1);
		pos = fontlist.searchItem(fe);
	}
	ComboBox_SetCurSel(list, pos);
}


int fm_validMapping(HWND wnd)
{
	if (ComboBox_GetCurSel(GetDlgItem(wnd, IDC_COMBO_SKINFONTS)) == -1) return 0;
	if (ComboBox_GetCurSel(GetDlgItem(wnd, IDC_COMBO_FONTS)) == -1) return 0;
	return 1;
}

int fm_hasSelection(HWND wnd)
{
	int n = mappingList.GetCount();
	for (int i = 0;i < n;i++)
		if (mappingList.GetSelected(i))
			return 1;
	return 0;
}

void fm_itemClicked(HWND wnd, int pos)
{
	HWND combo1 = GetDlgItem(wnd, IDC_COMBO_SKINFONTS);
	HWND combo2 = GetDlgItem(wnd, IDC_COMBO_FONTS);

	int n = mappingList.GetCount();
	for (int i = 0;i < n;i++)
	{
		if (mappingList.GetSelected(i))
		{
			wchar_t txt[4096] = {0};
			mappingList.GetText(i, 0, txt, 4096);
			ComboBox_SetCurSel(combo1, -1);
			if (skin_fontlist.findItem(txt))
				ComboBox_SelectStringW(combo1, -1, StringPrintfW(L" %s", txt));
			else
				ComboBox_SelectStringW(combo1, -1, txt);
			mappingList.GetText(i, 1, txt, 4096);

			ComboBox_SetCurSel(combo2, -1);
			ComboBox_SelectStringW(combo2, -1, txt);

			wchar_t type[64] = {0};
			mappingList.GetText(i, 3, type, 64);

			CheckDlgButton(wnd, IDC_RADIO_THISSKIN, WCSCASEEQLSAFE(type, WASABI_API_LNGSTRINGW(IDS_THIS_SKIN)));
			CheckDlgButton(wnd, IDC_RADIO_ALLSKINS, !WCSCASEEQLSAFE(type, WASABI_API_LNGSTRINGW(IDS_THIS_SKIN)));

			wchar_t scale[64] = {0};
			mappingList.GetText(i, 2, scale, 64);
			int s = WTOI(scale);
			SendMessageW(GetDlgItem(wnd, IDC_SLIDER_SCALE), TBM_SETPOS, 1, s);
			SetDlgItemTextW(wnd, IDC_STATIC_SCALE, StringPrintfW(L"%d%%", s));
			break;
		}
	}
	EnableWindow(GetDlgItem(wnd, IDC_BUTTON_SET), fm_validMapping(wnd) && fm_hasSelection(wnd));
	EnableWindow(GetDlgItem(wnd, IDC_BUTTON_NEW), fm_validMapping(wnd));
	EnableWindow(GetDlgItem(wnd, IDC_BUTTON_DEL), fm_hasSelection(wnd));
}

void fm_invalidate()
{
	Font::uninstallAll(1);
	WASABI_API_WNDMGR->wndTrackInvalidateAll();
}

void fm_scanMapping(HWND ctrl, const wchar_t *id)
{
	int global = 0;
	wchar_t t[256] = L"";
	int scale;
	StringW tmp;
	tmp.printf(L"Skin:%s/Font Mapping/%s", WASABI_API_SKIN->getSkinName(), id);
	WASABI_API_CONFIG->getStringPrivate(tmp, t, 250, L"");

	tmp.printf(L"Skin:%s/Font Mapping/%s_scale", WASABI_API_SKIN->getSkinName(), id);
	scale = WASABI_API_CONFIG->getIntPrivate(tmp, -1);

	if (!*t)
	{
		global = 1;
		tmp.printf(L"Font Mapping/%s", id);
		WASABI_API_CONFIG->getStringPrivate(tmp, t, 250, L"");
		tmp.printf(L"Skin:%s/Font Mapping/%s_scale", WASABI_API_SKIN->getSkinName(), id);
		scale = WASABI_API_CONFIG->getIntPrivate(tmp, -1);
	}
	if (t && *t)
	{
		if (!WCSCASESTR(t, L".ttf")) wcscat(t, L".ttf");
		font_entry *fe = fontlist_byfilename.findItem(t);
		if (fe)
		{
			int pos = mappingList.InsertItem(0, id, 0);
			mappingList.SetItemText(pos, 1, fe->face.getValue());
			mappingList.SetItemText(pos, 2, StringPrintfW(L"%d%%", scale != -1 ? scale : 100).getValue());
			mappingList.SetItemText(pos, 3, WASABI_API_LNGSTRINGW(global ? IDS_ALL_SKINS : IDS_THIS_SKIN));
		}
	}
}

void fm_rescanList(HWND lv)
{
	mappingList.Clear();

	wchar_t t[4096] = L"";
	WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/Mapped", WASABI_API_SKIN->getSkinName()), t, 4096, L"");
	ParamParser pp(t);
	for (int i = 0;i < pp.getNumItems();i++)
	{
		fm_scanMapping(lv, pp.enumItem(i));
	}
	WASABI_API_CONFIG->getStringPrivate(L"Font Mapping/Mapped", t, 4096, L"");
	ParamParser pp2(t);
	for (int i = 0;i < pp2.getNumItems();i++)
	{
		fm_scanMapping(lv, pp2.enumItem(i));
	}
}

void fm_saveMappingLists(HWND wnd)
{
	PtrListQuickSorted<StringW, StringWComparator> global_list;
	PtrListQuickSorted<StringW, StringWComparator> skin_list;

	int n = mappingList.GetCount();
	for (int i = 0;i < n;i++)
	{
		wchar_t txt[4096] = {0};
		mappingList.GetText(i, 0, txt, 4096);

		wchar_t type[64] = {0};
		mappingList.GetText(i, 3, type, 64);

		if (!_wcsicmp(type, WASABI_API_LNGSTRINGW(IDS_THIS_SKIN)))
		{
			if (skin_list.findItem(txt) != NULL) continue;
			skin_list.addItem(new StringW(txt));
		}
		else
		{
			if (global_list.findItem(txt) != NULL) continue;
			global_list.addItem(new StringW(txt));
		}
	}
	StringW s = L"";
	foreach(global_list)
	if (!s.isempty())
		s += L";";
	s += global_list.getfor()->getValue();
	endfor;
	WASABI_API_CONFIG->setStringPrivate(L"Font Mapping/Mapped", s);

	s = L"";
	foreach(skin_list)
	if (!s.isempty()) s += L";";
	s += skin_list.getfor()->getValue();
	endfor;
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/Mapped", WASABI_API_SKIN->getSkinName()), s);

	global_list.deleteAll();
	skin_list.deleteAll();
}

void fm_delMapping(HWND wnd, int pos = -1)
{
	HWND list = GetDlgItem(wnd, IDC_LIST_MAPPINGS);
	int n = mappingList.GetCount();
	int start = 0;
	if (pos != -1) { start = pos; n = pos + 1; }
	for (int i = start;i < n;i++)
	{
		if (pos != -1 || mappingList.GetSelected(i))
		{
			wchar_t txt[4096] = {0};
			mappingList.GetText(i, 0, txt, 4096);

			wchar_t m[64] = {0};
			mappingList.GetText(i, 3, m, 64);

			if (WCSCASEEQLSAFE(m, WASABI_API_LNGSTRINGW(IDS_THIS_SKIN)))
			{
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/%s", WASABI_API_SKIN->getSkinName(), txt), L"");
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/%s_scale", WASABI_API_SKIN->getSkinName(), txt), L"100");
			}
			else
			{
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Font Mapping/%s", txt), L"");
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Font Mapping/%s_scale", txt), L"100");
			}
			mappingList.DeleteItem(i);
			break;
		}
	}
	fm_saveMappingLists(wnd);
	fm_rescanList(list);
	fm_invalidate();
	if (pos == -1)
	{
		EnableWindow(GetDlgItem(wnd, IDC_BUTTON_DEL), 0);
		EnableWindow(GetDlgItem(wnd, IDC_BUTTON_SET), 0);
	}
}

void fm_newMapping(HWND wnd)
{
	HWND combo1 = GetDlgItem(wnd, IDC_COMBO_SKINFONTS);
	HWND combo2 = GetDlgItem(wnd, IDC_COMBO_FONTS);
	int global = !IsDlgButtonChecked(wnd, IDC_RADIO_THISSKIN);

	int pos = ComboBox_GetCurSel(combo1);
	int l = ComboBox_GetLBTextLenW(combo1, pos);
	wchar_t *txt = WMALLOC(l + 1);
	ComboBox_GetLBTextW(combo1, pos, txt);
	if (!*txt)
	{
		FREE(txt);
		return ;
	}
	wchar_t *delme = txt;
	if (*txt == ' ') txt++;
	font_entry *fe = skin_fontlist.findItem(txt);
	if (!fe) fe = fontlist.findItem(txt);
	if (!fe) return ;
	FREE(delme);


	int n = mappingList.GetCount();
	int oldpos = -1;
	for (int i = 0;i < n;i++)
	{
		wchar_t txt[4096] = {0};
		mappingList.GetText(i, 0, txt, 4096);
		if (!_wcsicmp(fe->face, txt)) { oldpos = i; break; }
	}

	if (oldpos != -1) fm_delMapping(wnd, oldpos);

	pos = ComboBox_GetCurSel(combo2);
	l = ComboBox_GetLBTextLenW(combo2, pos);
	txt = WMALLOC(l + 1);
	ComboBox_GetLBTextW(combo2, pos, txt);
	if (!*txt)
	{
		FREE(txt);
		return ;
	}
	font_entry *femap = fontlist.findItem(txt);
	if (!femap) return ;
	FREE(txt);

	StringW file = femap->filename;

	int v = (int)SendMessageW(GetDlgItem(wnd, IDC_SLIDER_SCALE), TBM_GETPOS, 0, 0);

	if (!global)
	{
		WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/%s", WASABI_API_SKIN->getSkinName(), fe->face), file);
		WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"Skin:%s/Font Mapping/%s_scale", WASABI_API_SKIN->getSkinName(), fe->face), v);
	}
	else
	{
		WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Font Mapping/%s", fe->face), file);
		WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"Font Mapping/%s_scale", fe->face), v);
	}

	fm_scanMapping(GetDlgItem(wnd, IDC_LIST_MAPPINGS), fe->face);
	fm_saveMappingLists(wnd);
	fm_invalidate();

	if (oldpos == -1)
	{
		int n = mappingList.GetCount();
		for (int i = 0;i < n;i++)
		{
			wchar_t txt[4096] = {0};
			mappingList.GetText(i, 0, txt, 4096);
			if (!_wcsicmp(fe->face, txt)) { oldpos = i; break; }
		}
	}

	if (oldpos != -1)
	{
		mappingList.SetSelected(oldpos);
		EnableWindow(GetDlgItem(wnd, IDC_BUTTON_DEL), 1);
		EnableWindow(GetDlgItem(wnd, IDC_BUTTON_SET), 1);
	}
}

void fm_modifyMapping(HWND wnd)
{
	int n = mappingList.GetCount();
	int i;
	for (i = 0;i < n;i++)
	{
		if (mappingList.GetSelected(i))
		{
			wchar_t txt[4096] = {0};
			mappingList.GetText(i, 0, txt, 4096);

			wchar_t m[64] = {0};
			mappingList.GetText(i, 3, m, 64);

			if (WCSCASEEQLSAFE(m, WASABI_API_LNGSTRINGW(IDS_THIS_SKIN)))
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Skin:%s/Font Mapping/%s", WASABI_API_SKIN->getSkinName(), txt), L"");
			else
				WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Font Mapping/%s", txt), L"");
			mappingList.DeleteItem(i);
			break;
		}
	}
	if (i == n) return ;
	fm_newMapping(wnd);
	mappingList.SetSelected(i);
}

void fillFontList2(HWND list, int reset = 1)
{
	if (reset) 
		ComboBox_ResetContent(list);

	int width = 0;
	foreach(fontlist)
	const wchar_t *str;
	int idx = ComboBox_AddStringW(list, (str = fontlist.getfor()->face.v()));
	ComboBox_SetItemDataW(list, idx, fontlist.getfor());
	width = ResizeComboBoxDropDown(list, 0, str, width);
	endfor;
}

void fillSkinFontList(HWND list)
{
	ComboBox_ResetContent(list);
	SendMessageW(list, CB_INITSTORAGE, 400, 32);	
	skin_fontlist.deleteAll();
	skin_fontlist_byfilename.removeAll();
	int n = Font::getNumFontDefs();
	for (int i = 0;i < n;i++)
	{
		FontDef *fd = Font::enumFontDef(i);
		if (!fd) continue;
		if (!fd->allowmapping) continue;
		font_entry *fe = new font_entry;
		fe->filename = fd->filename;
		fe->face = fd->id;
		skin_fontlist.addItem(fe);
		skin_fontlist_byfilename.addItem(fe);
		int idx = ComboBox_AddStringW(list, StringPrintfW(L" %s", fe->face));
		ComboBox_SetItemDataW(list, idx, fe);
	}
	fillFontList2(list, 0);
}

VOID CALLBACK FontMappingLoader(HWND hwndDlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwndDlg, idEvent);

	HWND ctrl = GetDlgItem(hwndDlg, IDC_COMBO_FONTS);
	HWND ctrl2 = GetDlgItem(hwndDlg, IDC_COMBO_SKINFONTS);

	fillFontList2(ctrl, 1);
	fillSkinFontList(ctrl2);

	EnableWindow(ctrl, TRUE);
	EnableWindow(ctrl2, TRUE);
}

BOOL CALLBACK fontMapperProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND lv = GetDlgItem(hwndDlg, IDC_LIST_MAPPINGS);
			mappingList.setwnd(lv);

			mappingList.AddCol(WASABI_API_LNGSTRINGW(IDS_FONT), 140);
			mappingList.AddCol(WASABI_API_LNGSTRINGW(IDS_MAPPING), 139);
			mappingList.AddCol(WASABI_API_LNGSTRINGW(IDS_SCALE), 48);
			mappingList.AddCol(WASABI_API_LNGSTRINGW(IDS_TYPE), 64);

			HWND ctrl = GetDlgItem(hwndDlg, IDC_COMBO_FONTS);
			ComboBox_AddStringW(ctrl, WASABI_API_LNGSTRINGW(IDS_LOADING));
			ComboBox_SetCurSel(ctrl, 0);
			EnableWindow(ctrl, FALSE);
			ctrl = GetDlgItem(hwndDlg, IDC_COMBO_SKINFONTS);
			ComboBox_AddStringW(ctrl, WASABI_API_LNGSTRINGW(IDS_LOADING));
			ComboBox_SetCurSel(ctrl, 0);
			EnableWindow(ctrl, FALSE);
			SetTimer(hwndDlg, 0xC0DF, 1, FontMappingLoader);

			fm_rescanList(lv);
			CheckDlgButton(hwndDlg, IDC_RADIO_THISSKIN, 1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_DEL), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_NEW), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SET), 0);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALE), TBM_SETRANGEMAX, 0, 200);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALE), TBM_SETRANGEMIN, 0, 25);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALE), TBM_SETPOS, 1, 100);
			return 1;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return 0;
		case IDOK:
			EndDialog(hwndDlg, 1);
			return 0;
		case IDC_BUTTON_NEW:
			fm_newMapping(hwndDlg);
			return 0;
		case IDC_BUTTON_DEL:
			fm_delMapping(hwndDlg);
			return 0;
		case IDC_BUTTON_SET:
			fm_modifyMapping(hwndDlg);
			return 0;
		case IDC_COMBO_FONTS:
		case IDC_COMBO_SKINFONTS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SET), fm_validMapping(hwndDlg) && fm_hasSelection(hwndDlg));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_NEW), fm_validMapping(hwndDlg));
			return 0;
		}
	case WM_HSCROLL:
		{
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SLIDER_SCALE))
			{
				int t = (int)SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALE), TBM_GETPOS, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_SCALE, StringPrintf("%d%%", t));
				return 0;
			}
			break;
		}
	case WM_NOTIFY:
		{
			int ctrl = (int)wParam;
			NMHDR *pnmh = (NMHDR*)lParam;
			if (ctrl == IDC_LIST_MAPPINGS)
			{
				if (pnmh->code == NM_CLICK)
				{
					NMLISTVIEW *nml = (NMLISTVIEW*)lParam;
					fm_itemClicked(hwndDlg, nml->iItem);
					break;
				}
			}
		}
	}

	const int controls[] = 
	{
		IDC_SLIDER_SCALE,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}

void updateBFReplacement(HWND hwndDlg)
{
	int replace = !IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALLOWBITMAPFONTS);
	EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_TTFOVERRIDE), (replace && fonts_loaded));
	EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE), replace);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_DECREASESIZE), replace);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TTFSCALE), replace);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_INCREASESIZE), replace);
	EnableWindow(GetDlgItem(hwndDlg, IDC_NO_7BIT_OVERRIDE), replace);
}

void updateFreetypeVisible(HWND hwndDlg)
{
	if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_FREETYPETTF))
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_CHARMAP), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_CHARMAP), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_CHARMAP), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_CHARMAP), TRUE);
	}
}


void updateAltFontsVisible(HWND hwndDlg)
{
	if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALTFONTS))
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_NO_ALT_7BIT_OVERRIDE), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_NO_ALT_7BIT_OVERRIDE), TRUE);
	}
}

// {504060F6-7D8C-4ebe-AE1D-A8BDF5EA1881}
static const GUID freetypeFontRendererGUID = 
{ 0x504060f6, 0x7d8c, 0x4ebe, { 0xae, 0x1d, 0xa8, 0xbd, 0xf5, 0xea, 0x18, 0x81 } };

VOID CALLBACK FontLoader(HWND hwndDlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwndDlg, idEvent);

	HWND ctrl = GetDlgItem(hwndDlg, IDC_COMBO_TTFOVERRIDE);
	HWND ctrl2 = GetDlgItem(hwndDlg, IDC_COMBO_DEFAULTFONT);

	fillFontLists(ctrl, ctrl2);

	int pos = -1;
	font_entry *fe = fontlist_byfilename.findItem(cfg_options_ttfoverridefont.getValue(), NULL);
	if (fe)
	{
		fontlist.sort(1);
		pos = fontlist.searchItem(fe);
	}
	ComboBox_SetCurSel(ctrl, pos);

	pos = -1;
	fe = fontlist_byfilename.findItem(cfg_options_defaultfont.getValue(), NULL);
	if (fe)
	{
		fontlist.sort(1);
		pos = fontlist.searchItem(fe);
	}
	ComboBox_SetCurSel(ctrl2, pos);

	EnableWindow(ctrl, (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALLOWBITMAPFONTS)));
	EnableWindow(ctrl2, TRUE);
	EnableWindow(ctrl2, TRUE);
	fonts_loaded = 1;
}

INT_PTR CALLBACK ffPrefsProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			fonts_loaded = 0;

			CheckDlgButton(hwndDlg, IDC_CHECK_ALLOWBITMAPFONTS, cfg_options_allowbitmapfonts.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_ALTFONTS, cfg_options_altfonts.getValueAsInt());

			// delay the loading of the font lists until after the dialog has loaded to improve ui response
			HWND ctrl = GetDlgItem(hwndDlg, IDC_COMBO_TTFOVERRIDE);
			ComboBox_AddStringW(ctrl, WASABI_API_LNGSTRINGW(IDS_LOADING));
			ComboBox_SetCurSel(ctrl, 0);
			EnableWindow(ctrl, FALSE);

			ctrl = GetDlgItem(hwndDlg, IDC_COMBO_DEFAULTFONT);
			ComboBox_AddStringW(ctrl, WASABI_API_LNGSTRINGW(IDS_LOADING));
			ComboBox_SetCurSel(ctrl, 0);
			EnableWindow(ctrl, FALSE);
			SetTimer(hwndDlg, 0xC0DE, 1, FontLoader);

			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE), TBM_SETRANGEMAX, 0, 200);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE), TBM_SETRANGEMIN, 0, 25);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE), TBM_SETPOS, 1, cfg_options_ttfoverridescale.getValueAsInt());
			SetDlgItemTextA(hwndDlg, IDC_STATIC_TTFSCALE, StringPrintf("%d%%", cfg_options_ttfoverridescale.getValueAsInt()));
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALEDEFAULTFONT), TBM_SETRANGEMAX, 0, 200);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALEDEFAULTFONT), TBM_SETRANGEMIN, 0, 25);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALEDEFAULTFONT), TBM_SETPOS, 1, cfg_options_defaultfontscale.getValueAsInt());
			SetDlgItemTextA(hwndDlg, IDC_STATIC_SCALEDEFAULTFONT, StringPrintf("%d%%", cfg_options_defaultfontscale.getValueAsInt()));

			HWND charmaps = GetDlgItem(hwndDlg, IDC_COMBO_CHARMAP);
			if (WASABI_API_SVC->service_getServiceByGuid(freetypeFontRendererGUID) == 0) // no freetype
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_FREETYPE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_FREETYPETTF), FALSE);
				EnableWindow(charmaps, FALSE);
			}
			else
			{
				const wchar_t *v = cfg_options_fontrenderer.getValue();
				CheckDlgButton(hwndDlg, IDC_CHECK_FREETYPETTF, WCSCASEEQLSAFE(v, L"FreeType"));

				ComboBox_AddStringW(charmaps, WASABI_API_LNGSTRINGW(IDS_AUTO_UNICODE_LATIN1_ASCII));
				ComboBox_AddStringW(charmaps, L"Unicode");
				ComboBox_AddStringW(charmaps, L"Apple Roman");
				ComboBox_AddStringW(charmaps, L"Adobe Latin-1");
				ComboBox_AddStringW(charmaps, L"Adobe Standard");
				ComboBox_AddStringW(charmaps, L"Adobe Custom");
				ComboBox_AddStringW(charmaps, L"Adobe Expert");
				ComboBox_AddStringW(charmaps, L"SJIS");
				ComboBox_AddStringW(charmaps, L"Big5");
				ComboBox_AddStringW(charmaps, L"Wansung");
				ComboBox_AddStringW(charmaps, L"Johab");
				ComboBox_SetCurSel(charmaps, cfg_options_freetypecharmap.getValueAsInt() + 1);
			}

			CheckDlgButton(hwndDlg, IDC_NO_7BIT_OVERRIDE, cfg_options_no7bitsttfoverride.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_NO_ALT_7BIT_OVERRIDE, cfg_options_noalt7bitsttfoverride.getValueAsInt());
			
			updateBFReplacement(hwndDlg);
			updateFreetypeVisible(hwndDlg);
			updateAltFontsVisible(hwndDlg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_FONTMAPPER), cfg_options_usefontmapper.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_USEFONTMAPPER, cfg_options_usefontmapper.getValueAsInt());
			return 1;
		}
	case WM_COMMAND:
		{
			int id = (int) LOWORD(wParam);
			int msg = (int)HIWORD(wParam);
			switch (id)
			{
			case IDC_CHECK_ALLOWBITMAPFONTS:
				updateBFReplacement(hwndDlg);
				cfg_options_allowbitmapfonts.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALLOWBITMAPFONTS));
				return 0;
			case IDC_CHECK_ALTFONTS:
				updateAltFontsVisible(hwndDlg);
				cfg_options_altfonts.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALTFONTS));
				return 0;
			case IDC_CHECK_FREETYPETTF:
				updateFreetypeVisible(hwndDlg);
				if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_FREETYPETTF))
					cfg_options_fontrenderer.setValue(L"Win32 TextOut");
				else
					cfg_options_fontrenderer.setValue(L"FreeType");
				return 0;
			case IDC_NO_7BIT_OVERRIDE:
				cfg_options_no7bitsttfoverride.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_NO_7BIT_OVERRIDE));
				return 0;
			case IDC_CHECK_NO_ALT_7BIT_OVERRIDE:
				cfg_options_noalt7bitsttfoverride.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_NO_ALT_7BIT_OVERRIDE));
				return 0;
			case IDC_BUTTON_FONTMAPPER:
				WASABI_API_DIALOGBOXW(IDD_FONTMAPPER, hwndDlg, fontMapperProc);
				return 0;
			case IDC_CHECK_USEFONTMAPPER:
				cfg_options_usefontmapper.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEFONTMAPPER));
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_FONTMAPPER), cfg_options_usefontmapper.getValueAsInt());
				return 0;
			case IDC_COMBO_TTFOVERRIDE:
				{
					if (msg == CBN_SELCHANGE)
					{
						HWND list = GetDlgItem(hwndDlg, IDC_COMBO_TTFOVERRIDE);
						int idx = ComboBox_GetCurSel(list);
						if (idx != -1)
						{
							font_entry *fe = (font_entry *)ComboBox_GetItemData(list, idx);
							if (fe && fontlist.haveItem(fe))
							{
								cfg_options_ttfoverridefont.setValue(fe->filename);
							}
						}
						return 0;
					}
				}
			case IDC_COMBO_DEFAULTFONT:
				{
					if (msg == CBN_SELCHANGE)
					{
						HWND list = GetDlgItem(hwndDlg, IDC_COMBO_DEFAULTFONT);
						int idx = ComboBox_GetCurSel(list);
						if (idx != -1)
						{
							font_entry *fe = (font_entry *)ComboBox_GetItemData(list, idx);
							if (fe && fontlist.haveItem(fe))
							{
								cfg_options_defaultfont.setValue(fe->filename);
							}
						}
						return 0;
					}
				}
			case IDC_COMBO_CHARMAP:
				{
					if (msg == CBN_SELCHANGE)
					{
						int idx = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_CHARMAP));
						if (idx >= 0)
						{
							cfg_options_freetypecharmap.setValueAsInt(idx - 1);
						}
						return 0;
					}
				}
			}
			break;
		}
	case WM_HSCROLL:
		{
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE))
			{
				int t = (int)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_TTFSCALE, StringPrintf("%d%%", t));
				SetTimer(hwndDlg, 100, 100, NULL);
				return 0;
			}
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SLIDER_SCALEDEFAULTFONT))
			{
				int t = (int)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_SCALEDEFAULTFONT, StringPrintf("%d%%", t));
				SetTimer(hwndDlg, 101, 100, NULL);
				return 0;
			}
			break;
		}
	case WM_TIMER:
		{
			if (wParam == 100)
			{
				KillTimer(hwndDlg, 100);
				int t = (int)SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALETTFOVERRIDE), TBM_GETPOS, 0, 0);
				cfg_options_ttfoverridescale.setValueAsInt(t);
			}
			else if (wParam == 101)
			{
				KillTimer(hwndDlg, 101);
				int t = (int)SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_SCALEDEFAULTFONT), TBM_GETPOS, 0, 0);
				cfg_options_defaultfontscale.setValueAsInt(t);
			}
			return 0;
		}
	case WM_DESTROY:
		subWnd = NULL;
		return 0;
	}

	const int controls[] = 
	{
		IDC_SLIDER_SCALETTFOVERRIDE,
		IDC_SLIDER_SCALEDEFAULTFONT,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}
