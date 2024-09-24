/***
* nochkclr.c - Dummy non-version-checking CLR call
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*******************************************************************************/


/***
*_check_commonlanguageruntime_version
*
*Purpose:
*       If you don't link to the CRT, you use this obj to fill the compiler's need for this symbol

*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
void __cdecl _check_commonlanguageruntime_version()
{
}
#ifdef __cplusplus
}
#endif
