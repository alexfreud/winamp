#ifndef NULLSOFT_WAC_DOWNLOAD_HTTP_RECEIVER_H
#define NULLSOFT_WAC_NETWORK_HTTP_RECEIVER_H

#include <atomic>

#include <QtCore>
//#include <QAuthenticator>
//#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QUrl>
#include <QUrlQuery>

//#include "wac_downloadManager_Headers.h"

#include "wac_download_http_receiver_api.h"

namespace wa
{
    namespace Components
    {
        class WAC_Download_HTTP_Receiver : public QObject, public api_wac_download_manager_http_receiver
        {
            Q_OBJECT

        public:
            WAC_Download_HTTP_Receiver();
            ~WAC_Download_HTTP_Receiver();

            void            open( api_dns *p_dns = API_DNS_AUTODNS, size_t p_recvbufsize = PACKET_SIZE, const char *p_proxy = NULL );

            std::size_t     AddRef();
            std::size_t     Release();

        protected:
            RECVS_DISPATCH;

        private:
            volatile std::atomic<std::size_t> _reference_count = 1;



        };
    }
}

#endif  // !NULLSOFT_WAC_NETWORK_HTTP_RECEIVER_H