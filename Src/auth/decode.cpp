#include <stdio.h>
#include <bfc/platform/types.h>
#include <strsafe.h>

static uint8_t quickhex(char c)
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

static uint8_t DecodeEscape(const char *&str)
{
	uint8_t a = quickhex(*++str);
	uint8_t b = quickhex(*++str);
	str++;
	return a * 16 + b;
}

static void DecodeEscapedUTF8(char *&output, const char *&input)
{
	uint8_t utf8_data[1024] = {0}; // hopefully big enough!!
	int num_utf8_words=0;
	bool error=false;

	while (input && *input && *input == '%' && num_utf8_words < sizeof(utf8_data))
	{
		if (isxdigit(input[1]) && isxdigit(input[2]))
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

	memcpy(output, utf8_data, num_utf8_words);
	output+=num_utf8_words;
	//StringCchCopyExA(output, num_utf8_words, (char *)utf8_data, &output, 0, 0);

	if (error)
	{
		*output++ = *input++;
	}
}

// benski> We have the luxury of knowing that decoding will ALWAYS produce smaller strings
// so we can do it in-place
void UrlDecode(char *str)
{
	const char *itr = str;
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