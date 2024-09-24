#pragma once

#ifdef WA_STR_EXPORT
#include <bfc/platform/export.h>
#define DLLIMPORT DLLEXPORT
#else
#define DLLIMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* if you need a temporary ref counted string to pass INTO a function
   and have a hardcoded string you want to define on the stack,
	 you can use this macro.
	 e.g. wa_str myStr = WA_STR_STATIC("test"); */
#define WA_STR_STATIC(x) ((wa_str)"\0\0\0\0" ## x);

typedef void *wa_str;

/* convert a C string into a ref counted string 
   if you own the string, try using wa_str_own instead */
DLLIMPORT wa_str wa_strdup(const char *); 

/* add a reference to a string
   returns the new pointer to use!!!
	 in some cases (e.g. WA_STR_STATIC strings) the string must be re-malloc'd 
	 so be sure to assign the return value to your string
	 e.g.  wa_str myCopy = wa_str_addref(str); */
DLLIMPORT wa_str wa_str_retain(wa_str str);

/* release a reference to a string */
DLLIMPORT void wa_str_release(wa_str str);

/* gets a C-style string from a ref counted string.
   only valid for as long as you hold a reference! */
DLLIMPORT const char *wa_str_get(wa_str str);

/* copies the contents of a ref counted string into the passed buffer
   *dest = 0 on empty string */
DLLIMPORT void wa_str_strcpy(wa_str str, char *dest, size_t destlen);

/* allocates a reference counted string large enough to hold the given character count
   len MUST include null terminator (e.g. pass 5 to malloc enough for "test")
	 data is set to where the character data can be written to.  
	 you'll need to null terminate the string you write */
DLLIMPORT wa_str wa_str_malloc(size_t len, char **data);

/* allocates a reference counted string using a C string you already have
   frees your string with the supplied free_func when reference count reaches 0 */
DLLIMPORT wa_str wa_str_own(char *ptr, void (*free_func)(char *));

/* a convenient typedef for the above function.
   if you want to pass the standard C free() function
	 use (WA_STR_FREE_FUNC)free */
typedef void (*WA_STR_FREE_FUNC)(char *); 

#ifdef __cplusplus
}
#endif