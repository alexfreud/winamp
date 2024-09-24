#ifndef NULLSOFT_IN_MP3_API_H
#define NULLSOFT_IN_MP3_API_H

#include "../Agave/Config/api_config.h"

#include "api/memmgr/api_memmgr.h"
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr

#include "../Agave/Language/api_language.h"

#include "api/application/api_application.h"
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#endif // !NULLSOFT_IN_MP3_API_H