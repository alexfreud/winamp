#ifndef NULLOSFT_MEDIALIBRARY_CLOUD_HEADER
#define NULLOSFT_MEDIALIBRARY_CLOUD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./ml_imagelist.h"

// Draws Cloud Status based on CLOUDDRAWPARAMS
BOOL MLCloudI_Draw(HDC	hdc, INT value, HMLIMGLST hmlil, INT index, RECT *prc);
BOOL MLCloudI_CalcMinRect(HMLIMGLST hmlil, RECT *prc);

#endif //NULLOSFT_MEDIALIBRARY_CLOUD_HEADER