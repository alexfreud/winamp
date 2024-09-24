#ifndef NULLSOFT_APEV2_FLAGS_H
#define NULLSOFT_APEV2_FLAGS_H

namespace APEv2
{
enum
{
	APEV2_SUCCESS = 0,
	APEV2_FAILURE = 1,
	APEV2_TOO_SMALL = 2,
	APEV2_KEY_NOT_FOUND=3,
	APEV2_NO_DATA = 4, /* Key found, but data is empty or corrupt */
	APEV2_END_OF_ITEMS=5,
};
/*
http://wiki.hydrogenaudio.org/index.php?title=Ape_Tags_Flags
*/
enum
{
	/* flags for header or item */
	FLAG_READONLY = 1,

	/* header/footer specific flags */
	FLAG_HEADER_HAS_HEADER = (1 << 31),
	FLAG_HEADER_NO_FOOTER = (1 << 30),
	FLAG_HEADER_IS_HEADER = (1 << 29),
	FLAG_HEADER_ENCODE_MASK = FLAG_READONLY|FLAG_HEADER_HAS_HEADER|FLAG_HEADER_NO_FOOTER,

	/* item specific flags */
	MASK_ITEM_TYPE = ((1 << 2) | (1 << 1)),

	FLAG_ITEM_TEXT = 0,
	FLAG_ITEM_BINARY = (1 << 1), /* We compare WITHOUT SHIFTING so all flag values are << 1 */
	FLAG_ITEM_LOCATOR = (2 << 1),
	FLAG_ITEM_RESERVED = (3 << 1),
};

}

#endif