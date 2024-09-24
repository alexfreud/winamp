//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


#ifndef _CIDASM_H
#define _CIDASM_H

extern PROCTYPE getCPUType(void);
extern void InitXMMReg( void );
extern void TrashXMMReg( void *Junk );
extern int VerifyXMMReg( void );
#endif /* _CIDASM_H */
