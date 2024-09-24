/*
 *
 *
 * Copyright (c) 2007 Will Fisher (will.fisher@gmail.com)
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

#include "iPodArtworkDB.h"
#include <algorithm>
#include <strsafe.h>

//utilities
#define SAFEDELETE(x) {if(x) delete (x); (x)=0;}
#define SAFEFREE(x) {if(x) free(x); (x)=0;}

static __forceinline unsigned short rev1(const BYTE *data)
{
	return ((unsigned short) data[0]);
}

static __forceinline unsigned short rev1i(const BYTE *data, int &ptr)
{
	unsigned short ret = rev1(data+ptr);
	ptr+=1;
	return ret;
}

static __forceinline unsigned short rev2(const BYTE *data)
{
	unsigned short ret;
	ret = ((unsigned short) data[1]) << 8;
	ret += ((unsigned short) data[0]);
	return ret;
}

static __forceinline unsigned short rev2i(const BYTE *data, int &ptr)
{
	unsigned short ret = rev2(data+ptr);
	ptr+=2;
	return ret;
}

// get 4 bytes from data, reversed
static __forceinline unsigned int rev4(const BYTE * data)
{
	unsigned int ret;
	ret = ((unsigned long) data[3]) << 24;
	ret += ((unsigned long) data[2]) << 16;
	ret += ((unsigned long) data[1]) << 8;
	ret += ((unsigned long) data[0]);
	return ret;
}

static __forceinline unsigned int rev4i(const BYTE * data, int &ptr)
{
	unsigned int ret = rev4(data+ptr);
	ptr+=4;
	return ret;
}

// get 4 bytes from data
static __forceinline unsigned long get4(const unsigned char * data)
{
	unsigned long ret;
	ret = ((unsigned long) data[0]) << 24;
	ret += ((unsigned long) data[1]) << 16;
	ret += ((unsigned long) data[2]) << 8;
	ret += ((unsigned long) data[3]);
	return ret;
}

static __forceinline unsigned long get4i(const unsigned char * data, int &ptr)
{
	unsigned long ret = get4(data+ptr);
	ptr+=4;
	return ret;
}

// get 8 bytes from data
static __forceinline unsigned __int64 get8(const unsigned char * data)
{
	unsigned __int64 ret;
	ret = get4(data);
	ret = ret << 32;
	ret+= get4(data+4);
	return ret;
}

// get 8 bytes from data
static __forceinline unsigned __int64 get8i(const unsigned char * data, int &ptr)
{
	unsigned __int64 ret = get8(data+ptr);
	ptr+=8;
	return ret;
}

// reverse 8 bytes in place
static __forceinline unsigned __int64 rev8(unsigned __int64 number)
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

static __forceinline void putmh(const char* x, BYTE *data, int &ptr) {
	data[0+ptr]=x[0];
	data[1+ptr]=x[1];
	data[2+ptr]=x[2];
	data[3+ptr]=x[3];
	ptr+=4;
}


//write 4 bytes reversed
static __forceinline void rev4(const unsigned long number, unsigned char * data)
{
	data[3] = (unsigned char)(number >> 24) & 0xff;
	data[2] = (unsigned char)(number >> 16) & 0xff;
	data[1] = (unsigned char)(number >>  8) & 0xff;
	data[0] = (unsigned char)number & 0xff;
}

static __forceinline void rev4i(const unsigned int number, BYTE* data, int &ptr)
{
	rev4(number,data+ptr);
	ptr+=4;
}

static __forceinline void rev2(const unsigned short number, unsigned char * data)
{
	data[1] = (unsigned char)(number >>  8) & 0xff;
	data[0] = (unsigned char)number & 0xff;
}

static __forceinline void rev2i(const unsigned short number, BYTE* data, int &ptr)
{
	rev2(number,data+ptr);
	ptr+=2;
}

static __forceinline void rev1(const unsigned char number, unsigned char * data)
{
	data[0] = number;
}

static __forceinline void rev1i(const unsigned char number, BYTE* data, int &ptr)
{
	rev1(number,data+ptr);
	ptr+=1;
}

// write 8 bytes normal
static __forceinline void put8(unsigned __int64 number, unsigned char * data)
{
	data[0] = (unsigned char)(number >> 56) & 0xff;
	data[1] = (unsigned char)(number >> 48) & 0xff;
	data[2] = (unsigned char)(number >> 40) & 0xff;
	data[3] = (unsigned char)(number >> 32) & 0xff;
	data[4] = (unsigned char)(number >> 24) & 0xff;
	data[5] = (unsigned char)(number >> 16) & 0xff;
	data[6] = (unsigned char)(number >> 8) & 0xff;
	data[7] = (unsigned char)number & 0xff;
}

static __forceinline void put8i(unsigned __int64 number, unsigned char * data, int &ptr) {
	put8(number,data+ptr);
	ptr+=8;
}

static __forceinline void pad(BYTE * data, int endpoint, int& startpoint) {
	if(endpoint == startpoint) return;
	ZeroMemory(data+startpoint, endpoint - startpoint);
	startpoint = endpoint;
}

///////////////////////////////////////////////////////////////////////////////
// ArtDB

ArtDB::ArtDB() :
	headerlen(0x84),
	totallen(0),
	unk1(0),
	unk2(2),
	unk3(0),
	nextid(0x40),
	unk5(0),
	unk6(0),
	unk7(0),
	unk8(0),
	unk9(0),
	unk10(0),
	unk11(0),
	imageListDS(0),
	albumListDS(0),
	fileListDS(0)
{
}

ArtDB::~ArtDB() {
	SAFEDELETE(imageListDS);
	SAFEDELETE(albumListDS);
	SAFEDELETE(fileListDS);
}

int ArtDB::parse(BYTE * data, int len, wchar_t drive) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhfd",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x84) return -1;
	totallen = rev4i(data,ptr);
	unk1 = rev4i(data,ptr);
	unk2 = rev4i(data,ptr);
	int numchildren = rev4i(data,ptr);
	unk3 = rev4i(data,ptr);
	nextid = rev4i(data,ptr);
	unk5 = rev8(get8i(data,ptr));
	unk6 = rev8(get8i(data,ptr));
	unk7 = rev4i(data,ptr);
	unk8 = rev4i(data,ptr);
	unk9 = rev4i(data,ptr);
	unk10 = rev4i(data,ptr);
	unk11 = rev4i(data,ptr);
	
	ptr=headerlen;
	
	for(int i=0; i<numchildren; i++) {
		ArtDataSet * d = new ArtDataSet;
		int p = d->parse(data+ptr,len-ptr);
		if(p == -1) return -1;
		switch(d->index) {
			case 1: imageListDS = d; break;
			case 2: albumListDS = d; break;
			case 3: fileListDS = d; break;
			default: delete d;
		}
		ptr+=p;
	}
	if(!imageListDS) imageListDS = new ArtDataSet(1);
	if(!albumListDS) albumListDS = new ArtDataSet(2);
	if(!fileListDS) fileListDS = new ArtDataSet(3);

	for(ArtImageList::ArtImageMapIterator i = imageListDS->imageList->images.begin(); i!=imageListDS->imageList->images.end(); i++) {
		if(i->second) {
			for(auto j = i->second->dataobjs.begin(); j != i->second->dataobjs.end(); j++) {
				if((*j)->image) {
					ArtFile *f = fileListDS->fileList->getFile((*j)->image->corrid);
					if(!f) {
						f = new ArtFile();
						f->corrid = (*j)->image->corrid;
						fileListDS->fileList->files.push_back(f);
					}
					f->images.push_back(new ArtFileImage((*j)->image->ithmboffset,(*j)->image->imagesize,1));
				}
			}
		}
	}

	for(auto i = fileListDS->fileList->files.begin(); i!=fileListDS->fileList->files.end(); i++) {
		wchar_t file[MAX_PATH] = {0};
		StringCchPrintfW(file, MAX_PATH, L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",drive,(*i)->corrid);
		(*i)->file = _wcsdup(file);
		(*i)->sortImages();
	}

	return totallen;
}

int ArtDB::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhfd",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // fill total len here later
	rev4i(unk1,data,ptr);
	rev4i(unk2,data,ptr); // always seems to be "2" when iTunes writes it
	rev4i(3,data,ptr); // num children
	rev4i(unk3,data,ptr);
	rev4i(nextid,data,ptr);
	put8i(rev8(unk5),data,ptr);
	put8i(rev8(unk6),data,ptr);
	rev4i(unk7,data,ptr);
	rev4i(unk8,data,ptr);
	rev4i(unk9,data,ptr);
	rev4i(unk10,data,ptr);
	rev4i(unk11,data,ptr);
	
	pad(data,headerlen,ptr);
	
	// write out children
	int p;
	p = imageListDS->write(data+ptr,len-ptr);
	if(p<0) return -1;
	ptr+=p;

	p = albumListDS->write(data+ptr,len-ptr);
	if(p<0) return -1;
	ptr+=p;

	p = fileListDS->write(data+ptr,len-ptr);
	if(p<0) return -1;
	ptr+=p;

	rev4(ptr,&data[8]); // fill in total length

	return ptr;
}

void ArtDB::makeEmptyDB(wchar_t drive) {
	imageListDS = new ArtDataSet(1);
	albumListDS = new ArtDataSet(2);
	fileListDS = new ArtDataSet(3);

	for(auto i = fileListDS->fileList->files.begin(); i!=fileListDS->fileList->files.end(); i++) {
		wchar_t file[MAX_PATH] = {0};
		StringCchPrintfW(file, MAX_PATH, L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",drive,(*i)->corrid);
		(*i)->file = _wcsdup(file);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ArtDatSet

ArtDataSet::ArtDataSet() :
	headerlen(0x60),
	totallen(0),
	index(0),
	imageList(0),
	albumList(0),
	fileList(0)
{
}

ArtDataSet::ArtDataSet(int idx) :
	headerlen(0x60),
	totallen(0),
	index(idx),
	imageList(0),
	albumList(0),
	fileList(0)
{
	switch(idx) {
		case 1: imageList = new ArtImageList; break;
		case 2: albumList = new ArtAlbumList; break;
		case 3: fileList = new ArtFileList; break;
		default: index=0;
	}
}

ArtDataSet::~ArtDataSet() {
	SAFEDELETE(imageList);
	SAFEDELETE(albumList);
	SAFEDELETE(fileList);
}

int ArtDataSet::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhsd",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x60) return -1;
	totallen = rev4i(data,ptr);
	index = rev4i(data,ptr);

	ptr=headerlen;
	
	int p=0;
	switch(index) {
		case 1: imageList = new ArtImageList; p = imageList->parse(data+ptr, len-ptr); break;
		case 2: albumList = new ArtAlbumList; p = albumList->parse(data+ptr, len-ptr); break;
		case 3: fileList = new ArtFileList; p = fileList->parse(data+ptr, len-ptr); break;
	}
	
	if(p < 0) return -1;
	return totallen;
}

int ArtDataSet::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhsd",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // fill total len here later
	rev4i(index,data,ptr);
	pad(data,headerlen,ptr);
	int p=0;
	switch(index) {
		case 1: p=imageList->write(data+ptr, len-ptr); break;
		case 2: p=albumList->write(data+ptr, len-ptr); break;
		case 3: p=fileList->write(data+ptr, len-ptr); break;
	}
	if(p<0) return -1;
	ptr+=p;

	rev4(ptr,&data[8]); // fill in total length
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// ArtImageList
ArtImageList::ArtImageList() :
	headerlen(0x5c)
{
}

ArtImageList::~ArtImageList() {
	for(ArtImageMapIterator f = images.begin(); f != images.end(); f++)
		delete f->second;
	images.clear();
}

int ArtImageList::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhli",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x5c) return -1;
	int children = rev4i(data,ptr);

	ptr=headerlen;

	for(int i=0; i<children; i++) {
		ArtImage * f = new ArtImage;
		int p = f->parse(data+ptr,len-ptr);
		if(p<0) {delete f; return -1;}
		ptr+=p;
		images.insert(ArtImageMapPair(f->songid,f));
	}
	return ptr;
}

int ArtImageList::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhli",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(images.size(),data,ptr);
	pad(data,headerlen,ptr);
	
	for(ArtImageMapIterator f = images.begin(); f != images.end(); f++) {
		int p = f->second->write(data+ptr,len-ptr);
		if(p<0) return -1;
		ptr+=p;
	}
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// ArtImage
ArtImage::ArtImage() :
	headerlen(0x98),
	totallen(0),
	id(0),
	songid(0),
	unk4(0),
	rating(0),
	unk6(0),
	originalDate(0),
	digitizedDate(0),
	srcImageSize(0)
{
}

ArtImage::~ArtImage() 
{
	for (auto obj : dataobjs)
	{
		delete obj;
	}
	dataobjs.clear();
}

int ArtImage::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhii",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x98) return -1;
	totallen = rev4i(data,ptr);
	int numchildren = rev4i(data,ptr);
	id = rev4i(data,ptr);
	songid = rev8(get8i(data,ptr));
	unk4 = rev4i(data,ptr);
	rating = rev4i(data,ptr);
	unk6 = rev4i(data,ptr);
	originalDate = rev4i(data,ptr);
	digitizedDate = rev4i(data,ptr);
	srcImageSize = rev4i(data,ptr);

	ptr = headerlen;
	for(int i=0; i<numchildren; i++) {
		ArtDataObject *d = new ArtDataObject;
		int p = d->parse(data+ptr,len-ptr);
		if(p<0) { delete d; return -1; }
		ptr+=p;
		// fuck with d. ugh.
		if((d->type == 2 || d->type == 5) && d->data) { // this is a container mhod
			d->image = new ArtImageName;
			int p2 = d->image->parse(d->data,d->datalen);
			if(p2>0) {
				SAFEFREE(d->data);
				d->datalen=0;
			} else SAFEDELETE(d->image);
		}
		dataobjs.push_back(d);
	}
	return totallen;
}

template<class T>
BYTE *expandMemWrite(T * x, int &len, int maxsize=1024000) {
	int s = 1024;
	for(;;) {
		BYTE *r = (BYTE*)malloc(s);
		int p = x->write(r,s);
		if(p>0) { 
			len=p;
			return r;
		}
		free(r);
		s = s+s;
		if(s > maxsize) break;
	}
	return NULL;
}

int ArtImage::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhii",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // fill in total length later
	rev4i(dataobjs.size(),data,ptr);
	rev4i(id,data,ptr);
	put8i(rev8(songid),data,ptr);
	rev4i(unk4,data,ptr);
	rev4i(rating,data,ptr);
	rev4i(unk6,data,ptr);
	rev4i(originalDate,data,ptr);
	rev4i(digitizedDate,data,ptr);
	rev4i(srcImageSize,data,ptr);
	pad(data,headerlen,ptr);
	
	for(auto f = dataobjs.begin(); f != dataobjs.end(); f++) {
		if((*f)->image) {
			int len=0;
			BYTE *b = expandMemWrite((*f)->image,len);
			if(!b) return -1;
			(*f)->data = b;
			(*f)->datalen = len;
		}
		int p = (*f)->write(data+ptr,len-ptr);
		if((*f)->image) {
			SAFEFREE((*f)->data);
			(*f)->datalen=0;
		}
		if(p<0) return -1;
		ptr+=p;
	}
	rev4(ptr,&data[8]); // fill in total length
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// ArtDataObj

ArtDataObject::ArtDataObject() :
	headerlen(0x18),
	type(0),
	data(0),
	datalen(0),
	image(0),
	unk1(0)
{
}

ArtDataObject::~ArtDataObject() {
	SAFEDELETE(image);
	SAFEFREE(data);
}

int ArtDataObject::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhod",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x18) return -1;
	int totallen = rev4i(data,ptr);
	if(len < totallen) return -1;
	type = rev2i(data,ptr);
	unk1 = (unsigned char)rev1i(data,ptr);
	short padding = rev1i(data,ptr);
	ptr = headerlen;
	if(type == 3 && rev2(&data[totallen-2]) == 0)
		datalen = wcslen((wchar_t*)(data+ptr+12))*sizeof(wchar_t) + 12;
	else
		datalen = totallen - headerlen - padding;
	if(datalen > 0x400 || datalen < 0) return -1;
	this->data = (BYTE*)malloc(datalen);
	memcpy(this->data,data+ptr,datalen);

	return totallen;
}

int ArtDataObject::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhod",data,ptr);
	rev4i(headerlen,data,ptr);
	short padding = (4 - ((headerlen + datalen) % 4));// % 4;
	if(padding == 4) padding = 0;
	rev4i(headerlen+datalen+padding,data,ptr);
	rev2i(type,data,ptr);
	rev1i(unk1,data,ptr);
	rev1i((unsigned char)padding,data,ptr);
	pad(data,headerlen,ptr);
	//write data
	memcpy(data+ptr,this->data,datalen);
	ptr+=datalen;
	//add padding...
	pad(data,ptr+padding,ptr);
	return ptr;
}

void ArtDataObject::GetString(wchar_t * str, int len) {
	if(rev4(data+4) != 2) { str[0]=0; return; }//not utf-16!
	int l = (rev4(data)/sizeof(wchar_t));
	StringCchCopyN(str, len, (wchar_t*)&data[12], l);
	//lstrcpyn(str,(wchar_t*)&data[12],min(l,len));
}

void ArtDataObject::SetString(wchar_t * str) {
	SAFEFREE(data);
	datalen = wcslen(str)*sizeof(wchar_t) + 12;
	data = (BYTE*)malloc(datalen);
	rev4(wcslen(str)*sizeof(wchar_t),data);
	rev4(2,data+4); //type 2 means utf-16
	rev4(0,data+8); //unk
	memcpy(data+12,str,wcslen(str)*sizeof(wchar_t));
}

///////////////////////////////////////////////////////////////////////////////
// ArtImageName
ArtImageName::ArtImageName() :
	headerlen(0x4c),
	totallen(0),
	corrid(0),
	ithmboffset(0),
	imagesize(0),
	vpad(0),
	hpad(0),
	imgh(0),
	imgw(0),
	filename(0)
{
}

ArtImageName::~ArtImageName() {
	SAFEDELETE(filename);
}

int ArtImageName::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhni",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x4c) return -1;
	totallen = rev4i(data,ptr);
	int children = rev4i(data,ptr);
	corrid = rev4i(data,ptr);
	ithmboffset = rev4i(data,ptr);
	imagesize = rev4i(data,ptr);
	vpad = (short)rev2i(data,ptr);
	hpad = (short)rev2i(data,ptr);
	imgw = rev2i(data,ptr);
	imgh = rev2i(data,ptr);
	
	ptr = headerlen;

	if(children) {
		filename = new ArtDataObject();
		int p = filename->parse(data+ptr,len-ptr);
		if(p<0) SAFEDELETE(filename);
	}
	return totallen;
}

int ArtImageName::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhni",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // fill in totallen later
	rev4i(filename?1:0,data,ptr); //num children
	rev4i(corrid,data,ptr);
	rev4i(ithmboffset,data,ptr);
	rev4i(imagesize,data,ptr);
	rev2i(vpad,data,ptr);
	rev2i(hpad,data,ptr);
	rev2i(imgw,data,ptr);
	rev2i(imgh,data,ptr);
	pad(data,headerlen,ptr);
	if(filename) {
		int p = filename->write(data+ptr,len-ptr);
		if(p<0) return -1;
		ptr+=p;
	}
	rev4(ptr,&data[8]); // fill in totallen
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// ArtAlbumList
ArtAlbumList::ArtAlbumList() :
	headerlen(0x5c)
{
}

ArtAlbumList::~ArtAlbumList() {}

int ArtAlbumList::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhla",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x5c) return -1;
	int children = rev4i(data,ptr);
	
	if(children != 0) return -1;
	
	return headerlen;
}

int ArtAlbumList::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhla",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // num children
	pad(data,headerlen,ptr);
	
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
// ArtFileList
ArtFileList::ArtFileList() :
	headerlen(0x5c)
{
}

ArtFileList::~ArtFileList() 
{
	for (auto file : files)
	{
		delete file;
	}
	files.clear();
}

int ArtFileList::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhlf",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x5c) return -1;
	int children = rev4i(data,ptr);
	
	ptr = headerlen;

	for(int i=0; i<children; i++) {
		ArtFile * f = new ArtFile;
		int p = f->parse(data+ptr,len-ptr);
		if(p<0) { delete f; return -1; }
		ptr+=p;
		files.push_back(f);
	}
	return ptr;
}

int ArtFileList::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhlf",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(files.size(),data,ptr); // num children
	pad(data,headerlen,ptr);
	
	for(auto f = files.begin(); f != files.end(); f++) {
		int p = (*f)->write(data+ptr,len-ptr);
		if(p<0) return -1;
		ptr+=p;
	}

	return ptr;
}

ArtFile * ArtFileList::getFile(int corrid) {
	for(auto i = files.begin(); i!=files.end(); i++) {
		if((*i)->corrid == corrid) return *i;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ArtFile
ArtFile::ArtFile() :
	headerlen(0x7c),
	corrid(0),
	imagesize(0),
	file(0)
{
}

ArtFile::~ArtFile() {
	SAFEFREE(file);
}

int ArtFile::parse(BYTE *data, int len) {
	int ptr=4;
	if(len < headerlen) return -1;
	if (_strnicmp((char *)data,"mhif",4)) return -1;
	headerlen = rev4i(data,ptr);
	if(headerlen < 0x7c) return -1;
	int totallen = rev4i(data,ptr);
	rev4i(data,ptr); // might not be numchildren, it's really unk1
	corrid = rev4i(data,ptr);
	imagesize = rev4i(data,ptr);

	return totallen;
}

int ArtFile::write(BYTE *data, int len) {
	int ptr=0;
	if(headerlen > len) return -1;
	putmh("mhif",data,ptr);
	rev4i(headerlen,data,ptr);
	rev4i(0,data,ptr); // total len, fill in later
	rev4i(0,data,ptr); // numchildren/unk1
	rev4i(corrid,data,ptr);
	rev4i(imagesize,data,ptr);
	pad(data,headerlen,ptr);
	// write children, if we had any...
	rev4(ptr,&data[8]); // fill in total len
	return ptr;
}

struct ArtFileImageSort {
  bool operator()(ArtFileImage*& ap,ArtFileImage*& bp) {
		return ap->start < bp->start;
	}
};

void ArtFile::sortImages() {
	std::sort(images.begin(),images.end(),ArtFileImageSort());
	for(size_t i = 1; i != images.size(); i++) 
	{
		if(images[i]->start == images[i-1]->start) 
		{
			images.erase(images.begin() + i);
			i--;
			images[i]->refcount++;
		}
	}
}

size_t ArtFile::getNextHole(size_t size) {
	size_t s=0;
	for(auto i = images.begin(); i!=images.end(); i++) {
		if((*i)->start - s >= size) return s;
		s = (*i)->start + (*i)->len;
	}
	return s;
}

bool writeDataToThumb(wchar_t *file, unsigned short * data, int len) {
	FILE * f = _wfopen(file,L"ab");
	if(!f) return false;
	fwrite(data,len,sizeof(short),f);
	fclose(f);
	return true;
}
