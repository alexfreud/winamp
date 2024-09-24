#ifndef NULLSOFT_IN_MP3_ID3V1_H
#define NULLSOFT_IN_MP3_ID3V1_H

#include <bfc/platform/types.h>
class ID3v1
{
public:
	ID3v1();
	int Decode(const void *data);
	// return -1 for empty, 1 for OK, 0 for "don't understand tag name"
	int GetString(const char *tag, wchar_t *data, int dataLen);
	// returns 1 for OK, 0 for "don't understand tag name"
	int SetString(const char *tag, const wchar_t *data);
	int Encode(void *data);
	bool IsDirty() { return dirty; }
	bool HasData() { return hasData; }
	void Clear();

private:
	char title[31],artist[31],album[31],comment[31];
	char year[5];
	unsigned char genre;
	unsigned char track;

	bool hasData;
	bool dirty;
};

#endif