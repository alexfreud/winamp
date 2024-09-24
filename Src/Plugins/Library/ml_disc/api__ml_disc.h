#ifndef NULLSOFT_ML_DISC_API_H
#define NULLSOFT_ML_DISC_API_H

#include <api/service/waServiceFactory.h>

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#endif  // !NULLSOFT_ML_DISC_API_H