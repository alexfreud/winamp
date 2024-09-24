#pragma once
#ifndef uniFile_H_
#define uniFile_H_

#include "uniString.h"
#include <stdio.h>

// functions that help bridge the difference between
// unicode file handling on various platforms

namespace uniFile
{
	// posix uses utf8 for unicode filenames. Msft uses utf16. To avoid lots
	// of platform conditional code, we will standardize on utf8 and convert
	// as necessary on Msft platforms. This should be okay since file opens are
	// not a high speed inner loop type of operation
	typedef uniString::utf8 filenameType;

	//typedef enum { OPENFILE_READ = 1,OPENFILE_WRITE = 2,OPENFILE_BINARY=4 } OpenFile_t;
	//FILE* fopen(const filenameType &f,int open_file_type) throw();

	FILE* fopen(const filenameType &f, const char *mode) throw();
	void unlink(const filenameType &f) throw();
	bool fileExists(const filenameType &f) throw();
	size_t fileSize(const filenameType &f) throw();
	time_t fileTime(const filenameType &f) throw();
}

#endif
