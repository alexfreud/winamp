#include "XMLParameters.h"
#include <wchar.h>

#ifdef __APPLE__
#include <Carbon/Carbon.h>
int _wcsicmp(const wchar_t *str1, const wchar_t *str2)
{
  CFStringRef cfstr1 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str1, wcslen(str1)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
  CFStringRef cfstr2 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str2, wcslen(str2)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
  int result = CFStringCompare(cfstr1, cfstr2, kCFCompareCaseInsensitive);
  CFRelease(cfstr1);
  CFRelease(cfstr2);
  return result;
}
#endif

XMLParameters::XMLParameters(const wchar_t **_parameters)
: parameters(_parameters), numParameters(0), numParametersCalculated(false)
{
}

void XMLParameters::CountTo(int x)
{
	if (numParametersCalculated || x < numParameters)
		return;

	while (1)
	{
		if (parameters[numParameters*2] == 0)
		{
			numParametersCalculated=true;
			return;
		}
		numParameters++;
		if (numParameters == x)
			return;
	}
}

void XMLParameters::Count()
{
	if (numParametersCalculated)
		return;

	while (1)
	{
		if (parameters[numParameters*2] == 0)
		{
			numParametersCalculated=true;
			return;
		}
		numParameters++;
	}
}

const wchar_t *XMLParameters::GetItemName(int i)
{
	CountTo(i);
	if (i < numParameters)
		return parameters[i*2];
	else
		return 0;
}

const wchar_t *XMLParameters::GetItemValueIndex(int i)
{
	CountTo(i);
	if (i < numParameters)
		return parameters[i*2+1];
	else
		return 0;
}

int XMLParameters::GetNumItems()
{
	Count();
	return numParameters;
}

const wchar_t *XMLParameters::GetItemValue(const wchar_t *name)
{
	int i=0;
	while(1)
	{
		CountTo(i+1);
		if (i<numParameters)
		{
			if (!_wcsicmp(name, parameters[i*2]))
				return parameters[i*2+1];
		}
		else
			return 0;
		i++;
	};
}

int XMLParameters::GetItemValueInt(const wchar_t *name, int def)
{
	const wchar_t *val = GetItemValue(name);
	if (val && *val)
		return wcstol(val, 0, 10);
	else
		return def;
}

const wchar_t *XMLParameters::EnumItemValues(const wchar_t *name, int nb)
{
	int i=0; 
	while(1)
	{
		CountTo(i+1);
		if (i<numParameters)
		{
			if (!_wcsicmp(name, parameters[i*2]) && nb--)
				return parameters[i*2+1];
		}
		else
			return 0;
		i++;
	};
}

#define CBCLASS XMLParameters
START_DISPATCH;
CB(XMLREADERPARAMS_GETITEMNAME, GetItemName)
CB(XMLREADERPARAMS_GETITEMVALUE, GetItemValueIndex)
CB(XMLREADERPARAMS_GETITEMVALUE2, GetItemValue)
CB(XMLREADERPARAMS_ENUMITEMVALUES, EnumItemValues)
CB(XMLREADERPARAMS_GETNBITEMS, GetNumItems)
END_DISPATCH;
#undef CBCLASS