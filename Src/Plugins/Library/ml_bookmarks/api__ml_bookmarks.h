#ifndef NULLSOFT_API_ML_BOOKMARKS_H
#define NULLSOFT_API_ML_BOOKMARKS_H

#include "api/service/waServiceFactory.h"

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include "api/application/api_application.h"
#define WASABI_API_APP applicationApi

#endif  // !NULLSOFT_API_ML_BOOKMARKS_H