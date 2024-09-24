#include "PathString.h"
#include <shlwapi.h>

PathString::PathString(const wchar_t *directory, const wchar_t *filename)
{
	PathCombineW(path, directory, filename);
}