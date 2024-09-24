#pragma once

#include "service/api_service.h"
extern api_service *metadata_service_manager;
#define WASABI2_API_SVC metadata_service_manager

#include "MetadataManager.h"
extern MetadataManager metadata_manager;
#define REPLICANT_API_METADATA (&metadata_manager)
