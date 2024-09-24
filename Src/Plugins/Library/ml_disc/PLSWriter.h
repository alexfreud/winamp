#ifndef NULLSOFT_PLSWRITERH
#define NULLSOFT_PLSWRITERH

#include <windows.h>

class PLSWriter
{
public:
	PLSWriter();
	void Open(char *filename);
	void SetFilename(char *filename);
	void SetTitle(char *title);
	void SetLength(int length);
	void Next(); // tells the pls writer to start writing info for the next item
	void Close();
private:
	void BeforeSet();
	unsigned int numEntries;
	int entryUsed;
	char plsFile[MAX_PATH];

};
#endif