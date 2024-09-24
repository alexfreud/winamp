#include "precomp_wasabi_bfc.h"
#include "url.h"

#include <bfc/wasabi_std.h>

void Url::encode(StringW &dest, int use_plus_for_space, int encoding, int style)
{
	if (dest.isempty()) return;
	StringW srcstr = dest;
	const wchar_t *src = srcstr;
	dest = NULL;
/*
	 if (encoding & URLENCODE_EXCLUDEHTTPPREFIX)
	   if (!_wcsnicmp(src, L"http://", 7)) 
		{
	     src += 7;
			dest += L"http://";
	   }
*/
	while (src && *src)
	{
		int encode = 1;

//    if (encoding & URLENCODE_NOTHING) encode = 0;

		if ((encoding & URLENCODE_EXCLUDEALPHANUM)
		    && (ISALPHA(*src) || ISDIGIT(*src) 
				|| *src == '_' || *src == '-' || *src == '.' || *src == '~')
		    && *src < 128)
			encode = 0;

//    if ((encoding & URLENCODE_EXCLUDE_8BIT) && (*src > 127)) encode = 0;
		if ((encoding & URLENCODE_EXCLUDE_ABOVEEQ32) && (*src >= 32)) encode = 0;
//    if ((encoding & URLENCODE_ENCODESPACE) && *src == ' ') encode = 1;
//    if ((encoding & URLENCODE_ENCODEXML) && (*src == '<' || *src == '>' || *src == '&')) encode = 1;
		if ((*src == '&' && (style == URLENCODE_STYLE_ANDPOUND || style == URLENCODE_STYLE_ANDPOUNDX)) ||
		    (*src == '%' && style == URLENCODE_STYLE_PERCENT)) encode = 1;
		if ((encoding & URLENCODE_EXCLUDESLASH) && (*src == '/' || *src == ':')) encode = 0;
		if (!encode)
		{
			dest += *src;
		}
		else if (use_plus_for_space && *src == ' ')
		{
			dest += '+';
		}
		else
		{
			switch (style)
			{
				case URLENCODE_STYLE_PERCENT:
					dest += StringPrintfW(L"%%%02X", (int) * src);
					break;
				case URLENCODE_STYLE_ANDPOUND:
					dest += StringPrintfW(L"&#%02d;", (int) * src);
					break;
				case URLENCODE_STYLE_ANDPOUNDX:
					dest += StringPrintfW(L"&#x%02X;", (int) * src);
					break;
			}
		}
		src++;
	}
}

void Url::encode(String &dest, int use_plus_for_space, int encoding, int style)
{
	if (dest.isempty()) return;
	String srcstr = dest;
	const char *src = srcstr;
	dest = NULL;
/*
	 if (encoding & URLENCODE_EXCLUDEHTTPPREFIX)
	   if (!_wcsnicmp(src, L"http://", 7)) 
		{
	     src += 7;
			dest += L"http://";
	   }
*/
	while (src && *src)
	{
		int encode = 1;

//    if (encoding & URLENCODE_NOTHING) encode = 0;

		if ((encoding & URLENCODE_EXCLUDEALPHANUM)
		    && (ISALPHA(*src) || ISDIGIT(*src) 
				|| *src == '_' || *src == '-' || *src == '.' || *src == '~')
		    && *(unsigned char *)src < 128)
			encode = 0;

//    if ((encoding & URLENCODE_EXCLUDE_8BIT) && (*src > 127)) encode = 0;
		if ((encoding & URLENCODE_EXCLUDE_ABOVEEQ32) && (*src >= 32)) encode = 0;
//    if ((encoding & URLENCODE_ENCODESPACE) && *src == ' ') encode = 1;
//    if ((encoding & URLENCODE_ENCODEXML) && (*src == '<' || *src == '>' || *src == '&')) encode = 1;
		if ((*src == '&' && (style == URLENCODE_STYLE_ANDPOUND || style == URLENCODE_STYLE_ANDPOUNDX)) ||
		    (*src == '%' && style == URLENCODE_STYLE_PERCENT)) encode = 1;
		if ((encoding & URLENCODE_EXCLUDESLASH) && (*src == '/' || *src == ':')) encode = 0;
		if (!encode)
		{
			dest += *src;
		}
		else if (use_plus_for_space && *src == ' ')
		{
			dest += '+';
		}
		else
		{
			switch (style)
			{
				case URLENCODE_STYLE_PERCENT:
					dest += StringPrintf("%%%02X", (unsigned char)*src);
					break;
				case URLENCODE_STYLE_ANDPOUND:
					dest += StringPrintf("&#%02d;", (unsigned char)*src);
					break;
				case URLENCODE_STYLE_ANDPOUNDX:
					dest += StringPrintf("&#x%02X;", (unsigned char)*src);
					break;
			}
		}
		src++;
	}
}

void Url::decode(StringW &str, int use_plus_for_space)
{
	if (str.isempty()) return;
	Url::decode(str.getNonConstVal());
}

static uint8_t quickhex(wchar_t c)
{
	int hexvalue = c;
	if (hexvalue & 0x10)
		hexvalue &= ~0x30;
	else
	{
		hexvalue &= 0xF;
		hexvalue += 9;
	}
	return hexvalue;
}

static uint8_t DecodeEscape(const wchar_t *&str)
{
	uint8_t a = quickhex(*++str);
	uint8_t b = quickhex(*++str);
	str++;
	return a * 16 + b;
}

static void DecodeEscapedUTF8(wchar_t *&output, const wchar_t *&input)
{
	uint8_t utf8_data[1024] = {0}; // hopefully big enough!!
	int num_utf8_words=0;
	bool error=false;

	while (input && *input && *input == '%' && num_utf8_words < sizeof(utf8_data))
	{
		if (iswxdigit(input[1]) && iswxdigit(input[2]))
		{
			utf8_data[num_utf8_words++]=DecodeEscape(input);
		}
		else if (input[1] == '%')
		{
			input+=2;
			utf8_data[num_utf8_words++]='%';
		}
		else
		{
			error = true;
			break;
		}
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, 0, 0);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, output, len);
	output += len;

	if (error)
	{
		*output++ = *input++;
	}
}

// benski> We have the luxury of knowing that decoding will ALWAYS produce smaller strings
// so we can do it in-place
void Url::decode(wchar_t *str)
{
	const wchar_t *itr = str;
	while (itr && *itr)
	{
		switch (*itr)
		{
			case '%':
				DecodeEscapedUTF8(str, itr);
				break;
			default:
				*str++ = *itr++;
				break;
		}
	}
	*str = 0;
}