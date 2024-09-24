#pragma once
#include "service/api_service.h"
extern api_service *serviceApi;
#define WASABI2_API_SVC serviceApi

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi
