/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */

#ifndef __MP4_UTIL_INCLUDED__
#define __MP4_UTIL_INCLUDED__
#include <assert.h>

#ifndef ASSERT
#define ASSERT(expr) \
	if (!(expr)) { \
		assert((expr)); \
	}
#endif
#define WARNING(expr) \
	if (expr) { \
	}

#define VERBOSE(exprverbosity, verbosity, expr)	{}

#define VERBOSE_ERROR(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_ERROR, verbosity, expr)

#define VERBOSE_WARNING(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_WARNING, verbosity, expr)

#define VERBOSE_READ(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_READ, verbosity, expr)

#define VERBOSE_READ_TABLE(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_READ | MP4_DETAILS_TABLE), verbosity, expr)

#define VERBOSE_READ_SAMPLE(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_READ | MP4_DETAILS_SAMPLE), verbosity, expr)

#define VERBOSE_READ_HINT(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_READ | MP4_DETAILS_HINT), verbosity, expr)

#define VERBOSE_WRITE(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_WRITE, verbosity, expr)

#define VERBOSE_WRITE_TABLE(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_WRITE | MP4_DETAILS_TABLE), verbosity, expr)

#define VERBOSE_WRITE_SAMPLE(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_WRITE | MP4_DETAILS_SAMPLE), verbosity, expr)

#define VERBOSE_WRITE_HINT(verbosity, expr)	\
	VERBOSE((MP4_DETAILS_WRITE | MP4_DETAILS_HINT), verbosity, expr)

#define VERBOSE_FIND(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_FIND, verbosity, expr)

#define VERBOSE_ISMA(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_ISMA, verbosity, expr)

#define VERBOSE_EDIT(verbosity, expr)		\
	VERBOSE(MP4_DETAILS_EDIT, verbosity, expr)

inline void Indent(FILE* pFile, u_int8_t depth) {
	fprintf(pFile, "%*c", depth, ' ');
}
#if 0
static inline void MP4Printf(const char* fmt, ...) 
#ifndef _WIN32
 __attribute__((format(__printf__, 1, 2)))
#endif
;

static inline void MP4Printf(const char* fmt, ...) 
{
	va_list ap;
	va_start(ap, fmt);
	// TBD API call to set error_msg_func instead of just printf
	vprintf(fmt, ap);
	va_end(ap);
}
#endif


class MP4Error {
public:
	MP4Error() {
		m_errno = 0;
		m_errstring = NULL;
		m_where = NULL;
		m_free = 0;
	}
	~MP4Error() {
	  if (m_free != 0) {
	    free((void *)m_errstring);
	  }
	}
	MP4Error(int err, const char* where = NULL) {
		m_errno = err;
		m_errstring = NULL;
		m_where = where;
		m_free = 0;
	}
	MP4Error(const char *format, const char *where, ...) {
	  char *string;
	  m_errno = 0;
	  string = (char *)malloc(512);
	  m_where = where;
	  if (string) {
	    va_list ap;
	    va_start(ap, where);
	    vsnprintf(string, 512, format, ap);
	    va_end(ap);
	    m_errstring = string;
	    m_free = 1;
	  } else {
	    m_errstring = format;
	    m_free = 0;
	  }
	}
	MP4Error(int err, const char* format, const char* where, ...) {
	  char *string;
	  m_errno = err;
	  string = (char *)malloc(512);
	  m_where = where;
	  if (string) {
	    va_list ap;
	    va_start(ap, where);
	    vsnprintf(string, 512, format, ap);
	    va_end(ap);
	    m_errstring = string;
	    m_free = 1;
	  } else {
	    m_errstring = format;
	    m_free = 0;
	  }
	}

	void Print(FILE* pFile = stderr);
	int m_free;
	int m_errno;
	const char* m_errstring;
	const char* m_where;
};

void MP4HexDump(
	u_int8_t* pBytes, u_int32_t numBytes,
	FILE* pFile = stdout, u_int8_t indent = 0);

inline void* MP4Malloc(size_t size) {
  if (size == 0) return NULL;
	void* p = malloc(size);
	if (p == NULL && size > 0) {
		throw new MP4Error(errno);
	}
	return p;
}

inline void* MP4Calloc(size_t size) {
  if (size == 0) return NULL;
	return memset(MP4Malloc(size), 0, size);
}

inline char* MP4Stralloc(const char* s1) {
	char* s2 = (char*)MP4Malloc(strlen(s1) + 1);
	strcpy(s2, s1);
	return s2;
}

#ifdef _NATIVE_WCHAR_T_DEFINED
inline wchar_t* MP4Stralloc(const wchar_t* s1) {
	wchar_t* s2 = (wchar_t*)MP4Malloc((wcslen(s1) + 1)*sizeof(wchar_t));
	wcscpy(s2, s1);
	return s2;
}
#endif

inline uint16_t *MP4Stralloc(const uint16_t *s1) 
{
	const uint16_t *itr=s1;
	size_t len=0;
	while (*itr)
	{
		itr++;
		len++;
	}
	uint16_t* s2 = (uint16_t*)MP4Malloc((len + 1)*sizeof(uint16_t));
	memcpy(s2, s1, (len + 1)*sizeof(uint16_t));
	return s2;
}

inline void* MP4Realloc(void* p, u_int32_t newSize) {
	// workaround library bug
	if (p == NULL && newSize == 0) {
		return NULL;
	}
	p = realloc(p, newSize);
	if (p == NULL && newSize > 0) {
		throw new MP4Error(errno);
	}
	return p;
}

inline void* MP4ReallocArray(void* p, u_int32_t numElements, u_int32_t elementSize) {
	// workaround library bug
	if (p == NULL && numElements == 0) {
		return NULL;
	}

	if (elementSize == 0 || _UI32_MAX/elementSize < numElements)
		throw new MP4Error;

	p = realloc(p, numElements*elementSize);

	if (p == NULL && numElements > 0) {
		throw new MP4Error(errno);
	}
	return p;
}

inline u_int32_t STRTOINT32(const char* s) {
  return ntohl(*(uint32_t *)s);
}

inline void INT32TOSTR(u_int32_t i, char* s) {
  *(uint32_t *)s = htonl(i);
  s[4] = 0;
}

inline MP4Timestamp MP4GetAbsTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	MP4Timestamp ret;
	ret = tv.tv_sec;
	ret += 2082844800;
	return ret;	// MP4 start date is 1/1/1904
	// 208284480 is (((1970 - 1904) * 365) + 17) * 24 * 60 * 60
}

u_int64_t MP4ConvertTime(u_int64_t t, 
	u_int32_t oldTimeScale, u_int32_t newTimeScale);

bool MP4NameFirstMatches(const char* s1, const char* s2);

bool MP4NameFirstIndex(const char* s, u_int32_t* pIndex);

char* MP4NameFirst(const char *s);

const char* MP4NameAfterFirst(const char *s);

char* MP4ToBase16(const u_int8_t* pData, u_int32_t dataSize);

char* MP4ToBase64(const u_int8_t* pData, u_int32_t dataSize);

const char* MP4NormalizeTrackType(const char* type,
				  uint32_t verbosity);

#endif /* __MP4_UTIL_INCLUDED__ */
