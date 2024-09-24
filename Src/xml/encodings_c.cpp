#include "expat/expat.h"
#include <CoreFoundation/CoreFoundation.h>
#include <wchar.h>
// TODO: make this work

struct AppleEncodings
{
	const wchar_t *name;
	CFStringEncoding codePage;
};

static const AppleEncodings encodings[] = 
{
	{L"iso-8859-2", kCFStringEncodingISOLatin2},
	{L"iso-8859-3", kCFStringEncodingISOLatin3},
	{L"iso-8859-4", kCFStringEncodingISOLatin4},
	{L"iso-8859-5", kCFStringEncodingISOLatinCyrillic},
	{L"iso-8859-6", kCFStringEncodingISOLatinArabic},
	{L"iso-8859-7", kCFStringEncodingISOLatinGreek},
	{L"iso-8859-8", kCFStringEncodingISOLatinHebrew},
	{L"iso-8859-9", kCFStringEncodingISOLatin5},
	{L"iso-8859-10", kCFStringEncodingISOLatin6},
	{L"iso-8859-11", kCFStringEncodingISOLatinThai},
	{L"iso-8859-13", kCFStringEncodingISOLatin7},
	{L"iso-8859-14", kCFStringEncodingISOLatin8},
	{L"iso-8859-15", kCFStringEncodingISOLatin9},
	{L"windows-1251", kCFStringEncodingWindowsCyrillic},
	{L"windows-1252", kCFStringEncodingWindowsLatin1},
	{L"windows-1253", kCFStringEncodingWindowsGreek},
	{L"windows-1254", kCFStringEncodingWindowsLatin5},
	{L"windows-1255", kCFStringEncodingWindowsHebrew},
	{L"windows-1256", kCFStringEncodingWindowsArabic},
	{L"windows-1257", kCFStringEncodingWindowsBalticRim},
	{L"windows-1258", kCFStringEncodingWindowsVietnamese},

};

static void MakeMap(int *map, CFStringEncoding encoding)
{
	unsigned char i=0;
	unsigned char mb[2] = {0,0};
	do
	{
		mb[0]=i;
		// if (IsDBLeadByteEx(codepage, i)) map[i]=-2; else {
    CFStringRef cfstr =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)mb, 1, encoding, false);
    if (!cfstr || CFStringGetBytes(cfstr, CFRangeMake(0,1), kCFStringEncodingUTF32, 0, false, (UInt8 *)(map+i), sizeof(*map), 0))
    {
      map[i]=-1;
    }
		// }
	} while (i++ != 255);
}

static bool wcsmatch(const wchar_t *a, const wchar_t *b)
{
  if (!a || !b)
    return false; // wtf
  
  while (*a || *b)
  {
    if (!*a)
      return false;
    if (!*b)
      return false;
    
    if (towlower(*a) != towlower(*b))
      return false;
    a++;
    b++;
  }

  return true;
}
#define NUM_ENCODINGS (sizeof(encodings)/sizeof(AppleEncodings))
int XMLCALL UnknownEncoding(void *data, const XML_Char *name, XML_Encoding *info)
{
	for (int i=0;i<NUM_ENCODINGS;i++)
	{
		if (!wcsmatch(name, encodings[i].name))
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
