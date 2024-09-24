#ifndef NULLSOFT_IN_MP4_VIRTUALIO_H
#define NULLSOFT_IN_MP4_VIRTUALIO_H
#include "main.h"
#include <api/service/svcs/svc_fileread.h>
#include <virtual_io.h>

void *CreateReader(const wchar_t *url, HANDLE killswitch);
void DestroyReader(void *reader);
void StopReader(void *reader);
extern Virtual_IO HTTPIO;
extern Virtual_IO UnicodeIO;

void *CreateUnicodeReader(const wchar_t *filename);
void DestroyUnicodeReader(void *reader);
int UnicodeClose(void *user);

#endif