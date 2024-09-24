#ifndef NULLSOFT_IN_WAVE_VIRTUALIO_H
#define NULLSOFT_IN_WAVE_VIRTUALIO_H
#include "main.h"
#include <api/service/svcs/svc_fileread.h>

extern SF_VIRTUAL_IO httpIO, unicode_io;
void *CreateReader(const wchar_t *url);
void DestroyReader(void *reader);

#endif