#ifndef NULLSOFT_AMFOBJECT_H
#define NULLSOFT_AMFOBJECT_H

#include "FLVUtil.h"
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <time.h>


class AMFType // no, not bowling, ActionScript Message Format
{
public:
	AMFType() : refCount(1) {}
	virtual ~AMFType() {}
	virtual size_t Read(uint8_t *data, size_t size)=0; // returns number of bytes read, 0 on failure
	uint8_t type;
	virtual void DebugPrint(int spaces, wchar_t *&str, size_t &len)=0;
	enum
	{
		TYPE_DOUBLE = 0x0,
		TYPE_BOOL = 0x1,
		TYPE_STRING = 0x2,
		TYPE_OBJECT = 0x3,
		TYPE_MOVIE = 0x4, 
		TYPE_NULL = 0x5,
		TYPE_REFERENCE = 0x7,
		TYPE_MIXEDARRAY = 0x8,
		TYPE_TERMINATOR = 0x9,
		TYPE_ARRAY = 0xA,
		TYPE_DATE = 0xB,
		TYPE_LONG_STRING = 0xC,
		TYPE_XML = 0xF,
	};

	ULONG AddRef(void)
	{
		return InterlockedIncrement((volatile LONG *)&refCount);
	}

	ULONG Release(void)
	{
		ULONG count = InterlockedDecrement((volatile LONG *)&refCount);
		if (count == 0)
			delete this;
		return count;
	}
private:
	ULONG refCount;
};

class AMFString : public AMFType
{
public:
	AMFString();
	~AMFString();
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size);
	wchar_t *str;
};

class AMFLongString : public AMFType
{
public:
	AMFLongString();
	~AMFLongString();
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size);
	wchar_t *str;
};

class AMFObj : public AMFType
{
public:
	size_t Read(uint8_t *data, size_t size);
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
};

class AMFArray : public AMFType
{
public:
	size_t Read(uint8_t *data, size_t size);
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	~AMFArray();
	std::vector<AMFType*> array;
};

class AMFMixedArray : public AMFType
{
public:
	size_t Read(uint8_t *data, size_t size);
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	~AMFMixedArray();
	typedef std::map<std::wstring, AMFType *> AMFTypeList;
	AMFTypeList array;
};

class AMFDouble : public AMFType
{
public:
void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size)
	{
		if (size < 8)
			return 0;

		val = FLV::ReadDouble(data);
		return 8;
	}
	double val;
};

class AMFTime : public AMFType
{
public:
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size)
	{
		if (size < 10)
			return 0;

		val = FLV::ReadDouble(data);
		data+=8;
		
		offset = FLV::Read16(data);

		return 10;
	}
	double val; // same epoch as time_t, just stored as a double instead of an unsigned int
	int16_t offset; // offset in minutes from UTC. presumably from the encoding machine's timezone
};

class AMFBoolean : public AMFType
{
public:
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size)
	{
		if (size < 1)
			return 0;
		boolean = !!data[0];
		return 1;
	}

	bool boolean;
};

class AMFTerminator : public AMFType
{
public:
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size)
	{
		return 0;
	}
};

class AMFReference : public AMFType
{
public:
	void DebugPrint(int spaces, wchar_t *&str, size_t &len);
	size_t Read(uint8_t *data, size_t size)
	{
		if (size < 2)
			return 0;
		val = FLV::Read16(data);
		return 2;
	}
	uint16_t val;
};

inline double AMFGetDouble(AMFType *obj)
{
	if (obj->type == 0x0)
		return ((AMFDouble *)obj)->val;

	return 0;
}

AMFType *MakeObject(uint8_t type);
#endif