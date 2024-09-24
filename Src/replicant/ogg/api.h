#pragma once
#include "../replicant/service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "../replicant/application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi
