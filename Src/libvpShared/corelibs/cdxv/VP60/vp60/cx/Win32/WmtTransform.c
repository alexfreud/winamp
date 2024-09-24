/****************************************************************************
 * 
 *   Module Title :     WmtTransform.c
 *
 *   Description  :     Subtraction functions.
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include "compdll.h"

/****************************************************************************
*  Macros
****************************************************************************/
#ifdef _MSC_VER 
#pragma warning(disable:4799)
#pragma warning(disable:4731)
#endif

/****************************************************************************
*  Module Statics
****************************************************************************/
_declspec(align(16)) static UINT16 Eight128s[8] = { 128, 128, 128, 128, 128, 128, 128, 128 };

/****************************************************************************
 * 
 *  ROUTINE       : WmtSUB8
 *
 *  INPUTS        : UINT8 *FiltPtr     : 
 *                  UINT8 *ReconPtr    : 
 *                  INT16 *DctInputPtr : 
 *                  UINT8 *old_ptr1    : 
 *                  UINT8 *new_ptr1    : 
 *                  INT32 SourceStride : 
 *                  INT32 ReconStride  :  
 *					
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Subtracts 2 8x8 blocks.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void WmtSUB8
(   
    UINT8 *FiltPtr, 
    UINT8 *ReconPtr, 
    INT16 *DctInputPtr, 
    UINT8 *old_ptr1, 
    UINT8 *new_ptr1, 
    INT32 SourceStride, 
    INT32 ReconStride 
)
{
    (void) old_ptr1;
    (void) new_ptr1;

    _asm
    {      
        mov         eax,    [FiltPtr]
        mov         ebx,    [ReconPtr]

        mov         ecx,    [DctInputPtr]
        mov         edi,    [SourceStride]

        mov         esi,    [ReconStride]
        pxor        xmm7,   xmm7

        lea         edx,    [ecx+128]       

WmtSub8Loop:
     
        movq        xmm0,   QWORD ptr [eax]
        movq        xmm1,   QWORD ptr [ebx]
                
        punpcklbw   xmm0,   xmm7
        punpcklbw   xmm1,   xmm7

        psubw       xmm0,   xmm1
        lea         ecx,    [ecx+16]

        cmp         ecx,     edx
        
        lea         eax,    [eax+edi]
        movdqa      [ecx-16],  xmm0

        lea         ebx,    [ebx+esi]
        jc          WmtSub8Loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : Sub8_128
 *
 *  INPUTS        : UINT8 *FiltPtr     : 
 *                  INT16 *DctInputPtr : 
 *                  UINT8 *old_ptr1    : 
 *                  UINT8 *new_ptr1    : 
 *                  INT32 SourceStride : 
 *					
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Subtracts 128 from each pixel in an 8x8 block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void WmtSUB8_128
(   
    UINT8 *FiltPtr, 
    INT16 *DctInputPtr, 
    UINT8 *old_ptr1, 
    UINT8 *new_ptr1, 
    INT32 SourceStride 
)
{
    (void) old_ptr1;
    (void) new_ptr1;

    _asm
    {
        mov     eax,    [FiltPtr]
        mov     edx,    [DctInputPtr]
        
        mov     ecx,    [SourceStride]
        lea     edi,    [edx + 128]

        pxor    xmm7,   xmm7
        movdqa  xmm1,   [Eight128s]

wmtsub8_128loop:
        
        movq    xmm0,   QWORD PTR [eax]
        punpcklbw   xmm0,   xmm7

        psubw   xmm0,   xmm1;        
        lea     edx,    [edx+16]

        cmp     edx,    edi        
        movdqa  [edx-16], xmm0

        lea     eax,    [eax+ecx]        
        jc     wmtsub8_128loop 
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     Sub8AV2
 *
 *  INPUTS        :     
 *						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Subtracts 2 8x8 blocks
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
/****************************************************************************
 * 
 *  ROUTINE       : WmtSUB8AV2
 *
 *  INPUTS        : UINT8 *FiltPtr     : 
 *                  UINT8 *ReconPtr1    : 
 *                  UINT8 *ReconPtr2    : 
 *                  INT16 *DctInputPtr : 
 *                  UINT8 *old_ptr1    : 
 *                  UINT8 *new_ptr1    : 
 *                  INT32 SourceStride : 
 *                  INT32 ReconStride  :  
 *					
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Subtracts 2 8x8 blocks.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void WmtSUB8AV2
(    
    UINT8 *FiltPtr, 
    UINT8 *ReconPtr1, 
    UINT8 *ReconPtr2, 
    INT16 *DctInputPtr, 
    UINT8 *old_ptr1, 
    UINT8 *new_ptr1, 
    INT32 SourceStride, 
    INT32 ReconStride 
)
{
    (void) old_ptr1;
    (void) new_ptr1;

    _asm
    {
        push        ebp

        mov         esi,    [FiltPtr]
        mov         edi,    [DctInputPtr]

        mov         eax,    [ReconPtr1]
        mov         ebx,    [ReconPtr2]

        mov         ecx,    [SourceStride]
        mov         edx,    [ReconStride]

        lea         ebp,    [edi+128]
        pxor        xmm7,   xmm7

WmtSUB8AV2loop:

        movq        xmm0,   QWORD PTR [eax]
        movq        xmm1,   QWORD PTR [ebx]

        punpcklbw   xmm0,   xmm7
        punpcklbw   xmm1,   xmm7

        paddw       xmm0,   xmm1
        movq        xmm2,   QWORD PTR [esi]
        
        psraw       xmm0,   1
        psubw       xmm2,   xmm0

        lea         edi,    [edi+16]
        cmp         edi,    ebp

        movdqa      [edi-16],   xmm2
        lea         eax,    [eax+edx]

        lea         ebx,    [ebx+edx]
        lea         esi,    [ecx+esi]

        jc          WmtSUB8AV2loop

        pop         ebp
    }
}
