#pragma once

#include <stdio.h>
#include "../replicant\foundation\win-x86\types.h"
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <assert.h>

#define XTEA_KEY_PAD 0
#define XTEA_DATA_PAD 0
#define __uint32 uint32_t
#define __uint8 uint8_t

using namespace std;

class uvAuth21 {
public:
	uvAuth21(void);
	string encrypt(string user, string password, string key);
	const char * encrypt(char * user,char * password,char * key);
	~uvAuth21(void);
private:

protected:
	static void XTEA_encipher(__uint32* v, __uint32* k);
	static __uint32 fourCharsToLong(__uint8 *s);
	static string XTEA_encipher(const __uint8* c_data, size_t c_data_cnt, const __uint8* c_key, size_t c_key_cnt);
	static std::vector<std::string> dosplit(const std::string &s, char delim);
	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
};