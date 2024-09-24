#ifndef NULLSOFT_WMHANDLERH
#define NULLSOFT_WMHANDLERH
#include <wmsdk.h>

#define NEXT(x) { if (next) next->x; }

enum DRM_INDIVIDUALIZATION_STATUS {
	INDI_UNDEFINED = 0x0000,
	INDI_BEGIN = 0x0001,
	INDI_SUCCEED = 0x0002,
	INDI_FAIL = 0x0004,
	INDI_CANCEL = 0x0008,
	INDI_DOWNLOAD = 0x0010,
	INDI_INSTALL = 0x0020
};

enum DRM_HTTP_STATUS {
	HTTP_NOTINITIATED = 0,
	HTTP_CONNECTING = 1,
	HTTP_REQUESTING = 2,
	HTTP_RECEIVING = 3,
	HTTP_COMPLETED = 4
};

typedef struct _WMGetLicenseData {
	DWORD   dwSize;
	HRESULT hr;
	WCHAR* wszURL;
	WCHAR* wszLocalFilename;
	BYTE* pbPostData;
	DWORD   dwPostDataSize;
} WM_GET_LICENSE_DATA;


typedef struct _WMIndividualizeStatus {
	HRESULT                      hr;
	DRM_INDIVIDUALIZATION_STATUS enIndiStatus;
	LPSTR                        pszIndiRespUrl;
	DWORD                        dwHTTPRequest;
	DRM_HTTP_STATUS              enHTTPStatus;
	DWORD                        dwHTTPReadProgress;
	DWORD                        dwHTTPReadTotal;
} WM_INDIVIDUALIZE_STATUS;

class WMHandler //: public Chainable<WMHandler>
{
public:
	WMHandler();
		~WMHandler();
	WMHandler &operator << (WMHandler &chain);
	WMHandler &operator >> (WMHandler &chain);
	WMHandler&operator << (WMHandler *chain);
	WMHandler &operator >> (WMHandler *chain);
	WMHandler &First();

	virtual void Opened() NEXT(Opened())
	virtual void OpenFailed();
	virtual void ReOpen();

	virtual void SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample)
	NEXT(SampleReceived(timeStamp, duration, outputNum, flags, sample))

	virtual void AudioDataReceived(void *data, unsigned long sizeBytes, DWORD timestamp)
	NEXT(AudioDataReceived(data, sizeBytes, timestamp))
	
	virtual void TimeReached(QWORD &timeReached) NEXT(TimeReached(timeReached))
	virtual void NewSourceFlags() NEXT(NewSourceFlags())
	virtual void HasVideo(bool &video) NEXT(HasVideo(video))
	virtual void Started();
	virtual void Stopped();
	virtual void Stopping() NEXT(Stopping())
	virtual void DRMExpired() NEXT(DRMExpired())
	
	virtual void Error();

	virtual void Kill() NEXT(Kill())
	virtual void PreRollComplete();

	virtual void EndOfFile();
	virtual void Closed();
	virtual void BufferingStarted();
	virtual void BufferingStopped();
	virtual void NewMetadata();
	virtual void Connecting() NEXT(Connecting())
	virtual void Locating() NEXT(Locating())

	virtual void Individualize();
	virtual void NeedsIndividualization() NEXT(NeedsIndividualization())
	virtual void IndividualizeStatus(WM_INDIVIDUALIZE_STATUS *status) NEXT(IndividualizeStatus(status))

	virtual void SignatureState(WMT_DRMLA_TRUST *&state);
	virtual void NoRights(wchar_t *licenseData);
	virtual void NoRightsEx(WM_GET_LICENSE_DATA *&licenseData);
	virtual void AcquireLicense(WM_GET_LICENSE_DATA *&licenseData);
	virtual void LicenseRequired();
	virtual void BrowserClosed() NEXT(BrowserClosed())
	virtual void LicenseAcquired() NEXT(LicenseAcquired())
	virtual void AllocateOutput(long outputNum, long bufferSize, INSSBuffer *&buffer);
	virtual void MonitorCancelled() NEXT(MonitorCancelled())
	virtual void SilentCancelled() NEXT(SilentCancelled())

	virtual void VideoCatchup(QWORD time);
	virtual void TimeToSync(QWORD timeStamp, __int64 &diff);
	virtual void OpenCalled() NEXT(OpenCalled())

		virtual void InitPlaylistBurn() NEXT(InitPlaylistBurn()) 
		virtual void AccessDenied() NEXT(AccessDenied())

private:
			WMHandler *next, *prev;
};
#undef NEXT
#endif
