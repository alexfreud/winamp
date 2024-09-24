#ifndef NULLSOFT_DOWNLOADSTATUSH
#define NULLSOFT_DOWNLOADSTATUSH

#include "../nu/AutoLock.h"
#include <map>


class DownloadStatus
{
public:
	class Status
	{
	public:
		Status();
		~Status();
		const Status &operator =(const Status &copy);
		Status(size_t _downloaded, size_t _maxSize, const wchar_t *source, const  wchar_t *title, const wchar_t *path);
		size_t downloaded, maxSize;

		int killswitch;
		wchar_t *source;
		wchar_t *title;
		wchar_t *path;
	private:
	
		void Init();
		void Reset();
	};

	void AddDownloadThread(DownloadToken token, const wchar_t *source, const wchar_t *title, const wchar_t *path);
	void DownloadThreadDone(DownloadToken token);
	bool UpdateStatus(DownloadToken token, size_t downloaded, size_t maxSize);
	bool CurrentlyDownloading();
	void GetStatusString(wchar_t *status, size_t len);
	typedef	std::map<DownloadToken, Status> Downloads;
	Downloads downloads;
	Nullsoft::Utility::LockGuard statusLock;
};

extern DownloadStatus downloadStatus;
#endif