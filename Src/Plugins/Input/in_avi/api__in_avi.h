#pragma once

#include "../Agave/Config/api_config.h"
#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

void WasabiInit();
void WasabiQuit();
