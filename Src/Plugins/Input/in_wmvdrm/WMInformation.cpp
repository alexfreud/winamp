#include "main.h"
#include "WMInformation.h"
#include "resource.h"
#include <exception>
#include <strsafe.h>

class AutoByte
{
public:
	AutoByte(size_t bytes)
			: data(0)
	{
		data = new BYTE[bytes];
	}
	~AutoByte()
	{
		if (data)
			delete[] data;
		data = 0;
	}
	operator void *()
	{
		return (void *)data;
	}

	BYTE *data;
};

static void StoreData(WMT_ATTR_DATATYPE type, BYTE *value, DWORD length, wchar_t *valueStr, size_t len)
{
	switch (type)
	{
	case WMT_TYPE_DWORD:
		StringCchPrintf(valueStr, len, L"%lu", *(DWORD *)value);
		break;

	case WMT_TYPE_STRING:
		lstrcpyn(valueStr, (wchar_t *)value, len);
		break;

	case -1: // hack  // if (attrName == L"WM/Text")
		StringCchPrintf(valueStr, len, L"%s/%s", UserTextDescription(value, length), UserTextString(value, length));
		break;
	case WMT_TYPE_BINARY:
		BinaryString(value, length, valueStr, len);
		break;
	case WMT_TYPE_BOOL:
		if (*(BOOL *)value)
		{
			lstrcpyn(valueStr, L"True", len);
		}
		else
		{
			lstrcpyn(valueStr, L"False", len);
		}
		break;
	case WMT_TYPE_QWORD:
		StringCchPrintf(valueStr, len, L"%I64u", *(QWORD *)value);
		break;
	case WMT_TYPE_WORD:
		StringCchPrintf(valueStr, len, L"%hu", *(WORD *)value);
		break;
	case WMT_TYPE_GUID:
		GuidString(*(GUID *)value, valueStr, len);
		break;
	default:
		WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,valueStr,len);
		break;
	}

}
WMInformation::WMInformation(const wchar_t *fileName, bool noBlock)
		: editor(0), editor2(0), header(0), header3(0), reader(0), header2(0), openError(false)
{
	if (fileName && fileName[0]
	    && WMCreateEditor(&editor) == S_OK)
	{
		if (SUCCEEDED(editor->QueryInterface(&editor2)))
		{
			if (SUCCEEDED(editor2->OpenEx(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE)))
			{
				// good to go
				editor2->QueryInterface(&header);
				editor->QueryInterface(&header2);
				editor2->QueryInterface(&header3);
				return ;
			}
		}
		else
		{
			editor2 = 0;
			if (SUCCEEDED(editor->Open(fileName)))
			{
				// good to go
				editor->QueryInterface(&header);
				editor->QueryInterface(&header2);
				editor->QueryInterface(&header3);
				return ;
			}
		}
		// can't open it through the metadata editor interface, let's open a reader

		if (editor)
			editor->Release();
		editor = 0;
		if (editor2)
			editor2->Release();
		editor2 = 0;
		if (FAILED(WMCreateReader(0, WMT_RIGHT_PLAYBACK, &reader)))
		{
			reader = 0;
			return ;
		}
		hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		callback >> this;

		if (FAILED(reader->Open(fileName, &callback, 0)))
		{
			reader->Release();
			reader = 0;
			return ;
		}
		if (noBlock)
			WaitForEvent(hEvent, INFINITE);
		else
			WaitForSingleObject(hEvent, INFINITE);

		CloseHandle(hEvent);
		if (openError)
		{
			reader->Release();
			reader = 0;
		}
		else
		{
			reader->QueryInterface(&header);
			reader->QueryInterface(&header2);
			reader->QueryInterface(&header3);
		}

	}
}

WMInformation::WMInformation(IWMReader *_reader)
		: reader(0),        // reader is if we create an internal reader, we don't want to save the passed one (so we don't close it on someone else :)
		editor(0), editor2(0), header(0),
		header3(0), header2(0),
		openError(false), hEvent(NULL)
{
	if (FAILED(_reader->QueryInterface(&header)))
		header = 0;
	if (FAILED(_reader->QueryInterface(&header2)))
		header2 = 0;
	if (FAILED(_reader->QueryInterface(&header3)))
		header3 = 0; 		// this error is OK, we can deal with it.
}

/*
WMInformation::WMInformation(IWMSyncReader  *reader)
: editor(0), editor2(0), header(0), header3(0)
{
reader->QueryInterface(&header);
reader->QueryInterface(&header3);
}*/

WMInformation::WMInformation(IWMMetadataEditor *_editor)
		: editor(_editor), editor2(0), header(0),
		  header3(0), reader(0), header2(0),
		  openError(false), hEvent(NULL)
{
	editor->AddRef();
	editor->QueryInterface(&editor2);
	editor->QueryInterface(&header);
	editor->QueryInterface(&header2);
	editor->QueryInterface(&header3);
}


WMInformation::~WMInformation()
{
	if (editor)
	{
		editor->Close();
		editor->Release();
		editor = 0;
	}
	if (editor2)
		editor2->Release();
	editor2 = 0;
	if (header)
		header->Release();
	header = 0;
	if (header2)
		header2->Release();
	header2 = 0;
	if (header3)
		header3->Release();
	header3 = 0;
	if (reader)
	{
		reader->Close();
		reader->Release();
		reader = 0;
	}
}

bool WMInformation::GetDataType(const wchar_t *name, WMT_ATTR_DATATYPE &type)
{
	if (!name)
		return false;

	WORD stream = 0;
	WORD dataLen = 0;
	if (header && SUCCEEDED(header->GetAttributeByName(&stream, name, &type, 0, &dataLen)))
		return true;
	else
		return false;

}

void WMInformation::DeleteAttribute(const wchar_t *attrName)
{

	WORD indexCount = 0;
	if (header3 && SUCCEEDED(header3->GetAttributeIndices(0, attrName, NULL, 0, &indexCount)))
	{
		WORD *indices = new WORD[indexCount];
		if (SUCCEEDED(header3->GetAttributeIndices(0, attrName, NULL, indices, &indexCount)))
		{
			for (size_t i = 0;i != indexCount;i++)
			{
				header3->DeleteAttribute(0, indices[i]);
			}
		}
	}
}


void WMInformation::SetAttribute_BinString(const wchar_t *attrName, wchar_t *value)
{
	if (!header || !attrName || !value)
		return ;

	if (!*value)
	{
		DeleteAttribute(attrName);
		return ;
	}

	AutoChar data(value);
	header->SetAttribute(0, attrName, WMT_TYPE_BINARY, (BYTE *)(char *)data, (WORD)strlen(data));
}

void WMInformation::GetAttribute_BinString(const wchar_t attrName[], wchar_t *valueStr, size_t len)
{
	if (!header)
	{
		valueStr[0]=0;
		return ;
	}

	WMT_ATTR_DATATYPE type;
	WORD length = 0;
	HRESULT hr;
	WORD streamNum = 0;
	if (!header || FAILED(header->GetAttributeByName(&streamNum,
	                      attrName,
	                      &type,
	                      0,
	                      &length)))
	{
		valueStr[0]=0;
		return ;
	}
	AutoByte v(length);
	BYTE *value = v.data;

	hr = header->GetAttributeByName(&streamNum,
	                                attrName,
	                                &type,
	                                value,
	                                &length);
	if (FAILED(hr))
	{
		valueStr[0]=0;
		return ;
	}


	int converted = MultiByteToWideChar(CP_ACP, 0, (const char *)value, length, valueStr, len-1);
	valueStr[converted]=0;
}

void WMInformation::SetAttribute(const wchar_t *attrName, wchar_t *value, WMT_ATTR_DATATYPE defaultType)
{
	if (!header || !attrName || !value)
		return ;

	if (!*value)
	{
		DeleteAttribute(attrName);
		return ;
	}

	WMT_ATTR_DATATYPE type;

	if (!GetDataType(attrName, type))
		type = defaultType;

	switch (type)
	{
	case WMT_TYPE_DWORD:
	{
		DWORD dwordValue = wcstoul(value, 0, 10);
		header->SetAttribute(0, attrName, WMT_TYPE_DWORD, (BYTE *) &dwordValue, sizeof(dwordValue));
	}
	break;
	case WMT_TYPE_STRING:
	{
		WORD size = static_cast<WORD>((lstrlen(value) + 1) * sizeof(wchar_t));
		header->SetAttribute(0, attrName, WMT_TYPE_STRING, (BYTE *)value, size);
	}
	break;
	case WMT_TYPE_BINARY:
	{
		// TODO
	}
	break;
	case WMT_TYPE_BOOL:
	{
		BOOL boolValue;
		if (!_wcsicmp(L"true", value))
			boolValue = TRUE;
		else
			boolValue = FALSE;

		header->SetAttribute(0, attrName, WMT_TYPE_BOOL, (BYTE *)&boolValue, sizeof(boolValue));
	}
	break;
	case WMT_TYPE_QWORD:
	{
		{
			QWORD qwordValue = _wcstoui64(value, 0, 10);
			header->SetAttribute(0, attrName, WMT_TYPE_QWORD, (BYTE *) &qwordValue, sizeof(qwordValue));
		}
	}
	break;
	case WMT_TYPE_WORD:
	{
		{
			WORD wordValue = static_cast<WORD>(wcstoul(value, 0, 10));
			header->SetAttribute(0, attrName, WMT_TYPE_WORD, (BYTE *) &wordValue, sizeof(wordValue));
		}
	}
	break;
	case WMT_TYPE_GUID:
	{
		GUID guidValue = StringGUID(value);
		header->SetAttribute(0, attrName, WMT_TYPE_GUID, (BYTE *) &guidValue, sizeof(guidValue));
	}
	break;
	}

}

bool WMInformation::GetAttributeSize(const wchar_t *name, size_t &size)
{
	WORD stream = 0;
	WORD resultSize;
	WMT_ATTR_DATATYPE type;

	if (!header || FAILED(header->GetAttributeByName(&stream, name, &type, 0, &resultSize)))
	{
		return false;
	}
	size = resultSize;
	return true;
}

DWORD WMInformation::GetDWORDAttr(const wchar_t name[])
{
	WORD stream = 0;
	DWORD result;

	WORD resultSizeWord = sizeof(result);
	DWORD resultSize = sizeof(result);
	WMT_ATTR_DATATYPE type = WMT_TYPE_DWORD;
	WORD count = 1;
	WORD indices[1] = {0};
	if ((!header3
	     || FAILED(header3->GetAttributeIndices(0, name, NULL, indices, &count))
	     || FAILED(header3->GetAttributeByIndexEx(0, indices[0], 0, 0, &type, NULL, (BYTE *) &result, &resultSize)))
	    &&
	    (!header || FAILED(header->GetAttributeByName(&stream, name, &type, (BYTE *)&result, &resultSizeWord))))
		return 0;
	else
		return result;

}


long WMInformation::GetLongAttr(const wchar_t name[])
{
	WORD stream = 0;
	long result;
	WORD resultSize = sizeof(result);
	WMT_ATTR_DATATYPE type;

	if (!header || FAILED(header->GetAttributeByName(&stream, name, &type, (BYTE *)&result, &resultSize)))
	{
		return 0;
	}

	return result;
}

bool WMInformation::GetBoolAttr(const wchar_t name[])
{
	WORD stream = 0;
	BOOL result;
	WORD resultSize = sizeof(result);
	WMT_ATTR_DATATYPE type;

	if (!header || FAILED(header->GetAttributeByName(&stream, name, &type, (BYTE *)&result, &resultSize)))
	{
		return false;
	}

	return !!result;
}

bool WMInformation::IsSeekable()
{
	return GetBoolAttr(g_wszWMSeekable);
}

long WMInformation::GetLengthMilliseconds()
{
	WORD stream = 0;
	long long duration = 0;
	WORD resultSize = sizeof(duration);
	WMT_ATTR_DATATYPE type;

	if (!header || FAILED(header->GetAttributeByName(&stream, g_wszWMDuration, &type, (BYTE *)&duration, &resultSize)))
	{
		return -1000;
	}

	duration /= 10000LL;
	return (long)duration;
}

long WMInformation::GetBitrate()
{
	return GetDWORDAttr(g_wszWMCurrentBitrate);
}

WORD WMInformation::GetNumberAttributes()
{
	WORD numAttr = 0;
	if ((!header3 || FAILED(header3->GetAttributeCountEx(0, &numAttr)))
	    && (!header || FAILED(header->GetAttributeCount(0, &numAttr))))
		return 0;
	else
		return numAttr;
}



void WMInformation::GetAttribute(WORD index, wchar_t *attrName, size_t attrLen, wchar_t *valueStr, size_t valueStrLen)
{
	wchar_t _attrName[1025] = {0};
	WORD nameLen = sizeof(_attrName) / sizeof(_attrName[0]);
	WMT_ATTR_DATATYPE type;
	WORD lang;
	WORD stream = 0;
	DWORD length = 0;
	WORD lengthWord = 0;

	if ((!header3	|| FAILED(header3->GetAttributeByIndexEx(0, index, _attrName, &nameLen, &type, &lang, 0, &length)))
	    && (!header || FAILED(header->GetAttributeByIndex(index, &stream, _attrName, &nameLen, &type, 0, &lengthWord))))
	{
		attrName[0]=0;
		valueStr[0]=0;
		return ;
	}
	if (lengthWord)
		length = lengthWord;

	AutoByte v(length);
	BYTE *value = v.data;

	lstrcpyn(attrName, _attrName, attrLen);
	if ((!header3 || FAILED(header3->GetAttributeByIndexEx(0, index, _attrName, &nameLen, &type, &lang, value, &length)))
	    && (!header || FAILED(header->GetAttributeByIndex(index, &stream, _attrName, &nameLen, &type, value, &lengthWord))))
	{
		attrName[0]=0;
		valueStr[0]=0;

		return ;
	}

	if (attrName == L"WM/Text")
	{
		type = (WMT_ATTR_DATATYPE)-1; // hack
		StringCchCat(attrName, attrLen, L":");
		StringCchCat(attrName, attrLen, UserTextDescription(value, length));
	}
	StoreData(type, value, length, valueStr, valueStrLen);
}

void WMInformation::GetAttribute(const wchar_t attrName[], wchar_t *valueStr, size_t len)
{
	if (!header)
	{
		valueStr[0]=0;
		return ;
	}

	WMT_ATTR_DATATYPE type;
	WORD length = 0;
	HRESULT hr;
	WORD streamNum = 0;
	if (!header || FAILED(header->GetAttributeByName(&streamNum,
	                      attrName,
	                      &type,
	                      0,
	                      &length)))
	{
		valueStr[0]=0;
		return ;
	}
	AutoByte v(length);
	BYTE *value = v.data;

	hr = header->GetAttributeByName(&streamNum,
	                                attrName,
	                                &type,
	                                value,
	                                &length);
	if (FAILED(hr))
	{
		valueStr[0]=0;
		return ;
	}

	if (attrName == L"WM/Text")
		type = (WMT_ATTR_DATATYPE)-1; // hack
	StoreData(type, value, length, valueStr, len);
}


bool WMInformation::MakeWritable(const wchar_t *fileName)
{
	if (!editor || !editor2)
		return false;

	if (FAILED(editor2->OpenEx(fileName, GENERIC_READ | GENERIC_WRITE, 0)))
	{
		return false;
	}
	return true;
}

bool WMInformation::Flush()
{
	if (!editor2 || FAILED(editor->Flush()))
		return false;

	return true;
}

bool WMInformation::IsAttribute(const wchar_t attrName[])
{
	WMT_ATTR_DATATYPE type = WMT_TYPE_BOOL;
	WORD length = sizeof(BOOL);
	WORD streamNum = 0;
	BOOL value;
	if (!header || FAILED(header->GetAttributeByName(&streamNum,
	                      attrName,
	                      &type,
	                      (BYTE *)&value,
	                      &length)))
	{
		return false;
	}
	else
	{
		return !!value;
	}
}

bool WMInformation::IsNotAttribute(const wchar_t attrName[])
{
	if (!header)
	{
		return false;
	}

	WMT_ATTR_DATATYPE type = WMT_TYPE_BOOL;
	WORD length = sizeof(BOOL);
	WORD streamNum = 0;
	BOOL value;
	if (!header || FAILED(header->GetAttributeByName(&streamNum,
	                      attrName,
	                      &type,
	                      (BYTE *)&value,
	                      &length)))
	{
		return false;
	}
	else
	{
		return !value;
	}
}

bool WMInformation::MakeReadOnly(const wchar_t *fileName)
{
	if (!editor || !editor2)
		return false;

	//editor->Close();
	if (FAILED(editor2->OpenEx(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE)))
	{
		return false;
	}
	return true;
}


bool WMInformation::NonWritable()
{
	if (!editor2)
		return true;
	else
		return false;
}

void WMInformation::DeleteUserText(const wchar_t *description)
{
	WORD indexCount = 0;
	WMT_ATTR_DATATYPE type = WMT_TYPE_BOOL;
	WORD nameLen = 128;
	if (header3 && SUCCEEDED(header3->GetAttributeIndices(0, L"WM/Text", NULL, 0, &indexCount)))
	{
		WORD *indices = new WORD[indexCount];
		if (SUCCEEDED(header3->GetAttributeIndices(0, L"WM/Text", NULL, indices, &indexCount)))
		{
			for (size_t index = 0;index != indexCount;index++)
			{
				WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
				WORD lang = 0;
				DWORD length = 0;
				wchar_t _attrName[128] = L"WM/Text";
				if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, 0, &length)))
				{
					AutoByte v(length);
					BYTE *value = v.data;

					if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, value, &length)))
					{
						if (UserTextDescription(value, length) == description)
						{
							header3->DeleteAttribute(0, indices[index]);
						}
					}
				}
			}
		}
	}
}

void WMInformation::SetUserText(const wchar_t *description, const wchar_t *valueStr)
{
	if (!header3 || !description || !valueStr)
		return;

	WM_USER_TEXT userText;
	userText.pwszDescription = (LPWSTR)description;
	userText.pwszText = (LPWSTR) valueStr;

	WORD index;
	header3->AddAttribute(0, L"WM/Text", &index, WMT_TYPE_BINARY, 0, (BYTE *) &userText, sizeof(userText));
}

void WMInformation::ClearAllAttributes()
{
	WORD numAttrs;
	header3->GetAttributeCountEx(0xFFFF, &numAttrs);
	while (numAttrs--)
	{
		header3->DeleteAttribute(0xFFFF, numAttrs);
	}
}

bool WMInformation::GetCodecName(wchar_t *storage, size_t len)
{
	if (!header2)
		return false;

	DWORD codecs=0;
	header2->GetCodecInfoCount(&codecs);
	for (DWORD i=0;i!=codecs;i++)
	{
		WORD nameLen=0, descriptionLen=0, infoLen = 0;
		WMT_CODEC_INFO_TYPE type;
		header2->GetCodecInfo(i, &nameLen, 0, &descriptionLen, 0, &type, &infoLen, 0);
		if (type == WMT_CODECINFO_AUDIO)
		{
			wchar_t *name = new wchar_t[nameLen];
			wchar_t *description = new wchar_t[descriptionLen];
			BYTE *info = new BYTE[infoLen];
			header2->GetCodecInfo(i, &nameLen, name, &descriptionLen, description, &type, &infoLen, info);
			lstrcpynW(storage, name, len);
			delete[] name;
			delete[]description;
			delete[] info;

			return true;
		}
	}
	return false;
}

bool WMInformation::GetPicture(void **data, size_t *len, wchar_t **mimeType, int pictype)
{
	WORD indexCount = 0;
	WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
	WORD nameLen = 128;
	if (header3 && SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, 0, &indexCount)))
	{
		WORD *indices = new WORD[indexCount];
		if (SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, indices, &indexCount)))
		{
			for (size_t index = 0;index != indexCount;index++)
			{
				WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
				WORD lang = 0;
				DWORD length = 0;
				wchar_t _attrName[128] = {0};
				if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, 0, &length)))
				{
					AutoByte v(length);
					BYTE *value = v.data;

					if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, value, &length)))
					{
						WM_PICTURE *picture = (WM_PICTURE *)value;
						if (picture->bPictureType == pictype)
						{
							*len = picture->dwDataLen;
							*data = WASABI_API_MEMMGR->sysMalloc(*len);
							memcpy(*data, picture->pbData, *len);
							wchar_t *type=0;
							if (picture->pwszMIMEType)
								type = wcschr(picture->pwszMIMEType, L'/');

							if (type && *type)
							{
								type++;

								wchar_t *type2 = wcschr(type, L'/');
								if (type2 && *type2) type2++;
								else type2 = type;

								size_t mimelen = wcslen(type2)+1;
								*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(mimelen*sizeof(wchar_t));
								StringCchCopyW(*mimeType, mimelen, type2);
							}
							else
								*mimeType = 0; // unknown!
							delete[] indices;
							return true;
						}
					}
				}
			}
		}
		delete[] indices;
	}
	return false;
}

bool WMInformation::SetPicture(void *data, size_t len, const wchar_t *mimeType, int type)
{
	WM_PICTURE picture;
	picture.bPictureType = type;
	picture.dwDataLen = len;
	picture.pbData = (BYTE *)data;
	picture.pwszDescription=L"";
	wchar_t mt[32] = {0};
	if (wcsstr(mimeType, L"/") != 0)
	{
		StringCchCopyW(mt, 32, mimeType);
	}
	else
	{
		StringCchPrintfW(mt, 32, L"image/%s", mimeType);
	}
	picture.pwszMIMEType = mt;
	return SUCCEEDED(header->SetAttribute(0, g_wszWMPicture, WMT_TYPE_BINARY, (const BYTE *)&picture, sizeof(picture)));
}

bool WMInformation::HasPicture(int pictype)
{
	WORD indexCount = 0;
	WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
	WORD nameLen = 128;
	if (header3 && SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, 0, &indexCount)))
	{
		WORD *indices = new WORD[indexCount];
		if (SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, indices, &indexCount)))
		{
			for (size_t index = 0;index != indexCount;index++)
			{
				WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
				WORD lang = 0;
				DWORD length = 0;
				wchar_t _attrName[128] = {0};
				if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, 0, &length)))
				{
					AutoByte v(length);
					BYTE *value = v.data;

					if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, value, &length)))
					{
						WM_PICTURE *picture = (WM_PICTURE *)value;
						if (picture->bPictureType == pictype)
						{
							delete[] indices;
							return true;
						}
					}
				}
			}
		}
		delete[] indices;
	}
	return false;
}

bool WMInformation::DeletePicture(int pictype)
{
	WORD indexCount = 0;
	WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
	WORD nameLen = 128;
	if (header3 && SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, 0, &indexCount)))
	{
		WORD *indices = new WORD[indexCount];
		if (SUCCEEDED(header3->GetAttributeIndices(0, g_wszWMPicture, NULL, indices, &indexCount)))
		{
			for (size_t index = 0;index != indexCount;index++)
			{
				WMT_ATTR_DATATYPE type = WMT_TYPE_BINARY;
				WORD lang = 0;
				DWORD length = 0;
				wchar_t _attrName[128] = {0};
				if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, 0, &length)))
				{
					AutoByte v(length);
					BYTE *value = v.data;

					if (SUCCEEDED(header3->GetAttributeByIndexEx(0, indices[index], _attrName, &nameLen, &type, &lang, value, &length)))
					{
						WM_PICTURE *picture = (WM_PICTURE *)value;
						if (picture->bPictureType == pictype)
						{
							header3->DeleteAttribute(0, indices[index]);
							delete[] indices;
							return true;
						}
					}
				}
			}
		}
		delete[] indices;
	}
	return false;
}