#include "AMFObject.h"
#include <strsafe.h>
void AMFMixedArray::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	AMFTypeList::iterator itr;
	StringCchCopyEx(str, len, L"Mixed Array [\n", &str, &len, 0);
	for (itr=array.begin();itr!=array.end();itr++)
	{
		for (int i=0;i<spaces;i++)
			StringCchCopyEx(str, len, L" ", &str, &len,0);

		if (itr->first.c_str() != 0 && itr->first.c_str()[0])
			StringCchPrintfEx(str, len, &str, &len, 0, L"%s: ", itr->first.c_str());
		if (itr->second)
			itr->second->DebugPrint(spaces+1, str, len);
		else
			StringCchCopyEx(str, len, L"(null)\n", &str, &len, 0);
	}
	StringCchCopyEx(str, len, L"]\n", &str, &len, 0);

}

size_t AMFMixedArray::Read(uint8_t *data, size_t size)
{
	size_t read = 0;
	uint32_t maxIndex = FLV::Read32(data);
// TODO?	array.reserve(maxIndex);
	data += 4;
	size -= 4;
	read += 4;

	while (size)
	{
		AMFString amfString;
		size_t skip = amfString.Read(data, size);
		data += skip;
		size -= skip;
		read += skip;

		uint8_t type = *data;
		data++;
		size--;
		read++;
		AMFType *obj = MakeObject(type);
		if (obj)
		{
			obj->type = type;
			size_t skip = obj->Read(data, size);
			data += skip;
			size -= skip;
			read += skip;

			array[amfString.str] = obj;
		}
		else
			break;

		if (type == TYPE_TERMINATOR)
			break;

	}

	return read;
}

AMFMixedArray::~AMFMixedArray()
{
	for (AMFTypeList::iterator itr=array.begin();itr!=array.end();itr++)
	{
		delete itr->second;
	}
}


void AMFObj::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchCopyEx(str, len, L"Object (TODO)\n", &str, &len, 0);
}

size_t AMFObj::Read(uint8_t *data, size_t size)
{
	size_t read = 0;
	while (size)
	{
		AMFString amfString;
		size_t skip = amfString.Read(data, size);
		data += skip;
		size -= skip;
		read += skip;

		uint8_t type = *data;
		data++;
		size--;
		read++;
		AMFType *obj = MakeObject(type);
		if (obj)
		{
			obj->type = type;
			size_t skip = obj->Read(data, size);
			data += skip;
			size -= skip;
			read += skip;
		}
		else
			return false;

		if (type == TYPE_TERMINATOR)
			break;
	}
	return read;
}


void AMFArray::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{

	StringCchCopyEx(str, len, L"Array [\n", &str, &len, 0);
	for (size_t i=0;i!=array.size();i++)
	{
		for (int s=0;s<spaces;s++)
			StringCchCopyEx(str, len, L" ", &str, &len,0);
		StringCchPrintfEx(str, len, &str, &len, 0, L"%u: ", i);
		array[i]->DebugPrint(spaces+1, str, len);
	}
	StringCchCopyEx(str, len, L"]\n", &str, &len, 0);
}

size_t AMFArray::Read(uint8_t *data, size_t size)
{
	size_t read = 0;
	uint32_t arrayLength = FLV::Read32(data);
	array.reserve(arrayLength);
	data += 4;
	read += 4;
	size -= 4;

	for (uint32_t i=0;i!=arrayLength;i++)
	{
		uint8_t type = *data;
		data++;
		read++;
		size--;

		AMFType *obj = MakeObject(type);
		size_t skip = obj->Read(data, size);
		//array[i]=obj;
		array.push_back(obj);
		data += skip;
		read += skip;
		size -= skip;
	}

	return read;
}

AMFArray::~AMFArray()
{
	for (size_t i=0;i!=array.size();i++)
	{
		delete array[i];
	}
}

/* --- String --- */
AMFString::AMFString() : str(0)
{}
AMFString::~AMFString()
{
	free(str);
}

void AMFString::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchPrintfEx(str, len, &str, &len, 0, L"%s\n", this->str);
}

size_t AMFString::Read(uint8_t *data, size_t size)
{
	if (size < 2)
		return 0;

	unsigned __int16 strlength = FLV::Read16(data);
	data += 2;
	size -= 2;

	if (strlength > size)
		return 0;

	char *utf8string = (char *)calloc(strlength, sizeof(char));
	memcpy(utf8string, data, strlength);

	int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8string, strlength, 0, 0);
	str = (wchar_t *)calloc(wideLen + 2, sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, utf8string, strlength, str, wideLen);
	str[wideLen] = 0;
	free(utf8string);

	return strlength + 2;
}

/* --- Long String --- */
AMFLongString::AMFLongString() : str(0)
{}
AMFLongString::~AMFLongString()
{
	free(str);
}

void AMFLongString::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchPrintfEx(str, len, &str, &len, 0, L"%s\n", this->str);
}

size_t AMFLongString::Read(uint8_t *data, size_t size)
{
	if (size < 4)
		return 0;

	uint32_t strlength = FLV::Read32(data);
	data += 4;
	size -= 4;

	if (strlength > size)
		return 0;

	char *utf8string = (char *)calloc(strlength, sizeof(char));
	memcpy(utf8string, data, strlength);

	int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8string, strlength, 0, 0);
	str = (wchar_t *)calloc(wideLen + 2, sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, utf8string, strlength, str, wideLen);
	str[wideLen] = 0;
	free(utf8string);

	return strlength + 4;
}

/* --- Double --- */
void AMFDouble::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchPrintfEx(str, len, &str, &len, 0, L"%f\n", val);
}

/* --- Boolean --- */
void AMFBoolean::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchPrintfEx(str, len, &str, &len, 0, L"%s\n", boolean?L"true":L"false");
}

/* --- Time --- */
static size_t MakeDateString(__time64_t convertTime, wchar_t *dest, size_t destlen)
{
	SYSTEMTIME sysTime;
	tm *newtime = _localtime64(&convertTime);
	dest[0] = 0; // so we can bail out easily
	if (newtime)
	{
		sysTime.wYear	= (WORD)(newtime->tm_year + 1900);
		sysTime.wMonth	= (WORD)(newtime->tm_mon + 1);
		sysTime.wDayOfWeek = (WORD)newtime->tm_wday;
		sysTime.wDay		= (WORD)newtime->tm_mday;
		sysTime.wHour	= (WORD)newtime->tm_hour;
		sysTime.wMinute	= (WORD)newtime->tm_min;
		sysTime.wSecond	= (WORD)newtime->tm_sec;
		sysTime.wMilliseconds = 0;

		int charsWritten = GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, dest, (int)destlen);
		if (charsWritten)
		{
			size_t dateSize = charsWritten-1;
			dest += dateSize;
			destlen -= dateSize;
			if (destlen)
			{
				*dest++ = L' ';
				destlen--;
				dateSize++;
			}

			int charsWritten2 = GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, dest, (int)destlen);
			if (charsWritten2)
			{
				dateSize+=(charsWritten2-1);
			}
			return dateSize;
		}
	}

	return 1;

}

void AMFTime::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	size_t written = MakeDateString((__time64_t)val, str, len);
	str+=written;
	len-=written;
	if (len>=2)
	{
		str[0]='\n';
		str[1]=0;
		len--;
		str++;
	}
}

/* --- Terminator --- */
void AMFTerminator::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchCopyEx(str, len, L"array terminator\n", &str, &len, 0);
}

/* --- Reference --- */
void AMFReference::DebugPrint(int spaces, wchar_t *&str, size_t &len)
{
	StringCchPrintfEx(str, len, &str, &len, 0, L"%u\n", val);
}
