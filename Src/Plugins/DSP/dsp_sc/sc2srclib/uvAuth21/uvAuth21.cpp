#include "uvAuth21.h"

uvAuth21::uvAuth21(void) {
}

uvAuth21::~uvAuth21(void) {
}

string uvAuth21::encrypt(string user,string pass,string key) {
    string blob;
    uint8_t* username  = (uint8_t*)user.data();
    uint8_t* password  = (uint8_t*)pass.data();
    uint8_t* cipherkey =  (uint8_t*)key.data();
	blob = XTEA_encipher(username,user.length(),cipherkey,key.length());
    blob += ':';
	blob += XTEA_encipher(password,pass.length(),cipherkey,key.length());
    return blob;
}

const char * uvAuth21::encrypt(char * user,char * pass,char * key) {
    static string blob;
    uint8_t* username  = (uint8_t*)user;
    uint8_t* password  = (uint8_t*)pass;
    uint8_t* cipherkey =  (uint8_t*)key;
	blob = XTEA_encipher(username,sizeof(user),cipherkey,sizeof(key));
    blob += ':';
	blob += XTEA_encipher(password,sizeof(pass),cipherkey,sizeof(key));
	return blob.c_str();
}

// from wikipedia. Slightly modified to be 32/64 bit clean
void uvAuth21::XTEA_encipher(__uint32* v, __uint32* k) {
	unsigned int num_rounds = 32;
    __uint32 v0=v[0], v1=v[1], i;
    __uint32 sum=0, delta=0x9E3779B9;
    for(i=0; i<num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum>>11) & 3]);
	}
    v[0]=v0; v[1]=v1;
}

__uint32 uvAuth21::fourCharsToLong(__uint8 *s) {
	__uint32 l = 0;
	l |= s[0]; l <<= 8;
	l |= s[1]; l <<= 8;
	l |= s[2]; l <<= 8;
	l |= s[3];
	return l;
}

 std::vector<std::string> &uvAuth21::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> uvAuth21::dosplit(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::string uvAuth21::XTEA_encipher(const __uint8* c_data,size_t c_data_cnt, const __uint8* c_key,size_t c_key_cnt)	{
	std::ostringstream oss;
	std::vector<__uint8> key(c_key,c_key + c_key_cnt);
	std::vector<__uint8> data(c_data,c_data + c_data_cnt);

	// key is always 128 bits
	while(key.size() < 16) key.push_back(XTEA_KEY_PAD); // pad key with zero
	__uint32 k[4] = { fourCharsToLong(&key[0]),fourCharsToLong(&key[4]),fourCharsToLong(&key[8]),fourCharsToLong(&key[12])};

	// data is multiple of 64 bits
	size_t siz = data.size();
	while(siz % 8) { siz+=1; data.push_back(XTEA_DATA_PAD);} // pad data with zero

	for(size_t x = 0; x < siz; x+=8) {
		__uint32 v[2];
		v[0] = fourCharsToLong(&data[x]);
		v[1] = fourCharsToLong(&data[x+4]);
		XTEA_encipher(v,k);
		oss << setw(8) << setfill('0') << hex << v[0];
		oss << setw(8) << setfill('0') << hex << v[1]; // hex values. uvox uses colon as seperator so we can't use chars for
										// fear of collision
	}
	return oss.str();
}