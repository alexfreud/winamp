#pragma once
#include "../nsavi/avi_reader.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"

class AVIReaderHTTP : public nsavi::avi_reader
{
public:
	AVIReaderHTTP(HANDLE killswitch, HANDLE seek_event);
	int Open(const wchar_t *url);
	void Close();
	int Connect();
	int Buffer();

	/* avi_reader implementation */
	int Read(void *buffer, uint32_t read_length, uint32_t *bytes_read);
	int Peek(void *buffer, uint32_t read_length, uint32_t *bytes_read);
	//void OverlappedHint(uint32_t read_length);
	int Seek(uint64_t position);
	uint64_t Tell();
	int Skip(uint32_t skip_bytes);
	uint64_t GetContentLength();
		void GetFilename(wchar_t *fn, size_t len);
private:
	/* internal methods */
	int Open(const char *url, uint64_t start_offset=0);
private:
	api_httpreceiver *http;
	uint64_t position;
	bool seekable;
	HANDLE handles[2];
};