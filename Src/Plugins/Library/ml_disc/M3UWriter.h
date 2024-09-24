#ifndef NULLSOFT_M3UWRITERH
#define NULLSOFT_M3UWRITERH

#include <stdio.h>
#include <windows.h>

class M3UWriter
{
public:
	M3UWriter();
	virtual ~M3UWriter();

	int Open(char *filename, int extendedMode);
	int Open( FILE *_fp, char *filename, int extendedMode );
	void SetFilename( char *filename );
	void SetExtended( char *filename, char *title, int length );
	void Close();

private:
	char  basePath[MAX_PATH];
	int   extended = 0;
	FILE *fp       = NULL;
};

#endif