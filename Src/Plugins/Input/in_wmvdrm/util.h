#ifndef NULLSOFT_UTILH
#define NULLSOFT_UTILH

#include <windows.h>

void WaitForEvent(HANDLE hEvent, DWORD msMaxWaitTime);
char *HRErrorCode(HRESULT hr);

void GuidString(GUID guid, wchar_t *target, size_t len);

GUID StringGUID(const wchar_t *source);

void BinaryString(unsigned char *binary, size_t size, wchar_t *final, size_t len);

const wchar_t *UserTextDescription(unsigned char *binary, size_t size);
const wchar_t *UserTextString(unsigned char *binary, size_t size);

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message);

#endif
