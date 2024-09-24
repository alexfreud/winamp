#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
size_t __cdecl strlen(const char *s)
{
	return lstrlenA(s);
}

int __cdecl strcmp(const char *a, const char *b)
{
	return lstrcmpA(a,b);
}

#ifdef __cplusplus
}
#endif