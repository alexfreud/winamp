#include "nxmutablestring.h"
#include "foundation/error.h"

/* currently this is closely coupled with the nx_string_t implementation. beware! */
extern HANDLE string_heap;

void NXMutableStringDestroy(nx_mutable_string_t mutable_string)
{
	if (mutable_string)
	{
		if (mutable_string->nx_string_data)
			NXStringRelease(mutable_string->nx_string_data);
		HeapFree(string_heap, 0, mutable_string);
	}
}

nx_mutable_string_t NXMutableStringCreateFromXML(const nsxml_char_t *characters, size_t num_characters)
{
	nx_mutable_string_t mutable_string = (nx_mutable_string_t)HeapAlloc(string_heap, 0, sizeof(nx_mutable_string_struct_t));
	NXStringCreateWithBytes(&mutable_string->nx_string_data, characters, num_characters*2, nx_charset_utf16le);
	mutable_string->allocation_length = num_characters;
	return mutable_string;
}

int NXMutableStringGrowFromXML(nx_mutable_string_t mutable_string, const nsxml_char_t *characters, size_t num_characters)
{
	if (mutable_string->nx_string_data->len + num_characters + 1 > mutable_string->allocation_length)
	{
		nx_string_t new_string = NXStringRealloc(mutable_string->nx_string_data, mutable_string->nx_string_data->len + num_characters + 1);
		if (!new_string)
			return NErr_OutOfMemory;
		mutable_string->nx_string_data = new_string;
		mutable_string->allocation_length = mutable_string->nx_string_data->len + num_characters + 1;
	}
	memcpy(mutable_string->nx_string_data->string + mutable_string->nx_string_data->len, characters, num_characters*sizeof(wchar_t));
	mutable_string->nx_string_data->len += num_characters;
	mutable_string->nx_string_data->string[mutable_string->nx_string_data->len]=0; /* well null terminate */

	return NErr_Success;
}