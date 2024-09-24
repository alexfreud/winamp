#ifndef NULLSOFT_MOREITEMSH
#define NULLSOFT_MOREITEMSH

struct moreitems 
{
	moreitems();
	~moreitems();
	const wchar_t *GetHiddenFilename(int index);
	int AddHiddenItem(const wchar_t *filename, const wchar_t *title, int length, int index, char *curtain);
	const char *GetHiddenCurtain(int index);
	int SetRange(int index, unsigned long start, unsigned long end);
	unsigned long GetStart(int index);
	unsigned long GetEnd(int index);
	wchar_t *strFile;
	size_t cbFile;
	wchar_t *strTitle;
	size_t cbTitle;
	char *strCurtain;
	size_t cbCurtain;
	int length;
	int index;
	unsigned long starttime;  // Start time in MS  (0, begin of file)
	unsigned long endtime; // End time in MS  (0, end of file)
	moreitems *Next; // Next Item in linked list
};

#endif