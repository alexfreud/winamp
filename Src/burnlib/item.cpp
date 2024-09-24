#include "./item.h"
#include <strsafe.h>

#define DECODEBUFFER_SIZE		4096*4

DWORD BurnerItem::zeroMem[]	= {0x00,};

BurnerItem::BurnerItem(void)
{
	fullname = NULL;
	title = NULL;
	length = 0;
	sizeBytes = 0;
	sizeSectors = 0;
	preGap = 0;
	fposition = 0;
	fhandle = NULL;
	heap = GetProcessHeap();
	ZeroMemory(ISRC, 12);
	itemStatus = BURNERITEM_READY;
	percentCompleted = 0;
	errorCode = BURNERITEM_SUCCESS;
	if (zeroMem[0] == 0x00) FillMemory(zeroMem, ZEROMEM_SIZE, 0xFFFFFFFF);
	
}

BurnerItem::~BurnerItem(void)
{
	Destroy();
}

HRESULT BurnerItem::Create(const wchar_t* fullname, const wchar_t *title, int length)
{
	if (this->fullname)
	{
		errorCode = BURNERITEM_ALREADYCREATED;
		return errorCode;
	}
	this->fullname = DuplicateString(heap, fullname, lstrlenW(fullname));
	this->title = DuplicateString(heap, title, lstrlenW(title));
	this->length = length;
	itemStatus = BURNERITEM_READY;
	errorCode = BURNERITEM_SUCCESS;
	return errorCode;
}

void BurnerItem::Destroy(void)
{
	if (fullname) 
	{
		HeapFree(heap, NULL, fullname);
		fullname = NULL;
	}
	if (title) 
	{
		HeapFree(heap, NULL, title);
		title = NULL;
	}

	length = 0;
	sizeBytes = 0;
	sizeSectors = 0;
	preGap = 0;
	fposition = 0;
	fhandle = NULL;
	ZeroMemory(ISRC, 12);

}

HRESULT BurnerItem::Decode(BurnManager *manager, void *fileHandle, BURNERITEMCALLBACK notifyCB, void *userparam)
{
	itemStatus = BURNERITEM_DECODING;
	if (notifyCB) notifyCB(this, userparam, BURNERITEM_DECODESTARTING, BURNERITEM_SUCCESS);
	if (!fullname)
	{
		errorCode = BURNERITEM_BADFILENAME;
		itemStatus = BURNERITEM_DECODED;
		if (notifyCB) notifyCB(this, userparam, BURNERITEM_DECODEFINISHED, errorCode);
		
		return errorCode;
	}

	ifc_audiostream *stream = manager->CreateDecoder(fullname);
	if (!stream)
	{
		errorCode = BURNERITEM_UNABLEOPENFILE;
		itemStatus = BURNERITEM_DECODED;
		if (notifyCB) notifyCB(this, userparam, BURNERITEM_DECODEFINISHED, errorCode);
		return errorCode;
	}

	fposition = 0;
	LONG fposLow(0), fposHigh(0);
	fposLow = SetFilePointer(fileHandle, 0, &fposHigh, FILE_CURRENT);
    fposition = (((__int64)fposHigh) << 32) | fposLow;
	DWORD decoded = 0;
	sizeBytes = 0;
	sizeSectors = 0;
	double percent = 0.0;
	int iopercent = -1;
	percentCompleted = 0;

	errorCode = BURNERITEM_DECODEPROGRESS;
	do
	{
		unsigned __int8 buffer[DECODEBUFFER_SIZE] = {0};
		int decode_killswitch=0, decode_error;
		decoded = (DWORD)stream->ReadAudio(buffer, DECODEBUFFER_SIZE, &decode_killswitch, &decode_error);
		if (decoded > 0)
		{
			DWORD written = 0;
			if (!WriteFile(fileHandle, buffer, decoded, &written, NULL)) 
			{
				errorCode = BURNERITEM_WRITEERROR;
				break;
			}
			sizeBytes += decoded;
			double step = ((double)decoded) / (((double)length) * 176.4f)*100.0f;
			percent += step;
			percentCompleted = (int)percent;
			if (iopercent != percentCompleted)
			{
				iopercent = percentCompleted;
				if (notifyCB)
				{
					if (BURNERITEM_CONTINUE != notifyCB(this, userparam, BURNERITEM_DECODEPROGRESS, percentCompleted))
					{
						errorCode = BURNERITEM_DECODECANCELING;
						break;
					}
				}
				
			}
		}
	} while(decoded);
	if (stream) manager->CloseDecoder(stream);
	itemStatus = BURNERITEM_DECODED;
	DWORD notifyCode = BURNERITEM_DECODEFINISHED;
	switch(errorCode)
	{
		case BURNERITEM_DECODECANCELING:
			errorCode = BURNERITEM_ABORTED;
			itemStatus = BURNERITEM_ABORTED;
			break;
		case BURNERITEM_DECODEPROGRESS:
			sizeSectors = (DWORD)(sizeBytes/2352 + ((sizeBytes%2352) ? 1 : 0));
			errorCode = BURNERITEM_SUCCESS;
			break;
	}
	if (notifyCB) notifyCB(this, userparam, notifyCode, errorCode);
	return errorCode;
}


HRESULT BurnerItem::AddStream(obj_primo *primoSDK, void *fileHandle)
{
	fhandle = fileHandle;
	needSetFilePos =  TRUE;
	streamedSize = 0;

	errorCode = primoSDK->AddAudioStream(StreamFiller, this, 0, GetSizeInSectors());
	return errorCode;
}
void BurnerItem::SetPreGap(unsigned int preGap)
{  
	this->preGap = preGap; 
}
void BurnerItem::SetISRC(unsigned __int8 *ISRC)
{
	CopyMemory(this->ISRC, ISRC, 12);
}

int BurnerItem::GetStatus(DWORD *retCode)
{ 
	if(retCode)
	{
		switch(itemStatus)
		{
			case BURNERITEM_DECODING:
			case BURNERITEM_BURNING:
				*retCode = percentCompleted;
				break;
			default:
				*retCode = errorCode;
		}
	}
	return itemStatus; 
}

DWORD BurnerItem::StreamFiller(PBYTE pBuffer, DWORD dwBytesRequested, PDWORD pdwBytesWritten, PVOID pContext)
{
	BurnerItem *item = (BurnerItem*) pContext;
	
	if (item->needSetFilePos)
	{
		LONG fposLow = (LONG) item->fposition;
		LONG fposHigh = (LONG) (item->fposition >> 32);
		SetFilePointer(item->fhandle, fposLow, &fposHigh, FILE_BEGIN);
		item->streamedSize = 0;
		item->needSetFilePos = FALSE;
	}
	
	DWORD needToRead;
	item->streamedSize += dwBytesRequested;
	// check if we need to fill with zero	
	if (item->streamedSize >  item->sizeBytes)
	{
		needToRead =  (DWORD)min(0, ((long)item->sizeBytes) - long(item->streamedSize - dwBytesRequested));
	}
	else
	{
		needToRead = dwBytesRequested;
	}
	// read from file
	if (needToRead)
	{
		*pdwBytesWritten = 0;
		if (!ReadFile(item->fhandle, pBuffer, needToRead, pdwBytesWritten, NULL))
		{
			DWORD le = GetLastError();
			item->itemStatus = BURNERITEM_BURNED;
			item->errorCode = BURNERITEM_READSTREAMERROR;
		}
		//else
		//{
		//	LONG fposLow(0), fposHigh(0);
		//	fposLow = SetFilePointer(item->fhandle, 0, &fposHigh, FILE_CURRENT);
		//	SetFilePointer(item->fhandle, -((LONG)*pdwBytesWritten), NULL, FILE_CURRENT);
		//	DWORD written(0);
		//	long needToZerro(*pdwBytesWritten)l
		//	while(needToZerro > 0)
		//	{
		//		DWORD write = min(ZEROMEM_SIZE*sizeof(DWORD), needToZerro);
		//		if (!WriteFile(item->fhandle, &zeroMem, write, &written, NULL)) break; 
		//		needToZerro -= written;
		//	}
		//	SetFilePointer(item->fhandle, fposLow, &fposHigh, FILE_BEGIN);
		//}
	}
	else
	{
		*pdwBytesWritten = 0;
	}
	return (BURNERITEM_SUCCESS == item->errorCode) ? PRIMOSDK_OK : PRIMOSDK_ERROR;
}
wchar_t* BurnerItem::DuplicateString(void *heap, const wchar_t *source, unsigned int cchSource)
{
	wchar_t *dest = (wchar_t*) HeapAlloc(heap, NULL, (cchSource + 1) * sizeof(wchar_t));
	CopyMemory(dest, source, cchSource * sizeof(wchar_t));
	dest[cchSource] = 0x0000;
	return dest;
}