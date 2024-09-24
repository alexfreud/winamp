/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel.h: main application interface header

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/


#ifndef __NS_EEL_H__
#define __NS_EEL_H__

#ifdef __cplusplus
extern "C" {
#endif

int NSEEL_init(); // returns 0 on success
#define NSEEL_addfunction(name,nparms,code,len) NSEEL_addfunctionex((name),(nparms),(code),(len),0)
void NSEEL_addfunctionex(char *name, int nparms, int code_startaddr, int code_len, void *pproc);
void NSEEL_quit();
int *NSEEL_getstats(); // returns a pointer to 5 ints... source bytes, static code bytes, call code bytes, data bytes, number of code handles
double *NSEEL_getglobalregs();

typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

NSEEL_VMCTX NSEEL_VM_alloc(); // return a handle
void NSEEL_VM_free(NSEEL_VMCTX ctx); // free when done with a VM and ALL of its code have been freed, as well

void NSEEL_VM_resetvars(NSEEL_VMCTX ctx);
double *NSEEL_VM_regvar(NSEEL_VMCTX ctx, char *name);

NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, char *code);
char *NSEEL_code_getcodeerror(NSEEL_VMCTX ctx);
void NSEEL_code_execute(NSEEL_CODEHANDLE code);
void NSEEL_code_free(NSEEL_CODEHANDLE code);
int *NSEEL_code_getstats(NSEEL_CODEHANDLE code); // 4 ints...source bytes, static code bytes, call code bytes, data bytes
  


// configuration:


//#define NSEEL_REENTRANT_EXECUTION
// defining this allows code to run in different threads at the same time
// this can be slower at times.

//#define NSEEL_MAX_VARIABLE_NAMELEN 8
// define this to override the max variable length (default is 8 bytes)

//#define NSEEL_MAX_TEMPSPACE_ENTRIES 2048
// define this to override the maximum working space in 8 byte units.
// 2048 is the default, and is way more than enough for most applications


//#define NSEEL_LOOPFUNC_SUPPORT
//#define NSEEL_LOOPFUNC_SUPPORT_MAXLEN (4096)
// define this for loop() support
// and define maxlen if you wish to override the maximum loop length (default is 4096 times)

#ifdef __cplusplus
}
#endif

#endif//__NS_EEL_H__