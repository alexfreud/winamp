#include "wac_download_http_receiver.h"


wa::Components::WAC_Download_HTTP_Receiver::WAC_Download_HTTP_Receiver()
{}

wa::Components::WAC_Download_HTTP_Receiver::~WAC_Download_HTTP_Receiver()
{}

void wa::Components::WAC_Download_HTTP_Receiver::open( api_dns * p_dns, size_t p_recvbufsize, const char *p_proxy )
{
	Q_UNUSED( p_dns )
	Q_UNUSED( p_recvbufsize )
	Q_UNUSED( p_proxy )
}


std::size_t wa::Components::WAC_Download_HTTP_Receiver::AddRef()
{
	return this->_reference_count.fetch_add( 1 );
}

std::size_t wa::Components::WAC_Download_HTTP_Receiver::Release()
{
	std::size_t l_reference_count = this->_reference_count.fetch_sub( 1 );
	if ( l_reference_count == 0 )
		delete this;

	return l_reference_count;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_Download_HTTP_Receiver
START_DISPATCH;
CB(  ADDREF,                                      AddRef )
CB(  RELEASE,                                     Release )
VCB( API_HTTPRECEIVER_OPEN,                       open )
//VCB( API_HTTPRECEIVER_ADDHEADER,                  addheader )
//VCB( API_HTTPRECEIVER_ADDHEADERVALUE,             addheadervalue )
//VCB( API_HTTPRECEIVER_CONNECT,                    connect )
//CB(  API_HTTPRECEIVER_RUN,                        run )
//CB(  API_HTTPRECEIVER_GETSTATUS,                  get_status )
//CB(  API_HTTPRECEIVER_GETBYTESAVAILABLE,          bytes_available )
//CB(  API_HTTPRECEIVER_GETBYTES,                   get_bytes )
//CB(  API_HTTPRECEIVER_PEEKBYTES,                  peek_bytes )
//CB(  API_HTTPRECEIVER_GETHEADER,                  getheader )
//CB(  API_HTTPRECEIVER_GETCONTENTLENGTH,           content_length )
//CB(  API_HTTPRECEIVER_GETALLHEADERS,              getallheaders )
//CB(  API_HTTPRECEIVER_GETREPLYCODE,               getreplycode )
//CB(  API_HTTPRECEIVER_GETREPLY,                   getreply )
//CB(  API_HTTPRECEIVER_GETERROR,                   geterrorstr )
//CB(  API_HTTPRECEIVER_GETCONNECTION,              get_con )
//VCB( API_HTTPRECEIVER_ALLOW_COMPRESSION,          AllowCompression )
//VCB( API_HTTPRECEIVER_RESET_HEADERS,              reset_headers )
//CB(  API_HTTPRECEIVER_GET_URL,                    get_url )
//VCB( API_HTTPRECEIVER_SET_SENDBUFSIZE,            set_sendbufsize )
//VCB( API_HTTPRECEIVER_SET_ACCEPT_ALL_REPLY_CODES, set_accept_all_reply_codes )
//VCB( API_HTTPRECEIVER_SET_PERSISTENT,             set_persistent )
END_DISPATCH;
#undef CBCLASS
