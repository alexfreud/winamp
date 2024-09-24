#pragma once
#include "../nu/AutoLock.h"
#include <vector>
#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

extern "C" HANDLE DuplicateCurrentThread();
namespace JSAPI2
{
	template <class API> struct CallbackInfo
	{
		CallbackInfo()
		{
			api = 0;
			threadId = 0;
			threadHandle = 0;
		}

		CallbackInfo(API *me)
		{
			api = me;
			threadId = GetCurrentThreadId();
			threadHandle = DuplicateCurrentThread();
		}

		~CallbackInfo()
		{
			CloseHandle(threadHandle);
			threadHandle = 0;
		}
		API *api;
		DWORD threadId;
		HANDLE threadHandle;
	};

	class TransportAPI;
	class MediaCoreAPI;
	class AsyncDownloaderAPI;
	class CallbackManager
	{
	public:
		CallbackManager();
		
	public:
		/** stuff for Winamp to call to trigger callbacks 
		** these are primarily responsible for getting over to the correct thread
		** to keep that particular logic out of the various functions
		*/
		void OnStop(int position, int is_full_stop);
		void OnPlay(const wchar_t *filename);
		void OnPause(bool pause_state);

		/** Stuff that's OK to call on any thread 
		*/
		bool OverrideMetadata(const wchar_t *filename, const wchar_t *tag, wchar_t *out, size_t outCch);

		/** stuff for Winamp to call to trigger callbacks 
		** these are primarily responsible for getting over to the correct thread
		** to keep that particular logic out of the various functions
		*/
		void OnInit(const wchar_t *url, const wchar_t *onlinesvcId);
		void OnConnect(const wchar_t *url, const wchar_t *onlinesvcId);
		void OnData(const wchar_t *url, size_t downloadedlen, size_t totallen, const wchar_t *onlinesvcId);
		void OnCancel(const wchar_t *url, const wchar_t *onlinesvcId);
		void OnError(const wchar_t *url, int error, const wchar_t *onlinesvcId);
		void OnFinish(const wchar_t *url, const wchar_t *destfilename, const wchar_t *onlinesvcId);
	public:
		/* stuff for other JSAPI2 classes to call */
		void Register(JSAPI2::TransportAPI *me);
		void Deregister(JSAPI2::TransportAPI *me);

		void Register(JSAPI2::MediaCoreAPI *me);
		void Deregister(JSAPI2::MediaCoreAPI *me);

		void Register(JSAPI2::AsyncDownloaderAPI *me);
		void Deregister(JSAPI2::AsyncDownloaderAPI *me);
	private:
		/* Transport API callbacks */
		typedef CallbackInfo<JSAPI2::TransportAPI> TransportCallback;
		typedef std::vector<TransportCallback*> TransportsList;
		TransportsList transports;

		typedef std::vector<MediaCoreAPI*> MediaCoreList;
		MediaCoreList mediaCores;

		typedef CallbackInfo<JSAPI2::AsyncDownloaderAPI> AsyncDownloaderCallback;
		typedef std::vector<AsyncDownloaderCallback*> AsyncDownloadersList;
		AsyncDownloadersList asyncDownloaders;

		Nullsoft::Utility::LockGuard callbackGuard;
	};

	extern CallbackManager callbackManager;
}