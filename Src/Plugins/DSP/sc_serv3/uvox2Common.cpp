#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/types.h>
#endif

#include <string.h>
#include <stdio.h>
#include "uvox2Common.h"
#include "stl/stringUtils.h"
#include <sstream>
#include <iomanip>
#include <assert.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

// from wikipedia. Slightly modified to be 32/64 bit clean
static void XTEA_encipher(__uint32* v, __uint32* k, unsigned int num_rounds = 32) 
{
	__uint32 v0 = v[0], v1 = v[1], sum = 0;
	for (unsigned int i = 0; i < num_rounds; i++) 
	{
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
		sum += 0x9E3779B9;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}

static void XTEA_decipher(__uint32* v, __uint32* k, unsigned int num_rounds = 32) 
{
	__uint32 v0 = v[0], v1 = v[1], sum = 0x9E3779B9 * num_rounds;
	for (unsigned int i = 0; i < num_rounds; i++) 
	{
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
		sum -= 0x9E3779B9;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}
/////

static __uint32 fourCharsToLong(__uint8 *s)
{
	__uint32 l = 0;
	l |= s[0]; l <<= 8;
	l |= s[1]; l <<= 8;
	l |= s[2]; l <<= 8;
	l |= s[3];
	return l;
}

static void longToFourChars(__uint32 l, __uint8 *r)
{
	r[3] = l & 0xff; l >>= 8;
	r[2] = l & 0xff; l >>= 8;
	r[1] = l & 0xff; l >>= 8;
	r[0] = l & 0xff; l >>= 8;
}

#define XTEA_KEY_PAD 0
#define XTEA_DATA_PAD 0

string XTEA_encipher(const __uint8* c_data, const size_t c_data_cnt,
					 const __uint8* c_key, const size_t c_key_cnt) throw()
{
	vector<__uint8> key(c_key, c_key + c_key_cnt);
	vector<__uint8> data(c_data, c_data + c_data_cnt);

	// key is always 128 bits
	while (key.size() < 16)
	{
		key.push_back(XTEA_KEY_PAD); // pad key with zero
	}
	__uint32 k[4] = {fourCharsToLong(&key[0]), fourCharsToLong(&key[4]),
					 fourCharsToLong(&key[8]), fourCharsToLong(&key[12])};

	// data is multiple of 64 bits
	size_t siz = data.size();
	while (siz % 8)
	{
		++siz;
		data.push_back(XTEA_DATA_PAD);
	} // pad data with zero

	ostringstream oss;
	for (size_t x = 0; x < siz; x += 8)
	{
		__uint32 v[2] = {fourCharsToLong(&data[x]), fourCharsToLong(&data[x+4])};
		XTEA_encipher(v, k);
		oss << setw(8) << setfill('0') << hex << v[0];
		oss << setw(8) << setfill('0') << hex << v[1];
		// hex values. uvox uses colon as seperator so
		// we can't use chars for fear of collision
	}

	return oss.str();
}

utf8 XTEA_decipher(const __uint8* c_data, const size_t c_data_cnt,
				   const __uint8* c_key, const size_t c_key_cnt) throw()
{
	utf8 result;

	vector<__uint8> key(c_key, c_key + c_key_cnt);
	vector<__uint8> data(c_data, c_data + c_data_cnt);

	// key is always 128 bits
	while (key.size() < 16)
	{
		key.push_back(XTEA_KEY_PAD); // pad key with zero
	}
	__uint32 k[4] = {fourCharsToLong(&key[0]), fourCharsToLong(&key[4]),
					 fourCharsToLong(&key[8]), fourCharsToLong(&key[12])};

	// data is multiple of 16 hex digits
	size_t siz = data.size();
	//assert(!(siz % 16)); // should never happen if data is good
	while (siz % 16)
	{
		++siz;
		data.push_back('0');
	} // pad data with zero

	for (size_t x = 0; x < siz; x += 16)
	{
		__uint32 v[2];
		sscanf((const char *)&data[x], "%8x", &v[0]);
		sscanf((const char *)&data[x+8], "%8x", &v[1]);

		XTEA_decipher(v, k);
		__uint8 ur[5] = {0};
		longToFourChars(v[0], ur);
		result += ur;
		longToFourChars(v[1], ur);
		result += ur;
	}
	return result;
}

// take data and create a uvox message appended to "v". Limit by MAX_PAYLOAD_SIZE.
// return amount of data UNconsumed
int formMessage(const __uint8 *data, const int len, const int type, vector<__uint8> &v) throw(runtime_error)
{
	if (len > MAX_PAYLOAD_SIZE)
	{
		throw runtime_error(string(__FUNCTION__) + " message payload " + tos(len) + " is too big");
	}

	const int amt = min(len, MAX_PAYLOAD_SIZE);
	uv2xHdr hdr2 = {UVOX2_SYNC_BYTE, 0, (u_short)htons(type), (u_short)htons((u_short)amt)};
	v.insert(v.end(), (const __uint8 *)(&hdr2), ((const __uint8 *)(&hdr2)) + sizeof(hdr2));
	v.insert(v.end(), data, data + amt);
	v.push_back(UV2X_EOM);
	return (len - amt);
}

// similar to above, except it writes data into a buffer pointed to by v
int formMessage(const __uint8 *data, const int len, const int type, __uint8 *v) throw(runtime_error)
{
	if (len > MAX_PAYLOAD_SIZE)
	{
		throw runtime_error(string(__FUNCTION__) + " message payload " + tos(len) + " is too big");
	}

	const int amt = min(len, MAX_PAYLOAD_SIZE);
	uv2xHdr hdr2 = {UVOX2_SYNC_BYTE, 0, (u_short)htons(type), (u_short)htons((u_short)amt)};
	memcpy(v, &hdr2, sizeof(hdr2));
	v += sizeof(hdr2);
	memcpy(v, data, amt);
	v += amt;
	v[0] = UV2X_EOM;
	return (len - amt);
}

// load vector v up with metadata packets.
void createMetadataPackets(const __uint8 *data, const int _len, const int type,
						   vector<__uint8> &v, const __uint16 metadataID) throw(runtime_error)
{
	const int amtPerPacket = (MAX_PAYLOAD_SIZE - UV2X_META_HDR_SIZE);

	// subdivide and load
	__uint16 total_segments = (__uint16)(_len / amtPerPacket) + ((_len % amtPerPacket) ? 1 : 0), segment = 1;
	int len = _len;
	while (len > 0)
	{
		uv2xMetadataHdr h = {(u_short)htons(metadataID), (u_short)htons(total_segments), (u_short)htons(segment)};
		const int amt = min(len, amtPerPacket);
		vector<__uint8> m((const __uint8 *)&h, ((const __uint8 *)&h) + sizeof(h));
		m.insert(m.end(), data, data + amt);
		formMessage(&m[0], (int)m.size(), type, v);
		data += amt;
		len -= amt;
		++segment;
	}
}
