#ifndef NULLSOFT_IN_MP3_LYRICS3_H
#define NULLSOFT_IN_MP3_LYRICS3_H

#include <bfc/platform/types.h>
class Lyrics3
{
public:
	Lyrics3();
	~Lyrics3();
	bool HasData() { return hasData; }
	bool IsDirty() { return dirty; }
	void Clear();
	void ResetDirty() { dirty=0; };
	int Decode(const void *data, size_t datalen);
// return -1 for empty, 1 for OK, 0 for "don't understand tag name"
	int GetString(const char *tag, wchar_t *data, int dataLen);
	int SetString(const char *tag, const wchar_t *data);

private:
	bool hasData;
	bool dirty;
	wchar_t *title, *album, *artist;
};

#endif