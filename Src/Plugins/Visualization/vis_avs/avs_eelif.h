/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef _AVS_EEL_IF_H_
#define _AVS_EEL_IF_H_

#include "../ns-eel2/ns-eel.h"

void AVS_EEL_IF_init();
void AVS_EEL_IF_quit();

NSEEL_CODEHANDLE AVS_EEL_IF_Compile(void *ctx, char *code);
void AVS_EEL_IF_Execute(NSEEL_CODEHANDLE handle, char visdata[2][2][576]);
void AVS_EEL_IF_resetvars(NSEEL_VMCTX ctx);
#define AVS_EEL_IF_VM_free(x) NSEEL_VM_free(x)
extern char last_error_string[1024];
extern int g_log_errors;
extern CRITICAL_SECTION g_eval_cs;

// our old-style interface
#define compileCode(exp) AVS_EEL_IF_Compile(AVS_EEL_CONTEXTNAME,(exp))
#define executeCode(x,y) AVS_EEL_IF_Execute(x,y)
#define freeCode(h) NSEEL_code_free(h)
#define resetVars(x) FIXME+++++++++
#define registerVar(x) NSEEL_VM_regvar(AVS_EEL_CONTEXTNAME,(x))
#define clearVars() AVS_EEL_IF_resetvars(AVS_EEL_CONTEXTNAME)

#define AVS_EEL_INITINST() AVS_EEL_CONTEXTNAME=NSEEL_VM_alloc()

#define AVS_EEL_QUITINST() NSEEL_VM_free(AVS_EEL_CONTEXTNAME)


#endif//_AVS_EEL_IF_H_