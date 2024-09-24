#include "utf.h"

#include "ByteReader.h"
#include "ByteWriter.h"
#include "foundation/error.h"
#include <string.h>

static const uint8_t mask_tab[6]={0x80,0xE0,0xF0,0xF8,0xFC,0xFE};

static const uint8_t val_tab[6]={0,0xC0,0xE0,0xF0,0xF8,0xFC};

// returns the number of utf-16 words required to store a given codepoint
static size_t ucs4_to_utf16_count(uint32_t codepoint)
{
	if (codepoint >= 0x110000)
		return 0;  // out of bounds

	if (codepoint >= 0x10000)
		return 2;

	return 1;
}

static int utf16LE_to_ucs4_character(bytereader_t const byte_reader, uint32_t *codepoint)
{
	uint16_t lead;

	lead = bytereader_read_u16_le(byte_reader);
	if (lead < 0xD800 || lead >= 0xE000)
	{
		*codepoint = lead;
		return NErr_Success;
	}

	if (lead < 0xDC00)
	{
		if (bytereader_size(byte_reader) >= 2)
		{
			uint16_t trail = bytereader_read_u16_le(byte_reader);
			if (trail >= 0xDC00 && trail < 0xE000)
			{
				*codepoint = 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
				return NErr_Success;
			}
		}
	}

	return NErr_Error; // invalid
}

static int utf16BE_to_ucs4_character(bytereader_t const byte_reader, uint32_t *codepoint)
{
	uint16_t lead;

	lead = bytereader_read_u16_be(byte_reader);
	if (lead < 0xD800 || lead >= 0xE000)
	{
		*codepoint = lead;
		return NErr_Success;
	}

	if (lead < 0xDC00)
	{
		if (bytereader_size(byte_reader) >= 2)
		{
			uint16_t trail = bytereader_read_u16_be(byte_reader);
			if (trail >= 0xDC00 && trail < 0xE000)
			{
				*codepoint = 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
				return NErr_Success;
			}
		}
	}

	return NErr_Error; // invalid
}

static size_t utf8_to_ucs4_character(const char *utf8, size_t len, uint32_t *codepoint)
{
	uint32_t res=0;
	size_t n;
	size_t cnt=0;
	while(1)
	{
		if ((*utf8&mask_tab[cnt])==val_tab[cnt]) break;
		if (++cnt==6) return 0;
	}
	cnt++;


	if (cnt==2 && !(*utf8&0x1E)) 
		return 0;

	if (cnt==1)
		res=*utf8;
	else
		res=(0xFF>>(cnt+1))&*utf8;

	if (cnt > len)
		return 0;

	for (n=1;n<cnt;n++)
	{
		if ((utf8[n]&0xC0) != 0x80)
			return 0;
		if (!res && n==2 && !((utf8[n]&0x7F) >> (7 - cnt)))
			return 0;

		res=(res<<6)|(utf8[n]&0x3F);
	}

	if (codepoint)
		*codepoint=res;

	return cnt;
}

// returns the number of utf-8 bytes required to store a given codepoint
static size_t ucs4_to_utf8_count(uint32_t codepoint)
{
	if (codepoint < 0x80)
		return 1;
	else if (codepoint < 0x800)
		return 2;
	else if (codepoint < 0x10000)
		return  3;
	else if (codepoint < 0x200000)
		return  4;
	else if (codepoint < 0x4000000)
		return  5;
	else if (codepoint <= 0x7FFFFFFF)
		return  6;
	else
		return 0;
}

static size_t ucs4_to_utf8_character(char *target, uint32_t codepoint, size_t max)
{
	size_t count = ucs4_to_utf8_count(codepoint);

	if (!count)
		return 0;

	if (count>max) return 0;

	if (target == 0)
		return count;

	switch (count)
	{
	case 6:
		target[5] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
		codepoint |= 0x4000000;
	case 5:
		target[4] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
		codepoint |= 0x200000;
	case 4:
		target[3] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
		codepoint |= 0x10000;
	case 3:
		target[2] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
		codepoint |= 0x800;
	case 2:
		target[1] = 0x80 | (codepoint & 0x3F);
		codepoint = codepoint >> 6;
		codepoint |= 0xC0;
	case 1:
		target[0] = codepoint;
	}

	return count;
}

static size_t ucs4_to_utf16LE_character(bytewriter_t byte_writer, uint32_t codepoint)
{
	if (codepoint >= 0x110000)
		return 0;

	if (codepoint >= 0x10000)
	{
		if (bytewriter_size(byte_writer) < 4)
			return 0;

		bytewriter_write_u16_le(byte_writer, ((codepoint - 0x10000) >> 10) + 0xD800); // high surrogate
		bytewriter_write_u16_le(byte_writer, ((codepoint - 0x10000) & 0x3FF) + 0xDC00); // low surrogate
		return 2;
	}
	else
	{
		bytewriter_write_u16_le(byte_writer, codepoint);
		return 1;
	}
}

static size_t ucs4_to_utf16BE_character(bytewriter_t byte_writer, uint32_t codepoint)
{
	if (codepoint >= 0x110000)
		return 0;

	if (codepoint >= 0x10000)
	{
		if (bytewriter_size(byte_writer) < 4)
			return 0;

		bytewriter_write_u16_be(byte_writer, ((codepoint - 0x10000) >> 10) + 0xD800); // high surrogate
		bytewriter_write_u16_be(byte_writer, ((codepoint - 0x10000) & 0x3FF) + 0xDC00); // low surrogate
		return 2;
	}
	else
	{
		bytewriter_write_u16_be(byte_writer, codepoint);
		return 1;
	}
}

size_t utf16LE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;
	bytereader_s byte_reader;
	bytereader_init(&byte_reader, src, source_len*2);

	if (!dst) // they just want the size
	{
		while (bytereader_size(&byte_reader))
		{
			if (utf16LE_to_ucs4_character(&byte_reader, &codepoint) != NErr_Success)
				break;

			characters_processed = ucs4_to_utf8_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(bytereader_size(&byte_reader) && position<out_len)
	{
		if (utf16LE_to_ucs4_character(&byte_reader, &codepoint) != NErr_Success)
			break;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;
		position+=characters_processed;
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}

size_t utf16BE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;
	bytereader_s byte_reader;
	bytereader_init(&byte_reader, src, source_len*2);

	if (!dst) // they just want the size
	{
		while (bytereader_size(&byte_reader))
		{
			if (utf16BE_to_ucs4_character(&byte_reader, &codepoint) != NErr_Success)
				break;

			characters_processed = ucs4_to_utf8_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(bytereader_size(&byte_reader) && position<out_len)
	{
		if (utf16BE_to_ucs4_character(&byte_reader, &codepoint) != NErr_Success)
			break;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;
		position+=characters_processed;
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}


size_t ucs4_to_utf8(const uint32_t *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;
	bytereader_s byte_reader;
	bytereader_init(&byte_reader, src, source_len*4);

	if (!dst) // they just want the size
	{
		while (bytereader_size(&byte_reader) > 3)
		{
			codepoint = bytereader_read_u32_le(&byte_reader);
			
			characters_processed = ucs4_to_utf8_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(bytereader_size(&byte_reader) > 3 && position<out_len)
	{
		codepoint = bytereader_read_u32_le(&byte_reader);

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;
		position+=characters_processed;
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}

size_t utf8_to_utf16LE(const char *src, size_t source_len, uint16_t *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t characters_processed;
	bytewriter_s byte_writer;

	if (!dst) // they just want the size
	{
			size_t position=0;
		while (source_len)
		{
			characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			source_len -= characters_processed;
			src += characters_processed;

			characters_processed = ucs4_to_utf16_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}


	bytewriter_init(&byte_writer, dst, out_len*2);
	while(source_len && bytewriter_size(&byte_writer))
	{
		characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;

		characters_processed=ucs4_to_utf16LE_character(&byte_writer, codepoint);
		if (!characters_processed)
			break;
	}
	if (bytewriter_size(&byte_writer))
		bytewriter_write_u16_le(&byte_writer, 0);
	return out_len - bytewriter_size(&byte_writer)/2;
}

size_t utf8_to_utf16BE(const char *src, size_t source_len, uint16_t *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t characters_processed;
	bytewriter_s byte_writer;

	if (!dst) // they just want the size
	{
		size_t position=0;
		while (source_len)
		{
			characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			source_len -= characters_processed;
			src += characters_processed;

			characters_processed = ucs4_to_utf16_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}
	bytewriter_init(&byte_writer, dst, out_len*2);
	while(source_len && bytewriter_size(&byte_writer))
	{
		characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;

		characters_processed=ucs4_to_utf16BE_character(&byte_writer, codepoint);
		if (!characters_processed)
			break;

	}
	if (bytewriter_size(&byte_writer))
		bytewriter_write_u16_be(&byte_writer, 0);
	
	return out_len - bytewriter_size(&byte_writer)/2;

}

size_t utf8_to_ISO_8859_1(const char *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			source_len -= characters_processed;
			src += characters_processed;
			position++;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;

		if (codepoint < 256)
			dst[position++] = codepoint;
		else
			dst[position++] = '?';
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}

size_t ISO_8859_1_to_utf8(const char *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			codepoint = *src++;
			source_len--;

			characters_processed = ucs4_to_utf8_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		codepoint = *src++;		

		source_len--;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;

		position+=characters_processed;
	}
	if (position<out_len)
		dst[position]=0;
	return position;
}

size_t utf8_to_ucs4(const char *src, size_t source_len, uint32_t *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t characters_processed;
	bytewriter_s byte_writer;

	if (!dst) // they just want the size
	{
		size_t position=0;
		while (source_len)
		{
			characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			source_len -= characters_processed;
			src += characters_processed;

			characters_processed = 1;

			position+=characters_processed;
		}
		return position;
	}

	bytewriter_init(&byte_writer, dst, out_len*4);
	while(source_len && bytewriter_size(&byte_writer))
	{
		characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;

		bytewriter_write_u32_le(&byte_writer, codepoint);		
	}
	if (bytewriter_size(&byte_writer))
		bytewriter_write_u32_le(&byte_writer, 0);
	return out_len - bytewriter_size(&byte_writer)/4;
}

size_t ASCII_to_utf8(const char *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			codepoint = *src++;
			source_len--;

			characters_processed = ucs4_to_utf8_count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		codepoint = *src++;		

		source_len--;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;

		position+=characters_processed;
	}
	if (position<out_len)
		dst[position]=0;
	return position;
}

size_t utf8_to_ASCII(const char *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint;
	size_t position=0;
	size_t characters_processed;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			source_len -= characters_processed;
			src += characters_processed;
			position++;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;

		if (codepoint < 128)
			dst[position++] = codepoint;
		else
			dst[position++] = '?';
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}

size_t utf8_strnlen(const char *src, size_t source_len, size_t codepoints)
{
	uint32_t codepoint = 0;
	size_t position=0;
	size_t i=0;

	for (i=0;i<codepoints && *src;i++)
	{
		size_t characters_processed = utf8_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		source_len -= characters_processed;
		src += characters_processed;
		position+=characters_processed;
	}
	return position;

}
