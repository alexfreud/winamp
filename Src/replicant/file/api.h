#pragma once
#include "service/api_service.h"
#include "application/api_application.h"
#include "syscb/api_syscb.h"
#include "metadata/api_metadata.h"
#include "filelock/api_filelock.h"

DECLARE_EXTERNAL_SERVICE(api_service,          WASABI2_API_SVC);
DECLARE_EXTERNAL_SERVICE(api_application,      WASABI2_API_APP);
DECLARE_EXTERNAL_SERVICE(api_syscb,            WASABI2_API_SYSCB);
DECLARE_EXTERNAL_SERVICE(api_metadata,         REPLICANT_API_METADATA);
DECLARE_EXTERNAL_SERVICE(api_filelock,         REPLICANT_API_FILELOCK);
