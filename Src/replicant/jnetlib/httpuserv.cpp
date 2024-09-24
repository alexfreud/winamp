/*
** JNetLib
** Copyright (C) 2012 Nullsoft, Inc.
** Author: Ben Allison
** File: httpuserv.cpp - JNL HTTPU (HTTP over UDP) serving implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "httpuserv.h"

#include "foundation/error.h"

/*
States for m_state:
-1 error (connection closed, etc)
0 not read request yet.
1 reading headers
2 headers read, have not sent reply
3 sent reply
4 closed
*/

JNL_HTTPUServ::JNL_HTTPUServ()
{
	m_reply_headers=0;
	m_reply_string=0;
	m_recv_request=0;
	m_errstr=0;
	m_reply_ready=0;
	m_method = 0;
	http_ver = 0;
}

JNL_HTTPUServ::~JNL_HTTPUServ()
{
	free(m_recv_request);
	free(m_reply_string);
	free(m_reply_headers);
	free(m_errstr);
	free(m_method);
}

static size_t strlen_whitespace(const char *str)
{
	size_t size=0;
	while (str && *str && *str != ' ' && *str != '\r' && *str!='\n')
	{
		str++;
		size++;
	}
	return size;
}

int JNL_HTTPUServ::process(JNL_UDPConnection *m_con)
{ // returns: < 0 on error, 0 on connection close, 1 if reading request, 2 if reply not sent, 3 if reply sent, sending data.

	reset();
	if (m_con->get_state()==JNL_CONNECTION_STATE_ERROR)
	{
		seterrstr(m_con->get_errstr());
		return -1;
	}

	if (m_con->get_state()==JNL_CONNECTION_STATE_CLOSED) 
		return 4;

	if (m_con->recv_lines_available()>0)
	{
		char *buf=(char*)malloc(m_con->recv_bytes_available()-1);
		m_con->recv_line(buf,m_con->recv_bytes_available()-1);
		free(m_recv_request);
		m_recv_request=(char*)malloc(strlen(buf)+2);
		strcpy(m_recv_request,buf);
		m_recv_request[strlen(m_recv_request)+1]=0;
		free(buf);
		buf=m_recv_request;
		while (buf && *buf) buf++;
		while (buf >= m_recv_request && *buf != ' ') buf--;
		if (strncmp(buf+1,"HTTP",4))// || strncmp(m_recv_request,"GET ",3))
		{
			seterrstr("malformed HTTP request");
		}
		else
		{
			http_ver = atoi(buf+8);

			size_t method_len = strlen_whitespace(m_recv_request);
			m_method = (char *)malloc(method_len + 1);
			if (m_method)
			{
				memcpy(m_method, m_recv_request, method_len);
				m_method[method_len]=0;
			}
			else
			{
				seterrstr("malformed HTTP request");
				return -1;
			}

			if (buf >= m_recv_request) buf[0]=buf[1]=0;

			buf=strstr(m_recv_request,"?");
			if (buf)
			{
				*buf++=0; // change &'s into 0s now.
				char *t=buf;
				int stat=1;
				while (t && *t) 
				{
					if (*t == '&' && !stat) { stat=1; *t=0; }
					else stat=0;
					t++;
				}
			}
		}
	}
	else
	{
		seterrstr("malformed HTTP request");
		return -1;
	}

	while (m_con->recv_lines_available()>0)
	{
		char buf[8192] = {0};
		m_con->recv_line(buf, 8192);
		if (!buf[0]) 
			break; 

		recvheaders.Add(buf);
	}


	return NErr_Success;
}

void JNL_HTTPUServ::send_reply(JNL_UDPConnection *m_con)
{
	m_con->send_string((char*)(m_reply_string?m_reply_string:"HTTP/1.1 200 OK"));
	m_con->send_string("\r\n");
	if (m_reply_headers) m_con->send_string(m_reply_headers);
	m_con->send_string("\r\n");

}

const char *JNL_HTTPUServ::get_request_uri()
{
	// file portion of http request
	if (!m_recv_request) return NULL;
	char *t=m_recv_request;
	while (t && *t && *t != ' ') t++;
	if (!t || !*t) return NULL;
	while (t && *t && *t == ' ') t++;
	return t;
}

const char *JNL_HTTPUServ::get_request_parm(const char *parmname) // parameter portion (after ?)
{
	const char *t=m_recv_request;
	while (*t) t++;
	t++;
	while (t && *t)
	{
		while (t && *t && *t == '&') t++;
		if (!strncasecmp(t,parmname,strlen(parmname)) && t[strlen(parmname)] == '=')
		{
			return t+strlen(parmname)+1;
		}
		t+=strlen(t)+1;
	}
	return NULL;
}

const char *JNL_HTTPUServ::getheader(const char *headername)
{
	return recvheaders.GetHeader(headername);	
}

void JNL_HTTPUServ::set_reply_string(const char *reply_string) // should be HTTP/1.1 OK or the like
{
	free(m_reply_string);
	m_reply_string=(char*)malloc(strlen(reply_string)+1);
	strcpy(m_reply_string,reply_string);
}

void JNL_HTTPUServ::set_reply_header(const char *header) // "Connection: close" for example
{
	if (m_reply_headers)
	{
		char *tmp=(char*)malloc(strlen(m_reply_headers)+strlen(header)+3);
		strcpy(tmp,m_reply_headers);
		strcat(tmp,header);
		strcat(tmp,"\r\n");
		free(m_reply_headers);
		m_reply_headers=tmp;
	}
	else
	{
		m_reply_headers=(char*)malloc(strlen(header)+3);
		strcpy(m_reply_headers,header);
		strcat(m_reply_headers,"\r\n");
	}
}

void JNL_HTTPUServ::reset()
{
	free(m_recv_request); m_recv_request = 0;
	free(m_reply_string); m_reply_string = 0;
	free(m_reply_headers); m_reply_headers = 0;
	free(m_errstr); m_errstr = 0;
	free(m_method); m_method =0;
	recvheaders.Reset();
	m_reply_ready=0;
}
