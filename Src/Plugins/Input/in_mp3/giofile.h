
/***************************************************************************\
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: giofile.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-02-11
 *   contents/description: HEADER - file I/O class for MPEG Decoder
 *
 *
\***************************************************************************/

#ifndef _GIOFILE_H
#define _GIOFILE_H

/* ------------------------ includes --------------------------------------*/
#include <windows.h>

#include "CVbriHeader.h"
#include "jnetlib/jnetlib.h"

#include "../vlb/dataio.h"
#include "ID3v2.h"
#include "LAMEInfo.h"
#include "../nu/RingBuffer.h"
#include "../apev2/tag.h"
#include "ifc_mpeg_stream_reader.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

class CGioFile : public DataIOControl, public ifc_mpeg_stream_reader
{
public:
	CGioFile();
	virtual ~CGioFile();

	int  Open(const wchar_t *pszName, int maxbufsizek);
	int  Close();
	int  Read(void *pBuffer, int cbToRead, int *pcbRead);
	int  Peek(void *pBuffer, int cbToRead, int *pcbRead);

	//dataiocontrol interface
	int IO(void *buf, int size, int count)
	{
		int l=0;
		Read(buf,count,&l);
		return l;
	}
	int Seek(long offset, int origin)
	{
		return 0;
	}
	//int Close() { return 0; }
	int EndOf(void)
	{
		return IsEof();
	}
	int DICGetLastError()
	{
		return DATA_IO_ERROR_NONE;
	}
	int DICGetDirection()
	{
		return DATA_IO_READ;
	}

	unsigned int GetAvgVBRBitrate(void)
	{
		if (m_vbr_ms && m_vbr_frames)
		{
			if (m_vbr_bytes && encodingMethod != ENCODING_METHOD_CBR)
				return (unsigned int)(m_vbr_bytes * 8 / m_vbr_ms);
			else
				return (unsigned int)(mpeg_length * 8 / m_vbr_ms);
		}
		return 0;
	}
	bool IsEof() const;
	bool lengthVerified;
	bool isSeekReset()
	{

		if (m_is_stream && m_seek_reset && m_is_stream_seek)
		{
			m_seek_reset = false;
			m_is_stream_seek = false;
			return true;
		}
		else
			return false;
	}

	bool IsStreamSeekable()
	{
		return m_is_stream_seekable;
	}

	int GetHeaderOffset();

	int PercentAvailable()
	{
		if (!m_is_stream) return 0;
		if (!recvbuffersize) return 0;
		if (!m_connection) return 0;
		if (constate != 5) return 0;
		uint64_t bytes_100 = jnl_connection_receive_bytes_available(m_connection)*100;
		bytes_100 /= recvbuffersize;
		return (int)bytes_100;
	}

	int RunStream()
	{
		if (m_is_stream && m_connection)
		{
			if (constate != 5) Read(NULL,0,NULL);
			else jnl_connection_run(m_connection, -1, -1, NULL, NULL);
			int p=jnl_connection_get_state(m_connection);
			if (p==JNL_CONNECTION_STATE_ERROR||
			    p==JNL_CONNECTION_STATE_CLOSED)
				return 2;
			return 1;
		}
		return 0;
	}
	int IsStream()
	{
		return m_is_stream;
	}
	void GetStreamInfo(wchar_t *, size_t len);

	DWORD GetContentLength(void)   const;
	DWORD GetCurrentPosition(void) const;
	void SetCurrentPosition(long dwPos, int How);

	enum { GIO_FILE_BEGIN, GIO_FILE_CURRENT, GIO_FILE_END };

	uint64_t mpeg_length; /* length of valid audio data */
	uint64_t mpeg_position; /* starting position of first valid decodable non-header MPEG frame */
	uint64_t file_position; /* position within the MPEG data that we've read so far */

	uint64_t m_vbr_bytes;
	int m_vbr_frames, m_vbr_ms;
	int encodingMethod;
	uint64_t m_vbr_samples;
	int prepad, postpad;
	void Seek(int posms, int br);
	bool IsSeekable();

	unsigned char id3v1_data[128];

	char stream_url[256];
	char stream_name[256];
	char stream_genre[256];
	char stream_current_title[256];

	char *m_content_type;
	int uvox_last_message;
	char *uvox_3901;
	char *uvox_3902;

	typedef struct {
		char *uvox_stream_artwork;
		int uvox_stream_artwork_len;
		int uvox_stream_artwork_type;
		char *uvox_playing_artwork;
		int uvox_playing_artwork_len;
		int uvox_playing_artwork_type;
	} UVOX_ARTWORK;
	UVOX_ARTWORK uvox_artwork;

	ID3v2 info;
	float GetGain();
	unsigned char m_vbr_toc[100];
	int m_vbr_frame_len;
	int m_vbr_flag;
	CVbriHeader *m_vbr_hdr;

	FILETIME last_write_time;

	void *GetID3v1()
	{
		if (m_id3v1_len == 128)
			return id3v1_data;
		else
			return 0;
	}

	void *GetID3v2(uint32_t *len)
	{
		if (stream_id3v2_buf)
		{
			*len = stream_id3v2_read;
			return stream_id3v2_buf;
		}
		else
			return 0;
	}

	void *GetLyrics3(uint32_t *len)
	{
		if (lyrics3_data)
		{
			*len = lyrics3_size;
			return lyrics3_data;
		}
		else
			return 0;
	}

	void *GetAPEv2(uint32_t *len)
	{
		if (apev2_data)
		{
			*len = m_apev2_len;
			return apev2_data;
		}
		return 0;
	}

protected:
	void ReadiTunesGaps();
private:
	/* ID3v2 */
	int m_id3v1_len;

	/* ID3v2 */
	uint32_t stream_id3v2_read;
	char *stream_id3v2_buf;
	int m_id3v2_len;

	/* Lyrics3 */
	uint32_t lyrics3_size;
	char *lyrics3_data;

	/* APEv2 */
	uint32_t m_apev2_len;
	char *apev2_data;
	APEv2::Tag apev2;

	int m_is_stream;
	jnl_dns_t m_dns;
	jnl_connection_t m_connection;
	char last_full_url[4096];
	int is_stream_seek;
	char *host;
	char *proxy_host;
	char *req;
	unsigned short port;
	char *lpinfo;
	char *proxy_lp;
	char *request;
	int constate;
	char save_filename[256];
	char server_name[128];
	int recvbuffersize;
	
	int64_t stream_bytes_read;
	int64_t stream_metabytes_read;
	unsigned int timeout_start;
	char force_lpinfo[256];
	unsigned char *m_full_buffer;
	int is_uvox;
	int uvox_stream_data_len;
	int uvox_message_cnt, uvox_desyncs;
	int uvox_sid,uvox_maxbr, uvox_avgbr,uvox_maxmsg;

	unsigned char *uvox_stream_data;

	char *uvox_meta[2][32];
	int meta_interval,meta_pos;
	uint64_t m_full_buffer_pos/*,m_full_buffer_len*/;
	char stream_title_save[580];
	char last_title_sent[256];

	RingBuffer peekBuffer;
	//unsigned char m_peekbuf[8192];
	//int m_peekbuf_used;
	bool no_peek_hack;

	int doConnect(const char *str, int start_offset);
	void processMetaData(char *data, int lent, int msgId = 0);

	int m_redircnt;
	int m_auth_tries;

	HANDLE hFile;
	bool   fEof;
	

	char dlg_realm[256];
	static INT_PTR CALLBACK httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	int m_http_response;
	bool m_seek_reset;
	bool m_is_stream_seek;
	bool m_is_stream_seekable;
	bool m_useaproxy;
	RECVS_DISPATCH;
};

/*-------------------------------------------------------------------------*/
#endif
