#include "main.h"
#include "./addressEncoder.h"


#include <wininet.h>
#include <strsafe.h>


typedef struct __ENCODEBUFFER
{
	LPWSTR buffer;
	size_t bufferMax;
	LPWSTR cursor;
	size_t remaining; 
} ENCODEBUFFER;

HRESULT AddressEncoder_ReAllocBuffer(ENCODEBUFFER *decoder, size_t cchBufferSize)
{
	if (NULL == decoder) 
		return E_INVALIDARG;

	if (cchBufferSize == decoder->bufferMax)
		return S_FALSE;

	if (cchBufferSize < decoder->bufferMax)
		return E_FAIL;

	LPWSTR test = Plugin_ReAllocString(decoder->buffer, cchBufferSize);
	if (NULL == test)
		return E_OUTOFMEMORY;
		
	decoder->cursor = test  + (decoder->cursor - decoder->buffer);
	decoder->remaining += (cchBufferSize - decoder->bufferMax); 
	decoder->buffer = test;
	decoder->bufferMax = cchBufferSize;
	
	return S_OK;
}
HRESULT AddressEncoder_AppendAnsiString(ENCODEBUFFER *decoder, LPCSTR pszString, size_t cchString)
{
	if (NULL == decoder)
		return E_INVALIDARG;

	INT cchConverted; 
	while(0 ==(cchConverted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pszString, (int)cchString, decoder->cursor, (int)decoder->remaining)))
	{
		DWORD errorCode = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == errorCode)
		{
			INT cchNeed = MultiByteToWideChar(CP_UTF8, 0, pszString, (int)cchString, NULL, 0) - (INT)decoder->remaining;
			if (cchNeed < 32) cchNeed = 32;
			HRESULT hr = AddressEncoder_ReAllocBuffer(decoder, decoder->bufferMax + cchNeed);
			if (FAILED(hr)) 
				return hr;
		}
		else
		{
			return HRESULT_FROM_WIN32(errorCode);
		}
	}

	if (0 != cchConverted)
	{
		decoder->cursor += cchConverted;
		decoder->remaining -= cchConverted;
	}

	return S_OK;
}

HRESULT AddressEncoder_AppendString(ENCODEBUFFER *decoder, LPCWSTR pszString, size_t cchString)
{
	if (NULL == decoder)
		return E_INVALIDARG;

	LPWSTR cursor;
	size_t remaining;

	HRESULT hr;
	if (cchString >= decoder->remaining)
	{
		hr = AddressEncoder_ReAllocBuffer(decoder, decoder->bufferMax + (cchString - decoder->remaining) + 1);
		if (FAILED(hr)) return hr;
	}

	hr = StringCchCopyNEx(decoder->cursor, decoder->remaining, pszString, cchString, &cursor, &remaining, 0);
	if (SUCCEEDED(hr))
	{
		decoder->cursor = cursor;
		decoder->remaining = remaining;
	}
	return hr;
}

HRESULT AddressEncoder_GetEscapeBlock(LPCWSTR pszEscape, LPCWSTR *ppszEnd, size_t *pcchEscapeLen, LPSTR pszBuffer, UINT *pcbBufferMax)
{
	if (NULL == pszEscape || (NULL != pszBuffer && NULL == pcbBufferMax))
		return E_INVALIDARG;

	UINT cbBinary = 0;
	WORD charInfo;
	WCHAR szDigit[3] = {0};

	HRESULT hr = S_OK;

	LPCWSTR cursor = pszEscape;
	while (L'%' == *cursor)
	{
		LPCWSTR testChar = CharNext(cursor);
		if (L'\0' == *testChar || 
			FALSE == GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, testChar, 1, &charInfo) ||
			0 == (C1_XDIGIT & charInfo))
		{
			break;
		}
		szDigit[0] = *testChar;

		testChar = CharNext(testChar);
		if (L'\0' == *testChar || 
			FALSE == GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, testChar, 1, &charInfo) ||
			0 == (C1_XDIGIT & charInfo))
		{
			break;
		}

		szDigit[1] = *testChar;
		CharUpperBuff(szDigit, 2);

		BYTE binaryData = ((szDigit[0] - ((szDigit[0] <= L'9' ? L'0' : L'A' - 10))) << 4) & 0xf0;
		binaryData += (szDigit[1] - ((szDigit[1] <= L'9' ? L'0' : L'A' - 10))) & 0x0f;
		if (NULL != pszBuffer)
		{
			if (cbBinary < *pcbBufferMax)
				pszBuffer[cbBinary] = binaryData;
			else
				hr = E_OUTOFMEMORY;
		}
		cbBinary++;

		cursor = CharNext(testChar);
	}

	if (cursor == pszEscape)
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_BLOCK_LENGTH);

	if (NULL != ppszEnd)
		*ppszEnd = cursor;
	
	if (NULL != pcchEscapeLen)
		*pcchEscapeLen = (size_t)(cursor - pszEscape);

	if (NULL != pcbBufferMax)
		*pcbBufferMax = cbBinary;
	
	return hr;
}

HRESULT AddressEncoder_DecodeString(LPCWSTR pszUrl, LPWSTR *ppResult)
{	
	if (NULL == pszUrl)
	{
		*ppResult = NULL;
		return S_FALSE;
	}

	UINT cchUrl = 0;
	UINT escapeSize  = 0;
	UINT escapeBlockSize,escapeBlockMaxSize = 0;
	LPCWSTR escapeBlockEnd;
	for (LPCWSTR cursor = pszUrl; L'\0' != *cursor;)
	{		
		if (L'%' == *cursor && SUCCEEDED(AddressEncoder_GetEscapeBlock(cursor, &escapeBlockEnd, NULL, NULL, &escapeBlockSize)))
		{
			escapeSize += escapeBlockSize;
			if (escapeBlockSize > escapeBlockMaxSize)
				escapeBlockMaxSize = escapeBlockSize;

			cursor = escapeBlockEnd;
		}
		else
		{
			cchUrl++;
			cursor = CharNext(cursor);
		}
	}

	if (0 == escapeSize)
	{
		*ppResult = Plugin_CopyString(pszUrl);
		if (NULL == *ppResult) return E_OUTOFMEMORY;
		return S_FALSE;
	}

	HRESULT hr = S_OK;
		
	ENCODEBUFFER decoder;
	ZeroMemory(&decoder, sizeof(decoder));
	
	LPSTR escapeBuffer = Plugin_MallocAnsiString(escapeBlockMaxSize);
	if (NULL == escapeBuffer)
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		hr = AddressEncoder_ReAllocBuffer(&decoder, cchUrl + (escapeSize + 1)* sizeof(WCHAR));
		if (SUCCEEDED(hr))
		{
			LPCWSTR cursor = pszUrl; 
			LPCWSTR copyBlock =  cursor;
			
			for (;;)
			{		
				escapeBlockSize = escapeBlockMaxSize;
				if (L'%' == *cursor && SUCCEEDED(AddressEncoder_GetEscapeBlock(cursor, &escapeBlockEnd, NULL, escapeBuffer, &escapeBlockSize)))
				{
					if (copyBlock != cursor)
					{
						hr = AddressEncoder_AppendString(&decoder, copyBlock, cursor - copyBlock);
						if (FAILED(hr)) 
							break;
						copyBlock = cursor;
					}

					HRESULT convertResult = AddressEncoder_AppendAnsiString(&decoder, escapeBuffer, escapeBlockSize);
					if (L'\0' == *cursor)
						break;
					
					cursor = escapeBlockEnd;
					if (SUCCEEDED(convertResult))
					{
						copyBlock = cursor;
					}
					
					
					
					continue;
				}
				
				if (L'\0' == *cursor)
				{
					if (copyBlock != cursor)
						hr = AddressEncoder_AppendString(&decoder, copyBlock, cursor - copyBlock);
					break;
				}
				else
					cursor = CharNext(cursor);
			}

		}

	}

	if (NULL != escapeBuffer)
		Plugin_FreeAnsiString(escapeBuffer);

	if (FAILED(hr))
	{
		Plugin_FreeString(decoder.buffer);
		decoder.buffer = NULL;
	}
	else
	{
		*decoder.cursor = L'\0';
	}

	*ppResult = decoder.buffer;
	
	return hr;
}
HRESULT AddressEncoder_GetWideBlock(LPCWSTR pszWide, LPCWSTR *pszEnd, LPWSTR pszBuffer, size_t *pcchBufferMax)
{
	LPCWSTR cursor = pszWide;
	if (NULL == pszWide) 
		return E_INVALIDARG;
	
	if (NULL != pszBuffer && NULL == pcchBufferMax) 
		return E_INVALIDARG;
	
	while (L'\0' == *cursor || *cursor > 0xFF)
	{
		if (L'\0' == *cursor)
			break;
		cursor = CharNext(cursor);
	}

	if (NULL != pszEnd)
		*pszEnd = cursor;

	HRESULT hr = S_OK;
	size_t cchBuffer = 0;

	if (cursor == pszWide)
	{
		hr = S_FALSE;
	}
	else
	{
		size_t bytesCount = WideCharToMultiByte(CP_UTF8, 0, pszWide, (int)(cursor - pszWide), NULL, 0, NULL, NULL);
		if (0 == bytesCount)
		{
			DWORD errorCode = GetLastError();
			if (ERROR_SUCCESS != errorCode)
				hr = HRESULT_FROM_WIN32(errorCode);
		}
		else
		{
			cchBuffer = 3 * bytesCount;
			if (NULL != pszBuffer)
			{
				if (*pcchBufferMax >= cchBuffer)
				{
					LPWSTR p = pszBuffer;
					BYTE *bytes = ((BYTE*)(pszBuffer + *pcchBufferMax)) - bytesCount;
					WideCharToMultiByte(CP_UTF8, 0, pszWide,  (int)(cursor - pszWide), (LPSTR)bytes, (int)bytesCount, NULL, NULL);
					for (size_t i = 0; i < bytesCount; i++)
					{
						BYTE b = bytes[i];
						*p++ = L'%';
						BYTE c = (b >> 4) & 0x0F;
						*p++ = (c < 10) ? (L'0' + c) : (L'A' + (c -10));
						c = b & 0x0F;
						*p++ = (c < 10) ? (L'0' + c) : (L'A' + (c -10));
					}

				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
			}
		}
	}

	if (NULL != pcchBufferMax)
	{
		*pcchBufferMax = cchBuffer;
	}


	return hr;
}
HRESULT AddressEncoder_EncodeWideChars(LPCWSTR pszAddress, size_t cchAddress, LPWSTR *ppResult)
{
	if (NULL == ppResult)
		return E_POINTER;
	
	if (NULL == pszAddress)
	{
		*ppResult = NULL;
		return S_FALSE;
	}

	LPCWSTR blockEnd;
	size_t blockSize;
	size_t cchResultMax = 0;
	BOOL needEncode = FALSE;
	for (LPCWSTR cursor = pszAddress; L'\0' != *cursor;)
	{
		if (*cursor > 0xFF && SUCCEEDED(AddressEncoder_GetWideBlock(cursor, &blockEnd, NULL, &blockSize)))
		{
			cursor = blockEnd;
			cchResultMax += blockSize;
			needEncode = TRUE;
		}
		else
		{
			cursor = CharNext(cursor);
			cchResultMax++;
		}
	}

	if (FALSE == needEncode)
	{
		*ppResult = NULL;
		return S_FALSE;
	}

	HRESULT hr;
	cchResultMax++;
	LPWSTR result = Plugin_MallocString(cchResultMax);
	if (NULL == result)
		hr = E_OUTOFMEMORY;
	else
	{
		LPWSTR cursor = result;
		size_t remaining = cchResultMax;
		LPCWSTR address = pszAddress;
		LPCWSTR addressBlock = address;	

		for (;;)
		{		
			if (*address > 0xFF)
			{
				if (addressBlock != address)
				{
					hr = StringCchCopyNEx(cursor, remaining, addressBlock, (size_t)(address - addressBlock), &cursor, &remaining, 0);
					if (FAILED(hr))	break;
				}

				blockSize = remaining;
				hr = AddressEncoder_GetWideBlock(address, &address, cursor, &blockSize);
				if (FAILED(hr)) break;

				cursor += blockSize;
				remaining -= blockSize;

				addressBlock = address;
				continue;
			}
			
			if (L'\0' == *address)
			{
				if (addressBlock != address)
				{
					hr = StringCchCopyNEx(cursor, remaining, addressBlock, (size_t)(address - addressBlock), &cursor, &remaining, 0);
				}
				break;
			}
			else
				address = CharNext(address);
		}

		*cursor = L'\0';
	}

	

	if (FAILED(hr))
	{
		Plugin_FreeString(result);
		result = NULL;
	}

	*ppResult = result;
	return hr;
}

HRESULT AddressEncoder_EncodeString(LPCWSTR pszAddress, LPWSTR pszBuffer, size_t *pcchBufferMax, UINT flags)
{
	if (NULL == pszBuffer || NULL == pcchBufferMax) 
		return E_INVALIDARG;
	
	if (NULL == pszAddress || L'\0' == *pszAddress)
	{
		*pszBuffer = L'\0';
		*pcchBufferMax = 0;
		return S_OK;
	}

	INT cchAddress = lstrlen(pszAddress);
	LPCWSTR begin, end;
	begin = pszAddress;
	end = pszAddress + cchAddress;
	WORD charType;
	while (L'\0' != *begin &&
		FALSE != GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, begin, 1, &charType) && 0 != (C1_SPACE & charType))
	{
		begin = CharNext(begin);
	}
	while (begin != end && 
		FALSE != GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, begin, 1, &charType) && 0 != (C1_SPACE & charType))
	{
		end = CharPrev(begin, end);
	}

	if (end <= begin)
	{
		*pszBuffer = L'\0';
		*pcchBufferMax = 0;
		return S_OK;
	}

	LPWSTR encoded;
	HRESULT hr = AddressEncoder_EncodeWideChars(begin, (end - begin), &encoded);
	if (FAILED(hr)) return hr;

	if (S_OK == hr)
	{
		begin = encoded;
		end = begin + lstrlen(begin);
	}
	
	DWORD bufferLen = (DWORD)(*pcchBufferMax);
	if (FALSE == InternetCanonicalizeUrl(begin, pszBuffer, &bufferLen, flags))
	{
		DWORD errorCode = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == errorCode)
		{
			*pcchBufferMax = bufferLen;
			hr = ENC_E_INSUFFICIENT_BUFFER;	
		}
		else 
		{
			size_t cchNeeded = (end - begin);
			if (cchNeeded < *pcchBufferMax)
			{
				hr = StringCchCopyN(pszBuffer, *pcchBufferMax, begin, cchNeeded);
				if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
				{ 
					hr = ENC_E_INSUFFICIENT_BUFFER;
				}
			}
			else
			{
				hr = ENC_E_INSUFFICIENT_BUFFER;
			}
			*pcchBufferMax = cchNeeded + 1;
		}
	}

	else
		hr = S_OK;
	
	
	Plugin_FreeString(encoded);
	return hr;
}