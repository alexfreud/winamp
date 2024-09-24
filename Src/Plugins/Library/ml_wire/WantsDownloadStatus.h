#ifndef NULLSOFT_WANTSDOWNLOADSTATUSH
#define NULLSOFT_WANTSDOWNLOADSTATUSH

class WantsDownloadStatus
{
public:
	virtual void DownloadStarted() {}
	virtual void UpdateDownloadStatus(size_t downloaded, size_t totalBytes) {}
	virtual void DownloadDone() {}
	virtual void DownloadFailed() {}
	
};

#endif