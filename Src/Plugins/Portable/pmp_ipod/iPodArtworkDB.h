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

#ifndef __IPODARTDB_H__
#define __IPODARTDB_H__

#pragma once

//#include <stdio.h>
#include <windows.h>
#include <vector>
#include <map>
#include <bfc/platform/types.h>

class ArtDB;
class ArtDataSet;
class ArtImageList;
class ArtImage;
class ArtImageName;
class ArtDataObject;
class ArtAlbumList;
class ArtFileList;
class ArtFile;
class ArtFileImage;

// this contains our whole art database
class ArtDB { //mhfd
public:
	int headerlen; //0x84
	int totallen;
	int unk1;
	int unk2; // must be 2
	// numchildren // should be 3
	int unk3;
	uint32_t nextid; // for ArtImage ids, starts at 0x40
	__int64 unk5;
	__int64 unk6;
	int unk7; // 2
	int unk8; // 0
	int unk9; // 0
	int unk10;
	int unk11;
	ArtDataSet * imageListDS;
	ArtDataSet * albumListDS;
	ArtDataSet * fileListDS;

	ArtDB();
	~ArtDB();
	int parse(BYTE * data, int len, wchar_t drive);
	int write(BYTE * data, int len);
	
	void makeEmptyDB(wchar_t drive);
};

class ArtDataSet { //mhsd
public:
	int headerlen; //0x60
	int totallen;
	int index; // 1=image list, 2=album list, 3=file list
	ArtImageList * imageList;
	ArtAlbumList * albumList;
	ArtFileList * fileList;

	ArtDataSet();
	ArtDataSet(int idx);
	~ArtDataSet();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
};

// contains a list of images
class ArtImageList { //mhli
public:
	typedef std::map<uint64_t ,ArtImage*> ArtImageMap;
	typedef ArtImageMap::iterator ArtImageMapIterator;
	typedef ArtImageMap::value_type ArtImageMapPair;

	int headerlen; //0x5c
	//int numchildren;
	ArtImageMap images;
	
	ArtImageList();
	~ArtImageList();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
};

// contains a reference to an image within an .ithmb file
class ArtImage { //mhii
public:
	int headerlen; //0x98
	int totallen;
	//int numchildren;
	uint32_t id;
	uint64_t songid;
	int32_t unk4;
	int32_t rating;
	int32_t unk6;
	uint32_t originalDate; //0
	uint32_t digitizedDate; //0
	uint32_t srcImageSize; // in bytes
	std::vector<ArtDataObject*> dataobjs;

	ArtImage();
	~ArtImage();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
};

class ArtDataObject { //mhod
public:
	int headerlen; //0x18
	// total length
	short type;
	unsigned char unk1;
	// unsigned char padding; // must pad to a multiple of 4 bytes! this is usually 2, but can be 0,1,2 or 3
	BYTE * data;
	int datalen;
	ArtImageName * image;

	ArtDataObject();
	~ArtDataObject();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);

	void GetString(wchar_t * str, int len);
	void SetString(wchar_t * str);
};

class ArtImageName { //mhni
public:
	int headerlen; //0x4c
	int totallen;
	//num children = 1
	unsigned int corrid;
	unsigned int ithmboffset;
	unsigned int imagesize; // in bytes
	short vpad;
	short hpad;
	unsigned short imgh;
	unsigned short imgw;
	ArtDataObject* filename;

	ArtImageName();
	~ArtImageName();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
};

// this is only used in photo databases (which we don't care about) so it's only a stub
class ArtAlbumList { //mhla
public:
	int headerlen; //0x5c
	//num children, should be 0 for artwork db
	ArtAlbumList();
	~ArtAlbumList();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
};

// this contains the list of .ithmb files
class ArtFileList { //mhlf
public:
	int headerlen; //0x5c
	// num children
	std::vector<ArtFile*> files;
	
	ArtFileList();
	~ArtFileList();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);
	ArtFile * getFile(int corrid);
};

// this talks about a .ithmb file
class ArtFile { //mhif
public:
	int headerlen; //0x7c
	unsigned int corrid;
	unsigned int imagesize; // bytes
	
	ArtFile();
	~ArtFile();
	int parse(BYTE *data, int len);
	int write(BYTE *data, int len);

	std::vector<ArtFileImage*> images;
	wchar_t * file;
	void sortImages();
	size_t getNextHole(size_t size);
};

class ArtFileImage {
public:
	size_t start;
	size_t len;
	int refcount;
	ArtFileImage(size_t start, size_t len, int refcount) : start(start), len(len), refcount(refcount) {}
};

bool writeDataToThumb(wchar_t *file, unsigned short * data, int len);

#endif //__IPODARTDB_H__