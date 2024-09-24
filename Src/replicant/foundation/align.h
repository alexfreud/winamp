#pragma once

# if defined(__GNUC__)
#include <stdlib.h> // for posix_memalign
#define NALIGN(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#include <malloc.h> // for _aligned_malloc
#define NALIGN(x) __declspec (align(x))
#endif
