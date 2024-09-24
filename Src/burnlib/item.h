#pragma once

#include "./main.h"
#include "../Agave/DecodeFile/api_decodefile.h"
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "./manager.h"
//#include "../primo/obj_primo.h"


#define BURNERITEM_SUCCESS		0x0000

#define BURNERITEM_STATUS		0x0000
#define BURNERITEM_ERROR			0x1000

// states 

#define BURNERITEM_SKIPPED			0x0100
#define BURNERITEM_READY				0x0101
#define BURNERITEM_LICENSING			0x0102
#define BURNERITEM_LICENSED			0x0103
#define BURNERITEM_DECODING			0x0104
#define BURNERITEM_DECODED			0x0105
#define BURNERITEM_BURNING			0x0106
#define BURNERITEM_BURNED			0x0107

// error codes
#define BURNERITEM_FAILED				((BURNERITEM_ERROR) + 0x001)
#define BURNERITEM_BADFILENAME			((BURNERITEM_ERROR) + 0x002)
#define BURNERITEM_UNABLEOPENFILE		((BURNERITEM_ERROR) + 0x003)
#define BURNERITEM_WRITEERROR			((BURNERITEM_ERROR) + 0x004)
#define BURNERITEM_DECODEERROR			((BURNERITEM_ERROR) + 0x005)
#define BURNERITEM_ALREADYCREATED		((BURNERITEM_ERROR) + 0x006)
#define BURNERITEM_ADDSTREAMFAILED		((BURNERITEM_ERROR) + 0x007)
#define BURNERITEM_READSTREAMERROR		((BURNERITEM_ERROR) + 0x008)
#define BURNERITEM_ABORTED				((BURNERITEM_ERROR) + 0x009)	
#define BURNERITEM_CANCELING				((BURNERITEM_ERROR) + 0x00A)	

// statuses
#define BURNERITEM_DECODESTARTING		((BURNERITEM_STATUS) + 0x001)
#define BURNERITEM_DECODEPROGRESS		((BURNERITEM_STATUS) + 0x002)
#define BURNERITEM_DECODECANCELING		((BURNERITEM_STATUS) + 0x003)
#define BURNERITEM_DECODEFINISHED		((BURNERITEM_STATUS) + 0x004)



// callback returns
#define BURNERITEM_CONTINUE	0
#define BURNERITEM_STOP		1


typedef DWORD (WINAPI *BURNERITEMCALLBACK)(void*, void*, DWORD, DWORD); // sender, parameter, notifyCode, errorCode

#define ZEROMEM_SIZE		1024
class BurnerItem
{
	friend class BurnerPlaylist;
public:

	BURNLIB_API BurnerItem(void);
	BURNLIB_API ~BurnerItem(void);

public:
	BURNLIB_API HRESULT Create(const wchar_t *fullname, const wchar_t *title, int length);
	BURNLIB_API void Destroy(void);
    BURNLIB_API HRESULT Decode(BurnManager *manager, void *fileHandle, BURNERITEMCALLBACK notifyCB, void *userparam);
	BURNLIB_API HRESULT AddStream(obj_primo *primoSDK, void *fileHandle);

public:
	BURNLIB_API const wchar_t* GetFullName(void) { return fullname; }
	BURNLIB_API const wchar_t* GetTitle(void)  { return title; }
	BURNLIB_API int	GetLength(void)  { return length; }
	BURNLIB_API unsigned __int64 GetSize(void)  { return sizeBytes; }
	BURNLIB_API DWORD GetSizeInSectors(void)  { return sizeSectors; }
	BURNLIB_API unsigned int GetPreGap(void)  { return preGap; }
	BURNLIB_API unsigned __int8* GetISRC(void)  { return ISRC; }
	BURNLIB_API __int64 GetDecodedFilePosition(void) { return fposition; }
	BURNLIB_API int GetStatus(DWORD *retCode); // if retCode not NULL - can return completed percentage or error code
	void SetPreGap(unsigned int preGap);
	void SetISRC(unsigned __int8 *ISRC);

protected:
	static wchar_t* DuplicateString(void *heap, const wchar_t *source, unsigned int cchSource);
	static DWORD StreamFiller(PBYTE pBuffer, DWORD dwBytesRequested, PDWORD pdwBytesWritten, PVOID pContext);
	

protected:
	void				*heap;
	wchar_t				*fullname;
	wchar_t				*title;
	int					length;
	unsigned __int64	sizeBytes;
	DWORD				sizeSectors;
	unsigned int		preGap;
	unsigned __int8		ISRC[12];
	void*				fhandle;
	__int64				fposition;
	BOOL				needSetFilePos;
	unsigned __int64	streamedSize;
	int					percentCompleted;
	int					itemStatus;
	DWORD				errorCode;
	static DWORD			zeroMem[ZEROMEM_SIZE];

	
};