#ifndef NULLOSFT_MEDIALIBRARY_FOLDERBROWSER_CONTROL_INTERNAL_HEADER
#define NULLOSFT_MEDIALIBRARY_FOLDERBROWSER_CONTROL_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./ml_ipc_0313.h"
#include "../nu/trace.h"

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define COLUMN_DEFAULT_WIDTH		120
#define COLUMN_MIN_WIDTH			48
#define COLUMN_MAX_WIDTH			640
#define COLUMN_EXTRALSPACE		32
#define SIZER_WIDTH				0
#define SIZER_OVERLAP_LEFT		1		// how many pixels sizer steels from neighbors
#define SIZER_OVERLAP_RIGHT		3		// how many pixels sizer steels from neighbors


#define FBIS_SELECTED		0x00000001
#define FBIS_HIGHLIGHTED	0x00000002



#endif //NULLOSFT_MEDIALIBRARY_FOLDERBROWSER_CONTROL_INTERNAL_HEADER