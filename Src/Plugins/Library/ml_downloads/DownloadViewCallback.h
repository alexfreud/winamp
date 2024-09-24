#include <atomic>

#include "Main.h"
#include "Downloaded.h"

#include "DownloadStatus.h"
#include "DownloadsDialog.h"
#include "api__ml_downloads.h"
#include "api/service/waServiceFactory.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"


class DownloadViewCallback : public ifc_downloadManagerCallback
{
public:
	DownloadViewCallback();

	void   OnInit( DownloadToken token );
	void   OnConnect( DownloadToken token );
	void   OnData( DownloadToken token, void *data, size_t datalen );
	void   OnCancel( DownloadToken token );
	void   OnError( DownloadToken token, int error );
	void   OnFinish( DownloadToken token );

	size_t AddRef();
	size_t Release();

private: // private destructor so no one accidentally calls delete directly on this reference counted object
	~DownloadViewCallback();

protected:
	RECVS_DISPATCH;

private:
	std::atomic<std::size_t> _ref_count = 1;
};

