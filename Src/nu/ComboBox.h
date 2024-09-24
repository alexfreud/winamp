#ifndef NULLSOFT_UTILITY_COMBOBOX_H
#define NULLSOFT_UTILITY_COMBOBOX_H
#include <windows.h>
class ComboBox
{
public:
	ComboBox(HWND hwndDlg, int id)
	{
		cbHwnd = GetDlgItem(hwndDlg, id);
	}

	ComboBox(HWND control)
	{
		cbHwnd = control;
	}

	operator HWND()
	{
		return cbHwnd;
	}

	LRESULT AddString(const wchar_t *string)
	{
		return SendMessageW(cbHwnd, CB_ADDSTRING, 0, (LPARAM)string);
	}

	LRESULT AddString(const wchar_t *string, LPARAM data)
	{
		LRESULT index = SendMessageW(cbHwnd, CB_ADDSTRING, 0, (LPARAM)string);
		SendMessage(cbHwnd, CB_SETITEMDATA, index, data);
		return index;
	}

	LRESULT AddString(const char *string)
	{
		return SendMessageA(cbHwnd, CB_ADDSTRING, 0, (LPARAM)string);
	}

	void SetItemData(LPARAM index, LPARAM data)
	{
		SendMessage(cbHwnd, CB_SETITEMDATA, index, data);
	}

	int GetCount()
	{
		return (int)SendMessage(cbHwnd, CB_GETCOUNT, 0, 0);
	}

	LRESULT GetItemData(LPARAM index)
	{
		return SendMessage(cbHwnd, CB_GETITEMDATA, index, 0);
	}

	void Clear()
	{
		SendMessage(cbHwnd, CB_RESETCONTENT, 0, 0);
	}

	void SelectItem(LPARAM index)
	{
		SendMessage(cbHwnd, CB_SETCURSEL, index, 0);
	}

	LPARAM GetSelection()
	{
		return SendMessage(cbHwnd, CB_GETCURSEL, 0, 0);
	}

	LRESULT SelectString(const wchar_t *str)
	{
		return SendMessageW(cbHwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)str);
	}

	LRESULT GetTextLen(int index)
	{
		return SendMessageW(cbHwnd, CB_GETLBTEXTLEN, (WPARAM)index, 0);
	}

	void GetText(int index, wchar_t *str)
	{
		SendMessageW(cbHwnd, CB_GETLBTEXT, (WPARAM)index, (LPARAM)str);
	}
#if (_WIN32_WINNT >= 0x0501)
	void SetCueBanner(const wchar_t *str)
	{
		SendMessageW(cbHwnd, CB_SETCUEBANNER, 0, (LPARAM)str);
		//CB_SETCUEBANNER;
	}
#endif

	void SetEditText(const wchar_t *str)
	{
		SendMessageW(cbHwnd, WM_SETTEXT, 0, (LPARAM)str);
	}

	unsigned int GetEditText(wchar_t *str, unsigned int cch)
	{
		return (unsigned int)SendMessageW(cbHwnd, WM_GETTEXT, (WPARAM)cch, (LPARAM)str);
	}


	HWND cbHwnd;
};

#endif