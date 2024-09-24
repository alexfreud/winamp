#ifndef NULLSOFT_DOWNLOADSPARSEH
#define NULLSOFT_DOWNLOADSPARSEH

#include "DownloadThread.h"

class DownloadsParse : public DownloadThread
{
public:
	virtual void ReadNodes(const wchar_t *url);

};

#endif
