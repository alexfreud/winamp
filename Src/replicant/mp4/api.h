#pragma once
#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "metadata/api_metadata.h"
extern api_metadata *metadata_api;
#define REPLICANT_API_METADATA metadata_api

#include "filelock/api_filelock.h"
extern api_filelock *filelock_api;
#define REPLICANT_API_FILELOCK filelock_api