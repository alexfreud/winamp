/*
** JNetLib
** Copyright (C) 2001 Nullsoft, Inc.
** Author: Justin Frankel
** File: wac_network_http_server.h - JNL interface for doing HTTP GET/POST serving.
** License: see jnetlib.h
** This class just manages the http reply/sending, not where the data
** comes from, etc.
** for a mini-web server see wac_network_web_server.h
*/

#ifndef NULLSOFT_WAC_NETWORK_HTTP_SERVER_H
#define NULLSOFT_WAC_NETWORK_HTTP_SERVER_H

#include "headers.h"

#include "wac_network_http_server_api.h"

#include "wac_network_connection.h"

namespace wa
{
    namespace Components
    {
        class WAC_Network_HTTP_Server : public api_httpserv
        {
        public:
            WAC_Network_HTTP_Server( WAC_Network_Connection *con = NULL );
            ~WAC_Network_HTTP_Server();

            int         run(); // returns: < 0 on error, 0 on request not read yet, 1 if reading headers, 2 if reply not sent, 3 if reply sent, sending data. 4 on connection closed.

            const char *geterrorstr()                                 { return m_errstr; }

            // use these when state returned by run() is 2 
            const char *get_request_file();                                                                   // file portion of http request
            const char *get_request_parm( const char *parmname );                                             // parameter portion (after ?)
            const char *getallheaders()                               { return recvheaders.GetAllHeaders(); } // double null terminated, null delimited list
            const char *getheader( const char *headername );
            const char *get_method()                                  { return m_method; }

            void        set_reply_string( const char *reply_string );                                         // should be HTTP/1.1 OK or the like
            void        add_reply_header( const char *header );                                               // i.e. "content-size: 12345"

            void        send_reply()                                  { m_reply_ready = 1; }                  // send reply, state will advance to 3.

        ////////// sending data ///////////////
            int         bytes_inqueue()                               { if ( m_state == 3 || m_state == -1 || m_state == 4 ) return (int)m_con->send_bytes_in_queue(); else return 0; }
            int         bytes_cansend()                               { if ( m_state == 3 ) return (int)m_con->send_bytes_available(); else return 0; }
            void        write_bytes( char *bytes, int length )        { m_con->send( bytes, length ); }

            void        close( int quick )                            { m_con->close( quick ); m_state = 4; }

            api_connection *get_con()                                 { return m_con; }

            void        reset();                                                                              // prepare for another request on the same connection (HTTP/1.1)

            int         get_http_version()                            { return http_ver; }
            int         get_keep_alive()                              { return keep_alive; }

        protected:
            void        seterrstr( const char *str )                  { if ( m_errstr ) free( m_errstr ); m_errstr = _strdup( str ); }

            int             m_reply_ready;
            int             m_state;
            int             http_ver;
            int             keep_alive;

            char           *m_errstr;
            char           *m_reply_headers;
            char           *m_reply_string;
            JNL_Headers     recvheaders;
            char           *m_recv_request; // either double-null terminated, or may contain parameters after first null.
            char           *m_method;

            WAC_Network_Connection *m_con;

            RECVS_DISPATCH;
        };
    }
}

#endif  //!NULLSOFT_WAC_NETWORK_HTTP_SERVER_H
