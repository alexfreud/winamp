#include "./trace.h"
#include <strsafe.h>

#ifdef _DEBUG

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void DebugPrintfW(LPCWSTR format, ...)
{
	va_list argList;
	wchar_t *pstr(NULL);
	void *buffer;
	size_t allocated(0), remaining(0);
	int attempt(0);

	va_start(argList, format);
	
	do
	{	
		attempt++;
		if(attempt)
		{
			allocated += (512 * attempt);
			buffer = realloc(pstr, allocated * sizeof(wchar_t));
			if (NULL == buffer)
				break;

			pstr = (wchar_t*)buffer;
		}
		
	}
	while (STRSAFE_E_INSUFFICIENT_BUFFER == StringCchVPrintfExW(pstr, allocated, NULL, &remaining, 
																STRSAFE_NULL_ON_FAILURE, format, argList));
	
	OutputDebugStringW(pstr);

	if (NULL != pstr) 
		free(pstr);

	va_end(argList);
	
}

void DebugPrintfA(LPCSTR format, ...)
{
	va_list argList;
	char *pstr(NULL);
	void *buffer;
	size_t allocated(0), remaining(0);
	int attempt(0);

	va_start(argList, format);
	
	do
	{	
		attempt++;
		if(attempt)
		{
			allocated += (512 * attempt);
			buffer = realloc(pstr, allocated * sizeof(char));
			if (NULL == buffer)
				break;

			pstr = (char*)buffer;
		}
		
	}
	while (STRSAFE_E_INSUFFICIENT_BUFFER == StringCchVPrintfExA(pstr, allocated, NULL, &remaining, 
																STRSAFE_NULL_ON_FAILURE, format, argList));
	
	OutputDebugStringA(pstr);
	
	if (NULL != pstr) 
		free(pstr);
	
	va_end(argList);	
}


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_DEBUG*/