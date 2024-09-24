#ifndef _COMMON_H
#define _COMMON_H

#ifdef WIN32
// disable "dll-interface to be used by clients of class" warning message.
#pragma warning(disable: 4251)
#endif /* WIN32 */

#ifdef COMEXP
#undef COMEXP
#endif
#define COMEXP 


#ifndef __cplusplus 
#define EXTC extern 
#else
#define EXTC extern "C" 
#endif

#include <bfc/wasabi_std.h>

#endif /* _COMMON_H */
