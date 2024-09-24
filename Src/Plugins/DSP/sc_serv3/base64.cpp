#include <vector>
#include "base64.h"

using namespace std;

static const std::string base64_chars = 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

static inline bool is_base64(__uint8 c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

const std::vector<__uint8> base64::decode(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	int i = 0, in_ = 0;
	__uint8 char_array_4[4], char_array_3[3];
	std::vector<__uint8> ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; ++in_;
		if (i == 4)
		{
			for (i = 0; i <4; i++)
			{
				char_array_4[i] = (__uint8)base64_chars.find(char_array_4[i]);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			{
				ret.push_back(char_array_3[i]);
			}

			i = 0;
		}
	}

	if (i)
	{
		int j = 0;
		for (j = i; j <4; j++)
		{
			char_array_4[j] = 0;
		}

		for (j = 0; j <4; j++)
		{
			char_array_4[j] = (__uint8)base64_chars.find(char_array_4[j]);
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
		{
			ret.push_back(char_array_3[j]);
		}
	}

	return ret;
}
