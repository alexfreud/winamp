#ifdef _WIN32
#include "win/bitmap.h"
#elif defined(__APPLE__)
#include "mac/osx_bitmap_cgimage.h"
#else
#error port me
#endif