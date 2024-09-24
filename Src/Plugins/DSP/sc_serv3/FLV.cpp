#include "FLV.h"
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "streamData.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

/*
** This is a bit messy but it'll generate
** FLV tags for the purpose of streaming.
**
** Where possible values are hard-coded
** so we just re-insert into the output.
*/

#define FLV_SIGNATURE       {'F', 'L', 'V'}
#define FLV_VERSION         ((__uint8)0x1)

#define FLV_FLAG_VIDEO      ((__uint8)0x1)
#define FLV_FLAG_AUDIO      ((__uint8)0x4)

#define FLV_HDR_SIZE		((__uint32)0x9000000)	// pre-converted to big-endian

// ensure we've got the structures correctly packed
#pragma pack(push, 1)
typedef struct {
	__uint8		signature[3];	// FLV_SIGNATURE
    __uint8		version;		// FLV_VERSION
    __uint8		flags;			// FLV_FLAG_*
	__uint32	header_size;	// FLV_HDR_SIZE
	__uint32	prev_size;		// 0 (this is the size of the previous tag including
								//    its header in bytes i.e. 11 + the 'data_size'
								//    of the previous tag). we include this in the
								//    header so that we're always outputting a valid
								//    start point before the tag structure is added.
} flv_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	__uint8		type;			// for first packet set to AMF Metadata ???
	__uint8		data_size[3];	// size of packet data only (24)
	__uint8		ts_lower[3];	// for first packet set to NULL (24)
	__uint8		ts_upper;		// extension to create a __uint32 value
	__uint8		stream_id[3];	// for first stream of same type set to NULL (24)
	/* data comes after this*/
} flv_tag;
#pragma pack(pop)

/*
__uint32	prev_size;		// for first packet set to NULL otherwise this is the
							// size of the previous tag including its header in B
							// and is 11 plus the 'data_size' of the previous tag.
*/

// these are pre-encoded for the correct endianess
// though for metadata usage, we convert them back
#define FLV_MP3_AUDIO		((__uint8)0x20)		// (((__uint8)0x2) << 4)
#define FLV_AAC_AUDIO		((__uint8)0xA0)		// (((__uint8)0xA) << 4)

#define FLV_SAMPLE_RATE_5	((__uint8)0x0)		// (((__uint8)0x0) << 2)
#define FLV_SAMPLE_RATE_11	((__uint8)0x4)		// (((__uint8)0x1) << 2)
#define FLV_SAMPLE_RATE_22	((__uint8)0x8)		// (((__uint8)0x2) << 2)
#define FLV_SAMPLE_RATE_44	((__uint8)0xC)		// (((__uint8)0x3) << 2)

#define FLV_SAMPLE_SIZE_8	((__uint8)0x0)		// (((__uint8)0x0) << 1)
#define FLV_SAMPLE_SIZE_16	((__uint8)0x2)		// (((__uint8)0x1) << 1)

#define FLV_MONO_AUDIO		((__uint8)0x0)
#define FLV_STEREO_AUDIO	((__uint8)0x1)


// reads 24 bits from data, converts from big endian, and returns as a 32bit int
inline __uint32 Read24(__uint8* data)
{
	__uint32 returnVal = 0; 
	__uint8* swap = (__uint8*)&returnVal;

	swap[0] = data[2];
	swap[1] = data[1];
	swap[2] = data[0];

	return returnVal;
}

int dataString(vector<__uint8> &out_data, const char *buf)
{
	int amt = (int)strlen(buf);
	__uint8 bytes[2] = {(__uint8)((amt >> 8) & 0xff), (__uint8)((amt >> 0) & 0xff)};
	// length of the data string
	out_data.insert(out_data.end(), (const __uint8*)&bytes, (const __uint8*)&bytes + sizeof(bytes));
	// body of the data string
	out_data.insert(out_data.end(), (const __uint8*)buf, (const __uint8*)buf + amt);
	return amt + 2;
}

int scriptDataType(vector<__uint8> &out_data, const __uint8 type)
{
	out_data.insert(out_data.end(), (const __uint8*)&type, ((const __uint8*)&type) + sizeof(type));
	return 1;
}

int scriptDataString(vector<__uint8> &out_data, const char *name, const char *value)
{
	return dataString(out_data, name) + scriptDataType(out_data, 0x2) + dataString(out_data, value);
}

int scriptDataBool(vector<__uint8> &out_data, const char *name, const bool value)
{
	return dataString(out_data, name) + scriptDataType(out_data, 0x1) + scriptDataType(out_data, !!value);
}

int scriptDataDouble(vector<__uint8> &out_data, const char *name, const double value)
{
	int amt = dataString(out_data, name) + scriptDataType(out_data, 0x0);

	union
	{
		__uint8 dc[8];
		double dd;
	} d;
	d.dd = value;

	unsigned char b[8];
	b[0] = d.dc[7];
	b[1] = d.dc[6];
	b[2] = d.dc[5];
	b[3] = d.dc[4];
	b[4] = d.dc[3];
	b[5] = d.dc[2];
	b[6] = d.dc[1];
	b[7] = d.dc[0];

	out_data.insert(out_data.end(), (const __uint8*)b, (const __uint8*)b + sizeof(b));

	return amt + 8;
}

void createMetadataTag(vector<__uint8> &out_data, const bool mp3,
					   const bool mono, const int bitrate,
					   const __uint8 flv_sr, const streamData::streamID_t sid)
{
	__uint8 m[] = {'\x12', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'};
	out_data.insert(out_data.end(), (const __uint8*)m, (const __uint8*)m + sizeof(m));

	__uint32 s = scriptDataType(out_data, 0x2) +
				 dataString(out_data, "onMetadata") +
				 // start of the script array data blocks
				 scriptDataType(out_data, 0x8);

	__uint8 data_size[4] = {'\x00', '\x00', '\x00', '\x0C'/* number of items in the array */};
	out_data.insert(out_data.end(), (const __uint8*)&data_size, ((const __uint8*)&data_size) + sizeof(data_size));
	s += 4;

	streamData::streamInfo info;
	streamData::extraInfo extra;
	streamData::getStreamInfo(sid, info, extra);

	s += scriptDataString(out_data, "name", (!info.m_streamName.empty() ? info.m_streamName.hideAsString().c_str() : ""));
	// TODO - as we're not updating as we go at the moment, this will be set to the station name so there's something set
	s += scriptDataString(out_data, "title", (!info.m_streamName.empty() ? info.m_streamName.hideAsString().c_str() : ""));
	//s += scriptDataString(out_data, "title", (!info.m_currentSong.empty() ? info.m_currentSong.hideAsString().c_str() : ""));
	s += scriptDataString(out_data, "genre", (!info.m_streamGenre[0].empty() ? info.m_streamGenre[0].hideAsString().c_str() : ""));

	//s += scriptDataBool(out_data, "hasMetadata", false/* TODO */);
	s += scriptDataBool(out_data, "hasAudio", true);
	s += scriptDataBool(out_data, "hasVideo", false);
	s += scriptDataBool(out_data, "hasKeyframes", false);

	s += scriptDataBool(out_data, "canSeekToEnd", false);

	s += scriptDataBool(out_data, "stereo", !mono);

	s += scriptDataDouble(out_data, "audiodatarate", (double)bitrate);
	s += scriptDataDouble(out_data, "audiocodecid", (double)((mp3 ? FLV_MP3_AUDIO : FLV_AAC_AUDIO) >> 4));
	s += scriptDataDouble(out_data, "audiosamplerate", (double)(flv_sr >> 2));
	s += scriptDataDouble(out_data, "audiosamplesize", (double)(FLV_SAMPLE_SIZE_16 >> 1));

	unsigned char end[] = {'\x00', '\x00', '\x09'};
	out_data.insert(out_data.end(), (const __uint8*)end, (const __uint8*)end + sizeof(end));
	s += 3;

	// now we know how much we've got
	// we can update the tags' length
	__uint32 s2 = s + 11;
	out_data[16] = (s & 0xFF);
	s >>= 8;
	out_data[15] = (s & 0xFF);
	s >>= 8;
	out_data[14] = (s & 0xFF);

	// we set this at the end so if we terminate the stream
	// then there's more chance of it validating correctly.
	data_size[3] = (s2 & 0xFF);
	s2 >>= 8;
	data_size[2] = (s2 & 0xFF);
	s2 >>= 8;
	data_size[1] = (s2 & 0xFF);
	s2 >>= 8;
	data_size[0] = (s2 & 0xFF);

	out_data.insert(out_data.end(), (const __uint8*)&data_size, ((const __uint8*)&data_size) + sizeof(data_size));
}

void createFLVTag(vector<__uint8> &out_data, const char *buf,
				  const int amt, int &timestamp, const bool mp3,
				  const bool mono, const unsigned int samplerate,
				  const int bitrate, const __uint8 *asc_header,
				  const streamData::streamID_t sid)
{
	if (amt > 0)
	{
		// we need to generate this early so we've got
		// it for being provided in createMetadataTag
		__uint8 flv_sr = FLV_SAMPLE_RATE_44;
		if (mp3)
		{
			// how do we handle the formats not allowed...?
			switch (samplerate)
			{
				case 22050:
				{
					flv_sr = FLV_SAMPLE_RATE_22;
					break;
				}
				case 11025:
				{
					flv_sr = FLV_SAMPLE_RATE_11;
					break;
				}
			}
		}

		const bool first = (timestamp == 0);
		if (first)
		{
			const flv_header hdr = {FLV_SIGNATURE, FLV_VERSION, FLV_FLAG_AUDIO, FLV_HDR_SIZE, 0};
			out_data.insert(out_data.end(), (const __uint8*)&hdr, ((const __uint8*)&hdr) + sizeof(flv_header));

			createMetadataTag(out_data, mp3, mono, bitrate, flv_sr, sid);

			if (!mp3)
			{
				// we send a simple frame at this point just so for AAC the decoder
				// is able to be setup correctly as needed else it'll fail to play.
				__uint8 p[] = {'\x08', '\x00', '\x00', '\x04', '\x00', '\x00', '\x00', '\x00',
							   '\x00', '\x00', '\x00', (__uint8)'\xAF', '\x00', asc_header[0],
							   asc_header[1], '\x00', '\x00', '\x00', '\x0F'};
				out_data.insert(out_data.end(), (const __uint8*)p, (const __uint8*)p + sizeof(p));
			}
		}

		// if we were to do something else, then we'd need to
		// change the initial value as needed for it's format
		flv_tag tag = {((__uint8)0x8), {0}, {0}, 0, {0}};
		// we need to know the size of things before we output
		// the actual frame, so we calculate now and adjust it
		// based on the format of the frame needing to be sent
		__uint32 v = (first ? (!mp3 ? 2 : 1) : 1 + !mp3) + amt;

		tag.data_size[2] = (v & 0xFF);
		v >>= 8;
		tag.data_size[1] = (v & 0xFF);
		v >>= 8;
		tag.data_size[0] = (v & 0xFF);

		// this sets the 24-bit time
		v = timestamp;
		tag.ts_lower[2] = (v & 0xFF);
		v >>= 8;
		tag.ts_lower[1] = (v & 0xFF);
		v >>= 8;
		tag.ts_lower[0] = (v & 0xFF);
		// this sets the extended time
		// so we provide a 32-bit time
		v >>= 8;
		tag.ts_upper = (v & 0xFF);

		// depending on the format, we adjust timestamp
		// for MP3, we're looking at 26ms / frame
		// for AAC, we're looking at 1024 samples / frame
		timestamp += (mp3 ? 26 : (1024000 / samplerate));

		out_data.insert(out_data.end(), (const __uint8*)&tag, ((const __uint8*)&tag) + sizeof(flv_tag));

		// for AAC data, the default has to be set as 16-bit 44kHZ stereo
		// though the decoder will actuall figure things out as required.
		// for MP3 data, we fill in things based on the frame data found
		__uint8 flv_audio_data_tag = ((mp3 ? FLV_MP3_AUDIO : FLV_AAC_AUDIO) | flv_sr | FLV_SAMPLE_SIZE_16 | (!mono ? FLV_STEREO_AUDIO : FLV_MONO_AUDIO));
		out_data.insert(out_data.end(), (const __uint8*)&flv_audio_data_tag, ((const __uint8*)&flv_audio_data_tag) + sizeof(flv_audio_data_tag));

		if (!mp3)
		{
			// this is done so we now distinguish the actual
			// raw AAC data sans ADTS header vs the required
			// AAC sequence header which is sent before the
			// first AAC data frame is sent
			__uint8 packet_type = 0x1;
			out_data.insert(out_data.end(), (const __uint8*)&packet_type, ((const __uint8*)&packet_type) + sizeof(packet_type));
		}

		// body of the data
		out_data.insert(out_data.end(), (const __uint8*)buf, ((const __uint8*)buf) + amt);

		// we set this at the end so if we terminate the stream
		// then there's more chance of it validating correctly.
		v = (11 + Read24(tag.data_size));
		__uint8 data_size[4] = {0};
		data_size[3] = (v & 0xFF);
		v >>= 8;
		data_size[2] = (v & 0xFF);
		v >>= 8;
		data_size[1] = (v & 0xFF);
		v >>= 8;
		data_size[0] = (v & 0xFF);

		out_data.insert(out_data.end(), (const __uint8*)&data_size, ((const __uint8*)&data_size) + sizeof(data_size));
	}
}
