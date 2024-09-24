#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <foundation/types.h>

#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "syscb/api_syscb.h"
extern api_syscb *syscbApi;
#define WASABI2_API_SYSCB syscbApi

//#include "application/api_android.h"
//extern api_android *androidApi;
//#define WASABI2_API_ANDROID androidApi
