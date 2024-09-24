/*
*
*
* Copyright (c) 2004 Samuel Wood (sam.wood@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*
*/


// For more information on how all this stuff works, see:
// http://www.ipodlinux.org/ITunesDB



// iPodDB.cpp: implementation of the iPod classes.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4786)

#include "iPodDB.h"
#include <bfc/platform/types.h>
#include <assert.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <vector>

/*
#ifdef ASSERT
#undef ASSERT
#define ASSERT(x) {}
#endif
*/
//#define IPODDB_PROFILER		// Uncomment to enable profiler measurments

#ifdef IPODDB_PROFILER
/* profiler code from Foobar2000's PFC library:
*
*  Copyright (c) 2001-2003, Peter Pawlowski
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
*	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
*	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
*	Neither the name of the author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
class profiler_static
{
private:
	const char * name;
	__int64 total_time,num_called;

public:
	profiler_static(const char * p_name)
	{
		name = p_name;
		total_time = 0;
		num_called = 0;
	}
	~profiler_static()
	{
		char blah[512] = {0};
		char total_time_text[128] = {0};
		char num_text[128] = {0};
		_i64toa(total_time,total_time_text,10);
		_i64toa(num_called,num_text,10);
		_snprintf(blah, sizeof(blah), "profiler: %s - %s cycles (executed %s times)\n",name,total_time_text,num_text);
		OutputDebugStringA(blah);
	}
	void add_time(__int64 delta) {total_time+=delta;num_called++;}
};

class profiler_local
{
private:
	static __int64 get_timestamp();
	__int64 start;
	profiler_static * owner;
public:
	profiler_local(profiler_static * p_owner)
	{
		owner = p_owner;
		start = get_timestamp();
	}
	~profiler_local()
	{
		__int64 end = get_timestamp();
		owner->add_time(end-start);
	}

};

__declspec(naked) __int64 profiler_local::get_timestamp()
{
	__asm
	{
		rdtsc
			ret
	}
}


#define profiler(name) \
	static profiler_static profiler_static_##name(#name); \
	profiler_local profiler_local_##name(&profiler_static_##name);

#endif


#ifdef _DEBUG
#define MYDEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
//allocations to be of _CLIENT_BLOCK type

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new MYDEBUG_NEW
#endif


//
// useful functions
//////////////////////////////////////////////////////////////////////
inline BOOL WINAPI IsCharSpaceW(wchar_t c) { return (c == L' ' || c == L'\t'); }
inline bool IsTheW(const wchar_t *str) { if (str && (str[0] == L't' || str[0] == L'T') && (str[1] == L'h' || str[1] == L'H') && (str[2] == L'e' || str[2] == L'E') && (str[3] == L' ')) return true; else return false; }
#define SKIP_THE_AND_WHITESPACE(x) { wchar_t *save##x=(wchar_t*)x; while (IsCharSpaceW(*x) && *x) x++; if (IsTheW(x)) x+=4; while (IsCharSpaceW(*x)) x++; if (!*x) x=save##x; }
///#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb) {
	if (!pa) pa=L"";
	else SKIP_THE_AND_WHITESPACE(pa)
		if (!pb) pb=L"";
		else SKIP_THE_AND_WHITESPACE(pb)
			return lstrcmpi(pa,pb);
}
#undef SKIP_THE_AND_WHITESPACE

// convert Macintosh timestamp to windows timestamp
time_t mactime_to_wintime (const unsigned long mactime)
{
	if (mactime != 0) return (time_t)(mactime - 2082844800);
	else return (time_t)mactime;
}

// convert windows timestamp to Macintosh timestamp
unsigned long wintime_to_mactime (const __time64_t time)
{
	return (unsigned long)(time + 2082844800);
}

char * UTF16_to_UTF8(wchar_t * str)
{
	const unsigned int tempstrLen = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	char * tempstr=(char *)malloc(tempstrLen + 1);
	int ret=WideCharToMultiByte( CP_UTF8, 0, str, -1, tempstr, tempstrLen, NULL, NULL );
	tempstr[tempstrLen]='\0';

	if (!ret) DWORD bob=GetLastError();

	return tempstr;
}
wchar_t* UTF8_to_UTF16(char *str)
{
	const unsigned int tempstrLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

	wchar_t *tempstr = (wchar_t*)malloc((tempstrLen * 2) + 2);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, tempstr, tempstrLen);
	tempstr[tempstrLen] = '\0';

	return tempstr;
}

// get 2 bytes from data, reversed
static __forceinline uint16_t rev2(const uint8_t * data)
{
	uint16_t ret;
	ret = ((uint16_t) data[1]) << 8;
	ret += ((uint16_t) data[0]);
	return ret;
}

static __forceinline void rev2(const unsigned short number, unsigned char * data)
{
	data[1] = (unsigned char)(number >>  8) & 0xff;
	data[0] = (unsigned char)number & 0xff;
}

// get 4 bytes from data, reversed
static __forceinline uint32_t rev4(const uint8_t * data)
{
	unsigned long ret;
	ret = ((unsigned long) data[3]) << 24;
	ret += ((unsigned long) data[2]) << 16;
	ret += ((unsigned long) data[1]) << 8;
	ret += ((unsigned long) data[0]);
	return ret;
}

// get 4 bytes from data
static __forceinline uint32_t get4(const uint8_t * data)
{
	unsigned long ret;
	ret = ((unsigned long) data[0]) << 24;
	ret += ((unsigned long) data[1]) << 16;
	ret += ((unsigned long) data[2]) << 8;
	ret += ((unsigned long) data[3]);
	return ret;
}

// get 8 bytes from data
static __forceinline unsigned __int64 get8(const uint8_t * data)
{
	unsigned __int64 ret;
	ret = get4(data);
	ret = ret << 32;
	ret += get4(&data[4]);
	return ret;
}

// reverse 8 bytes in place
static __forceinline unsigned __int64 rev8(uint64_t number)
{
	unsigned __int64 ret;
	ret = (number&0x00000000000000FF) << 56;
	ret+= (number&0x000000000000FF00) << 40;
	ret+= (number&0x0000000000FF0000) << 24;
	ret+= (number&0x00000000FF000000) << 8;
	ret+= (number&0x000000FF00000000) >> 8;
	ret+= (number&0x0000FF0000000000) >> 24;
	ret+= (number&0x00FF000000000000) >> 40;
	ret+= (number&0xFF00000000000000) >> 56;
	return ret;
}

//write 4 bytes reversed
static __forceinline void rev4(const unsigned long number, uint8_t * data)
{
	data[3] = (uint8_t)(number >> 24) & 0xff;
	data[2] = (uint8_t)(number >> 16) & 0xff;
	data[1] = (uint8_t)(number >>  8) & 0xff;
	data[0] = (uint8_t)number & 0xff;
}

//write 4 bytes normal
static __forceinline void put4(const unsigned long number, uint8_t * data)
{
	data[0] = (uint8_t)(number >> 24) & 0xff;
	data[1] = (uint8_t)(number >> 16) & 0xff;
	data[2] = (uint8_t)(number >>  8) & 0xff;
	data[3] = (uint8_t)number & 0xff;
}

// write 8 bytes normal
static __forceinline void put8(const unsigned __int64 number, uint8_t * data)
{
	data[0] = (uint8_t)(number >> 56) & 0xff;
	data[1] = (uint8_t)(number >> 48) & 0xff;
	data[2] = (uint8_t)(number >> 40) & 0xff;
	data[3] = (uint8_t)(number >> 32) & 0xff;
	data[4] = (uint8_t)(number >> 24) & 0xff;
	data[5] = (uint8_t)(number >> 16) & 0xff;
	data[6] = (uint8_t)(number >> 8) & 0xff;
	data[7] = (uint8_t)number & 0xff;
}



// get 3 bytes from data, reversed
static __forceinline unsigned long rev3(const uint8_t * data)
{
	unsigned long ret = 0;
	ret += ((unsigned long) data[2]) << 16;
	ret += ((unsigned long) data[1]) << 8;
	ret += ((unsigned long) data[0]);
	return ret;
}

//write 3 bytes normal (used in iTunesSD)
static __forceinline void put3(const unsigned long number, uint8_t * data)
{
	data[0] = (uint8_t)(number >> 16) & 0xff;
	data[1] = (uint8_t)(number >>  8) & 0xff;
	data[2] = (uint8_t)number & 0xff;
}

//write 3 bytes reversed
static __forceinline void rev3(const unsigned long number, uint8_t * data)
{
	data[2] = (uint8_t)(number >> 16) & 0xff;
	data[1] = (uint8_t)(number >>  8) & 0xff;
	data[0] = (uint8_t)number & 0xff;
}

// pass data and ptr, updates ptr automatically (by reference)
static __forceinline void write_uint32_t(uint8_t *data, size_t &offset, uint32_t value)
{
	rev4(value, &data[offset]);
	offset+=4;
}

static __forceinline uint32_t read_uint32_t(const uint8_t *data, size_t &offset)
{
	const uint8_t *ptr = &data[offset];
	offset+=4;
	return rev4(ptr);
}

static __forceinline uint32_t read_uint16_t(const uint8_t *data, size_t &offset)
{
	const uint8_t *ptr = &data[offset];
	offset+=2;
	return rev2(ptr);
}

static unsigned __int64 Generate64BitID()
{
	GUID tmp;
	CoCreateGuid(&tmp);
	unsigned __int64 one   = tmp.Data1;
	unsigned __int64 two   = tmp.Data2;
	unsigned __int64 three = tmp.Data3;
	unsigned __int64 four  = rand();
	return(one << 32 | two << 16 | three | four);
}

// useful function to convert from UTF16 to chars
char * UTF16_to_char(wchar_t * str, int length)
{
	char * tempstr=(char *)malloc(length/2+1);
	int ret=WideCharToMultiByte( CP_MACCP, 0, str, length/2, tempstr, length/2, "x", NULL );
	tempstr[length/2]='\0';

	if (!ret) DWORD bob=GetLastError();

	return tempstr;
}

// Case insensitive version of wcsstr
wchar_t *wcsistr (const wchar_t *s1, const wchar_t *s2)
{
	wchar_t *cp = (wchar_t*) s1;
	wchar_t *s, *t, *endp;
	wchar_t l, r;

	endp = (wchar_t*)s1 + ( lstrlen(s1) - lstrlen(s2)) ;
	while (cp && *cp && (cp <= endp))
	{
		s = cp;
		t = (wchar_t*)s2;
		while (s && *s && t && *t)
		{
			l = towupper(*s);
			r = towupper(*t);
			if (l != r)
				break;
			s++, t++;
		}

		if (*t == 0)
			return cp;

		cp = CharNext(cp);
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// iPodObj - Base for all iPod classes
//////////////////////////////////////////////////////////////////////

iPodObj::iPodObj() :
size_head(0),
size_total(0)
{
}

iPodObj::~iPodObj()
{
}

//////////////////////////////////////////////////////////////////////
// iPod_mhbd - iTunes database class
//////////////////////////////////////////////////////////////////////

iPod_mhbd::iPod_mhbd() :
unk1(1),
dbversion(0x0c),		// iTunes 4.2 = 0x09, 4.5 = 0x0a, 4.7 = 0x0b, 4.7.1 = 0x0c, 0x0d, 0x13
children(2),
id(0),
platform(2),
language('ne'), // byte-swapped 'en'
library_id(0),
timezone(0),
audio_language(0),
unk80(1),
unk84(15),
subtitle_language(0),
unk164(0),
unk166(0),
unk168(0)
{
	// get timezone info
	_tzset(); // this function call ensures that _timezone global var is valid
	timezone = -_timezone;

	id = Generate64BitID();

	mhsdsongs = new iPod_mhsd(1);
	mhsdplaylists = new iPod_mhsd(3);
	mhsdsmartplaylists = new iPod_mhsd(5);
}

iPod_mhbd::~iPod_mhbd()
{
	delete mhsdsongs;
	delete mhsdplaylists;
}

long iPod_mhbd::parse(const uint8_t *data)
{
	size_t ptr=0;

	//check mhbd header
	if (_strnicmp((char *)&data[ptr],"mhbd",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=read_uint32_t(data, ptr);
	size_total=read_uint32_t(data, ptr);

	//ASSERT(size_head == 0xbc);

	// get unk's and numchildren
	unk1=read_uint32_t(data, ptr);
	dbversion=read_uint32_t(data, ptr);
	children=read_uint32_t(data, ptr);
	id=rev8(get8(&data[ptr]));
	ptr+=8;
	if(id == 0)
	{
		// Force the id to be a valid value.
		// This may not always be the right thing to do, but I can't think of any reason why it wouldn't be ok...
		id = Generate64BitID();
	}
	platform=read_uint16_t(data, ptr);

	ptr = 0x46;
	language = read_uint16_t(data, ptr);
	library_id = rev8(get8(&data[ptr]));
	ptr+=8;
	unk80 = read_uint32_t(data, ptr);
	unk84 = read_uint32_t(data, ptr);

	ptr = 0xA0;
	audio_language = read_uint16_t(data, ptr);
	subtitle_language =read_uint16_t(data, ptr);
	unk164 = read_uint16_t(data, ptr);
	unk166 = read_uint16_t(data, ptr);
	unk168 = read_uint16_t(data, ptr);

	// timezone is at 0x6c, but we want to calculate this based on the computer timezone
	// TODO: 4 byte field at 0xA0 that contains FFFFFFFF for the ipod shuffle I'm playing with

	//if (children != 2) return -1;

	//skip over nulls
	ptr=size_head;

	// get the mhsd's
	bool parsedPlaylists = false;
	for(unsigned int i=0; i<children; i++) 
	{
		iPod_mhsd * mhsd = new iPod_mhsd(0);
		long ret = mhsd->parse(&data[ptr]);
		if(ret<0) return ret;
		else ptr+=ret;
		if(mhsd->index == 1) 
		{
			delete mhsdsongs;
			mhsdsongs = mhsd;
		}
		else if(mhsd->index == 3 && !parsedPlaylists) 
		{
			delete mhsdplaylists;
			mhsdplaylists = mhsd;
			parsedPlaylists = true;
		}
		else if(mhsd->index == 2 && !parsedPlaylists) 
		{
			delete mhsdplaylists;
			mhsdplaylists = mhsd;
		} 
		else if(mhsd->index == 5) 
		{
			delete mhsdsmartplaylists;
			mhsdsmartplaylists = mhsd;
		}
		else
		{
			delete mhsd;
		}
	}

	return size_total;
}

long iPod_mhbd::write(unsigned char * data, const unsigned long datasize)
{
	return write(data,datasize,NULL);
}

extern void GenerateHash(unsigned char *pFWID, unsigned char *pDataBase, long lSize, unsigned char *pHash);

long iPod_mhbd::write(unsigned char * data, const unsigned long datasize, unsigned char *fwid)
{
	//const unsigned int headsize=0xbc; // for db version 0x19
	const unsigned int headsize=188; // for db version 0x2A
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhbd header
	data[0]='m';data[1]='h';data[2]='b';data[3]='d';
	ptr+=4;

	// write sizes
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	rev4(0x00,&data[ptr]);  // placeholder for total size (fill in later)
	ptr+=4;

	//write unks
	rev4(unk1,&data[ptr]);
	ptr+=4;
	rev4(0x2a/*dbversion*/,&data[ptr]);
	ptr+=4;

	//write numchildren
	//ASSERT (children == 2); // seen no other case in an iTunesDB yet
	children = 4;
	rev4(children,&data[ptr]);
	ptr+=4;

	// fill this in later (it's the db id, it has to be 0 for the hash generation)
	put8(0,&data[ptr]);
	ptr+=8;

	rev2(2, &data[ptr]); // platform (2 == Windows)
	ptr+=2;

	// fill up the rest of the header with nulls
	for (unsigned int i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	rev2(1, &data[0x30]);

	rev2(language, &data[0x46]);
	put8(library_id, &data[0x48]);
	rev4(unk80, &data[0x50]);
	rev4(unk84, &data[0x54]);
	rev4(timezone, &data[0x6c]);
	rev2(2, &data[0x70]);
	rev2(audio_language, &data[0xA0]);
	rev2(subtitle_language, &data[0xA2]);
	rev2(unk164, &data[0xA4]);
	rev2(unk166, &data[0xA6]);
	rev2(unk168, &data[0xA8]);

	long ret;
	// write the mhla (album list)
	iPod_mhsd mhsd_mhla(4);
	iPod_mhla *album_list = mhsd_mhla.mhla;

	iPod_mhlt::mhit_map_t::const_iterator begin = mhsdsongs->mhlt->mhit.begin();
	iPod_mhlt::mhit_map_t::const_iterator end   = mhsdsongs->mhlt->mhit.end();
	for(iPod_mhlt::mhit_map_t::const_iterator it = begin; it != end; it++)
	{
		wchar_t * artist = L"";
		wchar_t * album = L"";
		iPod_mhit *m = static_cast<iPod_mhit*>((*it).second);
		iPod_mhod *mhartist = m->FindString(MHOD_ARTIST);
		iPod_mhod *mhalbum = m->FindString(MHOD_ALBUM);

		if(mhartist && mhartist->str)
			artist = mhartist->str;

		if(mhalbum && mhalbum->str)
			album = mhalbum->str;

		m->album_id = mhsd_mhla.mhla->GetAlbumId(artist, album);
	}

	ret=mhsd_mhla.write(&data[ptr], datasize-ptr, 4);
	ASSERT(ret >= 0);
	if (ret<0) return ret;
	else ptr+=ret;

	// write the mhsd's
	ret=mhsdsongs->write(&data[ptr], datasize-ptr);
	ASSERT(ret >= 0);
	if (ret<0) return ret;
	else ptr+=ret;

	ret=mhsdplaylists->write(&data[ptr], datasize-ptr, 3);
	ASSERT(ret >= 0);
	if (ret<0) return ret;
	else ptr+=ret;

	ret=mhsdplaylists->write(&data[ptr], datasize-ptr, 2);
	ASSERT(ret >= 0);
	if (ret<0) return ret;
	else ptr+=ret;

	if(mhsdsmartplaylists->mhlp_smart) {
		ret=mhsdsmartplaylists->write(&data[ptr], datasize-ptr, 5);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	} else children--;

	// fix the total size
	rev4(ptr,&data[8]);
	rev4(children,&data[20]);

	if(fwid)
		GenerateHash(fwid,data,ptr,data+0x58); // fuck you, gaydickian

	put8(rev8(id),&data[0x18]); // put this back in -- it has to be 0 for the hash generation.

	return ptr;
}


//////////////////////////////////////////////////////////////////////
// iPod_mhsd - Holds tracklists and playlists
//////////////////////////////////////////////////////////////////////

iPod_mhsd::iPod_mhsd() :
index(0),
mhlt(NULL),
mhlp(NULL),
mhlp_smart(NULL),
mhla(NULL)
{
}

iPod_mhsd::iPod_mhsd(int newindex) :
index(newindex),
mhlt(NULL),
mhlp(NULL),
mhlp_smart(NULL),
mhla(NULL)
{
	switch(newindex)
	{
	case 1: mhlt=new iPod_mhlt(); break;
	case 2:
	case 3:
	case 5: mhlp=new iPod_mhlp(); break;
	case 4: mhla=new iPod_mhla(); break;
	default: index=0;
	}
}

iPod_mhsd::~iPod_mhsd()
{
	delete mhlt;
	delete mhlp;
	delete mhla;
}

long iPod_mhsd::parse(const uint8_t *data)
{
	unsigned long ptr=0;

	//check mhsd header
	if (_strnicmp((char *)&data[ptr],"mhsd",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=rev4(&data[ptr]);
	ptr+=4;
	size_total=rev4(&data[ptr]);
	ptr+=4;

	ASSERT(size_head == 0x60);

	// get index number
	index=rev4(&data[ptr]);
	ptr+=4;

	// skip null padding
	ptr=size_head;

	long ret;

	// check to see if this is a holder for an mhlt or an mhlp
	if (!_strnicmp((char *)&data[ptr],"mhlt",4))
	{
		if (mhlt==NULL)
		{
			mhlt=new iPod_mhlt();
			//index=1;
		}
		ret=mhlt->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}
	else if (!_strnicmp((char *)&data[ptr],"mhlp",4) && (index == 2 || index == 3))
	{
		if (mhlp==NULL)
		{
			mhlp=new iPod_mhlp();
			if(index != 2) index=3;
		}
		ret=mhlp->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}
	else if (!_strnicmp((char *)&data[ptr],"mhlp",4) && index == 5) // smart playlists
	{
		if (mhlp_smart==NULL)
			mhlp_smart=new iPod_mhlp();
		ret=mhlp_smart->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}
	else {
	}
	//return -1;

	//if (ptr != size_total) return -1;

	return size_total;
}

long iPod_mhsd::write(unsigned char * data, const unsigned long datasize, int index)
{
	const unsigned int headsize=0x60;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhsd header
	data[0]='m';data[1]='h';data[2]='s';data[3]='d';
	ptr+=4;

	// write sizes
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	rev4(0x00,&data[ptr]);  // placeholder for total size (fill in later)
	ptr+=4;

	// write index number
	rev4(index,&data[ptr]);
	ptr+=4;

	// fill up the rest of the header with nulls
	for (unsigned int i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	// write out the songs or the playlists, depending on index
	long ret;
	if (index==1)	// mhlt
		ret=mhlt->write(&data[ptr],datasize-ptr);
	else if (index==2 || index==3) // mhlp
		ret=mhlp->write(&data[ptr],datasize-ptr,index);
	else if (index == 4) // mhla
		ret=mhla->write(&data[ptr],datasize-ptr);
	else if (index==5) // mhlp_smart
		ret=mhlp_smart->write(&data[ptr],datasize-ptr,3);
	else return -1;

	ASSERT(ret>=0);
	if (ret<0) return ret;
	else ptr+=ret;

	// fix the total size
	rev4(ptr,&data[8]);

	return ptr;
}



//////////////////////////////////////////////////////////////////////
// iPod_mhlt - TrackList class
//////////////////////////////////////////////////////////////////////

iPod_mhlt::iPod_mhlt() :
mhit(),
next_mhit_id(100)
{
}

iPod_mhlt::~iPod_mhlt()
{
	// It is unnecessary (and slow) to clear the map, since the object is being destroyed anyway
	ClearTracks(false);
}

long iPod_mhlt::parse(const uint8_t *data)
{
	long ptr=0;

	//check mhlt header
	if (_strnicmp((char *)&data[ptr],"mhlt",4)) return -1;
	ptr+=4;

	// get size
	size_head=rev4(&data[ptr]);
	ptr+=4;

	ASSERT(size_head == 0x5c);

	// get num children (num songs on iPod)
	const unsigned long children=rev4(&data[ptr]);		// Only used locally - child count is obtained from the mhit list
	ptr+=4;

	//skip nulls
	ptr=size_head;

	long ret;

	// get children one by one
	for (unsigned long i=0;i<children;i++)
	{
		iPod_mhit *m = new iPod_mhit;
		ret=m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0)
		{
			delete m;
			return ret;
		}

		ptr+=ret;

		mhit.insert(mhit_value_t(m->id, m));
		mhit_indexer.push_back(m->id);
	}

	if (!mhit.empty())
	{
		//InterlockedExchange(&next_mhit_id, mhit.back().first);
		uint32_t id = mhit_indexer[mhit_indexer.size() - 1];
		InterlockedExchange(&next_mhit_id, id);
	}
	return ptr;
}

uint32_t iPod_mhlt::GetNextID()
{
	return (uint32_t)InterlockedIncrement(&next_mhit_id);
}

long iPod_mhlt::write(unsigned char * data, const unsigned long datasize)
{
	const unsigned int headsize=0x5c;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhlt header
	data[0]='m';data[1]='h';data[2]='l';data[3]='t';
	ptr+=4;

	// write size
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;

	// write numchildren (numsongs)
	const unsigned long children = GetChildrenCount();
	rev4(children,&data[ptr]);
	ptr+=4;

	// fill up the rest of the header with nulls
	for (unsigned long i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	long ret;

	// write children one by one
	mhit_map_t::const_iterator begin = mhit.begin();
	mhit_map_t::const_iterator end   = mhit.end();
	for(mhit_map_t::const_iterator it = begin; it != end; it++)
	{
		iPod_mhit *m = static_cast<iPod_mhit*>(it->second);

#ifdef _DEBUG
		const unsigned int mapID = (*it).first;
		ASSERT(mapID == m->id);
#endif

		ret=m->write(&data[ptr],datasize-ptr);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}

	return ptr;
}

iPod_mhit * iPod_mhlt::NewTrack()
{
	iPod_mhit *track = new iPod_mhit;
	if (track != NULL)
	{
		track->addedtime = wintime_to_mactime(time(0));
		track->id = GetNextID();
	}

	return track;
}

void iPod_mhlt::AddTrack(iPod_mhit *new_track)
{
	mhit_indexer.push_back(new_track->id);
	mhit.insert(mhit_value_t(new_track->id, new_track));
}

bool iPod_mhlt::DeleteTrack(const unsigned long index)
{
	//unsigned int i=0;
	//for(mhit_map_t::const_iterator it = mhit.begin(); it != mhit.end(); it++, i++)
	//{
	//	if(i == index)
	//	{
	//		iPod_mhit *m = static_cast<iPod_mhit*>(it->second);
	//		return(DeleteTrackByID(m->id));
	//	}
	//}
	//return false;

	if (index > mhit_indexer.size())
	{
		return false;
	}

	auto key = mhit_indexer[index];
	auto it = mhit.find(key);
	if (mhit.end() == it)
	{
		return false;
	}

	return DeleteTrackByID(it->first);
}

bool iPod_mhlt::DeleteTrackByID(const unsigned long id)
{
	mhit_map_t::iterator it = mhit.find(id);
	if(it != mhit.end())
	{
		iPod_mhit *m = static_cast<iPod_mhit*>(it->second);
		mhit.erase(it);
		// remove also from indexer!!
		for (size_t n = 0; n < mhit_indexer.size(); ++n)
		{
			if (id == mhit_indexer[n])
			{
				mhit_indexer.erase(mhit_indexer.begin() + n);
				break;
			}
		}
		delete m;
		return true;
	}
	return false;
}

iPod_mhit * iPod_mhlt::GetTrack(uint32_t index) const
{
	//mhit_map_t::value_type value = mhit.at(index);
	//return value.second;
	
	if (index > mhit_indexer.size())
	{
		return nullptr;
	}
	uint32_t key = mhit_indexer[index];
	auto it = mhit.find(key);
	 if (mhit.end() == it)
	 {
		 return nullptr;
	 }

	 return it->second;
}

iPod_mhit * iPod_mhlt::GetTrackByID(const unsigned long id)
{
	mhit_map_t::const_iterator it = mhit.find(id);
	if(it == mhit.end())
		return NULL;

	return static_cast<iPod_mhit*>(it->second);
}

bool iPod_mhlt::ClearTracks(const bool clearMap)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlt_ClearTracks);
#endif

	mhit_map_t::const_iterator begin = mhit.begin();
	mhit_map_t::const_iterator end   = mhit.end();
	for(mhit_map_t::const_iterator it = begin; it != end; it++)
	{
		delete static_cast<iPod_mhit*>(it->second);
	}

	if (clearMap)
	{
		mhit.clear();
		mhit_indexer.clear();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// iPod_mhit - Holds info about a song
//////////////////////////////////////////////////////////////////////

iPod_mhit::iPod_mhit() :
id(0),
visible(1),
filetype(0),
vbr(0),
type(0),
compilation(0),
stars(0),
lastmodifiedtime(0),
size(0),
length(0),
tracknum(0),
totaltracks(0),
year(0),
bitrate(0),
samplerate(0),
samplerate_fixedpoint(0),
volume(0),
starttime(0),
stoptime(0),
soundcheck(0),
playcount(0),
playcount2(0),
lastplayedtime(0),
cdnum(0),
totalcds(0),
userID(0),
addedtime(0),
bookmarktime(0),
dbid(0),
BPM(0),
app_rating(0),
checked(0),
unk9(0),
artworkcount(0),
artworksize(0),
unk11(0),
samplerate2(0),
releasedtime(0),
unk14(0),
unk15(0),
unk16(0),
skipcount(0),
skippedtime(0),
hasArtwork(2),				// iTunes 4.7.1 always seems to write 2 for unk19
skipShuffle(0),
rememberPosition(0),
unk19(0),	
dbid2(0),
lyrics_flag(0),
movie_flag(0),
mark_unplayed(0),
unk20(0),
unk21(0),
pregap(0),
samplecount(0),
unk25(0),
postgap(0),
unk27(0),
mediatype(0),
seasonNumber(0),
episodeNumber(0),
unk31(0),
unk32(0),
unk33(0),
unk34(0),
unk35(0),
unk36(0),
unk37(0),
gaplessData(0),
unk39(0),
albumgapless(0),
trackgapless(0),
unk40(0),
unk41(0),
unk42(0),
unk43(0),
unk44(0),
unk45(0),
unk46(0),
album_id(0),
unk48(0),
unk49(0),
unk50(0),
unk51(0),
unk52(0),
unk53(0),
unk54(0),
unk55(0),
unk56(0),
mhii_link(0),

mhod()
{
	// Create a highly randomized 64 bit value for the dbID
	dbid = Generate64BitID();
	dbid2 = dbid;
	for(int i=0; i<25; i++) mhodcache[i]=NULL;
	mhod.reserve(8);
}

iPod_mhit::~iPod_mhit()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_destructor);
#endif

	const unsigned long count = GetChildrenCount();
	for (unsigned long i=0;i<count;i++)
		delete mhod[i];
}

long iPod_mhit::parse(const uint8_t *data)
{
	size_t ptr=0;
	unsigned long temp=0;

	//check mhit header
	if (_strnicmp((char *)&data[ptr],"mhit",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=read_uint32_t(data, ptr);
	size_total=read_uint32_t(data, ptr);

	ASSERT(size_head == 0x9c || size_head == 0xf4 || size_head == 0x148 || size_head == 0x184 || size_head == 0x248);		// if this is false, this is a new mhit format

	// get rest of data
	unsigned long mhodnum=read_uint32_t(data, ptr);	// Only used locally
	id=read_uint32_t(data, ptr);
	visible=read_uint32_t(data, ptr);
	filetype=read_uint32_t(data, ptr);

	vbr = data[ptr++];
	type = data[ptr++];
	compilation = data[ptr++];
	stars = data[ptr++];

	lastmodifiedtime=read_uint32_t(data, ptr);
	size=read_uint32_t(data, ptr);
	length=read_uint32_t(data, ptr);
	tracknum=read_uint32_t(data, ptr);
	totaltracks=read_uint32_t(data, ptr);
	year=read_uint32_t(data, ptr);
	bitrate=read_uint32_t(data, ptr);

	temp=rev4(&data[ptr]);
	ptr+=4;
	samplerate = (uint16_t)(temp >> 16);
	samplerate_fixedpoint = (uint16_t)(temp & 0x0000ffff);

	volume=read_uint32_t(data, ptr);
	starttime=read_uint32_t(data, ptr);
	stoptime=read_uint32_t(data, ptr);
	soundcheck=read_uint32_t(data, ptr);
	playcount=read_uint32_t(data, ptr);
	playcount2=read_uint32_t(data, ptr);
	lastplayedtime=read_uint32_t(data, ptr);
	cdnum=read_uint32_t(data, ptr);;
	totalcds=read_uint32_t(data, ptr);
	userID=read_uint32_t(data, ptr);
	addedtime=read_uint32_t(data, ptr);
	bookmarktime=read_uint32_t(data, ptr);
	dbid=rev8(get8(&data[ptr]));
	ptr+=8;
	if(dbid == 0)
	{
		// Force the dbid to be a valid value.
		// This may not always be the right thing to do, but I can't think of any reason why it wouldn't be ok...
		dbid = Generate64BitID();
	}

	temp=rev4(&data[ptr]);
	BPM=temp>>16;
	app_rating=(temp&0xff00) >> 8;
	checked = (uint8_t)(temp&0xff);
	ptr+=4;

	artworkcount=rev2(&data[ptr]);
	ptr+=2;
	unk9=rev2(&data[ptr]);
	ptr+=2;

	artworksize=read_uint32_t(data, ptr);
	unk11=read_uint32_t(data, ptr);
	memcpy(&samplerate2, &data[ptr], sizeof(float));
	ptr+=4;

	releasedtime=read_uint32_t(data, ptr);
	unk14=read_uint32_t(data, ptr);
	unk15=read_uint32_t(data, ptr);
	unk16=read_uint32_t(data, ptr);

	// Newly added as of dbversion 0x0c
	if(size_head >= 0xf4)
	{
		skipcount=read_uint32_t(data, ptr);
		skippedtime=read_uint32_t(data, ptr);
		hasArtwork=data[ptr++];
		skipShuffle=data[ptr++];
		rememberPosition=data[ptr++];
		unk19=data[ptr++];
		dbid2=rev8(get8(&data[ptr]));
		ptr+=8;
		if(dbid2 == 0)
			dbid2 = dbid;
		lyrics_flag=data[ptr++];
		movie_flag=data[ptr++];
		mark_unplayed=data[ptr++];
		unk20=data[ptr++];
		unk21=read_uint32_t(data, ptr); // 180
		pregap=read_uint32_t(data, ptr);
		samplecount=rev8(get8(&data[ptr])); //sample count
		ptr+=8;
		unk25=read_uint32_t(data, ptr); // 196
		postgap=read_uint32_t(data, ptr);
		unk27=read_uint32_t(data, ptr);
		mediatype=read_uint32_t(data, ptr);
		seasonNumber=read_uint32_t(data, ptr);
		episodeNumber=read_uint32_t(data, ptr);
		unk31=read_uint32_t(data, ptr);
		unk32=read_uint32_t(data, ptr);
		unk33=read_uint32_t(data, ptr);
		unk34=read_uint32_t(data, ptr);
		unk35=read_uint32_t(data, ptr);
		unk36=read_uint32_t(data, ptr);
	}

	if(size_head >= 0x148) 
	{ // dbversion 0x13
		unk37=read_uint32_t(data, ptr);
		gaplessData=read_uint32_t(data, ptr);
		unk39=read_uint32_t(data, ptr);
		trackgapless = read_uint16_t(data, ptr);
		albumgapless = read_uint16_t(data, ptr);
		unk40=read_uint32_t(data, ptr); // 260
		unk41=read_uint32_t(data, ptr); // 264
		unk42=read_uint32_t(data, ptr); // 268
		unk43=read_uint32_t(data, ptr); // 272
		unk44=read_uint32_t(data, ptr); // 276
		unk45=read_uint32_t(data, ptr); // 280
		unk46=read_uint32_t(data, ptr); // 284
		album_id=read_uint32_t(data, ptr); // 288 - libgpod lists "album_id"
		unk48=read_uint32_t(data, ptr); // 292 - libgpod lists first half of an id
		unk49=read_uint32_t(data, ptr); // 296 - libgpod lists second half of an id
		unk50=read_uint32_t(data, ptr); // 300 - libgpod lists file size
		unk51=read_uint32_t(data, ptr); // 304
		unk52=read_uint32_t(data, ptr); // 308 - libgpod mentions 8 bytes of 0x80
		unk53=read_uint32_t(data, ptr); // 312 - libgpod mentions 8 bytes of 0x80
		unk54=read_uint32_t(data, ptr); // 316 
		unk55=read_uint32_t(data, ptr); // 320
		unk56=read_uint32_t(data, ptr); // 324
	}
	if(size_head >= 0x184) 
	{
		ptr = 0x148; // line it up, just in case
		ptr += 22; // dunno what the first 22 bytes are
		album_id = read_uint16_t(data, ptr);
		mhii_link = read_uint32_t(data, ptr);

	}

#ifdef _DEBUG
	// If these trigger an assertion, something in the database format has changed/been added
	ASSERT(visible == 1);
	ASSERT(unk11 == 0);
	ASSERT(unk16 == 0);
//	ASSERT(unk19 == 0);
	//	ASSERT(hasArtwork == 2);		// iTunes always sets unk19 to 2, but older programs won't have set it
	ASSERT(unk20 == 0);
	ASSERT(unk21 == 0);
	ASSERT(unk25 == 0);
	//	ASSERT(unk27 == 0);
	//ASSERT(unk31 == 0);
	ASSERT(unk32 == 0);
	ASSERT(unk33 == 0);
	ASSERT(unk34 == 0);
	ASSERT(unk35 == 0);
	ASSERT(unk36 == 0);
	ASSERT(unk37 == 0);
	ASSERT(unk39 == 0);
	ASSERT(unk40 == 0);
	ASSERT(unk41 == 0);
	ASSERT(unk42 == 0);
	ASSERT(unk43 == 0);
	ASSERT(unk44 == 0);
	ASSERT(unk45 == 0);
	ASSERT(unk46 == 0);
	//  ASSERT(unk47 == 0);
	//ASSERT(unk48 == 0);
	//	ASSERT(unk49 == 0);
	ASSERT(unk50 == 0 || unk50 == size);
	ASSERT(unk51 == 0);
	//	ASSERT(unk52 == 0);
	ASSERT(unk53 == 0 || unk53 == 0x8080 || unk53 == 0x8081);
	ASSERT(unk54 == 0);
	ASSERT(unk55 == 0);
	ASSERT(unk56 == 0);
#endif

	// skip nulls
	ptr=size_head;

	long ret;
	for (unsigned long i=0;i<mhodnum;i++)
	{
		iPod_mhod *m = new iPod_mhod;
		ret=m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0)
		{
			delete m;
			return ret;
		}

		ptr+=ret;
		mhod.push_back(m);
		if(m->type <= 25 && m->type >= 1) mhodcache[m->type-1] = m;
	}

	return size_total;
}


long iPod_mhit::write(unsigned char * data, const unsigned long datasize)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_write);
#endif

	//const unsigned int headsize=0x148;		// was 0x9c in db version <= 0x0b
	const unsigned int headsize=0x184;		// db version 0x19
	// check for header size
	if (headsize>datasize) return -1;

	size_t ptr=0;

	//write mhlt header
	data[0]='m';data[1]='h';data[2]='i';data[3]='t';
	ptr+=4;

	// write sizes
	write_uint32_t(data, ptr, headsize);  // header size
	write_uint32_t(data, ptr, 0); // placeholder for total size (fill in later)

	unsigned long temp, i;

	// Remove all empty MHOD strings before continuing
	for(i=0;i<GetChildrenCount();i++)
	{
		iPod_mhod *m = mhod[i];
		ASSERT(m != NULL);
		if(m == NULL)
			continue;

		if(m->type < 50 && m->length == 0)
		{
			//DeleteString(m->type);
			//i = 0;
		}
	}

	// write stuff out
	unsigned long mhodnum = GetChildrenCount();
	write_uint32_t(data, ptr, mhodnum);
	write_uint32_t(data, ptr, id);
	write_uint32_t(data, ptr, visible);

	if(filetype == 0)
	{
		iPod_mhod *mhod = FindString(MHOD_LOCATION);
		if(mhod)
		{
			filetype = GetFileTypeID(mhod->str);
		}
	}
	write_uint32_t(data, ptr, filetype);

	vbr = data[ptr++] = vbr;
	type = data[ptr++] = type;
	compilation = data[ptr++] = compilation;
	stars = data[ptr++] = stars;

	write_uint32_t(data, ptr, lastmodifiedtime);
	write_uint32_t(data, ptr, size);
	write_uint32_t(data, ptr, length);
	write_uint32_t(data, ptr, tracknum);
	write_uint32_t(data, ptr, totaltracks);

	write_uint32_t(data, ptr, year);
	write_uint32_t(data, ptr, bitrate);


	temp = samplerate << 16 | samplerate_fixedpoint & 0x0000ffff;
	rev4(temp,&data[ptr]);
	ptr+=4;

	write_uint32_t(data, ptr, volume);
	write_uint32_t(data, ptr, starttime);
	write_uint32_t(data, ptr, stoptime);
	write_uint32_t(data, ptr, soundcheck);
	write_uint32_t(data, ptr, playcount);
	write_uint32_t(data, ptr, playcount2);
	write_uint32_t(data, ptr, lastplayedtime);
	write_uint32_t(data, ptr, cdnum);
	write_uint32_t(data, ptr, totalcds);
	write_uint32_t(data, ptr, userID);
	write_uint32_t(data, ptr, addedtime);
	write_uint32_t(data, ptr, bookmarktime);
	put8(rev8(dbid),&data[ptr]);
	ptr+=8;

	temp = BPM << 16 | (app_rating & 0xff) << 8 | (checked & 0xff);
	write_uint32_t(data, ptr, temp);

	rev2(artworkcount, &data[ptr]);
	ptr+=2;
	rev2(unk9, &data[ptr]);
	ptr+=2;

	write_uint32_t(data, ptr, artworksize);
	write_uint32_t(data, ptr, unk11);

	// If samplerate2 is not set, base it off of samplerate
	if(samplerate2 == 0)
	{
		// samplerate2 is the binary representation of the samplerate, as a 32 bit float
		const float foo = (float)samplerate;
		memcpy(&data[ptr], &foo, 4);
	}
	else
	{
		memcpy(&data[ptr], &samplerate2, 4);
	}
	ptr+=4;

	rev4(releasedtime,&data[ptr]);
	ptr+=4;
	rev4(unk14,&data[ptr]);
	ptr+=4;
	rev4(unk15,&data[ptr]);
	ptr+=4;
	rev4(unk16,&data[ptr]);
	ptr+=4;

	// New data as of dbversion 0x0c
	if(headsize >= 0xf4)
	{
		rev4(skipcount,&data[ptr]);
		ptr+=4;
		rev4(skippedtime,&data[ptr]);
		ptr+=4;
		data[ptr++]=hasArtwork;
		data[ptr++]=skipShuffle;
		data[ptr++]=rememberPosition;
		data[ptr++]=unk19;
		put8(rev8(dbid2),&data[ptr]);
		ptr+=8;
		data[ptr++]=lyrics_flag;
		data[ptr++]=movie_flag;
		data[ptr++]=mark_unplayed;
		data[ptr++]=unk20;
		rev4(unk21,&data[ptr]);
		ptr+=4;
		rev4(pregap,&data[ptr]);
		ptr+=4;
		put8(rev8(samplecount),&data[ptr]);
		ptr+=8;
		rev4(unk25,&data[ptr]);
		ptr+=4;
		rev4(postgap,&data[ptr]);
		ptr+=4;
		rev4(unk27,&data[ptr]);
		ptr+=4;
		rev4(mediatype,&data[ptr]);
		ptr+=4;
		rev4(seasonNumber,&data[ptr]);
		ptr+=4;
		rev4(episodeNumber,&data[ptr]);
		ptr+=4;
		rev4(unk31,&data[ptr]);
		ptr+=4;
		rev4(unk32,&data[ptr]);
		ptr+=4;
		rev4(unk33,&data[ptr]);
		ptr+=4;
		rev4(unk34,&data[ptr]);
		ptr+=4;
		rev4(unk35,&data[ptr]);
		ptr+=4;
		rev4(unk36,&data[ptr]);
		ptr+=4;
	}
	if(headsize >= 0x148) 
	{
		rev4(unk37,&data[ptr]); ptr+=4;
		rev4(gaplessData,&data[ptr]); ptr+=4;
		rev4(unk39,&data[ptr]); ptr+=4;

		temp = albumgapless << 16 | (trackgapless & 0xffff);
		rev4(temp, &data[ptr]); ptr+=4;

		rev4(unk40,&data[ptr]); ptr+=4;
		rev4(unk41,&data[ptr]); ptr+=4;
		rev4(unk42,&data[ptr]); ptr+=4;
		rev4(unk43,&data[ptr]); ptr+=4;
		rev4(unk44,&data[ptr]); ptr+=4;
		rev4(unk45,&data[ptr]); ptr+=4;
		rev4(unk46,&data[ptr]); ptr+=4;
		rev4(album_id,&data[ptr]); ptr+=4;
		rev4(unk48,&data[ptr]); ptr+=4;
		rev4(unk49,&data[ptr]); ptr+=4;
		rev4(size,&data[ptr]); ptr+=4;
		rev4(unk51,&data[ptr]); ptr+=4;
		rev4(unk52,&data[ptr]); ptr+=4;
		rev4(unk53,&data[ptr]); ptr+=4;
		rev4(unk54,&data[ptr]); ptr+=4;
		rev4(unk55,&data[ptr]); ptr+=4;
		rev4(unk56,&data[ptr]); ptr+=4;
	}

	if (headsize >= 0x184)
	{
		ptr = 0x148; // line it up, just in case
		memset(&data[ptr], 0, 22); // write a bunch of zeroes
		ptr+=22;
		rev2(album_id, &data[ptr]); ptr+=2;
		rev4(mhii_link,&data[ptr]); ptr+=4;
		memset(&data[ptr], 0, 32); // write a bunch of zeroes
		ptr+=32;
	}

	ASSERT(ptr==headsize); // if this ain't true, I screwed up badly somewhere above

	long ret;
	for (i=0;i<mhodnum;i++)
	{
		ret=mhod[i]->write(&data[ptr],datasize-ptr);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}

	// fix the total size
	rev4(ptr,&data[8]);

	return ptr;
}

iPod_mhod * iPod_mhit::AddString(const int type)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_AddString);
#endif

	iPod_mhod * m;
	if (type)
	{
		m = FindString(type);
		if (m != NULL)
		{
			return m;
		}
	}

	m=new iPod_mhod;
	if (m!=NULL && type) m->type=type;
	mhod.push_back(m);
	if(m->type <= 25 && m->type >= 1) mhodcache[m->type-1] = m;
	return m;
}

iPod_mhod * iPod_mhit::FindString(const unsigned long type) const
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_FindString);
#endif

	if(type <= 25 && type >= 1) return mhodcache[type-1];

	const unsigned long children = GetChildrenCount();
	for (unsigned long i=0;i<children;i++)
	{
		if (mhod[i]->type == type) return mhod[i];
	}

	return NULL;
}

unsigned long iPod_mhit::DeleteString(const unsigned long type)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_DeleteString);
#endif
	if(type <= 25 && type >= 1) mhodcache[type-1] = NULL;
	unsigned long count=0;

	for (unsigned long i=0; i != GetChildrenCount(); i++)
	{
		if (mhod[i]->type == type)
		{
			iPod_mhod * m = mhod.at(i);
			mhod.erase(mhod.begin() + i);
			delete m;
			i = i > 0 ? i - 1 : 0;  // do this to ensure that it checks the new entry in position i next
			count++;
		}
	}
	return count;
}

unsigned int iPod_mhit::GetFileTypeID(const wchar_t *filename)
{
	ASSERT(filename);
	if(filename == NULL)
		return(0);

	// Incredibly, this is really the file extension as ASCII characters
	// e.g. 0x4d = 'M', 0x50 = 'P', 0x33 = '3', 0x20 = '<space>'
	if(wcsistr(filename, L".mp3") != NULL)
		return FILETYPE_MP3;
	else if(wcsistr(filename, L".m4a") != NULL)
		return FILETYPE_M4A;
	else if(wcsistr(filename, L".m4b") != NULL)
		return(0x4d344220);
	else if(wcsistr(filename, L".m4p") != NULL)
		return(0x4d345020);
	else if(wcsistr(filename, L".wav") != NULL)
		return FILETYPE_WAV;

	return(0);
}


iPod_mhit& iPod_mhit::operator=(const iPod_mhit& src)
{
	Duplicate(&src,this);
	return *this;
}

void iPod_mhit::Duplicate(const iPod_mhit *src, iPod_mhit *dst)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhit_Duplicate);
#endif

	if(src == NULL || dst == NULL)
		return;

	dst->id = src->id;
	dst->visible = src->visible;
	dst->filetype = src->filetype;
	dst->vbr = src->vbr;
	dst->type = src->type;
	dst->compilation = src->compilation;
	dst->stars = src->stars;
	dst->lastmodifiedtime = src->lastmodifiedtime;
	dst->size = src->size;
	dst->length = src->length;
	dst->tracknum = src->tracknum;
	dst->totaltracks = src->totaltracks;
	dst->year = src->year;
	dst->bitrate = src->bitrate;
	dst->samplerate = src->samplerate;
	dst->samplerate_fixedpoint = src->samplerate_fixedpoint;
	dst->volume = src->volume;
	dst->starttime = src->starttime;
	dst->stoptime = src->stoptime;
	dst->soundcheck = src->soundcheck;
	dst->playcount = src->playcount;
	dst->playcount2 = src->playcount2;
	dst->lastplayedtime = src->lastplayedtime;
	dst->cdnum = src->cdnum;
	dst->totalcds = src->totalcds;
	dst->userID = src->userID;
	dst->addedtime = src->addedtime;
	dst->bookmarktime = src->bookmarktime;
	dst->dbid = src->dbid;
	dst->BPM = src->BPM;
	dst->app_rating = src->app_rating;
	dst->checked = src->checked;
	dst->unk9 = src->unk9;
	dst->artworksize = src->artworksize;
	dst->unk11 = src->unk11;
	dst->samplerate2 = src->samplerate2;
	dst->releasedtime = src->releasedtime;
	dst->unk14 = src->unk14;
	dst->unk15 = src->unk15;
	dst->unk16 = src->unk16;
	dst->skipcount = src->skipcount;
	dst->skippedtime = src->skippedtime;
	dst->hasArtwork = src->hasArtwork;
	dst->skipShuffle = src->skipShuffle;
	dst->rememberPosition = src->rememberPosition;
	dst->unk19 = src->unk19;
	dst->dbid2 = src->dbid2;
	dst->lyrics_flag = src->lyrics_flag;
	dst->movie_flag = src->movie_flag;
	dst->mark_unplayed = src->mark_unplayed;
	dst->unk20 = src->unk20;
	dst->unk21 = src->unk21;
	dst->pregap = src->pregap;
	dst->samplecount = src->samplecount;
	dst->unk25 = src->unk25;
	dst->postgap = src->postgap;
	dst->unk27 = src->unk27;
	dst->mediatype = src->mediatype;
	dst->seasonNumber = src->seasonNumber;
	dst->episodeNumber = src->episodeNumber;
	dst->unk31 = src->unk31;
	dst->unk32 = src->unk32;
	dst->unk33 = src->unk33;
	dst->unk34 = src->unk34;
	dst->unk35 = src->unk35;
	dst->unk36 = src->unk36;
	dst->unk37 = src->unk37;
	dst->gaplessData = src->gaplessData;
	dst->unk39 = src->unk39;
	dst->albumgapless = src->albumgapless;
	dst->trackgapless = src->trackgapless;
	dst->unk40 = src->unk40;
	dst->unk41 = src->unk41;
	dst->unk42 = src->unk42;
	dst->unk43 = src->unk43;
	dst->unk44 = src->unk44;
	dst->unk45 = src->unk45;
	dst->unk46 = src->unk46;
	dst->album_id = src->album_id;
	dst->unk48 = src->unk48;
	dst->unk49 = src->unk49;
	dst->unk50 = src->unk50;
	dst->unk51 = src->unk51;
	dst->unk52 = src->unk52;
	dst->unk53 = src->unk53;
	dst->unk54 = src->unk54;
	dst->unk55 = src->unk55;
	dst->unk56 = src->unk56;

	dst->mhii_link = src->mhii_link;

	const unsigned int mhodSize = src->mhod.size();
	for(unsigned int i=0; i<mhodSize; i++)
	{
		iPod_mhod *src_mhod = src->mhod[i];
		if(src_mhod == NULL)
			continue;

		iPod_mhod *dst_mhod = dst->AddString(src_mhod->type);
		if(dst_mhod)
			dst_mhod->Duplicate(src_mhod, dst_mhod);
	}
}

int iPod_mhit::GetEQSetting()
{
	iPod_mhod *mhod = FindString(MHOD_EQSETTING);
	if(mhod == NULL)
		return(EQ_NONE);

	ASSERT(lstrlen(mhod->str) == 9);
	if(lstrlen(mhod->str) != 9)
		return(EQ_NONE);

	wchar_t strval[4] = {0};
	lstrcpyn(strval, mhod->str + 3, 3);
	int val = _wtoi(strval);
	ASSERT(val >= EQ_ACOUSTIC && val <= EQ_VOCALBOOSTER);
	return(val);
}

void iPod_mhit::SetEQSetting(int value)
{
	DeleteString(MHOD_EQSETTING);
	if(value < 0)
		return;

	ASSERT(value >= EQ_ACOUSTIC && value <= EQ_VOCALBOOSTER);

	wchar_t strval[10] = {0};
	_snwprintf(strval, 9, L"#!#%d#!#", value);
	strval[9] = '\0';

	iPod_mhod *mhod = AddString(MHOD_EQSETTING);
	ASSERT(mhod);
	if(mhod == NULL)
		return;

	mhod->SetString(strval);
}

//////////////////////////////////////////////////////////////////////
// iPod_mhod - Holds strings for a song or playlist, among other things
//////////////////////////////////////////////////////////////////////

iPod_mhod::iPod_mhod() :
type(0),
unk1(0),
unk2(0),
position(1),
length(0),
unk3(1),
unk4(0),
str(NULL),
binary(NULL),
liveupdate(1),
checkrules(0),
matchcheckedonly(0),
limitsort_opposite(0),
limittype(0),
limitsort(0),
limitvalue(0),
unk5(0),
rules_operator(SPLMATCH_AND),
parseSmartPlaylists(true)
{
}


iPod_mhod::~iPod_mhod()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhod_destructor);
#endif

	if (str) delete [] str;
	if (binary) delete [] binary;

	const unsigned int size = rule.size();
	for (unsigned int i=0;i<size;i++)
		delete rule[i];
}

long iPod_mhod::parse(const uint8_t *data)
{
	size_t ptr=0;
	unsigned long i = 0;

	//check mhod header
	if (_strnicmp((char *)&data[ptr],"mhod",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=read_uint32_t(data, ptr);
	size_total=read_uint32_t(data, ptr);

	ASSERT(size_head == 0x18);

	// get type
	type=read_uint32_t(data, ptr);

	// dunno what these are, but they are definitely common among all the types of mhods
	// smartlist types prove that.
	// they're usually zero though.. null padding for the MHOD header?
	unk1=read_uint32_t(data, ptr);
	unk2=read_uint32_t(data, ptr);

	if (iPod_mhod::IsSimpleStringType(type))
	{
		// string types get parsed
		// 1 == UTF-16, 2 == UTF-8
		// TODO: handle UTF-8
		encoding_type=read_uint32_t(data, ptr);
		length=read_uint32_t(data, ptr);
		unk3=read_uint32_t(data, ptr);
		unk4=read_uint32_t(data, ptr);

		str=new wchar_t[length + 1];
		memcpy(str,&data[ptr],length);
		str[length / 2] = '\0';
		ptr+=length;
	}
	else if (type == MHOD_ENCLOSUREURL || type == MHOD_RSSFEEDURL)
	{
		// Apple makes life hard again!  These are almost like regular/simple MHOD string types,
		// except the string is UTF-8 encoded and there is no length or position fields
		length = size_total - size_head;
		ASSERT(length > 0);
		if(length > 0)
		{
			char *tmp = new char[length + 1];
			ASSERT(tmp);
			if(tmp != NULL)
			{
				memcpy(tmp, &data[ptr], length);
				tmp[length] = '\0';

				wchar_t *tmpUTF = UTF8_to_UTF16(tmp);
				unsigned int len = wcslen(tmpUTF);
				str=new wchar_t[len + 1];
				wcsncpy(str, tmpUTF, len);
				str[len] = '\0';

				free(tmpUTF);
			}

			ptr+=length;
		}
	}
	else if (type==MHOD_SPLPREF)
	{
		if(parseSmartPlaylists)
		{
			liveupdate=data[ptr]; ptr++;
			checkrules=data[ptr]; ptr++;
			checklimits=data[ptr]; ptr++;
			limittype=data[ptr]; ptr++;
			limitsort=data[ptr]; ptr++;
			ptr+=3;
			limitvalue=read_uint32_t(data, ptr);
			matchcheckedonly=data[ptr]; ptr++;

			// if the opposite flag is on, set limitsort's high bit
			limitsort_opposite=data[ptr]; ptr++;
			if(limitsort_opposite)
				limitsort += 0x80000000;
		}
	}
	else if (type==MHOD_SPLDATA)
	{
		if(parseSmartPlaylists)
		{
			// strangely, SPL Data is the only thing in the file that *isn't* byte reversed.
			// check for SLst header
			if (_strnicmp((char *)&data[ptr],"SLst",4)) return -1;
			ptr+=4;
			unk5=get4(&data[ptr]); ptr+=4;
			const unsigned int numrules=get4(&data[ptr]); ptr+=4;
			rules_operator=get4(&data[ptr]); ptr+=4;
			ptr+=120;

			rule.reserve(numrules);

			for (i=0;i<numrules;i++)
			{
				SPLRule * r = new SPLRule;
				ASSERT(r);
				if(r == NULL)
					continue;

				r->field=get4(&data[ptr]); ptr+=4;
				r->action=get4(&data[ptr]); ptr+=4;
				ptr+=44;
				r->length=get4(&data[ptr]); ptr+=4;

#ifdef _DEBUG
				switch(r->action)
				{
				case SPLACTION_IS_INT:
				case SPLACTION_IS_GREATER_THAN:
				case SPLACTION_IS_NOT_GREATER_THAN:
				case SPLACTION_IS_LESS_THAN:
				case SPLACTION_IS_NOT_LESS_THAN:
				case SPLACTION_IS_IN_THE_RANGE:
				case SPLACTION_IS_NOT_IN_THE_RANGE:
				case SPLACTION_IS_IN_THE_LAST:
				case SPLACTION_IS_STRING:
				case SPLACTION_CONTAINS:
				case SPLACTION_STARTS_WITH:
				case SPLACTION_DOES_NOT_START_WITH:
				case SPLACTION_ENDS_WITH:
				case SPLACTION_DOES_NOT_END_WITH:
				case SPLACTION_IS_NOT_INT:
				case SPLACTION_IS_NOT_IN_THE_LAST:
				case SPLACTION_IS_NOT:
				case SPLACTION_DOES_NOT_CONTAIN:
				case SPLACTION_BINARY_AND:
				case SPLACTION_UNKNOWN2:
					break;

				default:
					// New action!
					//printf("New Action Discovered = %x\n",r->action);
					ASSERT(0);
					break;
				}
#endif

				const bool hasString = iPod_slst::GetFieldType(r->field) == iPod_slst::ftString;

				if(hasString)
				{
					// For some unknown reason, smart playlist strings have UTF-16 characters that are byte swapped
					unsigned char *c = (unsigned char*)r->string;
					const unsigned len = min(r->length, SPL_MAXSTRINGLENGTH);
					for(unsigned int i=0; i<len; i+=2)
					{
						*(c + i) = data[ptr + i + 1];
						*(c + i + 1) = data[ptr + i];
					}

					ptr += r->length;
				}
				else
				{
					// from/to combos always seem to be 0x44 in length in all cases...
					// fix this to be smarter if it turns out not to be the case
					ASSERT(r->length == 0x44);

					r->fromvalue=get8(&data[ptr]); ptr+=8;
					r->fromdate=get8(&data[ptr]); ptr+=8;
					r->fromunits=get8(&data[ptr]); ptr+=8;

					r->tovalue=get8(&data[ptr]); ptr+=8;
					r->todate=get8(&data[ptr]); ptr+=8;
					r->tounits=get8(&data[ptr]); ptr+=8;

					// SPLFIELD_PLAYLIST seems to use the unks here...
					r->unk1=get4(&data[ptr]); ptr+=4;
					r->unk2=get4(&data[ptr]); ptr+=4;
					r->unk3=get4(&data[ptr]); ptr+=4;
					r->unk4=get4(&data[ptr]); ptr+=4;
					r->unk5=get4(&data[ptr]); ptr+=4;
				}

				rule.push_back(r);
			}
		}
	}
	else if(type == MHOD_PLAYLIST)
	{
		position=read_uint32_t(data, ptr);

		// Skip to the end
		ptr+=16;
	}
	else
	{
		// non string/smart playlist types get copied in.. with the header and such being ignored
		binary=new unsigned char[size_total-size_head];
		memcpy(binary,&data[ptr],size_total-size_head);
		// in this case, we'll use the length field to store the length of the binary stuffs,
		//		since it's not being used for anything else in these entries.
		// this helps in the writing phase of the process.
		// note that if, for some reason, you decide to create a mhod for type 50+ from scratch,
		//		you need to set the length = the size of your binary space
		length=size_total-size_head;
	}

	return size_total;
}

long iPod_mhod::write(unsigned char * data, const unsigned long datasize)
{
	const unsigned long headsize=0x18;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhod header
	data[0]='m';data[1]='h';data[2]='o';data[3]='d';
	ptr+=4;

	// write sizes
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	rev4(0x00,&data[ptr]);  // placeholder for total size (fill in later)
	ptr+=4;

	// write stuff out
	rev4(type,&data[ptr]);
	ptr+=4;
	rev4(unk1,&data[ptr]);
	ptr+=4;
	rev4(unk2,&data[ptr]);
	ptr+=4;

	if (iPod_mhod::IsSimpleStringType(type))
	{
		// check for string size
		if (16+length+headsize>datasize) return -1;

		rev4(position,&data[ptr]);
		ptr+=4;
		rev4(length,&data[ptr]);
		ptr+=4;
		rev4(unk3,&data[ptr]);
		ptr+=4;
		rev4(unk4,&data[ptr]);
		ptr+=4;

		const unsigned int len = length / 2;
		for (unsigned int i=0;i<len;i++)
		{
			data[ptr]=str[i] & 0xff;
			ptr++;
			data[ptr]=(str[i] >> 8) & 0xff;
			ptr++;
		}
	}
	else if (type == MHOD_ENCLOSUREURL || type == MHOD_RSSFEEDURL)
	{
		// Convert the UTF-16 string back to UTF-8
		char *utf8Str = UTF16_to_UTF8(str);
		const unsigned int len = strlen(utf8Str);
		if (16+len+headsize>datasize) { free(utf8Str); return -1; }
		memcpy(data + ptr, utf8Str, len);
		free(utf8Str);
		ptr += len;
	}
	else if (type==MHOD_SPLPREF)
	{
		if (16+74 > datasize) return -1;

		// write the type 50 mhod
		data[ptr]=liveupdate; ptr++;
		data[ptr]=checkrules; ptr++;
		data[ptr]=checklimits; ptr++;
		data[ptr]=(unsigned char)(limittype); ptr++;
		data[ptr]=(unsigned char)((limitsort & 0x000000ff)); ptr++;
		data[ptr]=0; ptr++;
		data[ptr]=0; ptr++;
		data[ptr]=0; ptr++;
		rev4(limitvalue,&data[ptr]); ptr+=4;
		data[ptr]=matchcheckedonly; ptr++;
		// set the limitsort_opposite flag by checking the high bit of limitsort
		data[ptr] = limitsort & 0x80000000 ? 1 : 0;	 ptr++;

		// insert 58 nulls
		memset(data + ptr, 0, 58);	ptr += 58;
	}
	else if (type==MHOD_SPLDATA)
	{
		const unsigned int ruleCount = rule.size();

		if (16+136+ (ruleCount*(124+515)) > datasize) return -1;

		// put "SLst" header
		data[ptr]='S';data[ptr+1]='L';data[ptr+2]='s';data[ptr+3]='t';
		ptr+=4;
		put4(unk5,&data[ptr]); ptr+=4;
		put4(ruleCount,&data[ptr]); ptr+=4;
		put4(rules_operator,&data[ptr]); ptr+=4;
		memset(data + ptr, 0, 120); ptr+=120;

		for (unsigned int i=0;i<ruleCount;i++)
		{
			SPLRule *r = rule[i];
			ASSERT(r);
			if(r == NULL)
				continue;

			put4(r->field,&data[ptr]); ptr+=4;
			put4(r->action,&data[ptr]); ptr+=4;
			memset(data + ptr, 0, 44); ptr+=44;
			put4(r->length,&data[ptr]); ptr+=4;

			const bool hasString = iPod_slst::GetFieldType(r->field) == iPod_slst::ftString;
			if(hasString)
			{
				// Byte swap the characters
				unsigned char *c = (unsigned char*)r->string;
				for(unsigned int i=0; i<r->length; i+=2)
				{
					data[ptr + i] = *(c + i + 1);
					data[ptr + i + 1] = *(c + i);
				}

				ptr += r->length;
			}
			else
			{
				put8(r->fromvalue,&data[ptr]); ptr+=8;
				put8(r->fromdate,&data[ptr]); ptr+=8;
				put8(r->fromunits,&data[ptr]); ptr+=8;
				put8(r->tovalue,&data[ptr]); ptr+=8;
				put8(r->todate,&data[ptr]); ptr+=8;
				put8(r->tounits,&data[ptr]); ptr+=8;

				put4(r->unk1,&data[ptr]); ptr+=4;
				put4(r->unk2,&data[ptr]); ptr+=4;
				put4(r->unk3,&data[ptr]); ptr+=4;
				put4(r->unk4,&data[ptr]); ptr+=4;
				put4(r->unk5,&data[ptr]); ptr+=4;

			}
		} // end for
	}
	else if(type == MHOD_PLAYLIST)
	{
		if (16+20 > datasize) return -1;

		rev4(position,&data[ptr]);  // position in playlist
		ptr+=4;
		rev4(0,&data[ptr]);  // four nulls
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
	}
	else	// not a known type, use the binary
	{
		// check for binary size
		if (length+headsize>datasize) return -1;
		for (unsigned int i=0;i<length;i++)
		{
			data[ptr]=binary[i];
			ptr++;
		}
	}

	// fix the total size
	rev4(ptr,&data[8]);

	return ptr;
}

void iPod_mhod::SetString(const wchar_t *string)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhod_SetString);
#endif

	if(!string)
		return;

	delete [] str;
	length = 0;

	unsigned int stringLen = min(wcslen(string), 512);
	str = new wchar_t[stringLen + 1];
	memset(str, 0, sizeof(wchar_t) * (stringLen + 1));

	if(str)
	{
		wcsncpy(str, string, stringLen);
		length = stringLen * 2;
	}
}

bool iPod_mhod::IsSimpleStringType(const unsigned int type)
{
	switch(type)
	{
	case MHOD_TITLE:
	case MHOD_LOCATION:
	case MHOD_ALBUM:
	case MHOD_ARTIST:
	case MHOD_GENRE:
	case MHOD_FILETYPE:
	case MHOD_EQSETTING:
	case MHOD_COMMENT:
	case MHOD_CATEGORY:
	case MHOD_COMPOSER:
	case MHOD_GROUPING:
	case MHOD_DESCRIPTION:
	case MHOD_SUBTITLE:
	case MHOD_ALBUMARTIST:
	case MHOD_ARTIST_SORT:
	case MHOD_TITLE_SORT:
	case MHOD_ALBUM_SORT:
	case MHOD_ALBUMARTIST_SORT:
	case MHOD_COMPOSER_SORT:
	case MHOD_SHOW_SORT:
	case MHOD_ALBUMLIST_ALBUM:
	case MHOD_ALBUMLIST_ARTIST:
		return(true);
	}

	return(false);
}


void iPod_mhod::Duplicate(iPod_mhod *src, iPod_mhod *dst)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhod_Duplicate);
#endif

	if(src == NULL || dst == NULL)
		return;

	dst->type = src->type;
	dst->unk1 = src->unk1;
	dst->unk2 = src->unk2;
	dst->position = src->position;
	dst->length = src->length;
	dst->unk3 = src->unk3;
	dst->unk4 = src->unk4;
	dst->liveupdate = src->liveupdate;
	dst->checkrules = src->checkrules;
	dst->checklimits = src->checklimits;
	dst->limittype = src->limittype;
	dst->limitsort = src->limitsort;
	dst->limitvalue = src->limitvalue;
	dst->matchcheckedonly = src->matchcheckedonly;
	dst->limitsort_opposite = src->limitsort_opposite;
	dst->unk5 = src->unk5;
	dst->rules_operator = src->rules_operator;

	if(src->str)
	{
		dst->SetString(src->str);
	}
	else if(src->binary)
	{
		dst->binary = new unsigned char[src->length];
		if(dst->binary)
			memcpy(dst->binary, src->binary, src->length);
	}


	const unsigned int ruleLen = src->rule.size();
	for(unsigned int i=0; i<ruleLen; i++)
	{
		SPLRule *srcRule = src->rule[i];
		if(srcRule)
		{
			SPLRule *dstRule = new SPLRule;
			if(dstRule)
				memcpy(dstRule, srcRule, sizeof(SPLRule));

			dst->rule.push_back(dstRule);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// iPod_mhlp - Holds playlists
//////////////////////////////////////////////////////////////////////

iPod_mhlp::iPod_mhlp() :
mhyp(),
beingDeleted(false)
{
	// Always start off with an empty, hidden, default playlist
	ASSERT(GetChildrenCount() == 0);
	GetDefaultPlaylist();
}

iPod_mhlp::~iPod_mhlp()
{
	// This is unnecessary (and slow) to clear the vector list,
	// since the object is being destroyed anyway...
	beingDeleted = true;
	ClearPlaylists();
}

long iPod_mhlp::parse(const uint8_t *data)
{
	long ptr=0;

	//check mhlp header
	if (_strnicmp((char *)&data[ptr],"mhlp",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=rev4(&data[ptr]);
	ptr+=4;
	const unsigned long children=rev4(&data[ptr]);		// Only used locally - child count is obtained from the mhyp vector list
	ptr+=4;

	ASSERT(size_head == 0x5c);

	// skip nulls
	ptr=size_head;

	mhyp.reserve(children); // pre allocate the space, for speed

	ClearPlaylists();

	long ret;
	for (unsigned long i=0;i<children; i++)
	{
		iPod_mhyp *m = new iPod_mhyp;
		ret=m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0)
		{
			delete m;
			return ret;
		}

		// If this is really a smart playlist, we need to parse it again as a smart playlist
		if(m->FindString(MHOD_SPLPREF) != NULL)
		{
			delete m;
ptr+=ret;
continue;
			m = new iPod_slst;
			ASSERT(m);
			ret = m->parse(&data[ptr]);
			ASSERT(ret >= 0);
			if(ret < 0)
			{
				delete m;
				return ret;
			}

		}

		ptr+=ret;
		mhyp.push_back(m);
	}

	return ptr;
}

long iPod_mhlp::write(unsigned char * data, const unsigned long datasize, int index)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlp_write);
#endif

	const unsigned int headsize=0x5c;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhlp header
	data[0]='m';data[1]='h';data[2]='l';data[3]='p';
	ptr+=4;

	// write sizes
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;

	// write num of children
	const unsigned long children = GetChildrenCount();
	rev4(children,&data[ptr]);
	ptr+=4;

	// fill up the rest of the header with nulls
	unsigned int i;
	for (i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	long ret;
	for (i=0;i<children; i++)
	{
		ret=mhyp[i]->write(&data[ptr],datasize-ptr,index);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		ptr+=ret;
	}

	return ptr;
}

iPod_mhyp * iPod_mhlp::AddPlaylist()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlp_AddPlaylist);
#endif

	iPod_mhyp * m = new iPod_mhyp;
	ASSERT(m);
	if (m != NULL)
	{
		mhyp.push_back(m);
		return m;
	}
	else return NULL;
}


iPod_mhyp * iPod_mhlp::FindPlaylist(const unsigned __int64 playlistID)
{
	const unsigned long count = GetChildrenCount();
	for (unsigned long i=0; i<count; i++)
	{
		iPod_mhyp * m = GetPlaylist(i);
		if (m->playlistID == playlistID)
			return m;
	}
	return NULL;
}


// deletes the playlist at a position
bool iPod_mhlp::DeletePlaylist(const unsigned long pos)
{
	if (GetChildrenCount() > pos)
	{
		iPod_mhyp *m = GetPlaylist(pos);
		mhyp.erase(mhyp.begin() + pos);
		delete m;
		return true;
	}
	else return false;
}

bool iPod_mhlp::DeletePlaylistByID(const unsigned __int64 playlistID)
{
	if(playlistID == 0)
		return(false);

	const unsigned int count = GetChildrenCount();
	for(unsigned int i=0; i<count; i++)
	{
		iPod_mhyp *m = GetPlaylist(i);
		ASSERT(m);
		if(m == NULL)
			continue;

		if(m->playlistID == playlistID)
			return(DeletePlaylist(i));
	}

	return(false);
}



iPod_mhyp * iPod_mhlp::GetDefaultPlaylist()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlp_GetDefaultPlaylist);
#endif

	if (!mhyp.empty())
		return GetPlaylist(0);
	else
	{
		// Create a new hidden playlist, and set a default title
		iPod_mhyp * playlist = AddPlaylist();
		ASSERT(playlist);
		if(playlist)
		{
			playlist->hidden = 1;

			iPod_mhod *mhod = playlist->AddString(MHOD_TITLE);
			if(mhod)
				mhod->SetString(L"iPod");
		}

		return playlist;
	}
}

bool iPod_mhlp::ClearPlaylists(const bool createDefaultPlaylist)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlp_ClearPlaylists);
#endif

	const unsigned long count = GetChildrenCount();
	for (unsigned long i=0;i<count;i++)
	{
		iPod_mhyp *m=GetPlaylist(i);
		delete m;
	}

	if(!beingDeleted)
		mhyp.clear();

	// create the default playlist again, if it's gone
	// XXX - Normally this is the right thing to do, but if we
	// are about to parse the playlists from the iPod, we can't create
	// the default playlist.  Doing so will create two default/hidden
	// playlists - this one and the one that will be created when
	// the playlists are parsed!
	if(createDefaultPlaylist)
		GetDefaultPlaylist();

	return true;
}

void iPod_mhlp::RemoveDeadPlaylistEntries(iPod_mhlt *mhlt)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhlp_RemoveDeadPlaylistEntries);
#endif

	if(mhlt == NULL)
		return;

	const unsigned int playlistCount = GetChildrenCount();
	for(unsigned int i=0; i<playlistCount; i++)
	{
		iPod_mhyp *m = mhyp[i];
		ASSERT(m);
		if(m == NULL)
			continue;

		std::vector<unsigned int> deleteList;

		const unsigned int songCount = m->GetMhipChildrenCount();
		for(unsigned int j=0; j<songCount; j++)
		{
			iPod_mhip *p = m->mhip.at(j);
			ASSERT(p);
			if(p == NULL)
				continue;

			if(mhlt->GetTrackByID(p->songindex) != NULL)
				continue;

			// Found a dead song
			deleteList.push_back(p->songindex);
		}

		const unsigned int deleteCount = deleteList.size();
		for(unsigned int k=0; k<deleteCount; k++)
		{
			const bool retval = m->DeletePlaylistEntryByID(deleteList[k]);
			ASSERT(retval == true);
		}
	}
}

void iPod_mhlp::SortPlaylists() {
	std::sort(mhyp.begin(),mhyp.end(),iPod_mhyp());
}

//////////////////////////////////////////////////////////////////////
// iPod_mhyp - A Playlist
//////////////////////////////////////////////////////////////////////

iPod_mhyp::iPod_mhyp() :
hidden(0),
timestamp(0),
unk3(0),
numStringMHODs(0),
podcastflag(0),
numLibraryMHODs(0),
mhod(),
mhip(),
mhit(NULL),
isSmartPlaylist(false),
isPopulated(true),			// consider normal playlists to be populated always
writeLibraryMHODs(true)
{
	timestamp = wintime_to_mactime(time(NULL));

	// Create a highly randomized 64 bit value for the playlistID
	playlistID = Generate64BitID();
}

iPod_mhyp::~iPod_mhyp()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhyp_destructor);
#endif

	const unsigned long mhodCount = GetMhodChildrenCount();
	const unsigned long mhipCount = GetMhipChildrenCount();
	unsigned long i;
	for (i=0;i<mhodCount;i++)
		delete mhod[i];
	for (i=0;i<mhipCount;i++)
		delete mhip[i];
}

long iPod_mhyp::parse(const uint8_t *data)
{
	size_t ptr=0;

	//check mhyp header
	if (_strnicmp((char *)&data[ptr],"mhyp",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=read_uint32_t(data, ptr);
	size_total=read_uint32_t(data, ptr);

	ASSERT(size_head == 0x6c || size_head == 0x8c);

	const unsigned long mhodnum=read_uint32_t(data, ptr); // only used locally
	const unsigned long numsongs=read_uint32_t(data, ptr); // only used locally
	hidden=read_uint32_t(data, ptr);
	timestamp=read_uint32_t(data, ptr);
	playlistID = rev8(get8(&data[ptr]));
	if(playlistID == 0)
	{
		// Force the playlistID to be a valid value.
		// This may not always be the right thing to do, but I can't think of any reason why it wouldn't be ok...
		playlistID = Generate64BitID();
	}
	ptr+=8;
	unk3=read_uint32_t(data, ptr);
	unsigned long temp = rev4(&data[ptr]);
	numStringMHODs = temp && 0xFFFF;
	podcastflag = (uint16_t)(temp >> 16);
	ptr+=4;
	numLibraryMHODs=read_uint32_t(data, ptr);

	ptr=size_head;

	long ret;
	unsigned long i;

	mhod.reserve(mhodnum);	// pre allocate the space, for speed
	mhip.reserve(numsongs);

	for (i=0;i<mhodnum;i++)
	{
		iPod_mhod *m = new iPod_mhod;

		// parseSmartPlaylists is an optimization for when dealing with smart playlists.
		// Since the playlist has to be parsed in order to determine if it is a smart playlist,
		// and if it is, parsed again as a smart playlist, this optimization prevents some duplicate parsing.
		m->parseSmartPlaylists = isSmartPlaylist;
		ret=m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0)
		{
			delete m;
			continue;
			return ret;
		}

		ptr+=ret;

		// Don't add Type 52 MHODs - if the file changes, the indexes are no longer valid
		if(m->type == MHOD_LIBRARY || m->type == MHOD_LIBRARY_LETTER)
		{
			delete m;
			continue;
		}

		// Reset the Type 52 MHOD count to zero
		numLibraryMHODs = 0;

		if(m->type == MHOD_PLAYLIST)
			delete m; // fuck 'em!
		else
			mhod.push_back(m);

	}

	for (i=0;i<numsongs;i++)
	{
		iPod_mhip *m = new iPod_mhip;
		ret=m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if (ret<0)
		{
			delete m;
			return ret;
		}
		else 
		{
			ptr+=ret;
		}
		mhip.push_back(m);
	}

	return ptr;
}

long iPod_mhyp::write(unsigned char * data, const unsigned long datasize, int index)
{

	const unsigned int headsize=0x6c;
	// check for header size
	ASSERT(headsize <= datasize);
	if (headsize>datasize) return -1;

	size_t ptr=0;

	// List of Type 52 MHODs to write
	unsigned int indexType[] = { TYPE52_SONG_NAME, TYPE52_ARTIST, TYPE52_ALBUM, TYPE52_GENRE, TYPE52_COMPOSER };
	const unsigned int indexTypeCount = sizeof(indexType) / sizeof(unsigned int);

	//write mhyp header
	data[0]='m';data[1]='h';data[2]='y';data[3]='p';
	ptr+=4;

	// write sizes
	write_uint32_t(data, ptr, headsize);// header size
	write_uint32_t(data, ptr, 0); // placeholder for total size (fill in later)

	writeLibraryMHODs = true;
	bool writeType52MHOD = writeLibraryMHODs && (hidden > 0 && mhit != NULL);

	// fill in stuff
	const unsigned long mhodnum = GetMhodChildrenCount();

	numStringMHODs = (uint16_t)mhodnum;

	if(writeType52MHOD)
	{
			write_uint32_t(data, ptr, mhodnum + indexTypeCount); // Include the extra MHOD Type 52s that aren't in the MHOD list
		numLibraryMHODs = indexTypeCount;
	}
	else
	{
			write_uint32_t(data, ptr, mhodnum);
	}

	const unsigned long numsongs = GetMhipChildrenCount();
	write_uint32_t(data, ptr, numsongs);
	write_uint32_t(data, ptr, hidden);
	write_uint32_t(data, ptr, timestamp);

	put8(rev8(playlistID),&data[ptr]);
	ptr+=8;
	write_uint32_t(data, ptr, unk3);
	unsigned long temp = numStringMHODs | podcastflag << 16;
	rev4(temp,&data[ptr]);
	ptr+=4;
	write_uint32_t(data, ptr, numLibraryMHODs);

	unsigned long i;
	// fill up the rest of the header with nulls
	for (i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	long ret;
	for (i=0;i<mhodnum; i++)
	{
		ret=mhod[i]->write(&data[ptr],datasize-ptr);
		ASSERT(ret >= 0);
		if (ret<0) return ret;
		else ptr+=ret;
	}

	// If this is the default hidden playlist, create the type 52 MHODs that are used to accelerate the browse menus
	if(writeType52MHOD)
	{

		/*
		header identifier	| 4 | mhod
		header length 		| 4 | size of the mhod header
		total length 		| 4 | size of the header and all the index entries
		type 				| 4 | the type indicator ( 52 )
		unk1 				| 4 | unknown (always zero)
		unk2 				| 4 | unknown (always zero)
		index type 			| 4 | what this index is sorted on (see list above)
		count 				| 4 | number of entries. Always the same as the number of entries in the playlist, which is the same as the number of songs on the iPod.
		null padding 		| 40| lots of padding
		-----
		72 bytes (0x48)
		index entries 		| 4 * count | 	The index entries themselves. This is an index into the mhit list, in order, starting from 0 for the first mhit.
		*/
		for(unsigned int i=0; i<indexTypeCount; i++)
		{
			unsigned int mhodType = 0;
			const unsigned int mhod52Type = indexType[i];

			// Map the index type to the MHOD type
			switch(mhod52Type)
			{
			case TYPE52_SONG_NAME:
				mhodType = MHOD_TITLE;
				break;
			case TYPE52_ALBUM:
				mhodType = MHOD_ALBUM;
				break;
			case TYPE52_ARTIST:
				mhodType = MHOD_ARTIST;
				break;
			case TYPE52_GENRE:
				mhodType = MHOD_GENRE;
				break;
			case TYPE52_COMPOSER:
				mhodType = MHOD_COMPOSER;
				break;
			default:
				ASSERT(0);		// unknown type
				mhodType = 0;
			}

			if(mhodType == 0)
				continue;

			ASSERT(mhit->size() == numsongs);
			if(mhit->size() != numsongs)
				continue;


			// Start writing the MHOD...
			const unsigned long headsize = 0x18;
			const unsigned long totalsize = 0x48 + (4 * numsongs);

			//write mhod header
			data[ptr++]='m';data[ptr++]='h';data[ptr++]='o';data[ptr++]='d';

			// write sizes


			write_uint32_t(data, ptr, headsize); // header size
			write_uint32_t(data, ptr, totalsize); // total size = 72 * (4 * number of songs)

			// This is type 52 MHOD
			write_uint32_t(data, ptr, MHOD_LIBRARY); 

			// Unknowns (always zero)
			unsigned long unk1 = 0, unk2 = 0;
			write_uint32_t(data, ptr, unk1); 
			write_uint32_t(data, ptr, unk2); 

			// What kind of index is this?
			write_uint32_t(data, ptr, mhod52Type); 

			// Song Count
			write_uint32_t(data, ptr, numsongs); 

			// 40 NULLs
			for(unsigned int j=0;j<40;j++)
				data[ptr++]=0;

			// Build the sort by string vector list
			std::vector<indexMhit*> strList;
			strList.reserve(numsongs);

			unsigned int ki = 0;
			/*
			iPod_mhlt::mhit_map_t::const_iterator begin = mhit->begin();
			iPod_mhlt::mhit_map_t::const_iterator end   = mhit->end();
			for(iPod_mhlt::mhit_map_t::const_iterator it = begin; it != end; it++)
			{
			iPod_mhit *m = static_cast<iPod_mhit*>((*it).second);
			*/
			int kl = mhip.size();
			for(int k=0; k < kl; k++) {

				iPod_mhit *m = mhit->find(mhip.at(k)->songindex)->second;

				ASSERT(m != NULL);
				if(m == NULL)
					continue;

				indexMhit *foo = new indexMhit;
				foo->index = ki++;


				iPod_mhod *d;
				if(mhod52Type == TYPE52_COMPOSER) {
					foo->track = 0;
					foo->str[0] = L"";
					foo->str[1] = L"";
					d = m->FindString(MHOD_COMPOSER);
					foo->str[2] = d?d->str:L"";
				} else {
					foo->track = (int)m->tracknum;
					d = (mhod52Type >= TYPE52_GENRE)?m->FindString(MHOD_GENRE):NULL;
					foo->str[0] = d?d->str:L"";
					d = (mhod52Type >= TYPE52_ARTIST)?m->FindString(MHOD_ARTIST):NULL;
					foo->str[1] = d?d->str:L"";
					d = (mhod52Type >= TYPE52_ALBUM)?m->FindString(MHOD_ALBUM):NULL;
					foo->str[2] = d?d->str:L"";
				}
				d = m->FindString(MHOD_TITLE);
				foo->str[3] = d?d->str:L"";

				strList.push_back(foo);
			}

			// Sort the list alphabetically
			std::sort(strList.begin(), strList.end(), indexMhit());

			// Write out the index entries
			const unsigned indexCount = strList.size();
			ASSERT(indexCount == numsongs);
			for(unsigned int mi =0; mi<indexCount; mi++)
			{
				const unsigned int index = strList[mi]->index;
				write_uint32_t(data, ptr, index); 
			}

			// Free the list of indexMhits
			for(unsigned int li=0;li<indexCount;li++)
			{
				delete strList[li];
			}
		}
	}

	int SongNum = 0;
	for (i=0;i<numsongs;i++)
	{
		//FUCKO: use index here to write out podcast list differently
		if(index == 3 || mhip[i]->podcastgroupflag == 0) 
		{
			if(mhip[i]->podcastgroupflag == 0)
				SongNum++;
			unsigned long temp=0;
			if(index == 2)
			{
				temp = mhip[i]->podcastgroupref;
				mhip[i]->podcastgroupref = 0;
			}
			ret=mhip[i]->write(&data[ptr],datasize-ptr, SongNum);
			if(index == 2)
				mhip[i]->podcastgroupref=temp;
			ASSERT(ret >= 0);
			if (ret<0) 
				return ret;
			else
				ptr+=ret;	
		}
	}

	// fix the total size
	rev4(ptr,&data[8]);
	// fix number of songs written
	if(index == 2) rev4(SongNum,&data[16]);

	return ptr;
}

long iPod_mhyp::AddPlaylistEntry(iPod_mhip * entry, const unsigned long id)
{
	entry = new iPod_mhip;
	ASSERT(entry != NULL);
	if (entry !=NULL)
	{
		entry->songindex=id;
		entry->timestamp = wintime_to_mactime(time(NULL));

		mhip.push_back(entry);
		return GetMhipChildrenCount()-1;
	}
	else return -1;
}

long iPod_mhyp::FindPlaylistEntry(const unsigned long id) const
{
	const unsigned long children = GetMhipChildrenCount();
	for (unsigned long i=0;i<children;i++)
	{
		if (mhip.at(i)->songindex==id)
			return i;
	}
	return -1;
}

bool iPod_mhyp::DeletePlaylistEntry(unsigned long pos)
{
	if (GetMhipChildrenCount() >= pos)
	{
		iPod_mhip * m = GetPlaylistEntry(pos);
		if (pos < mhip.size())
		{
			mhip.erase(mhip.begin() + pos);
		}
		delete m;
		return true;
	}
	return false;
}

bool iPod_mhyp::DeletePlaylistEntryByID(unsigned long songindex)
{
	// Search the list of mhips until the matching mhip(s) are found
	bool retval = false;
	unsigned int count = GetMhipChildrenCount();
	for(unsigned int i=0; i<count; i++)
	{
		iPod_mhip *m = GetPlaylistEntry(i);
		if(m->songindex == songindex)
		{
			DeletePlaylistEntry(i);
			count = GetMhipChildrenCount();
			i=0;
			retval = true;
			continue;
		}
	}

	return(retval);
}


bool iPod_mhyp::ClearPlaylist()
{
	const unsigned long count = GetMhipChildrenCount();
	for (unsigned long i=0;i<count;i++)
	{
		iPod_mhip * m = GetPlaylistEntry(i);
		delete m;
	}
	mhip.clear();
	return true;
}

long iPod_mhyp::PopulatePlaylist(iPod_mhlt * tracks, int hidden_field)
{
	ASSERT(tracks != NULL);
	if(tracks == NULL)
		return(-1);

	const unsigned long trackCount = tracks->GetChildrenCount();
	if(trackCount == 0)
		return(0);

	ClearPlaylist();

	// Speed up getting the id as follows:
	// Iterate the whole tracks->mhit map, storing the ids in a local vector
	std::vector<unsigned long> ids;
	ids.reserve(trackCount);

	iPod_mhlt::mhit_map_t::const_iterator begin = tracks->mhit.begin();
	iPod_mhlt::mhit_map_t::const_iterator end   = tracks->mhit.end();
	for(iPod_mhlt::mhit_map_t::const_iterator it = begin; it != end; it++)
		ids.push_back(static_cast<unsigned long>((*it).first));

	for (unsigned long i=0;i<trackCount;i++)
	{
		// Add the playlist entry locally rather than using
		// AddPlaylistEntry() for speed optimization reasons
		iPod_mhip *entry = new iPod_mhip;
		ASSERT(entry != NULL);
		if(entry)
		{
			entry->songindex = ids[i];
			mhip.push_back(entry);
		}
	}

	hidden=hidden_field;
	return GetMhipChildrenCount();
}

iPod_mhod * iPod_mhyp::AddString(const int type)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhyp_AddString);
#endif

	iPod_mhod * m;
	if (type)
	{
		m = FindString(type);
		if (m!=NULL) return m;
	}

	m=new iPod_mhod;
	if (m!=NULL && type) m->type=type;

	mhod.push_back(m);
	return m;
}


iPod_mhod * iPod_mhyp::FindString(const unsigned long type)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhyp_FindString);
#endif

	const unsigned long children = GetMhodChildrenCount();
	for (unsigned long i=0;i<children;i++)
	{
		if (mhod[i]->type == type) return mhod[i];
	}

	return NULL;
}

unsigned long iPod_mhyp::DeleteString(const unsigned long type)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhyp_DeleteString);
#endif

	unsigned long count=0;

	for (unsigned long i=0; i != GetMhodChildrenCount(); i++)
	{
		if (mhod[i]->type == type)
		{
			iPod_mhod * m = mhod.at(i);
			mhod.erase(mhod.begin() + i);
			delete m;
			i = i > 0 ? i - 1 : 0;  // do this to ensure that it checks the new entry in position i next
			count++;
		}
	}
	return count;
}

void iPod_mhyp::SetPlaylistTitle(const wchar_t *string)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhyp_SetPlaylistTitle);
#endif

	if(string == NULL)
		return;

	iPod_mhod *mhod = AddString(MHOD_TITLE);
	ASSERT(mhod);
	if(mhod)
		mhod->SetString(string);
}

void iPod_mhyp::Duplicate(iPod_mhyp *src, iPod_mhyp *dst)
{
	if(src == NULL || dst == NULL)
		return;

	dst->hidden = src->hidden;
	dst->timestamp = src->timestamp;
	dst->playlistID = src->playlistID;
	dst->unk3 = src->unk3;
	dst->numStringMHODs = src->numStringMHODs;
	dst->podcastflag = src->podcastflag;
	dst->numLibraryMHODs = src->numLibraryMHODs;
	dst->isSmartPlaylist = src->isSmartPlaylist;
	dst->isPopulated = src->isPopulated;

	const unsigned int mhodCount = src->mhod.size();
	const unsigned int mhipCount = src->mhip.size();

	unsigned int i;
	for(i=0; i<mhodCount; i++)
	{
		iPod_mhod *mhod = dst->AddString();
		ASSERT(mhod);
		if(mhod)
		{
			iPod_mhod *srcMHOD = src->mhod[i];
			ASSERT(srcMHOD);
			if(srcMHOD)
				mhod->Duplicate(srcMHOD, mhod);
		}
	}

	for(i=0; i<mhipCount; i++)
	{
		iPod_mhip *mhip = NULL;
		if(dst->AddPlaylistEntry(mhip) >= 0 && mhip != NULL)
		{
			iPod_mhip *srcMHIP = src->mhip[i];
			ASSERT(srcMHIP);
			if(srcMHIP)
				mhip->Duplicate(srcMHIP, mhip);
		}
	}
}

bool iPod_mhyp::operator()(iPod_mhyp*& one, iPod_mhyp*& two) {
	if(one->hidden & 0xff) return true;
	if(two->hidden & 0xff) return false;
	wchar_t * a  = one->FindString(MHOD_TITLE)->str;
	wchar_t * b  = two->FindString(MHOD_TITLE)->str;
	return _wcsicmp(a?a:L"",b?b:L"") < 0;
}

//////////////////////////////////////////////////////////////////////
// iPod_mhip - Playlist Entry
//////////////////////////////////////////////////////////////////////
iPod_mhip::iPod_mhip() :
dataobjectcount(1),
podcastgroupflag(0),
groupid(0),
songindex(0),
timestamp(0),
podcastgroupref(0)
{
}

iPod_mhip::~iPod_mhip()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhip_destructor);
#endif
}

long iPod_mhip::parse(const uint8_t *data)
{
	long ptr=0;

	//check mhyp header
	if (_strnicmp((char *)&data[ptr],"mhip",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=rev4(&data[ptr]);
	ptr+=4;
	size_total=rev4(&data[ptr]);
	ptr+=4;

	ASSERT(size_head == 0x4c);

	// read in the useful info
	dataobjectcount=rev4(&data[ptr]); //data object count
	ptr+=4;
	podcastgroupflag=rev4(&data[ptr]); //podcast group flag
	ptr+=4;
	groupid=rev4(&data[ptr]); // group id
	ptr+=4;
	songindex=rev4(&data[ptr]); //trackid
	ptr+=4;
	timestamp=rev4(&data[ptr]); // timestamp
	ptr+=4;
	podcastgroupref=rev4(&data[ptr]); // group ref
	ptr+=4;

	ptr=size_head;

	// dump the mhod after the mhip, as it's just a position number in the playlist
	//   and useless to read in since we get them in order anyway
	for(uint32_t i=0; i!=dataobjectcount; i++) {
		iPod_mhod *m = new iPod_mhod;
		long ret = m->parse(&data[ptr]);
		ASSERT(ret >= 0);
		if(ret<0) return ret;
		ptr+=ret;
		if(m->type == 100) delete m; //fuck 'em
		else mhod.push_back(m);
	}

	return ptr;
}


long iPod_mhip::write(unsigned char * data, const unsigned long datasize, int entrynum)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhip_write);
#endif

	long ptr=0;

	const unsigned int headsize=0x4c;
	// check for header size and mhod size
	if (headsize+0x2c>datasize)
	{
		ASSERT(0);
		return -1;
	}

	//write mhip header
	data[0]='m';data[1]='h';data[2]='i';data[3]='p';
	ptr+=4;

	// write sizes
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	//rev4(headsize,&data[ptr]);  // mhips have no children or subdata
	ptr+=4;

	// fill in stuff
	rev4(mhod.size() + ((podcastgroupflag!=0)?0:1),&data[ptr]);
	ptr+=4;
	rev4(podcastgroupflag,&data[ptr]);
	ptr+=4;
	rev4(groupid,&data[ptr]);
	ptr+=4;
	rev4(songindex,&data[ptr]);
	ptr+=4;
	rev4(timestamp,&data[ptr]);
	ptr+=4;
	rev4(podcastgroupref,&data[ptr]);
	ptr+=4;

	// fill up the rest of the header with nulls
	for (unsigned int i=ptr;i<headsize;i++)
		data[i]=0;
	ptr=headsize;

	if(!podcastgroupflag) {
		// create an faked up mhod type 100 for position info
		//   (required at this point, albeit seemingly useless.. type 100 mhods are weird)
		data[ptr]='m';data[ptr+1]='h';data[ptr+2]='o';data[ptr+3]='d';
		ptr+=4;
		rev4(0x18,&data[ptr]);	// header size
		ptr+=4;
		rev4(0x2c,&data[ptr]);  // total size
		ptr+=4;
		rev4(100,&data[ptr]);  // type
		ptr+=4;
		rev4(0,&data[ptr]);  // two nulls
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		rev4(entrynum,&data[ptr]);  // position in playlist
		ptr+=4;
		rev4(0,&data[ptr]);  // four nulls
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		rev4(0,&data[ptr]);
		ptr+=4;
		// the above code could be more optimized, but this is simpler to read, methinks
	}

	for(unsigned long i = 0; i < mhod.size(); i++) {
		long ret = mhod[i]->write(&data[ptr],datasize-ptr);
		ASSERT(ret >= 0);
		if(ret<0) return ret;
		else ptr+=ret;
	}
	rev4(ptr,&data[8]);
	return ptr;
}

void iPod_mhip::Duplicate(iPod_mhip *src, iPod_mhip *dst)
{
	if(src == NULL || dst == NULL)
		return;

	dst->dataobjectcount = src->dataobjectcount;
	dst->podcastgroupflag = src->podcastgroupflag;
	dst->groupid = src->groupid;
	dst->songindex = src->songindex;
	dst->timestamp = src->timestamp;
	dst->podcastgroupref = src->podcastgroupref;
}



//////////////////////////////////////////////////////////////////////
// iPod_slst - Smart Playlist
//////////////////////////////////////////////////////////////////////

iPod_slst::iPod_slst() :
splPref(NULL),
splData(NULL)
{
	isSmartPlaylist = true;
	isPopulated = false;	// smart playlists are not considered populated by default
	Reset();
}

iPod_slst::~iPod_slst()
{
	RemoveAllRules();
}


iPod_slst::FieldType iPod_slst::GetFieldType(const unsigned long field)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_slst_GetFieldType);
#endif

	switch(field)
	{
	case SPLFIELD_SONG_NAME:
	case SPLFIELD_ALBUM:
	case SPLFIELD_ARTIST:
	case SPLFIELD_GENRE:
	case SPLFIELD_KIND:
	case SPLFIELD_COMMENT:
	case SPLFIELD_COMPOSER:
	case SPLFIELD_GROUPING:
		return(ftString);

	case SPLFIELD_BITRATE:
	case SPLFIELD_SAMPLE_RATE:
	case SPLFIELD_YEAR:
	case SPLFIELD_TRACKNUMBER:
	case SPLFIELD_SIZE:
	case SPLFIELD_PLAYCOUNT:
	case SPLFIELD_DISC_NUMBER:
	case SPLFIELD_BPM:
	case SPLFIELD_RATING:
	case SPLFIELD_TIME:				// time is the length of the track in milliseconds
	case SPLFIELD_VIDEO_KIND:
		return(ftInt);

	case SPLFIELD_COMPILATION:
		return(ftBoolean);

	case SPLFIELD_DATE_MODIFIED:
	case SPLFIELD_DATE_ADDED:
	case SPLFIELD_LAST_PLAYED:
		return(ftDate);

	case SPLFIELD_PLAYLIST:
		return(ftPlaylist);

	default:
		// Unknown field type
		ASSERT(0);
	}

	return(ftUnknown);
}

iPod_slst::ActionType iPod_slst::GetActionType(const unsigned long field, const unsigned long action)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_slst_GetActionType);
#endif

	const FieldType fieldType = GetFieldType(field);
	switch(fieldType)
	{
	case ftString:
		switch(action)
		{
		case SPLACTION_IS_STRING:
		case SPLACTION_IS_NOT:
		case SPLACTION_CONTAINS:
		case SPLACTION_DOES_NOT_CONTAIN:
		case SPLACTION_STARTS_WITH:
		case SPLACTION_DOES_NOT_START_WITH:
		case SPLACTION_ENDS_WITH:
		case SPLACTION_DOES_NOT_END_WITH:
			return(atString);

		case SPLACTION_IS_NOT_IN_THE_RANGE:
		case SPLACTION_IS_INT:
		case SPLACTION_IS_NOT_INT:
		case SPLACTION_IS_GREATER_THAN:
		case SPLACTION_IS_NOT_GREATER_THAN:
		case SPLACTION_IS_LESS_THAN:
		case SPLACTION_IS_NOT_LESS_THAN:
		case SPLACTION_IS_IN_THE_RANGE:
		case SPLACTION_IS_IN_THE_LAST:
		case SPLACTION_IS_NOT_IN_THE_LAST:
			return(atInvalid);

		default:
			// Unknown action type
			ASSERT(0);
			return(atUnknown);
		}
		break;

	case ftInt:
		switch(action)
		{
		case SPLACTION_IS_INT:
		case SPLACTION_IS_NOT_INT:
		case SPLACTION_IS_GREATER_THAN:
		case SPLACTION_IS_NOT_GREATER_THAN:
		case SPLACTION_IS_LESS_THAN:
		case SPLACTION_IS_NOT_LESS_THAN:
			return(atInt);

		case SPLACTION_IS_NOT_IN_THE_RANGE:
		case SPLACTION_IS_IN_THE_RANGE:
			return(atRange);

		case SPLACTION_IS_STRING:
		case SPLACTION_CONTAINS:
		case SPLACTION_STARTS_WITH:
		case SPLACTION_DOES_NOT_START_WITH:
		case SPLACTION_ENDS_WITH:
		case SPLACTION_DOES_NOT_END_WITH:
		case SPLACTION_IS_IN_THE_LAST:
		case SPLACTION_IS_NOT_IN_THE_LAST:
		case SPLACTION_IS_NOT:
		case SPLACTION_DOES_NOT_CONTAIN:
			return(atInvalid);

		default:
			// Unknown action type
			ASSERT(0);
			return(atUnknown);
		}
		break;

	case ftBoolean:
		return(atNone);

	case ftDate:
		switch(action)
		{
		case SPLACTION_IS_INT:
		case SPLACTION_IS_NOT_INT:
		case SPLACTION_IS_GREATER_THAN:
		case SPLACTION_IS_NOT_GREATER_THAN:
		case SPLACTION_IS_LESS_THAN:
		case SPLACTION_IS_NOT_LESS_THAN:
			return(atDate);

		case SPLACTION_IS_IN_THE_LAST:
		case SPLACTION_IS_NOT_IN_THE_LAST:
			return(atInTheLast);

		case SPLACTION_IS_IN_THE_RANGE:
		case SPLACTION_IS_NOT_IN_THE_RANGE:
			return(atRange);

		case SPLACTION_IS_STRING:
		case SPLACTION_CONTAINS:
		case SPLACTION_STARTS_WITH:
		case SPLACTION_DOES_NOT_START_WITH:
		case SPLACTION_ENDS_WITH:
		case SPLACTION_DOES_NOT_END_WITH:
		case SPLACTION_IS_NOT:
		case SPLACTION_DOES_NOT_CONTAIN:
			return(atInvalid);

		default:
			// Unknown action type
			ASSERT(0);
			return(atUnknown);
		}
		break;

	case ftPlaylist:
		switch(action)
		{
		case SPLACTION_IS_INT:
		case SPLACTION_IS_NOT_INT:
			return (atPlaylist);

		case SPLACTION_IS_GREATER_THAN:
		case SPLACTION_IS_NOT_GREATER_THAN:
		case SPLACTION_IS_LESS_THAN:
		case SPLACTION_IS_NOT_LESS_THAN:
		case SPLACTION_IS_IN_THE_LAST:
		case SPLACTION_IS_NOT_IN_THE_LAST:
		case SPLACTION_IS_IN_THE_RANGE:
		case SPLACTION_IS_NOT_IN_THE_RANGE:
		case SPLACTION_IS_STRING:
		case SPLACTION_CONTAINS:
		case SPLACTION_STARTS_WITH:
		case SPLACTION_DOES_NOT_START_WITH:
		case SPLACTION_ENDS_WITH:
		case SPLACTION_DOES_NOT_END_WITH:
		case SPLACTION_IS_NOT:
		case SPLACTION_DOES_NOT_CONTAIN:
			return (atInvalid);

		default:
			ASSERT(0);
			return(atUnknown);
		}

	case ftUnknown:
		// Unknown action type
		ASSERT(0);
		break;
	}

	return(atUnknown);
}


void iPod_slst::UpdateMHODPointers()
{
	if(IsSmartPlaylist() == false)
		return;

	splPref = AddString(MHOD_SPLPREF);
	splData = AddString(MHOD_SPLDATA);
}

void iPod_slst::SetPrefs(const bool liveupdate, const bool rules_enabled, const bool limits_enabled,
												 const unsigned long limitvalue, const unsigned long limittype, const unsigned long limitsort)
{
	UpdateMHODPointers();

	ASSERT(splPref);
	if(splPref)
	{
		splPref->liveupdate = liveupdate ? 1 : 0;
		splPref->checkrules = rules_enabled ? 1 : 0;
		splPref->checklimits = limits_enabled ? 1 : 0;
		splPref->matchcheckedonly = 0;
		splPref->limitsort_opposite = limitsort & 0x80000000 ? 1 : 0;
		splPref->limittype = limittype;
		splPref->limitsort = limitsort & 0x000000ff;
		splPref->limitvalue = limitvalue;
	}
}

unsigned long iPod_slst::GetRuleCount()
{
	UpdateMHODPointers();

	ASSERT(splData);
	if(splData == NULL)
		return(0);

	return(splData->rule.size());
}


int iPod_slst::AddRule(const unsigned long field,
											 const unsigned long action,
											 const wchar_t * string,	// use string for string based rules
											 const unsigned __int64 value,	// use value for single variable rules
											 const unsigned __int64 from,		// use from and to for range based rules
											 const unsigned __int64 to,
											 const unsigned __int64 units)	// use units for "In The Last" based rules
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_slst_AddRule);
#endif

	UpdateMHODPointers();

	ASSERT(splPref != NULL && splData != NULL);
	if (splPref == NULL || splData == NULL) return -1;

	// create a new rule
	SPLRule *r = new SPLRule;
	ASSERT(r);
	if(r == NULL)
		return(-1);

	const FieldType  ft = GetFieldType(field);
	const ActionType at = GetActionType(field, action);

	if(ft == ftUnknown || at == atUnknown)
	{
		ASSERT(0);
		return(-1);
	}


	r->field = field;
	r->action = action;
	r->unk1 = 0;
	r->unk2 = 0;
	r->unk3 = 0;
	r->unk4 = 0;
	r->unk5 = 0;

	if(ft == ftString)
	{
		// it's a string type (SetString() sets the length)
		r->SetString(string);
	}
	else
	{
		// All non-string rules currently have a length of 68 bytes
		r->length = 0x44;

		/* Values based on ActionType:
		*   int:         fromvalue = value, fromdate = 0, fromunits = 1, tovalue = value, todate = 0, tounits = 1
		*   playlist:    same as int
		*   boolean:     same as int, except fromvalue/tovalue are 1 if set, 0 if not set
		*   date:        same as int
		*   range:       same as int, except use from and to, instead of value
		*   in the last: fromvalue = 0x2dae2dae2dae2dae (SPLDATE_IDENTIFIER), fromdate = 0xffffffffffffffff - value, fromunits = seconds in period,
		*			      tovalue = 0x2dae2dae2dae2dae (SPLDATE_IDENTIFIER), todate = 0xffffffffffffffff - value, tounits = seconds in period
		*/
		switch(at)
		{
		case atInt:
		case atPlaylist:
		case atDate:
			r->fromvalue = value;
			r->fromdate = 0;
			r->fromunits = 1;
			r->tovalue = value;
			r->todate = 0;
			r->tounits = 1;
			break;
		case atBoolean:
			r->fromvalue = value > 0 ? 1 : 0;
			r->fromdate = 0;
			r->fromunits = 1;
			r->tovalue = r->fromvalue;
			r->todate = 0;
			r->tounits = 1;
			break;
		case atRange:
			r->fromvalue = from;
			r->fromdate = 0;
			r->fromunits = 1;
			r->tovalue = to;
			r->todate = 0;
			r->tounits = 1;
			break;
		case atInTheLast:
			r->fromvalue = SPLDATE_IDENTIFIER;
			r->fromdate = ConvertNumToDateValue(value);
			r->fromunits = units;
			r->tovalue = SPLDATE_IDENTIFIER;
			r->todate = ConvertNumToDateValue(value);
			r->tounits = units;
			break;
		case atNone:
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	// set isPopulated to false, since we're modifying the rules
	isPopulated = false;

	// push the rule into the mhod
	splData->rule.push_back(r);
	return(splData->rule.size() - 1);
}

int iPod_slst::AddRule(const SPLRule &rule)
{
	UpdateMHODPointers();

	ASSERT(splPref != NULL && splData != NULL);
	if (splPref == NULL || splData == NULL)
		return -1;

	SPLRule *r = new SPLRule;
	ASSERT(r);
	if(r == NULL)
		return(-1);

	r->action = rule.action;
	r->field = rule.field;
	r->fromdate = rule.fromdate;
	r->fromunits = rule.fromunits;
	r->fromvalue = rule.fromvalue;
	r->length = rule.length;
	r->SetString(rule.string);
	r->todate = rule.todate;
	r->tounits = rule.tounits;
	r->tovalue = rule.tovalue;
	r->unk1 = rule.unk1;
	r->unk2 = rule.unk2;
	r->unk3 = rule.unk3;
	r->unk4 = rule.unk4;
	r->unk5 = rule.unk5;

	// set isPopulated to false, since we're modifying the rules
	isPopulated = false;

	// push the rule into the mhod
	splData->rule.push_back(r);
	return(splData->rule.size() - 1);
}

void iPod_slst::RemoveAllRules()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_slst_RemoveAllRules);
#endif

	// Note: Don't update MHOD pointers here...if they don't already exist, there is no point in creating them
	if(splData == NULL)
		return;

	//while(splData->rule.size() > 0)
	//{
	//	// set isPopulated to false, since we're modifying the rules
	//	isPopulated = false;

	//	SPLRule *r = splData->rule[0];
	//	splData->rule.eraseindex(0);
	//	delete r;
	//}
	auto it = splData->rule.begin();
	while ( it != splData->rule.end())
	{
		// set isPopulated to false, since we're modifying the rules
		isPopulated = false;

		SPLRule* r = *it;
		it = splData->rule.erase(it);
		delete r;
	}

	splData->rule.clear();
}


void iPod_slst::Reset()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_slst_Reset);
#endif

	// Note: Don't update MHOD pointers here...if they don't already exist, there is no point in creating them
	RemoveAllRules();

	isPopulated = false;

	if(splPref)
	{
		splPref->liveupdate = 1;
		splPref->checkrules = 1;
		splPref->checklimits = 0;
		splPref->matchcheckedonly = 0;
		splPref->limittype = LIMITTYPE_SONGS;
		splPref->limitsort = LIMITSORT_RANDOM;
		splPref->limitsort_opposite = 0;
		splPref->limitvalue = 25;
	}

	if(splData)
	{
		splData->rules_operator = SPLMATCH_AND;
		splData->unk5 = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// iPod_mhdp - Class for parsing the Play Counts file
//////////////////////////////////////////////////////////////////////

iPod_mhdp::iPod_mhdp()
{
	children = 0;
	entry = 0;
}

iPod_mhdp::~iPod_mhdp()
{
	free(entry);
}

long iPod_mhdp::parse(const uint8_t *data)
{
	long ptr=0;

	//check mhdp header
	if (_strnicmp((char *)&data[ptr],"mhdp",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=rev4(&data[ptr]);
	ptr+=4;
	entrysize=rev4(&data[ptr]);
	ptr+=4;
	if (entrysize != 0x10 && entrysize != 0x0c && entrysize != 0x14 && entrysize != 0x1c)
	{
		ASSERT(0);
		return -1;	// can't understand new versions of this file
	}

	children=rev4(&data[ptr]);	 
	ptr+=4;

	// skip dummy space
	ptr=size_head;

	unsigned long i;

	entry = (PCEntry *)malloc(children * sizeof(PCEntry));

	for (i=0;i<children;i++)
	{
		PCEntry &e = entry[i];

		e.playcount=rev4(&data[ptr]);
		ptr+=4;
		e.lastplayedtime=rev4(&data[ptr]);
		ptr+=4;
		e.bookmarktime=rev4(&data[ptr]);
		ptr+=4;
		if (entrysize >= 0x10)
		{
			e.stars=rev4(&data[ptr]);
			ptr+=4;
		}
		else
			e.stars = 0;

		if (entrysize >= 0x14)
		{
			e.unk1 = rev4(&data[ptr]);
			ptr+=4;
		}
		else
			e.unk1 = 0;

		if (entrysize >= 0x1c)
		{
			e.skipcount = rev4(&data[ptr]);
			ptr+=4;
			e.skippedtime = rev4(&data[ptr]);
			ptr+=4;
		}
		else
		{
			e.skipcount = 0;
			e.skippedtime = 0;
		}
	}

	return children;
}


//////////////////////////////////////////////////////////////////////
// iPod_mhpo - Class for parsing the OTGPlaylist file
//////////////////////////////////////////////////////////////////////

iPod_mhpo::iPod_mhpo() :
size_head(0),
unk1(0),
unk2(0)
{
	idList = 0;
	children = 0;
}

iPod_mhpo::~iPod_mhpo()
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhpo_destructor);
#endif
	free(idList);
}

long iPod_mhpo::parse(const uint8_t *data)
{
	long ptr=0;

	//check mhdp header
	if (_strnicmp((char *)&data[ptr],"mhpo",4)) return -1;
	ptr+=4;

	// get sizes
	size_head=rev4(&data[ptr]);
	ptr+=4;

	unk1=rev4(&data[ptr]);
	ptr+=4;

	children=rev4(&data[ptr]);	 // Only used locally
	ptr+=4;

	unk2=rev4(&data[ptr]);
	ptr+=4;

	if (ptr!=size_head) return -1; // if this isn't true, I screwed up somewhere

	unsigned long i;

	idList = (uint32_t *)malloc(children * sizeof(uint32_t));

	for (i=0;i<children;i++)
	{
		unsigned long temp;
		temp=rev4(&data[ptr]);
		ptr+=4;
		idList[i]=temp;
	}

	return ptr;
}

long iPod_mhpo::write(unsigned char * data, const unsigned long datasize)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iPod_mhpo_write);
#endif

	long ptr=0;

	const unsigned int headsize=0x14;
	// check for header size + child size
	if (headsize+(GetChildrenCount()*4)>datasize) return -1;

	//write mhpo header
	data[0]='m';data[1]='h';data[2]='p';data[3]='o';
	ptr+=4;

	// write size
	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;

	// fill in stuff
	rev4(unk1,&data[ptr]);
	ptr+=4;
	const unsigned long children = GetChildrenCount();
	rev4(children,&data[ptr]);
	ptr+=4;
	rev4(unk2,&data[ptr]);
	ptr+=4;

	for (unsigned long i=0;i<children;i++)
	{
		rev4(idList[i],&data[ptr]);
		ptr+=4;

	}
	return ptr;
}


iPod_mhyp * iPod_mhpo::CreatePlaylistFromOTG(iPod_mhbd * iPodDB, wchar_t * name)
{
	// create playlist
	iPod_mhyp * myplaylist = iPodDB->mhsdplaylists->mhlp->AddPlaylist();
	if (myplaylist == NULL)
		return NULL;

	// set name
	iPod_mhod * playlistName = myplaylist->AddString(MHOD_TITLE);
	playlistName->SetString(name);

	unsigned long count = GetChildrenCount();
	// for every track
	for (unsigned long i=0;i<count;i++)
	{
		// find the track
		iPod_mhit * track = iPodDB->mhsdsongs->mhlt->GetTrack(idList[i]);

		// myplaylist->AddPlaylistEntry(iPod_mhip * entry, track->id);

		// Add the playlist entry locally rather than using
		// AddPlaylistEntry() for speed optimization reasons
		iPod_mhip *entry = new iPod_mhip;
		ASSERT(entry != NULL);
		if(entry && track)
		{
			entry->songindex = track->id;
			myplaylist->mhip.push_back(entry);
		}
	}

	return myplaylist;
}


//////////////////////////////////////////////////////////////////////
// iPod_mqed - Class for parsing the EQ Presets file
//////////////////////////////////////////////////////////////////////
iPod_mqed::iPod_mqed() :
unk1(1),
unk2(1),
eqList(),
size_head(0x68)
{
}

iPod_mqed::~iPod_mqed()
{
	const unsigned long count = GetChildrenCount();
	for (unsigned long i=0;i<count;i++)
	{
		iPod_pqed * m=eqList[i];
		delete m;
	}
	eqList.clear();
}

long iPod_mqed::parse(const uint8_t *data)
{
	unsigned long ptr=0;
	uint32_t i;

	//check mqed header
	if (_strnicmp((char *)&data[ptr],"mqed",4)) return -1;
	ptr+=4;

	size_head=rev4(&data[ptr]);
	ptr+=4;

	ASSERT(size_head == 0x68);

	unk1=rev4(&data[ptr]);
	ptr+=4;
	unk2=rev4(&data[ptr]);
	ptr+=4;
	unsigned long numchildren=rev4(&data[ptr]);
	ptr+=4;
	unsigned long childsize=rev4(&data[ptr]);
	ptr+=4;

	ASSERT(childsize == 588);
	if (childsize != 588) return -1;

	// skip the nulls
	ptr=size_head;

	for (i=0;i<numchildren;i++)
	{
		iPod_pqed * e = new iPod_pqed;
		long ret = e->parse(&data[ptr]);
		if (ret < 0)
		{
			delete e;
			return ret;
		}

		eqList.push_back(e);
		ptr+=ret;
	}

	return ptr;
}


long iPod_mqed::write(unsigned char * data, const unsigned long datasize)
{
	unsigned long ptr=0;
	uint32_t i;

	//write mqed header
	data[0]='m';data[1]='q';data[2]='e';data[3]='d';
	ptr+=4;

	rev4(size_head,&data[ptr]);
	ptr+=4;
	rev4(unk1,&data[ptr]);
	ptr+=4;
	rev4(unk2,&data[ptr]);
	ptr+=4;

	rev4(GetChildrenCount(),&data[ptr]);
	ptr+=4;

	rev4(588,&data[ptr]);
	ptr+=4;

	// fill with nulls
	while (ptr<size_head)
	{
		data[ptr]=0; ptr++;
	}

	// write the eq settings
	for (i=0;i<GetChildrenCount();i++)
	{
		long ret=eqList[i]->write(&data[ptr],datasize-ptr);
		if (ret <0) return -1;
		ptr+=ret;
	}

	return ptr;
}



//////////////////////////////////////////////////////////////////////
// iPod_pqed - Class for parsing the EQ Entries
//////////////////////////////////////////////////////////////////////
iPod_pqed::iPod_pqed() :
name(NULL),
preamp(0)
{
	int i;
	for (i=0;i<10;i++)
		eq[i]=0;
	for (i=0;i<5;i++)
		short_eq[i]=0;
}

iPod_pqed::~iPod_pqed()
{
	if (name) delete [] name;
}

long iPod_pqed::parse(const uint8_t *data)
{
	unsigned long ptr=0;
	uint32_t i;

	//check pqed header
	if (_strnicmp((char *)&data[ptr],"pqed",4)) return -1;
	ptr+=4;

	// get string length
	length=data[ptr]; ptr++;
	length+=data[ptr]*256; ptr++;

	name=new wchar_t[length + 1];
	memcpy(name,&data[ptr],length);
	name[length / 2] = '\0';

	// skip the nulls
	ptr=516;

	preamp = rev4(&data[ptr]);
	ptr+=4;

	unsigned long numbands;
	numbands = rev4(&data[ptr]);
	ptr+=4;

	ASSERT (numbands == 10);
	if (numbands != 10) return -1;

	for (i=0;i!=numbands;i++)
	{
		eq[i]=rev4(&data[ptr]);
		ptr+=4;
	}

	numbands = rev4(&data[ptr]);
	ptr+=4;
	ASSERT (numbands == 5);
	if (numbands != 5) return -1;

	for (i=0;i!=numbands;i++)
	{
		short_eq[i]=rev4(&data[ptr]);
		ptr+=4;
	}

	return ptr;
}

long iPod_pqed::write(unsigned char * data, const unsigned long datasize)
{
	long ptr=0;
	uint32_t i;

	//write pqed header
	data[0]='p';data[1]='q';data[2]='e';data[3]='d';
	ptr+=4;

	// write 2 byte string length
	data[ptr++]=(uint8_t)(length & 0xff); 
	data[ptr++]=(uint8_t)((length >> 8) & 0xff); 

	for (i=0;i!=length;i++)
	{
		data[ptr++]=name[i] & 0xff; 
		data[ptr++]=(name[i] >> 8) & 0xff; 
	}

	// fill rest with nulls
	while (ptr<516)
	{
		data[ptr++]=0;
	}

	rev4(preamp,&data[ptr]);
	ptr+=4;

	rev4(10,&data[ptr]);
	ptr+=4;
	for (i=0;i<10;i++)
	{
		rev4(eq[i],&data[ptr]);
		ptr+=4;
	}

	rev4(5,&data[ptr]);
	ptr+=4;
	for (i=0;i<5;i++)
	{
		rev4(short_eq[i],&data[ptr]);
		ptr+=4;
	}

	return ptr;
}




iTunesStats::iTunesStats() :
mhlt(NULL)
{
	entry = 0;
	children = 0;
}

iTunesStats::~iTunesStats()
{
	free(entry);
}

long iTunesStats::parse(const uint8_t *data)
{
	long ptr=0;

	children = rev3(&data[ptr]);
	ptr+=3;
	unk1 = rev3(&data[ptr]);
	ptr+=3;

	unsigned long i;

	entry = (iTunesStatsEntry *)malloc(children*sizeof(iTunesStatsEntry));

	for (i=0;i<children;i++)
	{
		iTunesStatsEntry &e = entry[i];

		e.entry_size = rev3(&data[ptr]);
		ptr+=3;
		e.bookmarktime = rev3(&data[ptr]);
		ptr+=3;
		e.unk1 = rev3(&data[ptr]);
		ptr+=3;
		e.unk2 = rev3(&data[ptr]);
		ptr+=3;
		e.playcount = rev3(&data[ptr]);
		ptr+=3;
		e.skippedcount = rev3(&data[ptr]);
		ptr+=3;

#ifdef _DEBUG
		// If any of these trigger an assertion, something new is in the database format
		ASSERT(e.entry_size == 0x12);
		//ASSERT(e.unk1 == 0);
		ASSERT(e.unk2 == 0);
#endif
	}

	return children;
}

// Unlike Play Counts, iTunesStats needs to be created by the application - apparently only for the bookmark time
long iTunesStats::write(unsigned char * data, const unsigned long datasize)
{
	ASSERT(mhlt != NULL);
	if(mhlt == NULL)
		return(-1);

	const unsigned int numentries = mhlt->GetChildrenCount();
	const unsigned int total_size = 6 + (numentries * sizeof(iTunesStatsEntry));		// 6 bytes for the header
	ASSERT(datasize >= total_size);
	if(datasize < total_size)
		return(-1);

	long ptr=0;

	rev3(numentries, &data[ptr]);
	ptr+=3;

	put3(unk1, &data[ptr]);
	ptr+=3;

	// Create a new iTunesStatsEntry for each song, only preserving the bookmark time
	long ret = 0;
	for(unsigned int i=0; i<numentries; i++)
	{
		iPod_mhit *mhit = mhlt->GetTrack(i);
		ASSERT(mhit);
		if(mhit == NULL)
			return(-1);

		rev3(0x12, &data[ptr]);		// Entry size
		ptr+=3;
		rev3(mhit->bookmarktime / 256, &data[ptr]);
		ptr+=3;
		rev3(0, &data[ptr]);		// iTunesStatsEntry.unk1
		ptr+=3;
		rev3(0, &data[ptr]);		// iTunesStatsEntry.unk2
		ptr+=3;
		rev3(0, &data[ptr]);		// Don't write out the playcount
		ptr+=3;
		rev3(0, &data[ptr]);		// or the skipped count
		ptr+=3;
	}

	return(ptr);
}

iTunesShuffle::iTunesShuffle() :
datasize(0)
{
	numentries=0;
	entry = 0;
}

iTunesShuffle::~iTunesShuffle()
{
	free(entry);
}

long iTunesShuffle::parse(const uint8_t *data)
{
	ASSERT(datasize > 0);

	// iTunesShuffle is simply a list of reversed 3 byte indexes
	ASSERT(datasize % 3 == 0);
	if(datasize % 3 != 0)
		return(-1);

	free(entry);
	long ptr = 0;
	numentries = datasize / 3;
	entry = (uint32_t *)malloc(numentries*sizeof(uint32_t));
	for(unsigned int i=0; i<numentries; i++)
	{
		unsigned int value = rev3(&data[ptr]);
		if(value == 0xffffff)		// This can happen if iTunesSD::playflags doesn't have the shuffle bit set
			value = 0;

		entry[i]=value;
		ptr+=3;
	}

	return(ptr);
}

long iTunesShuffle::write(unsigned char *data, const unsigned long datasize)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iTunesShuffle_write);
#endif

	const unsigned int total_size = numentries * 3;
	ASSERT(datasize >= total_size);
	if(datasize < total_size)
		return(-1);

	long ptr = 0;

	for(unsigned int i=0; i<numentries; i++)
	{
		rev3(entry[i], &data[ptr]);
		ptr+=3;
	}

	return(ptr);
}

void iTunesShuffle::Randomize()
{
	Randomize(numentries);
}

void iTunesShuffle::Randomize(const unsigned int numsongs)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__iTunesShuffle_randomize);
#endif

	if(numsongs == 0)
		return;

	if (numentries < numsongs)
	{
		free(entry);
		entry = (uint32_t *)malloc(numsongs*sizeof(uint32_t));
	}

	numentries=numsongs;

	for (uint32_t i=0;i!=numsongs;i++)
	{
		entry[i] = i;
	}

	std::random_shuffle(entry, entry+numsongs);
}

//////////////////////////////////////////////////////////////////////
// MHIA - Album Item
//////////////////////////////////////////////////////////////////////

iPod_mhia::iPod_mhia()
: unk1(0), albumid(0), type(2)
{
	dbid = Generate64BitID();
}

iPod_mhia::~iPod_mhia()
{
	for (auto obj : mhod)
	{
		delete obj;
	}
	mhod.clear();
}

long iPod_mhia::parse(const uint8_t *data)
{
	return 0;
	/*
	long ptr = 0;
	if (_strnicmp((char *)&data[ptr],"mhia",4)) return -1;
	ptr+=4;

	uint32_t size_head=rev4(&data[ptr]);
	ptr+=4;
	uint32_t size_total=rev4(&data[ptr]);
	ptr+=4;
	uint32_t num_children=rev4(&data[ptr]);
	ptr+=4;
	unk1 = rev2(&data[ptr]);
	ptr+=2;
	albumid = rev4(&data[ptr]);
	ptr+=4;
	dbid = rev8(get8(&data[ptr]));
	ptr+=8;
	type = rev4(&data[ptr]);
	ptr+=4;

	ptr = size_head; // skip nulls

	for(uint32_t i = 0; i < num_children; i++)
	{
	iPod_mhod* m = new iPod_mhod();
	long ret = m->parse(data + ptr);
	if(ret == -1)
	{
	delete m;
	return -1;
	}
	mhod.push_back(m);
	ptr += ret;
	}

	return ptr;
	*/
}

long iPod_mhia::write(unsigned char * data, const unsigned long datasize)
{
	const unsigned int headsize=0x5C;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhla header
	data[0]='m';data[1]='h';data[2]='i';data[3]='a';
	ptr+=4;

	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	rev4(-1,&data[ptr]);	// total size (put this in later)
	ptr+=4;
	rev4((uint32_t)mhod.size(),&data[ptr]); // number of strings
	ptr+=4;
	rev2(unk1, &data[ptr]);
	ptr+=2;
	rev2(albumid, &data[ptr]);
	ptr+=2;
	put8(rev8(dbid), &data[ptr]);
	ptr+=8;
	rev4(type, &data[ptr]);
	ptr+=4;

	memset(&data[ptr], 0, 60); // a whole shitload of zeroes
	ptr+=60;

	for (size_t i=0;i!=mhod.size();i++)
	{
		long ret = mhod[i]->write(data + ptr, datasize - ptr);
		if (ret == -1)
			return -1;
		ptr += ret;
	}

	// put in total size
	rev4(ptr, &data[8]);

	return ptr;
}

//////////////////////////////////////////////////////////////////////
// MHLA - Album List
//////////////////////////////////////////////////////////////////////

iPod_mhla::iPod_mhla() : nextAlbumId(400)
{
}
iPod_mhla::~iPod_mhla()
{
}

long iPod_mhla::parse(const uint8_t *data)
{
	return 0;
	/*
	long ptr=0;

	if (_strnicmp((char *)&data[ptr],"mhla",4)) return -1;
	ptr+=4;

	uint32_t size_head=rev4(&data[ptr]);
	ptr+=4;
	uint32_t num_children=rev4(&data[ptr]);
	ptr+=4;

	ptr = size_head; // skip nulls

	for(uint32_t i = 0; i < num_children; i++)
	{
	iPod_mhia* m = new iPod_mhia();
	long ret = m->parse(data + ptr);
	if(ret == -1)
	{
	delete m;
	return -1;
	}
	mhia.push_back(m);
	ptr += ret;
	}

	return ptr;
	*/
}

long iPod_mhla::write(unsigned char * data, const unsigned long datasize)
{
	const unsigned int headsize=0x5C;
	// check for header size
	if (headsize>datasize) return -1;

	long ptr=0;

	//write mhla header
	data[0]='m';data[1]='h';data[2]='l';data[3]='a';
	ptr+=4;

	rev4(headsize,&data[ptr]);	// header size
	ptr+=4;
	rev4((uint32_t)albums.size(),&data[ptr]); // number of albums
	ptr+=4;
	memset(&data[ptr], 0, 80); // a whole shitload of zeroes
	ptr+=80;

	/*
	for (size_t i=0;i!=mhia.size();i++)
	{
	long ret = mhia[i]->write(data + ptr, datasize - ptr);
	if (ret == -1)
	return -1;
	ptr += ret;
	}
	*/

	for(albums_map_t::iterator i = albums.begin(); i!=albums.end(); i++)
	{
		iPod_mhia mhia;
		mhia.albumid = i->second;
		iPod_mhod* artist = new iPod_mhod();
		artist->SetString(i->first.artist);
		artist->type = MHOD_ALBUMLIST_ARTIST;
		mhia.mhod.push_back(artist);

		iPod_mhod* album = new iPod_mhod();
		album->SetString(i->first.album);
		album->type = MHOD_ALBUMLIST_ALBUM;
		mhia.mhod.push_back(album);

		long ret = mhia.write(data + ptr, datasize - ptr);
		if (ret == -1)
			return -1;
		ptr += ret;
	}

	return ptr;
}

uint16_t iPod_mhla::GetAlbumId(const wchar_t* artist, const wchar_t* album)
{
	ArtistAlbumPair key(artist, album);
	if(albums.find(key) == albums.end())
	{
		albums[key] = nextAlbumId;
		return nextAlbumId++;
	}
	return albums[key];
}

void iPod_mhla::ClearAlbumsList()
{
	nextAlbumId = 0;
	albums.clear();
}


