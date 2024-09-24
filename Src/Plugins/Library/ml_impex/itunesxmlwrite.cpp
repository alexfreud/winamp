//------------------------------------------------------------------------
//
// iTunes XML Library Writer
// Copyright © 2003-2014 Winamp SA
//
//------------------------------------------------------------------------

#include <windows.h>
#include <commdlg.h>
#include <api/xml/xmlwrite.h>
#include <bfc/string/url.h>
#include "itunesxmlwrite.h"
#include "../plist/types.h"
#include "api__ml_impex.h"
#include "resource.h"

//------------------------------------------------------------------------
iTunesXmlWrite::iTunesXmlWrite() {
}

//------------------------------------------------------------------------
iTunesXmlWrite::~iTunesXmlWrite() {
}

//------------------------------------------------------------------------
int iTunesXmlWrite::pickFile(HWND hwndDlg, const wchar_t *title) 
{
	wchar_t oldCurPath[MAX_PATH] = {0};
	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	OPENFILENAME l={sizeof(l),};
	wchar_t *temp;
	const int len=256*1024-128;

	temp = (wchar_t *)GlobalAlloc(GPTR,len*sizeof(*temp));
	l.hwndOwner = hwndDlg;
	//l.lpstrFilter = L"iTunes XML Library\0*.xml\0\0";		// IDS_ITUNES_XML_LIBRARY
	extern wchar_t* GetFilterListString(void);
	l.lpstrFilter = GetFilterListString();//L"iTunes XML Library\0*.xml\0\0";	// IDS_ITUNES_XML_LIBRARY
	l.lpstrFile = temp;
	l.nMaxFile = len-1;
	l.lpstrTitle = title ? title : WASABI_API_LNGSTRINGW(IDS_EXPORT_DATABASE);
	l.lpstrDefExt = L"xml";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&l)) 
	{
		wchar_t newCurPath[MAX_PATH] = {0};
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		SetCurrentDirectoryW(oldCurPath);
		file = temp;
		return 1;
	}
	SetCurrentDirectoryW(oldCurPath);
	return 0;
}

//------------------------------------------------------------------------
void iTunesXmlWrite::saveXml(plistKey *rootkey) {
	if (file.isempty()) return;
	XMLWrite w(file, L"plist version=\"1.0\"", L"plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"", 1);
	writeData(&w, rootkey->getData());
}

//------------------------------------------------------------------------
void iTunesXmlWrite::writeData(XMLWrite *writer, plistData *data) 
{
	switch (data->getType()) 
	{
	case PLISTDATA_KEY: 
		{
			plistKey *key = (plistKey *)data;
			writer->writeAttrib(data->getTypeString(), data->getString(), key->getData()->getType() == PLISTDATA_DICT || key->getData()->getType() == PLISTDATA_ARRAY || key->getData()->getType() == PLISTDATA_RAW);
			writeData(writer, key->getData());
			break;
		}
	case PLISTDATA_DICT: 
		{
			plistDict *dict = (plistDict *)data;
			writer->pushCategory(data->getTypeString());
			for (int i=0;i<dict->getNumKeys();i++) 
			{
				writeData(writer, dict->enumKey(i));
			}
			writer->popCategory();
			break;
		}
	case PLISTDATA_ARRAY: 
		{
			plistArray *array = (plistArray *)data;
			writer->pushCategory(data->getTypeString());
			for (int i=0;i<array->getNumItems();i++) {
				writeData(writer, array->enumItem(i));
			}
			writer->popCategory();
			break;
		}
	case PLISTDATA_INTEGER: 
	case PLISTDATA_DATE:
	case PLISTDATA_RAW:
		{
			const wchar_t *str = data->getString();
			if (str && *str)
			{
				writer->writeAttrib(data->getTypeString(), str, 1, 0);
			}
		}
		break;
	case PLISTDATA_STRING:
		{
			const wchar_t *str = data->getString();
			if (str && *str)
			{
				// not pretty but it'll strip out control characters
				// in the 0 - 31 range that will cause import issues
				wchar_t *temp = 0;
				int len = (int)wcslen(str) + 1;
				temp = (wchar_t*)calloc(len, sizeof(wchar_t));
				wchar_t *ptr = temp;
				while(str && *str)
				{
					int chr = *str;
					if (chr >= 0 && chr <= 31)
					{
						if(chr == 9 || chr == 10 || chr == 13)
						{
							*ptr = *str;
							ptr++;
						}
					}
					else
					{
						*ptr = *str;
						ptr++;
					}
					str = CharNextW(str);
				}
				*ptr=0;
				writer->writeAttrib(data->getTypeString(), (temp ? temp : str), 1, 0);
				if (temp) free(temp);
			}
		}
		break;
	case PLISTDATA_BOOLEAN:
		{
			//plistBoolean *booldata = (plistBoolean *)data;
			writer->writeAttribEmpty(data->getString(), 1, 0);
		}
		break;
	}
}