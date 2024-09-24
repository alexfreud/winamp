#pragma once
#ifndef uvox2Common_H_
#define uvox2Common_H_

#include "unicode/uniString.h"

std::string XTEA_encipher(const __uint8* c_data, const size_t c_data_cnt, const __uint8* c_key, const size_t c_key_cnt) throw();
uniString::utf8 XTEA_decipher(const __uint8* c_data, const size_t c_data_cnt, const __uint8* c_key, const size_t c_key_cnt) throw();

#pragma pack(push,1)

struct uv2xHdr
{ // uvox2 message
	__uint8  sync;
	__uint8  qos;
	__uint16 msgType;
	__uint16 msgLen; 
};

struct uv2xMetadataHdr 
{   /* uvox 2 metadata header */
	__uint16 id;   /* ID (cookie) identifying a metadata package */
	__uint16 span; /* Span of messages in the metadata package being assembled */
	__uint16 index;/* Index of the message in the metadata package being assembled */
};

#pragma pack(pop)

static const int MAX_MESSAGE_SIZE = (16 * 1024);
static const int MAX_CIPHER_KEY_SIZE = 256;

static const char UV2X_EOM = 0;
static const int UV2X_HDR_SIZE = sizeof(uv2xHdr);
static const int UV2X_OVERHEAD = (UV2X_HDR_SIZE + 1);  /* header+end_of_msg */
static const int UV2X_META_HDR_SIZE = sizeof(uv2xMetadataHdr);
static const int MAX_PAYLOAD_SIZE = MAX_MESSAGE_SIZE - UV2X_OVERHEAD;

#define UVOX2_SYNC_BYTE 0X5A

static const int MSG_AUTH					= 0x1001;
static const int MSG_BROADCAST_SETUP		= 0x1002;
static const int MSG_NEGOTIATE_BUFFER_SIZE	= 0x1003;
static const int MSG_STANDBY				= 0x1004;
static const int MSG_TERMINATE				= 0x1005;
static const int MSG_FLUSH_CACHED_METADATA	= 0x1006;
static const int MSG_LISTENER_AUTHENTICATION= 0x1007;
static const int MSG_MAX_PAYLOAD_SIZE		= 0x1008;
static const int MSG_CIPHER					= 0x1009; // cipher request for uvox 2.1
static const int MSG_MIME_TYPE				= 0x1040;
static const int MSG_FILE_TRANSFER_BEGIN	= 0x1050;
static const int MSG_FILE_TRANSFER_DATA		= 0x1051;

static const int MSG_BROADCAST_INTERRUPTION	= 0x2001;
static const int MSG_BROADCAST_TERMINATE	= 0x2002;

static const int MSG_ICYNAME				= 0x1100;
static const int MSG_ICYGENRE				= 0x1101;
static const int MSG_ICYURL					= 0x1102;
static const int MSG_ICYPUB					= 0x1103;

static const int MSG_METADATA_CONTENTINFO	= 0x3000;
static const int MSG_METADATA_URL			= 0x3001;
//static const int MSG_METADATA_XML			= 0x3901;
static const int MSG_METADATA_XML_NEW		= 0x3902;

// only id the start of the album art type as it's variable
static const int MSG_METADATA_ALBUMART		= 0x4000;
static const int MSG_METADATA_STATION_ART	= 0x0000;
static const int MSG_METADATA_PLAYING_ART	= 0x0100;
/*
	0x4    0x0xx    Station logo
	0x4    0x1xx    Album art

	00 = image/jpeg
	01 = image/png
	02 = image/bmp
	03 = image/gif
*/

static const int MSG_METADATA_TIMEREMAINING	= 0x5001;

static const int MP3_DATA					= 0x7000;
static const int VLB_DATA					= 0x8000;
static const int AAC_LC_DATA				= 0x8001;
static const int AACP_DATA					= 0x8003;
static const int OGG_DATA					= 0x8004;

/// these are the same
static const int MAX_METADATA_SEGMENTS		= 32;
#define MAX_METADATA_FRAGMENTS 32
///////

static const int MAX_METADATA_TIME(300); // five minutes

// take data and create a uvox message appended to "v". Limit by MAX_PAYLOAD_SIZE.
// return amount of data UNconsumed
int formMessage(const __uint8 *data, const int len, const int type, std::vector<__uint8> &v) throw(std::runtime_error);

// similar to above, except it writes data into a buffer pointed to by v
int formMessage(const __uint8 *data, const int len, const int type, __uint8 *v) throw(std::runtime_error);

// this one also returns actual full message size in msgSize
inline int formMessage(const std::string &dataIn, const int type, __uint8 *v, int &msgSize) throw(std::runtime_error)
{ 
	msgSize = (int)(dataIn.size() + 1); // include null

	int amt_left = formMessage((const __uint8 *)dataIn.c_str(), msgSize /* include null */, type, v); 
	msgSize += UV2X_OVERHEAD;
	return amt_left;
}

// load vector v up with metadata packets.
void createMetadataPackets(const __uint8 *data, const int len, const int type,
						   std::vector<__uint8> &v, const __uint16 metadataID = 1) throw(std::runtime_error);

#endif
