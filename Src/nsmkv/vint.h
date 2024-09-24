#pragma once
#include <bfc/platform/types.h>

uint8_t vint_get_number_bytes(uint8_t first_byte);
/* call if you already know the len (e.g. from vint_get_number_bytes earlier */
uint64_t vint_read_ptr_len(uint8_t len, const uint8_t *ptr);
int64_t vsint_read_ptr_len(uint8_t len, const uint8_t *ptr);

/* don't call this unless you're sure that you have enough room in the buffer! */
uint64_t vint_read_ptr(const uint8_t *ptr);
int64_t vsint_read_ptr(const uint8_t *ptr);

/* values encoded as all 1's are supposed to indicate 'unknown value' */
bool vint_unknown_length(uint8_t len, const uint8_t *ptr);