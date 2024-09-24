#include "expat.h"
#include <wchar.h>
#include <windows.h>
#include "../WAT/WAT.h"

struct WindowsEncodings
{
	const wchar_t *name;
	UINT codePage;
};

static const WindowsEncodings encodings[] = 
{
	{L"iso-8859-2", 28592},
	{L"iso-8859-3", 28593},
	{L"iso-8859-4", 28594},
	{L"iso-8859-5", 28595},
	{L"iso-8859-6", 28596},
	{L"iso-8859-7", 28597},
	{L"iso-8859-8", 28598},
	{L"iso-8859-9", 28599},
	{L"iso-8859-15", 28605},
	{L"windows-1251", 1251},
	{L"windows-1252", 1252},
	{L"windows-1253", 1253},
	{L"windows-1254", 1254},
	{L"windows-1255", 1255},
	{L"windows-1256", 1256},
	{L"windows-1257", 1257},
	{L"windows-1258", 1258},

};

void MakeMap(int *map, UINT codepage)
{
	unsigned char i=0;
	unsigned char mb[2] = {0,0};
	wchar_t temp[2] = {0};
	do
	{
		mb[0]=i;
		// if (IsDBLeadByteEx(codepage, i)) map[i]=-2; else {
		int len = MultiByteToWideChar(codepage, 0, (char *)mb,  2, 0, 0);
		switch(len)
		{
		case 0:
			map[i]=-1;
			break;
		case 2:
			{
			  MultiByteToWideChar(codepage, 0, (char *)mb, 2,  temp, 2);
				map[i]=temp[0];
		  }
			break;
		}
		// }
	} while (i++ != 255);
}

#define NUM_ENCODINGS (sizeof(encodings)/sizeof(WindowsEncodings))
int XMLCALL UnknownEncoding(void *data, const wchar_t *name, XML_Encoding *info)
{
	for (int i=0;i<NUM_ENCODINGS;i++)
	{
		if (!_wcsicmp(name, encodings[i].name))
		{
			MakeMap(info->map, encodings[i].codePage);
			info->data = 0;
			info->convert = 0;
			info->release = 0;
			return XML_STATUS_OK;
		}
	}
	return XML_STATUS_ERROR;
}

int XMLCALL UnknownEncoding(void* data, const char* name, XML_Encoding* info)
{
	auto wszName = wa::strings::wa_string(name).GetW().c_str();
	return UnknownEncoding(data, wszName, info);
}