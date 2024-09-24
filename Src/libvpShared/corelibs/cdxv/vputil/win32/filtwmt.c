/****************************************************************************
 *
 *   Module Title :     newLoopTest_asm.c 
 *
 *   Description  :     Codec specific functions
 *
 *   AUTHOR       :     Yaowu Xu
 *
 *****************************************************************************
 *   Revision History
 *
 *   1.02 YWX 03-Nov-00 Changed confusing variable name
 *   1.01 YWX 02-Nov-00 Added the set of functions
 *   1.00 YWX 19-Oct-00 configuration baseline
 *****************************************************************************
 */ 

/****************************************************************************
 *  Header Frames
 *****************************************************************************
 */


#define STRICT              /* Strict type checking. */
#include "codec_common.h"
#include <math.h>

 /****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define FILTER_WEIGHT 128
#define FILTER_SHIFT  7
__declspec(align(16)) short rd[]={64,64,64,64,64,64,64,64};


__declspec(align(16)) INT16  BilinearFilters_wmt[8][16] = 
{
{ 128,128,128,128,128,128,128,128,    0,  0, 0,   0,  0,  0,  0,  0 },
{ 112,112,112,112,112,112,112,112,   16, 16, 16, 16, 16, 16, 16, 16 },
{  96, 96, 96, 96, 96, 96, 96, 96,   32, 32, 32, 32, 32, 32, 32, 32 },
{  80, 80, 80, 80, 80, 80, 80, 80,   48, 48, 48, 48, 48, 48, 48, 48 },
{  64, 64, 64, 64, 64, 64, 64, 64,   64, 64, 64, 64, 64, 64, 64, 64 },
{  48, 48, 48, 48, 48, 48, 48, 48,   80, 80, 80, 80, 80, 80, 80, 80 },
{  32, 32, 32, 32, 32, 32, 32, 32,   96, 96, 96, 96, 96, 96, 96, 96 },
{  16, 16, 16, 16, 16, 16, 16, 16,  112,112,112,112,112,112,112,112 }
};

extern __declspec(align(16)) INT16  BicubicFilters_mmx[17][8][32];

_inline 
void FilterBlock1d_h_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]             ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi+ 16]         ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm6, [edi + 32]        ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm7, [edi + 48]        ; xmm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        dec         esi
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000

nextrow:

        // kernel 0 and 3 are potentially negative taps.  These negative tap filters 
        // must be done first or we could have problems saturating our high value 
        // tap filters
        movdqu		xmm3, [esi]             ; xmm3 = p-1..p14    
        movdqu      xmm4, xmm3              ; xmm4 = p-1..p14
        punpcklbw   xmm3, xmm0              ; xmm3 = p-1..p6
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        psrldq      xmm4, 3                 ; xmm4 = p2..p13
        movdqa      xmm5, xmm4              ; xmm5 = p2..p13
        punpcklbw   xmm5, xmm0              ; xmm5 = p2..p7
        pmullw      xmm5, xmm7              ; xmm5 *= kernel 3 modifiers
        paddsw      xmm3, xmm5              ; xmm3 += xmm5

        movdqu      xmm4, [esi+1]           ; xmm4 = p0..p13
        movdqa      xmm5, xmm4              ; xmm5 = p0..p13
        punpcklbw   xmm5, xmm0              ; xmm5 = p0..p7
        pmullw      xmm5, xmm2              ; xmm5 *= kernel 1 modifiers
        paddsw      xmm3, xmm5              ; xmm3 += xmm5

        psrldq      xmm4, 1                 ; xmm4 = p1..p13
        movdqa      xmm5, xmm4              ; xmm5 = p1..p13
        punpcklbw   xmm5, xmm0              ; xmm5 = p1..p7
        pmullw      xmm5, xmm6              ; xmm5 *= kernel 2 modifiers
        paddsw      xmm3, xmm5              ; xmm3 += xmm5

        paddsw      xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
        packuswb    xmm3, xmm0              ; pack and saturate

        movdq2q     mm0, xmm3
        movq        [edi],mm0               ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}

_inline 
void FilterBlock1d_v_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]          ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi + 16]     ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm6, [edi + 32]     ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm7, [edi + 48]     ; xmm3 *= kernel 0 modifiers.

        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        sub         esi, PixelsPerLine
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000


nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p0..p16
        punpcklbw   xmm3, xmm0              ; xmm3 = p0..p8
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        add         esi, edx                ; move source forward 1 line to avoid 3 * pitch

        movdqu		xmm4, [esi+2*edx]       ; xmm4 = p0..p16
        punpcklbw   xmm4, xmm0              ; xmm4 = p0..p8
        pmullw      xmm4, xmm7              ; xmm4 *= kernel 3 modifiers.
        paddsw      xmm3, xmm4              ; xmm3 += xmm4

        movdqu		xmm4, [esi ]            ; xmm4 = p0..p16
        punpcklbw   xmm4, xmm0              ; xmm4 = p0..p8
        pmullw      xmm4, xmm2              ; xmm4 *= kernel 1 modifiers.
        paddsw      xmm3, xmm4              ; xmm3 += xmm4

        movdqu		xmm4, [esi +edx]        ; xmm4 = p0..p16
        punpcklbw   xmm4, xmm0              ; xmm4 = p0..p8
        pmullw      xmm4, xmm6              ; xmm4 *= kernel 2 modifiers.
        paddsw      xmm3, xmm4              ; xmm3 += xmm4



        paddsw      xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
        packuswb    xmm3, xmm0              ; pack and unpack to saturate

        movdq2q     mm0, xmm3
        movq        [edi],mm0               ; store the results in the destination

        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row

    }
}


_inline 
void FilterBlock1d_hb8_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]          ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi + 16]     ; xmm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000

nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p-1..p14    
        movdqu      xmm5, xmm3              ; xmm4 = p-1..p14
        punpcklbw   xmm3, xmm0              ; xmm3 = p-1..p6
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        psrldq      xmm5, 1                 ; xmm4 = p0..p13
        punpcklbw   xmm5, xmm0              ; xmm5 = p0..p7
        pmullw      xmm5, xmm2              ; xmm5 *= kernel 1 modifiers
        paddw       xmm3, xmm5              ; xmm3 += xmm5

        paddw       xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
        packuswb    xmm3, xmm0              ; pack and unpack to saturate

        movdq2q     mm0, xmm3
        movq        [edi],mm0               ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}

_inline 
void FilterBlock1d_vb8_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]          ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi + 16]     ; xmm3 *= kernel 0 modifiers.
        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000


nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p0..p16
        punpcklbw   xmm3, xmm0              ; xmm3 = p0..p8
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        movdqu		xmm4, [esi +edx ]       ; xmm4 = p0..p16
        punpcklbw   xmm4, xmm0              ; xmm4 = p0..p8
        pmullw      xmm4, xmm2              ; xmm4 *= kernel 1 modifiers.
        paddw       xmm3, xmm4              ; xmm3 += xmm4

        paddw       xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
        packuswb    xmm3, xmm0              ; pack and unpack to saturate

        movdq2q     mm0, xmm3
        movq        [edi],mm0               ; store the results in the destination

        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         esi,edx
        add         edi,eax 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row

    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock2dBil
 *  
 *  INPUTS        :     Pointer to source data
 *						
 *  OUTPUTS       :     Filtered data
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Applies a bilinear filter on the intput data to produce
 *						a predictor block (UINT16)
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
_inline 
void FilterBlock2dBil_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, INT16 * HFilter, INT16 * VFilter )
{

    __asm
    {
        mov         eax,        HFilter             ; 
        mov         edi,        OutputPtr           ; 
        mov         esi,        SrcPtr              ;
        lea         ecx,        [edi+64]            ;
        mov         edx,        SrcPixelsPerLine     ;
               
        movdqa      xmm1,       [eax]               ;
        movdqa      xmm2,       [eax+16]            ;
        
        mov         eax,        VFilter             ;       
        pxor        xmm0,       xmm0                ;

        // get the first horizontal line done       ;
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movdqa      xmm4,       xmm3                ; make a copy of current line
        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06
        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx        
        
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        pmullw      xmm4,       xmm2                ;
        paddw       xmm3,       xmm4                ;   

        paddw       xmm3,       rd                  ; 
        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        
        movdqa      xmm5,       xmm3                ;

        add         esi,        edx                 ; next line
NextRow:
        pmullw      xmm5,       [eax]               ; 
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14

        movdqa      xmm4,       xmm3                ; make a copy of current line        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06

        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx                
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        movdqa      xmm6,       xmm5                ; 
        pmullw      xmm4,       xmm2                ;

        paddw       xmm3,       xmm4                ;   
        paddw       xmm3,       rd                  ; 

        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        movdqa      xmm5,       xmm3                ; make a copy for the next row
        
        pmullw      xmm3,       [eax+16]            ; 
        paddw       xmm6,       xmm3                ;
        

        paddw       xmm6,       rd                  ; xmm6 += round value
        psraw       xmm6,       FILTER_SHIFT        ; xmm6 /= 128

        packuswb    xmm6,       xmm0                ; pack and unpack to saturate
        movdq2q     mm0,        xmm6

        movq        [edi],      mm0                 ; store the results in the destination
        add         esi,        edx                 ; next line
        add         edi,        8                   ; 

        cmp         edi,        ecx                 ;
        jne         NextRow                         

    }

    // First filter 1d Horizontal
	//FilterBlock1d_hb8_wmt(SrcPtr, Intermediate, SrcPixelsPerLine, 1, 9, 8, HFilter );
	// Now filter Verticaly
	//FilterBlock1d_vb8_wmt(Intermediate, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter);


}

_inline 
void FilterUnpackBlock2dBil_wmt( UINT8 *SrcPtr, INT16 *OutputPtr, UINT32 SrcPixelsPerLine, INT16 * HFilter, INT16 * VFilter )
{

    __asm
    {
        mov         eax,        HFilter             ; 
        mov         edi,        OutputPtr           ; 
        mov         esi,        SrcPtr              ;
        lea         ecx,        [edi+128]            ;
        mov         edx,        SrcPixelsPerLine     ;
               
        movdqa      xmm1,       [eax]               ;
        movdqa      xmm2,       [eax+16]            ;
        
        mov         eax,        VFilter             ;       
        pxor        xmm0,       xmm0                ;

        // get the first horizontal line done       ;
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movdqa      xmm4,       xmm3                ; make a copy of current line
        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06
        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx        
        
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        pmullw      xmm4,       xmm2                ;
        paddw       xmm3,       xmm4                ;   

        paddw       xmm3,       rd                  ; 
        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        
        movdqa      xmm5,       xmm3                ;

        add         esi,        edx                 ; next line
NextRow:
        pmullw      xmm5,       [eax]               ; 
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14

        movdqa      xmm4,       xmm3                ; make a copy of current line        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06

        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx                
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        movdqa      xmm6,       xmm5                ; 
        pmullw      xmm4,       xmm2                ;

        paddw       xmm3,       xmm4                ;   
        paddw       xmm3,       rd                  ; 

        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        movdqa      xmm5,       xmm3                ; make a copy for the next row
        
        pmullw      xmm3,       [eax+16]            ; 
        paddw       xmm6,       xmm3                ;
        

        paddw       xmm6,       rd                  ; xmm6 += round value
        psraw       xmm6,       FILTER_SHIFT        ; xmm6 /= 128

        movdqu      [edi],      xmm6;
        
        /*
        packuswb    xmm6,       xmm0                ; pack and unpack to saturate
        movdq2q     mm0,        xmm6

        movq        [edi],      mm0                 ; store the results in the destination
        */
        add         esi,        edx                 ; next line
        add         edi,        16                   ; 

        cmp         edi,        ecx                 ;
        jne         NextRow                         

    }

    // First filter 1d Horizontal
	//FilterBlock1d_hb8_wmt(SrcPtr, Intermediate, SrcPixelsPerLine, 1, 9, 8, HFilter );
	// Now filter Verticaly
	//FilterBlock1d_vb8_wmt(Intermediate, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter);


}
_inline 
void FilterUnpackBlock1d_hb8_wmt( UINT8 *SrcPtr, INT16 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]          ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi + 16]     ; xmm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000

nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p-1..p14    
        movdqu      xmm5, xmm3              ; xmm4 = p-1..p14
        punpcklbw   xmm3, xmm0              ; xmm3 = p-1..p6
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        psrldq      xmm5, 1                 ; xmm4 = p0..p13
        punpcklbw   xmm5, xmm0              ; xmm5 = p0..p7
        pmullw      xmm5, xmm2              ; xmm5 *= kernel 1 modifiers
        paddw       xmm3, xmm5              ; xmm3 += xmm5

        paddw       xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
        
        /*
        packuswb    xmm3, xmm0              ; pack and unpack to saturate
        movdq2q     mm0, xmm3
        */

        movdqu      [edi],xmm3               ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}

_inline 
void FilterUnpackBlock1d_vb8_wmt( UINT8 *SrcPtr, INT16 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
    __asm
    {

        mov         edi, Filter
        movdqa      xmm1, [edi]          ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2, [edi + 16]     ; xmm3 *= kernel 0 modifiers.
        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000


nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p0..p16
        punpcklbw   xmm3, xmm0              ; xmm3 = p0..p8
        pmullw      xmm3, xmm1              ; xmm3 *= kernel 0 modifiers.

        movdqu		xmm4, [esi +edx ]       ; xmm4 = p0..p16
        punpcklbw   xmm4, xmm0              ; xmm4 = p0..p8
        pmullw      xmm4, xmm2              ; xmm4 *= kernel 1 modifiers.
        paddw       xmm3, xmm4              ; xmm3 += xmm4

        paddw       xmm3, rd                ; xmm3 += round value
        psraw       xmm3, FILTER_SHIFT      ; xmm3 /= 128
       
        /*packuswb    xmm3, xmm0              ; pack and unpack to saturate

        movdq2q     mm0, xmm3
        */
        movdqu      [edi],xmm3               ; store the results in the destination

        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         esi,edx
        add         edi,eax 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row

    }
}
 

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlockBil_8
 *  
 *  INPUTS        :     ReconPtr1, ReconPtr12
 *							Two pointers into the block of data to be filtered
 *							These pointers bound the fractional pel position
 *						PixelsPerLine
 *							Pixels per line in the buffer pointed to by ReconPtr1 & ReconPtr12
 *						Modx, ModY
 *							The fractional pel bits used to select a filter.
 *
 *				
 *  OUTPUTS       :     ReconRefPtr
 *							A pointer to an 8x8 buffer into which UINT8 filtered data is written.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Produces a bilinear filtered fractional pel prediction block
 *						with UINT8 output
 *
 *  SPECIAL NOTES :      
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlockBil_8_wmt( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY )
{
	int diff;

	// swap pointers so ReconPtr1 smaller (above, left, above-right or above-left )
	diff=ReconPtr2-ReconPtr1;

	// The ModX and ModY arguments are the bottom three bits of the signed motion vector components (at 1/8th pel precision).
	// This works out to be what we want... despite the pointer swapping that goes on below.
	// For example... if the X component of the vector is a +ve ModX = X%8.
	//                if the X component of the vector is a -ve ModX = 8+(X%8) where X%8 is in the range -7 to -1.

	if(diff<0) 
	{											// swap pointers so ReconPtr1 smaller
		UINT8 *temp=ReconPtr1;
		ReconPtr1=ReconPtr2;
		ReconPtr2=temp;
		diff= (int)(ReconPtr2-ReconPtr1);
	}

	if( diff==1 )
	{			
		FilterBlock1d_hb8_wmt(ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 8, BilinearFilters_wmt[ModX] );
	}
	else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
	{
		FilterBlock1d_vb8_wmt(ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters_wmt[ModY]);
	}
	else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
	{										
        FilterBlock2dBil_wmt( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
        //FilterBlock2dBil_8_wmt( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
	}
	else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
	{	
        FilterBlock2dBil_wmt( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
		//FilterBlock2dBil_8_wmt( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
	}
}

_inline void UnpackBlock_wmt( UINT8 *SrcPtr, UINT16 *OutputPtr, UINT32 SrcPixelsPerLine )
{
    __asm
    {
        mov         edi,OutputPtr
		mov			esi,SrcPtr

        mov         ecx, 8
        mov         eax, 16                 ; destination pitch?
		pxor		xmm0, xmm0              ; xmm0 = 00000000

nextrow:
        movdqu		xmm3, [esi]             ; xmm3 = p-1..p14    
        punpcklbw   xmm3, xmm0              ; xmm3 = p-1..p6
        movdqu     [edi],xmm3                ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock2d
 *  
 *  INPUTS        :     Pointer to source data
 *						
 *  OUTPUTS       :     Filtered data
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Applies a 2d 4 tap filter on the intput data to produce
 *						a predictor block (UINT16)
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlock2d_wmt( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, INT16 * HFilter, INT16 * VFilter )
{

    UINT8 Intermediate[256];

	// First filter 1d Horizontal
	FilterBlock1d_h_wmt(SrcPtr-SrcPixelsPerLine, Intermediate, SrcPixelsPerLine, 1, 11, 8, HFilter );

	// Now filter Verticaly
	FilterBlock1d_v_wmt(Intermediate+BLOCK_HEIGHT_WIDTH, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter);


}
 

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock
 *  
 *  INPUTS        :     ReconPtr1, ReconPtr12
 *							Two pointers into the block of data to be filtered
 *							These pointers bound the fractional pel position
 *						PixelsPerLine
 *							Pixels per line in the buffer pointed to by ReconPtr1 & ReconPtr12
 *						Modx, ModY
 *							The fractional pel bits used to select a filter.
 *						UseBicubic
 *							Whether to use the bicubuc filter set or the bilinear set
 *
 *				
 *  OUTPUTS       :     ReconRefPtr
 *							A pointer to an 8x8 buffer into which the filtered data is written.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Produces a filtered fractional pel prediction block
 *						using bilinear or bicubic filters
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlock_wmt( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic, UINT8 BicubicAlpha )
{
	int diff;
    UINT8 Intermediate[256];

	// swap pointers so ReconPtr1 smaller (above, left, above-right or above-left )
	diff=ReconPtr2-ReconPtr1;

	// The ModX and ModY arguments are the bottom three bits of the signed motion vector components (at 1/8th pel precision).
	// This works out to be what we want... despite the pointer swapping that goes on below.
	// For example... if the X component of the vector is a +ve ModX = X%8.
	//                if the X component of the vector is a -ve ModX = 8+(X%8) where X%8 is in the range -7 to -1.

	if(diff<0) 
	{											// swap pointers so ReconPtr1 smaller
		UINT8 *temp=ReconPtr1;
		ReconPtr1=ReconPtr2;
		ReconPtr2=temp;
		diff= (int)(ReconPtr2-ReconPtr1);
	}

    if(!diff)
    {
        return;
    }



    if(UseBicubic)
    {
        if( diff==1 )
        {											        // Fractional pixel in horizontal only
                FilterBlock1d_h_wmt(ReconPtr1, Intermediate, PixelsPerLine, 1, 8, 8, BicubicFilters_mmx[BicubicAlpha][ModX] );
        }
        else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
        {
                FilterBlock1d_v_wmt(ReconPtr1, Intermediate, PixelsPerLine, PixelsPerLine, 8, 8, BicubicFilters_mmx[BicubicAlpha][ModY]);
        }
        else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
        {										
                FilterBlock2d_wmt( ReconPtr1-1, Intermediate, PixelsPerLine, BicubicFilters_mmx[BicubicAlpha][ModX], BicubicFilters_mmx[BicubicAlpha][ModY] );
        }
        else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
        {	
                FilterBlock2d_wmt( ReconPtr1, Intermediate, PixelsPerLine, BicubicFilters_mmx[BicubicAlpha][ModX], BicubicFilters_mmx[BicubicAlpha][ModY] );
        }
        UnpackBlock_wmt( Intermediate, ReconRefPtr, 8 );
    }
    else
    {
   
        if( diff==1 )
        {	
            FilterUnpackBlock1d_hb8_wmt(ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 16, BilinearFilters_wmt[ModX] );
            
            // Fractional pixel in horizontal only
            /*
            FilterBlock1d_hb8_wmt(ReconPtr1, Intermediate, PixelsPerLine, 1, 8, 8, BilinearFilters_wmt[ModX] );
            UnpackBlock_wmt( Intermediate, ReconRefPtr, 8 );
            */
            
        }
        else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
        {
            FilterUnpackBlock1d_vb8_wmt(ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 16, BilinearFilters_wmt[ModY]);    
            /*
            FilterBlock1d_vb8_wmt(ReconPtr1, Intermediate, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters_wmt[ModY]);
            UnpackBlock_wmt( Intermediate, ReconRefPtr, 8 );
            */
        }
        else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
        {										

            FilterUnpackBlock2dBil_wmt( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
            /*
            FilterBlock2dBil_wmt( ReconPtr1-1, Intermediate, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
            UnpackBlock_wmt( Intermediate, ReconRefPtr, 8 );
            */
        }
        else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
        {	
            FilterUnpackBlock2dBil_wmt( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );    
            /*
            FilterBlock2dBil_wmt( ReconPtr1, Intermediate, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );
            UnpackBlock_wmt( Intermediate, ReconRefPtr, 8 );
            */
        }
    }
}

