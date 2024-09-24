/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel-addfuncs.h: defines macros useful for adding functions to the compiler

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

#ifndef __NS_EEL_ADDFUNCS_H__
#define __NS_EEL_ADDFUNCS_H__



typedef void (*NSEEL_PPPROC)(void *data, int data_size, void **userfunc_data);



// these are used for making your own naked functions in C.
/*
For example:
static double (*__acos)(double) = &acos;
__declspec ( naked ) void _asm_acos(void)
{
  FUNC1_ENTER

  *__nextBlock = __acos(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_acos_end(void) {}



If you want to do straight asm, then , well, you can use your imagination
(eax, ebx, ecx are input, eax is output, all points to "double")
if you need 8 bytes of temp space for your output, use esi and increment esi by 8
be sure to preserve edi, too.

*/



#define FUNC1_ENTER \
  double *parm_a, *__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov __nextBlock, esi }

#define FUNC2_ENTER \
  double *parm_a,*parm_b,*__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov dword ptr parm_b, ebx } \
  __asm { mov __nextBlock, esi }

#define FUNC3_ENTER \
  double *parm_a,*parm_b,*parm_c,*__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov dword ptr parm_b, ebx } \
  __asm { mov dword ptr parm_c, ecx } \
  __asm { mov __nextBlock, esi }

#define FUNC_LEAVE \
  __asm { mov eax, esi } \
  __asm { add esi, 8 }  \
  __asm { mov esp, ebp }

#define NSEEL_CGEN_CALL __fastcall


#endif//__NS_EEL_ADDFUNCS_H__