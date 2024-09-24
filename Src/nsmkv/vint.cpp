#include <bfc/platform/types.h>

#ifdef _MSC_VER
#include <intrin.h>
static uint32_t __inline clz(uint32_t value)
{
	DWORD leading_zero = 0;
	if (_BitScanReverse(&leading_zero, value))
	{
		return 31 - leading_zero;
	}
	else
	{
		return 32;
	}
}
#endif

uint8_t vint_get_number_bytes(uint8_t first_byte)
{
	return (uint8_t)clz((uint32_t)first_byte) - 24;
}

static uint8_t masks[] =
{
	0x7F, // 0111 1111
	0x3F, // 0011 1111
	0x1F, // 0001 1111
	0x0F, // 0000 1111
	0x07, // 0000 0111
	0x03, // 0000 0011
	0x01, // 0000 0001
	0x00, // 0000 0000
};

/* call if you already know the len (e.g. from vint_get_number_bytes earlier */
uint64_t vint_read_ptr_len(uint8_t len, const uint8_t *ptr)
{
	uint64_t ret = masks[len] & ptr[0];

	while (len--)
	{
		ret <<= 8;
		ret |= *++ptr;
	}
	return ret;
}

uint64_t vint_read_ptr(const uint8_t *ptr)
{
	uint8_t len = vint_get_number_bytes(ptr[0]);
	return vint_read_ptr_len(len, ptr);
}

bool vint_unknown_length(uint8_t len, const uint8_t *ptr)
{
	if (masks[len] != (masks[len] & ptr[0]))
		return false;

	while (len--)
	{
		if (*++ptr == 0xFF)
			return false;
	}
	return true;
}

static int64_t vsint_substr[] =
{
	0x3F,
		0x1FFF,
		0x0FFFFF,
		0x07FFFFFF,
		0x03FFFFFFFF,
		0x01FFFFFFFFFF,
		0x00FFFFFFFFFFFF,
		0x007FFFFFFFFFFFFF,
};

int64_t vsint_read_ptr_len(uint8_t len, const uint8_t *ptr)
{
	uint64_t val = vint_read_ptr_len(len, ptr);
	return val - vsint_substr[len];
}

int64_t vsint_read_ptr(const uint8_t *ptr)
{
	uint8_t len = vint_get_number_bytes(ptr[0]);
	uint64_t val = vint_read_ptr(ptr);
	return val - vsint_substr[len];
}