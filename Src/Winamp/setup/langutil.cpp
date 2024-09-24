#include "main.h"
#include "./langutil.h"



INT_PTR WADialogBoxParam(LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	INT_PTR ret(0);
	HINSTANCE hInst = (language_pack_instance) ? language_pack_instance : hMainInstance;
	while(hInst)
	{
		ret = DialogBoxParamW(hInst, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		if (-1 == ret && hInst != hMainInstance) hInst = hMainInstance;
		else break;
	}
	return ret;
}

HWND WACreateDialogParam(LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	HWND ret(NULL);
	HINSTANCE hInst = (language_pack_instance) ? language_pack_instance : hMainInstance;
	while(hInst)
	{
		ret = CreateDialogParamW(hInst, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		if (NULL == ret && hInst != hMainInstance) hInst = hMainInstance;
		else break;
	}
	return ret;
}

HBITMAP WALoadImage2(LPCWSTR pszSectionName, LPCWSTR lpImageName, BOOL bPremult)
{
	HBITMAP ret(NULL);
	HINSTANCE hInst = (language_pack_instance) ? language_pack_instance : hMainInstance;
	while(hInst)
	{
		ret = WALoadImage(hInst, pszSectionName, lpImageName, bPremult);
		if (NULL == ret && hInst != hMainInstance) hInst = hMainInstance;
		else break;
	}
	return ret;
}