#pragma once
#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* to utf8 */
	size_t utf16LE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len);
	size_t utf16BE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len);
	size_t ISO_8859_1_to_utf8(const char *src, size_t source_len, char *dst, size_t out_len);
	size_t ASCII_to_utf8(const char *src, size_t source_len, char *dst, size_t out_len);
	size_t ucs4_to_utf8(const uint32_t *src, size_t source_len, char *dst, size_t out_len);

	/* from utf8 */
	size_t utf8_to_utf16LE(const char *src, size_t source_len, uint16_t *dst, size_t out_len);
	size_t utf8_to_ISO_8859_1(const char *src, size_t source_len, char *dst, size_t out_len);
	size_t utf8_to_utf16BE(const char *src, size_t source_len, uint16_t *dst, size_t out_len);
	size_t utf8_to_ASCII(const char *src, size_t source_len, char *dst, size_t out_len);
	size_t utf8_to_ucs4(const char *src, size_t source_len, uint32_t *dst, size_t out_len);
	
	/* returns the number of bytes required to make the specified number of codepoints exactly fit */
	size_t utf8_strnlen(const char *src, size_t source_len, size_t codepoints);
#ifdef __cplusplus
}
#endif
