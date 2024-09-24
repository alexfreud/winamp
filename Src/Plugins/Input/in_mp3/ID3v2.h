#ifndef NULLSOFT_IN_MP3_ID3v2_H
#define NULLSOFT_IN_MP3_ID3v2_H

#include "../id3v2/id3_tag.h"

class ID3v2
{
public:
	ID3v2();
	bool HasData() { return hasData; }
	bool IsDirty() { return dirty; }
	int Decode(const void *data, size_t len);
	int Encode(const void *data, size_t len);
	uint32_t EncodeSize();
	// return -1 for empty, 1 for OK, 0 for "don't understand tag name"
	int GetString(const char *tag, wchar_t *data, int dataLen);
	int SetString(const char *tag, const wchar_t *data);

	int GetAlbumArt(const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int SetAlbumArt(const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *type);

	void Clear();
private:
	void add_set_id3v2_frame(ID3_FrameID id, const wchar_t *c);
	void add_set_latin_id3v2_frame(ID3_FrameID id, const wchar_t *c);
	bool hasData;
	bool dirty;
public:
	ID3_Tag id3v2;
};

#endif