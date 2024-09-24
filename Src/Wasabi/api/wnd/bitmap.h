#ifdef _WIN32
#include "platform/win32/bitmap.h"
#elif defined(__APPLE__)
#include "platform/osx/osx_bitmap_cgimage.h"
#else
#error port me
#endif