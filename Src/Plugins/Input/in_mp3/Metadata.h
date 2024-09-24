#ifndef NULLSOFT_IN_MP3_METADATA
#define NULLSOFT_IN_MP3_METADATA

#include "giofile.h"
#include "ID3v1.h"
#include "ID3v2.h"
#include "Lyrics3.h"
#include "apev2.h"

enum
{
	METADATA_SUCCESS = 0,
		SAVE_SUCCESS = 0,
	SAVE_ERROR_OPENING_FILE = 1,
	SAVE_ID3V1_WRITE_ERROR = 2,
	SAVE_ID3V2_WRITE_ERROR = 3,
	SAVE_ERROR_READONLY = 4,
	SAVE_ERROR_CANT_OPEN_TEMPFILE = 5,
	SAVE_ERROR_ERROR_OVERWRITING = 6,
	SAVE_LYRICS3_WRITE_ERROR = 7,
	SAVE_APEV2_WRITE_ERROR = 8,
};


class Metadata
{
public:
	Metadata()                                                        {}
	Metadata(CGioFile *_file, const wchar_t *_filename);
	~Metadata();

	int Open(const wchar_t *filename);
	int GetExtendedData(const char *tag, wchar_t *data, int dataLen);
	int SetExtendedData(const char *tag, const wchar_t *data);
	int Save();
	bool IsMe(const wchar_t *fn) { return filename && !_wcsicmp(filename, fn); }
	
	void AddRef() { InterlockedIncrement(&refs); }
	void Release() { if(!InterlockedDecrement(&refs)) delete this; }

private:
	bool IsDirty();
	void ReadTags(CGioFile *_file);
	int GetString(const char *tag, wchar_t *data, int dataLen);

	int sampleRate = 0;
	int bitrate    = 0;
	int vbr        = 0;
	int channels   = 0;
	int length_ms  = 0;
	CGioFile file;

public:
	ID3v1 id3v1;
	ID3v2 id3v2;
	Lyrics3 lyrics3;
	APE apev2;

	wchar_t *filename = 0;
protected:
	volatile LONG refs = 1;
};

#endif