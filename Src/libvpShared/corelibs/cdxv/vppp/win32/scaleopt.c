/****************************************************************************
*        
*   Module Title :     scaleopt.cpp
*
*   Description  :     Optimized scaling functions
*
****************************************************************************/

/****************************************************************************
*  Module Statics
****************************************************************************/        
__declspec(align(16)) const static unsigned short oneFifth[]  = { 51, 51, 51, 51 };
__declspec(align(16)) const static unsigned short twoFifths[] = { 102, 102, 102, 102 };
__declspec(align(16)) const static unsigned short threeFifths[] = { 154, 154, 154, 154 };
__declspec(align(16)) const static unsigned short fourFifths[] = { 205, 205, 205, 205 };
__declspec(align(16)) const static unsigned short roundValues[] = { 128, 128, 128, 128 };
__declspec(align(16)) const static unsigned short fourOnes[]= { 1, 1, 1, 1};
__declspec(align(16)) const static unsigned short const45_2[] = {205, 154, 102,  51 };
__declspec(align(16)) const static unsigned short const45_1[] = { 51, 102, 154, 205 };
__declspec(align(16)) const static unsigned char  mask45[] = { 0, 0, 0, 0, 0, 0, 255, 0};
__declspec(align(16)) const static unsigned short const35_2[] = { 154,  51, 205, 102 };
__declspec(align(16)) const static unsigned short const35_1[] = { 102, 205,  51, 154 };

#if defined(__cplusplus)
extern "C" {
#endif


/****************************************************************************
 * 
 *  ROUTINE       : HorizontalLine_3_5_Scale_MMX
 *
 *  INPUTS        : const unsigned char *source :
 *                  unsigned int sourceWidth    :
 *                  unsigned char *dest         :
 *                  unsigned int destWidth      :
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 3 to 5 up-scaling of a horizontal line of pixels.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void HorizontalLine_3_5_Scale_MMX 
(
    const unsigned char *source,
    unsigned int sourceWidth,
    unsigned char *dest,
    unsigned int destWidth
)
{
    (void) destWidth;

    __asm
    {

        push ebx

        mov         esi,    source
        mov         edi,    dest
        
        mov         ecx,    sourceWidth
        lea         edx,    [esi+ecx-3];

        movq        mm5,    const35_1       // mm5 = 66 xx cd xx 33 xx 9a xx
        movq        mm6,    const35_2       // mm6 = 9a xx 33 xx cd xx 66 xx
        
        movq        mm4,    roundValues     // mm4 = 80 xx 80 xx 80 xx 80 xx
        pxor        mm7,    mm7             // clear mm7

HorizLine_3_5_Loop:
        
        mov        eax,    DWORD PTR [esi] // eax = 00 01 02 03 
        mov        ebx,    eax             
        
        and         ebx,    0xffff00        // ebx = xx 01 02 xx 
        mov         ecx,    eax             // ecx = 00 01 02 03
        
        and         eax,    0xffff0000      // eax = xx xx 02 03
        xor         ecx,    eax             // ecx = 00 01 xx xx

        shr         ebx,    8               // ebx = 01 02 xx xx
        or          eax,    ebx             // eax = 01 02 02 03

        shl         ebx,    16              // ebx = xx xx 01 02
        movd        mm1,    eax             // mm1 = 01 02 02 03 xx xx xx xx 
        
        or          ebx,    ecx             // ebx = 00 01 01 02
        punpcklbw   mm1,    mm7             // mm1 = 01 xx 02 xx 02 xx 03 xx
        
        movd        mm0,    ebx             // mm0 = 00 01 01 02
        pmullw      mm1,    mm6             //

        punpcklbw   mm0,    mm7             // mm0 = 00 xx 01 xx 01 xx 02 xx
        pmullw      mm0,    mm5             //

        mov         [edi],  ebx             // writeoutput 00 xx xx xx    
        add         esi,    3

        add         edi,    5
        paddw       mm0,    mm1
        
        paddw       mm0,    mm4
        psrlw       mm0,    8

        cmp         esi,    edx        
        packuswb    mm0,    mm7
        
        movd        DWORD Ptr [edi-4], mm0
        jl          HorizLine_3_5_Loop

//Exit:        
        mov         eax,    DWORD PTR [esi] // eax = 00 01 02 03 
        mov         ebx,    eax             
        
        and         ebx,    0xffff00        // ebx = xx 01 02 xx 
        mov         ecx,    eax             // ecx = 00 01 02 03
        
        and         eax,    0xffff0000      // eax = xx xx 02 03
        xor         ecx,    eax             // ecx = 00 01 xx xx

        shr         ebx,    8               // ebx = 01 02 xx xx
        or          eax,    ebx             // eax = 01 02 02 03

        shl         eax,    8               // eax = xx 01 02 02
        and         eax,    0xffff0000      // eax = xx xx 02 02

        or          eax,    ebx             // eax = 01 02 02 02

        shl         ebx,    16              // ebx = xx xx 01 02
        movd        mm1,    eax             // mm1 = 01 02 02 02 xx xx xx xx 
        
        or          ebx,    ecx             // ebx = 00 01 01 02
        punpcklbw   mm1,    mm7             // mm1 = 01 xx 02 xx 02 xx 02 xx
        
        movd        mm0,    ebx             // mm0 = 00 01 01 02
        pmullw      mm1,    mm6             //

        punpcklbw   mm0,    mm7             // mm0 = 00 xx 01 xx 01 xx 02 xx
        pmullw      mm0,    mm5             //

        mov         [edi],  ebx             // writeoutput 00 xx xx xx    
        paddw       mm0,    mm1

        paddw       mm0,    mm4
        psrlw       mm0,    8

        packuswb    mm0,    mm7        
        movd        DWORD Ptr [edi+1], mm0

        pop ebx
    
    }

    /*
    const unsigned char *src = source;
    unsigned char *des = dest;
    unsigned int a, b, c ;
    unsigned int i;
    (void) destWidth;

    for ( i=0; i<sourceWidth-3; i+=3 )
    {     
        a = src[0];
        b = src[1];
        des [0] = (UINT8) (a);
        // 2 * left + 3 * right /5 
        des [1] = (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);
        c = src[2] ;
        // 4 * left + 1 * right /5
        des [2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
        // 1 * left + 4 * right /5
        des [3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);

        a = src[3];
        // 3 * left + 2 * right /5 
        des [4] = (UINT8) (( c * 154 + a * 102 + 128 ) >> 8);

        src += 3;
        des += 5;
    }

    a = src[0];
    b = src[1];
    des [0] = (UINT8) (a);
    // 2 * left + 3 * right /5 
    des [1] = (UINT8) (( a * 102 + 154 * b + 128 ) >> 8);
    c = src[2] ;
    // 4 * left + 1 * right /5
    des [2] = (UINT8) (( b * 205 + c * 51 + 128 ) >> 8);
    // 1 * left + 4 * right /5
    des [3] = (UINT8) (( b * 51 + c * 205 + 128 ) >> 8);
    
    des [4] = (UINT8) (c);
*/
}        


/****************************************************************************
 * 
 *  ROUTINE       : HorizontalLine_4_5_Scale_MMX
 *
 *  INPUTS        : const unsigned char *source :
 *                  unsigned int sourceWidth    :
 *                  unsigned char *dest         :
 *                  unsigned int destWidth      :
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 4 to 5 up-scaling of a horizontal line of pixels.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void HorizontalLine_4_5_Scale_MMX 
(
    const unsigned char *source,
    unsigned int sourceWidth,
    unsigned char *dest,
    unsigned int destWidth
)
{
    (void)destWidth;

    __asm
    {
        
        mov         esi,    source
        mov         edi,    dest
        
        mov         ecx,    sourceWidth
        lea         edx,    [esi+ecx-8];

        movq        mm5,    const45_1       // mm5 = 33 xx 66 xx 9a xx cd xx   
        movq        mm6,    const45_2       // mm6 = cd xx 9a xx 66 xx 33 xx
        
        movq        mm4,    roundValues     // mm4 = 80 xx 80 xx 80 xx 80 xx
        pxor        mm7,    mm7             // clear mm7

HorizLine_4_5_Loop:
        
        movq        mm0,    QWORD PTR [esi]           // mm0 = 00 01 02 03 04 05 06 07
        movq        mm1,    QWORD PTR [esi+1];        // mm1 = 01 02 03 04 05 06 07 08

        movq        mm2,    mm0             // mm2 = 00 01 02 03 04 05 06 07
        movq        mm3,    mm1             // mm3 = 01 02 03 04 05 06 07 08
        
        movd        DWORD PTR [edi],  mm0             // write output 00 xx xx xx        
        punpcklbw   mm0,    mm7             // mm0 = 00 xx 01 xx 02 xx 03 xx
        
        punpcklbw   mm1,    mm7             // mm1 = 01 xx 02 xx 03 xx 04 xx         
        pmullw      mm0,    mm5             // 00* 51 01*102 02*154 03*205

        pmullw      mm1,    mm6             // 01*205 02*154 03*102 04* 51
        punpckhbw   mm2,    mm7             // mm2 = 04 xx 05 xx 06 xx 07 xx
     
        movd        DWORD PTR [edi+5], mm2            // write ouput 05 xx xx xx
        pmullw      mm2,    mm5             // 04* 51 05*102 06*154 07*205

        punpckhbw   mm3,    mm7             // mm3 = 05 xx 06 xx 07 xx 08 xx
        pmullw      mm3,    mm6             // 05*205 06*154 07*102 08* 51    
        
        paddw       mm0,    mm1             // added round values
        paddw       mm0,    mm4

        psrlw       mm0,    8               // output: 01 xx 02 xx 03 xx 04 xx
        packuswb    mm0,    mm7     

        movd        DWORD PTR [edi+1], mm0  // write output 01 02 03 04
        add         edi,    10

        add         esi,    8                   
        paddw       mm2,    mm3             // 

        paddw       mm2,    mm4             // added round values
        cmp         esi,    edx

        psrlw       mm2,    8
        packuswb    mm2,    mm7

        movd        DWORD PTR [edi-4], mm2 // writeoutput 06 07 08 09
        jl         HorizLine_4_5_Loop

//Exit:        
        movq        mm0,    [esi]           // mm0 = 00 01 02 03 04 05 06 07
        movq        mm1,    mm0             // mm1 = 00 01 02 03 04 05 06 07

        movq        mm2,    mm0             // mm2 = 00 01 02 03 04 05 06 07
        psrlq       mm1,    8               // mm1 = 01 02 03 04 05 06 07 00

        movq        mm3,    mask45          // mm3 = 00 00 00 00 00 00 ff 00
        pand        mm3,    mm1             // mm3 = 00 00 00 00 00 00 07 00    
        
        psllq       mm3,    8               // mm3 = 00 00 00 00 00 00 00 07
        por         mm1,    mm3             // mm1 = 01 02 03 04 05 06 07 07
        
        movq        mm3,    mm1             

        movd        DWORD PTR [edi],  mm0   // write output 00 xx xx xx        
        punpcklbw   mm0,    mm7             // mm0 = 00 xx 01 xx 02 xx 03 xx
        
        punpcklbw   mm1,    mm7             // mm1 = 01 xx 02 xx 03 xx 04 xx         
        pmullw      mm0,    mm5             // 00* 51 01*102 02*154 03*205

        pmullw      mm1,    mm6             // 01*205 02*154 03*102 04* 51
        punpckhbw   mm2,    mm7             // mm2 = 04 xx 05 xx 06 xx 07 xx
     
        movd        DWORD PTR [edi+5], mm2  // write ouput 05 xx xx xx
        pmullw      mm2,    mm5             // 04* 51 05*102 06*154 07*205

        punpckhbw   mm3,    mm7             // mm3 = 05 xx 06 xx 07 xx 08 xx
        pmullw      mm3,    mm6             // 05*205 06*154 07*102 07* 51    
        
        paddw       mm0,    mm1             // added round values
        paddw       mm0,    mm4

        psrlw       mm0,    8               // output: 01 xx 02 xx 03 xx 04 xx
        packuswb    mm0,    mm7             // 01 02 03 04 xx xx xx xx

        movd        DWORD PTR [edi+1], mm0  // write output 01 02 03 04
        paddw       mm2,    mm3             // 

        paddw       mm2,    mm4             // added round values        
        psrlw       mm2,    8
        
        packuswb    mm2,    mm7
        movd        DWORD PTR [edi+6], mm2  // writeoutput 06 07 08 09


    }
/*        
    const unsigned char *src = source;
    unsigned char *des = dest;
    unsigned int a, b, c ;
    unsigned i;
	(void) destWidth;
    
    for ( i=0; i<sourceWidth-4; i+=4 )
    {
        a = src[0];
        b = src[1];
        des [0] = (UINT8) a;
        des [1] = (UINT8) (( a * 51 + 205 * b + 128) >> 8);
        c = src[2] * 154;
        a = src[3];
        des [2] = (UINT8) (( b * 102 + c + 128) >> 8);
        des [3] = (UINT8) (( c + 102 * a + 128) >> 8);
        b = src[4];
        des [4] = (UINT8) (( a * 205 + 51 * b + 128) >> 8);

        src += 4;
        des += 5;
    }

    a = src[0];
    b = src[1];
    des [0] = (UINT8) (a);
    des [1] = (UINT8) (( a * 51 + 205 * b + 128) >> 8);
    c = src[2] * 154;
    a = src[3];
    des [2] = (UINT8) (( b * 102 + c + 128) >> 8);
    des [3] = (UINT8) (( c + 102 * a + 128) >> 8);
    des [4] = (UINT8) (a);
*/
}        

/****************************************************************************
 * 
 *  ROUTINE       : VerticalBand_4_5_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 4 to 5 up-scaling of a 4 pixel high band of pixels.
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has a "C" only
 *                  version.
 *
 ****************************************************************************/
void VerticalBand_4_5_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm 
    {
        
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size
        
        lea         edi,    [esi+ecx*2]             // tow lines below
        add         edi,    ecx                     // three lines below

        pxor        mm7,    mm7                     // clear out mm7        
        mov         edx,    destWidth               // Loop counter

VS_4_5_loop:

        movq        mm0,    QWORD ptr [esi]         // src[0];
        movq        mm1,    QWORD ptr [esi+ecx]     // src[1];

        movq        mm2,    mm0                     // Make a copy 
        punpcklbw   mm0,    mm7                     // unpack low to word
    
        movq        mm5,    oneFifth                
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm0,    mm5                     // a * 1/5
        
        movq        mm3,    mm1                     // make a copy 
        punpcklbw   mm1,    mm7                     // unpack low to word

        pmullw      mm2,    mm5                     // a * 1/5
        movq        mm6,    fourFifths               // constan 

        movq        mm4,    mm1                     // copy of low b
        pmullw      mm4,    mm6                     // b * 4/5

        punpckhbw   mm3,    mm7                     // unpack high to word 
        movq        mm5,    mm3                     // copy of high b

        pmullw      mm5,    mm6                     // b * 4/5
        paddw       mm0,    mm4                     // a * 1/5 + b * 4/5

        paddw       mm2,    mm5                     // a * 1/5 + b * 4/5
        paddw       mm0,    roundValues             // + 128

        paddw       mm2,    roundValues             // + 128
        psrlw       mm0,    8

        psrlw       mm2,    8
        packuswb    mm0,    mm2                     // des [1]

        movq        QWORD ptr [esi+ecx], mm0        // write des[1]
        movq        mm0,    [esi+ecx*2]             // mm0 = src[2]

        // mm1, mm3 --- Src[1]
        // mm0 --- Src[2]
        // mm7 for unpacking

        movq        mm5,    twoFifths                 
        movq        mm2,    mm0                     // make a copy 

        pmullw      mm1,    mm5                     // b * 2/5 
        movq        mm6,    threeFifths             


        punpcklbw   mm0,    mm7                     // unpack low to word
        pmullw      mm3,    mm5                     // b * 2/5

        movq        mm4,    mm0                     // make copy of c
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm4,    mm6                     // c * 3/5
        movq        mm5,    mm2                     

        pmullw      mm5,    mm6                     // c * 3/5
        paddw       mm1,    mm4                     // b * 2/5 + c * 3/5
        
        paddw       mm3,    mm5                     // b * 2/5 + c * 3/5
        paddw       mm1,    roundValues             // + 128

        paddw       mm3,    roundValues             // + 128
        psrlw       mm1,    8

        psrlw       mm3,    8
        packuswb    mm1,    mm3                     // des[2]

        movq        QWORD ptr [esi+ecx*2], mm1      // write des[2]
        movq        mm1,    [edi]                   // mm1=Src[3];

        // mm0, mm2 --- Src[2]
        // mm1 --- Src[3]
        // mm6 --- 3/5
        // mm7 for unpacking

        pmullw      mm0,    mm6                     // c * 3/5
        movq        mm5,    twoFifths               // mm5 = 2/5

        movq        mm3,    mm1                     // make a copy
        pmullw      mm2,    mm6                     // c * 3/5 

        punpcklbw   mm1,    mm7                     // unpack low
        movq        mm4,    mm1                     // make a copy 

        punpckhbw   mm3,    mm7                     // unpack high
        pmullw      mm4,    mm5                     // d * 2/5

        movq        mm6,    mm3                     // make a copy
        pmullw      mm6,    mm5                     // d * 2/5
    
        paddw       mm0,    mm4                     // c * 3/5 + d * 2/5
        paddw       mm2,    mm6                     // c * 3/5 + d * 2/5

        paddw       mm0,    roundValues             // + 128
        paddw       mm2,    roundValues             // + 128
                  
        psrlw       mm0,    8
        psrlw       mm2,    8

        packuswb    mm0,    mm2                     // des[3]
        movq        QWORD ptr [edi], mm0            // write des[3]

        //  mm1, mm3 --- Src[3]
        //  mm7 -- cleared for unpacking

        movq        mm0,    [edi+ecx*2]             // mm0, Src[0] of the next group

        movq        mm5,    fourFifths              // mm5 = 4/5
        pmullw      mm1,    mm5                     // d * 4/5

        movq        mm6,    oneFifth                // mm6 = 1/5
        movq        mm2,    mm0                     // make a copy     

        pmullw      mm3,    mm5                     // d * 4/5
        punpcklbw   mm0,    mm7                     // unpack low
        
        pmullw      mm0,    mm6                     // an * 1/5
        punpckhbw   mm2,    mm7                     // unpack high

        paddw       mm1,    mm0                     // d * 4/5 + an * 1/5
        pmullw      mm2,    mm6                     // an * 1/5

        paddw       mm3,    mm2                     // d * 4/5 + an * 1/5
        paddw       mm1,    roundValues             // + 128

        paddw       mm3,    roundValues             // + 128
        psrlw       mm1,    8
        
        psrlw       mm3,    8
        packuswb    mm1,    mm3                     // des[4]

        movq        QWORD ptr [edi+ecx], mm1        // write des[4]

        add         edi,    8
        add         esi,    8

        sub         edx,    8
        jg         VS_4_5_loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : LastVerticalBand_4_5_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : None
 *
 *  FUNCTION      : 4 to 5 up-scaling of the last 4-pixel high band in an image. 
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has an "C" only
 *                  version.
 *
 ****************************************************************************/
void LastVerticalBand_4_5_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm 
    {
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size
        
        lea         edi,    [esi+ecx*2]             // tow lines below
        add         edi,    ecx                     // three lines below

        pxor        mm7,    mm7                     // clear out mm7        
        mov         edx,    destWidth               // Loop counter

LastVS_4_5_loop:

        movq        mm0,    QWORD ptr [esi]         // src[0];
        movq        mm1,    QWORD ptr [esi+ecx]     // src[1];

        movq        mm2,    mm0                     // Make a copy 
        punpcklbw   mm0,    mm7                     // unpack low to word
    
        movq        mm5,    oneFifth                
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm0,    mm5                     // a * 1/5
        
        movq        mm3,    mm1                     // make a copy 
        punpcklbw   mm1,    mm7                     // unpack low to word

        pmullw      mm2,    mm5                     // a * 1/5
        movq        mm6,    fourFifths               // constan 

        movq        mm4,    mm1                     // copy of low b
        pmullw      mm4,    mm6                     // b * 4/5

        punpckhbw   mm3,    mm7                     // unpack high to word 
        movq        mm5,    mm3                     // copy of high b

        pmullw      mm5,    mm6                     // b * 4/5
        paddw       mm0,    mm4                     // a * 1/5 + b * 4/5

        paddw       mm2,    mm5                     // a * 1/5 + b * 4/5
        paddw       mm0,    roundValues             // + 128

        paddw       mm2,    roundValues             // + 128
        psrlw       mm0,    8

        psrlw       mm2,    8
        packuswb    mm0,    mm2                     // des [1]

        movq        QWORD ptr [esi+ecx], mm0        // write des[1]
        movq        mm0,    [esi+ecx*2]             // mm0 = src[2]

        // mm1, mm3 --- Src[1]
        // mm0 --- Src[2]
        // mm7 for unpacking

        movq        mm5,    twoFifths                 
        movq        mm2,    mm0                     // make a copy 

        pmullw      mm1,    mm5                     // b * 2/5 
        movq        mm6,    threeFifths             


        punpcklbw   mm0,    mm7                     // unpack low to word
        pmullw      mm3,    mm5                     // b * 2/5

        movq        mm4,    mm0                     // make copy of c
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm4,    mm6                     // c * 3/5
        movq        mm5,    mm2                     

        pmullw      mm5,    mm6                     // c * 3/5
        paddw       mm1,    mm4                     // b * 2/5 + c * 3/5
        
        paddw       mm3,    mm5                     // b * 2/5 + c * 3/5
        paddw       mm1,    roundValues             // + 128

        paddw       mm3,    roundValues             // + 128
        psrlw       mm1,    8

        psrlw       mm3,    8
        packuswb    mm1,    mm3                     // des[2]

        movq        QWORD ptr [esi+ecx*2], mm1      // write des[2]
        movq        mm1,    [edi]                   // mm1=Src[3];

        movq        QWORD ptr [edi+ecx], mm1        // write des[4];

        // mm0, mm2 --- Src[2]
        // mm1 --- Src[3]
        // mm6 --- 3/5
        // mm7 for unpacking

        pmullw      mm0,    mm6                     // c * 3/5
        movq        mm5,    twoFifths               // mm5 = 2/5

        movq        mm3,    mm1                     // make a copy
        pmullw      mm2,    mm6                     // c * 3/5 

        punpcklbw   mm1,    mm7                     // unpack low
        movq        mm4,    mm1                     // make a copy 

        punpckhbw   mm3,    mm7                     // unpack high
        pmullw      mm4,    mm5                     // d * 2/5

        movq        mm6,    mm3                     // make a copy
        pmullw      mm6,    mm5                     // d * 2/5
    
        paddw       mm0,    mm4                     // c * 3/5 + d * 2/5
        paddw       mm2,    mm6                     // c * 3/5 + d * 2/5

        paddw       mm0,    roundValues             // + 128
        paddw       mm2,    roundValues             // + 128

        psrlw       mm0,    8
        psrlw       mm2,    8

        packuswb    mm0,    mm2                     // des[3]
        movq        QWORD ptr [edi], mm0            // write des[3]

        //  mm1, mm3 --- Src[3]
        //  mm7 -- cleared for unpacking
        add         edi,    8
        add         esi,    8

        sub         edx,    8
        jg          LastVS_4_5_loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : VerticalBand_3_5_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                   
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 3 to 5 up-scaling of a 3-pixel high band of pixels.
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has an "C" only
 *                  version.
 *
 ****************************************************************************/
void VerticalBand_3_5_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm 
    {
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size
        
        lea         edi,    [esi+ecx*2]             // tow lines below
        add         edi,    ecx                     // three lines below

        pxor        mm7,    mm7                     // clear out mm7        
        mov         edx,    destWidth               // Loop counter

VS_3_5_loop:

        movq        mm0,    QWORD ptr [esi]         // src[0];
        movq        mm1,    QWORD ptr [esi+ecx]     // src[1];

        movq        mm2,    mm0                     // Make a copy 
        punpcklbw   mm0,    mm7                     // unpack low to word
    
        movq        mm5,    twoFifths               // mm5 = 2/5                
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm0,    mm5                     // a * 2/5
        
        movq        mm3,    mm1                     // make a copy 
        punpcklbw   mm1,    mm7                     // unpack low to word

        pmullw      mm2,    mm5                     // a * 2/5
        movq        mm6,    threeFifths             // mm6 = 3/5

        movq        mm4,    mm1                     // copy of low b
        pmullw      mm4,    mm6                     // b * 3/5

        punpckhbw   mm3,    mm7                     // unpack high to word 
        movq        mm5,    mm3                     // copy of high b

        pmullw      mm5,    mm6                     // b * 3/5
        paddw       mm0,    mm4                     // a * 2/5 + b * 3/5

        paddw       mm2,    mm5                     // a * 2/5 + b * 3/5
        paddw       mm0,    roundValues             // + 128

        paddw       mm2,    roundValues             // + 128
        psrlw       mm0,    8

        psrlw       mm2,    8
        packuswb    mm0,    mm2                     // des [1]

        movq        QWORD ptr [esi+ecx], mm0        // write des[1]
        movq        mm0,    [esi+ecx*2]             // mm0 = src[2]

        // mm1, mm3 --- Src[1]
        // mm0 --- Src[2]
        // mm7 for unpacking

        movq        mm4,    mm1                     // b low
        pmullw      mm1,    fourFifths              // b * 4/5 low    

        movq        mm5,    mm3                     // b high
        pmullw      mm3,    fourFifths              // b * 4/5 high
        
        movq        mm2,    mm0                     // c
        pmullw      mm4,    oneFifth                // b * 1/5

        punpcklbw   mm0,    mm7                     // c low
        pmullw      mm5,    oneFifth                // b * 1/5

        movq        mm6,    mm0                     // make copy of c low
        punpckhbw   mm2,    mm7                     // c high

        pmullw      mm6,    oneFifth                // c * 1/5 low
        movq        mm7,    mm2                     // make copy of c high

        pmullw      mm7,    oneFifth                // c * 1/5 high
        paddw       mm1,    mm6                     // b * 4/5 + c * 1/5 low

        paddw       mm3,    mm7                     // b * 4/5 + c * 1/5 high
        movq        mm6,    mm0                     // make copy of c low

        pmullw      mm6,    fourFifths              // c * 4/5 low
        movq        mm7,    mm2                     // make copy of c high

        pmullw      mm7,    fourFifths              // c * 4/5 high

        paddw       mm4,    mm6                     // b * 1/5 + c * 4/5 low
        paddw       mm5,    mm7                     // b * 1/5 + c * 4/5 high

        paddw       mm1,    roundValues             // + 128
        paddw       mm3,    roundValues             // + 128

        psrlw       mm1,    8
        psrlw       mm3,    8

        packuswb    mm1,    mm3                     // des[2]
        movq        QWORD ptr [esi+ecx*2], mm1      // write des[2]

        paddw       mm4,    roundValues             // + 128
        paddw       mm5,    roundValues             // + 128

        psrlw       mm4,    8
        psrlw       mm5,    8

        packuswb    mm4,    mm5                     // des[3]
        movq        QWORD ptr [edi], mm4            // write des[3]

        //  mm0, mm2 --- Src[3]
        
        pxor        mm7,    mm7                     // clear mm7 for unpacking
        movq        mm1,    [edi+ecx*2]             // mm1 = Src[0] of the next group

        movq        mm5,    threeFifths             // mm5 = 3/5
        pmullw      mm0,    mm5                     // d * 3/5

        movq        mm6,    twoFifths                // mm6 = 2/5
        movq        mm3,    mm1                     // make a copy     

        pmullw      mm2,    mm5                     // d * 3/5
        punpcklbw   mm1,    mm7                     // unpack low
        
        pmullw      mm1,    mm6                     // an * 2/5
        punpckhbw   mm3,    mm7                     // unpack high

        paddw       mm0,    mm1                     // d * 3/5 + an * 2/5
        pmullw      mm3,    mm6                     // an * 2/5

        paddw       mm2,    mm3                     // d * 3/5 + an * 2/5
        paddw       mm0,    roundValues             // + 128

        paddw       mm2,    roundValues             // + 128
        psrlw       mm0,    8
        
        psrlw       mm2,    8
        packuswb    mm0,    mm2                     // des[4]

        movq        QWORD ptr [edi+ecx], mm0        // write des[4]

        add         edi,    8
        add         esi,    8

        sub         edx,    8
        jg          VS_3_5_loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : LastVerticalBand_3_5_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                   
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 3 to 5 up-scaling of a 3-pixel high band of pixels.
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has an "C" only
 *                  version.
 *
 ****************************************************************************/
void LastVerticalBand_3_5_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm 
    {
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size
        
        lea         edi,    [esi+ecx*2]             // tow lines below
        add         edi,    ecx                     // three lines below

        pxor        mm7,    mm7                     // clear out mm7        
        mov         edx,    destWidth               // Loop counter


LastVS_3_5_loop:

        movq        mm0,    QWORD ptr [esi]         // src[0];
        movq        mm1,    QWORD ptr [esi+ecx]     // src[1];

        movq        mm2,    mm0                     // Make a copy 
        punpcklbw   mm0,    mm7                     // unpack low to word
    
        movq        mm5,    twoFifths               // mm5 = 2/5                
        punpckhbw   mm2,    mm7                     // unpack high to word

        pmullw      mm0,    mm5                     // a * 2/5
        
        movq        mm3,    mm1                     // make a copy 
        punpcklbw   mm1,    mm7                     // unpack low to word

        pmullw      mm2,    mm5                     // a * 2/5
        movq        mm6,    threeFifths             // mm6 = 3/5

        movq        mm4,    mm1                     // copy of low b
        pmullw      mm4,    mm6                     // b * 3/5

        punpckhbw   mm3,    mm7                     // unpack high to word 
        movq        mm5,    mm3                     // copy of high b

        pmullw      mm5,    mm6                     // b * 3/5
        paddw       mm0,    mm4                     // a * 2/5 + b * 3/5

        paddw       mm2,    mm5                     // a * 2/5 + b * 3/5
        paddw       mm0,    roundValues             // + 128

        paddw       mm2,    roundValues             // + 128
        psrlw       mm0,    8

        psrlw       mm2,    8
        packuswb    mm0,    mm2                     // des [1]

        movq        QWORD ptr [esi+ecx], mm0        // write des[1]
        movq        mm0,    [esi+ecx*2]             // mm0 = src[2]

        

        // mm1, mm3 --- Src[1]
        // mm0 --- Src[2]
        // mm7 for unpacking

        movq        mm4,    mm1                     // b low
        pmullw      mm1,    fourFifths              // b * 4/5 low    
        
        movq        QWORD ptr [edi+ecx], mm0        // write des[4]

        movq        mm5,    mm3                     // b high
        pmullw      mm3,    fourFifths              // b * 4/5 high
        
        movq        mm2,    mm0                     // c
        pmullw      mm4,    oneFifth                // b * 1/5

        punpcklbw   mm0,    mm7                     // c low
        pmullw      mm5,    oneFifth                // b * 1/5

        movq        mm6,    mm0                     // make copy of c low
        punpckhbw   mm2,    mm7                     // c high

        pmullw      mm6,    oneFifth                // c * 1/5 low
        movq        mm7,    mm2                     // make copy of c high

        pmullw      mm7,    oneFifth                // c * 1/5 high
        paddw       mm1,    mm6                     // b * 4/5 + c * 1/5 low

        paddw       mm3,    mm7                     // b * 4/5 + c * 1/5 high
        movq        mm6,    mm0                     // make copy of c low

        pmullw      mm6,    fourFifths              // c * 4/5 low
        movq        mm7,    mm2                     // make copy of c high

        pmullw      mm7,    fourFifths              // c * 4/5 high

        paddw       mm4,    mm6                     // b * 1/5 + c * 4/5 low
        paddw       mm5,    mm7                     // b * 1/5 + c * 4/5 high

        paddw       mm1,    roundValues             // + 128
        paddw       mm3,    roundValues             // + 128

        psrlw       mm1,    8
        psrlw       mm3,    8

        packuswb    mm1,    mm3                     // des[2]
        movq        QWORD ptr [esi+ecx*2], mm1      // write des[2]

        paddw       mm4,    roundValues             // + 128
        paddw       mm5,    roundValues             // + 128

        psrlw       mm4,    8
        psrlw       mm5,    8

        packuswb    mm4,    mm5                     // des[3]
        movq        QWORD ptr [edi], mm4            // write des[3]

        //  mm0, mm2 --- Src[3]
        
        add         edi,    8
        add         esi,    8

        sub         edx,    8
        jg          LastVS_3_5_loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : VerticalBand_1_2_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                   
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 1 to 2 up-scaling of a band of pixels.
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has an "C" only
 *                  version.
 *
 ****************************************************************************/
void VerticalBand_1_2_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm
    {
    
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size

        pxor        mm7,    mm7                     // clear out mm7        
        mov         edx,    destWidth               // Loop counter

VS_1_2_loop:

        movq        mm0,    [esi]                   // get Src[0]
        movq        mm1,    [esi + ecx * 2]         // get Src[1]

        movq        mm2,    mm0                     // make copy before unpack
        movq        mm3,    mm1                     // make copy before unpack

        punpcklbw   mm0,    mm7                     // low Src[0]
        movq        mm6,    fourOnes                // mm6= 1, 1, 1, 1

        punpcklbw   mm1,    mm7                     // low Src[1]
        paddw       mm0,    mm1                     // low (a + b)
        
        punpckhbw   mm2,    mm7                     // high Src[0]
        paddw       mm0,    mm6                     // low (a + b + 1)

        punpckhbw   mm3,    mm7                    
        paddw       mm2,    mm3                     // high (a + b )

        psraw       mm0,    1                       // low (a + b +1 )/2
        paddw       mm2,    mm6                     // high (a + b + 1)

        psraw       mm2,    1                       // high (a + b + 1)/2
        packuswb    mm0,    mm2                     // pack results

        movq        [esi+ecx], mm0                  // write out eight bytes
        add         esi,    8

        sub         edx,    8
        jg          VS_1_2_loop
    }
 
}

/****************************************************************************
 * 
 *  ROUTINE       : LastVerticalBand_1_2_Scale_MMX
 *
 *  INPUTS        : unsigned char *dest    :
 *                  unsigned int destPitch :
 *                  unsigned int destWidth :
 *                   
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 1 to 2 up-scaling of band of pixels.
 *
 *  SPECIAL NOTES : The routine uses the first line of the band below 
 *                  the current band. The function also has an "C" only
 *                  version.
 *
 ****************************************************************************/
void LastVerticalBand_1_2_Scale_MMX
(
    unsigned char *dest,
    unsigned int destPitch,
    unsigned int destWidth
)
{
    __asm
    {
        mov         esi,    dest                    // Get the source and destination pointer
        mov         ecx,    destPitch               // Get the pitch size

        mov         edx,    destWidth               // Loop counter

LastVS_1_2_loop:

        movq        mm0,    [esi]                   // get Src[0]
        movq        [esi+ecx], mm0                  // write out eight bytes

        add         esi,    8
        sub         edx,    8

        jg         LastVS_1_2_loop
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : HorizontalLine_1_2_Scale
 *
 *  INPUTS        : const unsigned char *source :
 *                  unsigned int sourceWidth    :
 *                  unsigned char *dest         :
 *                  unsigned int destWidth      :
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 1 to 2 up-scaling of a horizontal line of pixels.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void HorizontalLine_1_2_Scale_MMX 
(
    const unsigned char *source,
    unsigned int sourceWidth,
    unsigned char *dest,
    unsigned int destWidth
)
{
	(void) destWidth;

    __asm
    {
        mov         esi,    source
        mov         edi,    dest

        pxor        mm7,    mm7
        movq        mm6,    fourOnes

        mov         ecx,    sourceWidth

HS_1_2_Loop:

        movq        mm0,    [esi]
        movq        mm1,    [esi+1]

        movq        mm2,    mm0
        movq        mm3,    mm1

        movq        mm4,    mm0
        punpcklbw   mm0,    mm7

        punpcklbw   mm1,    mm7
        paddw       mm0,    mm1

        paddw       mm0,    mm6
        punpckhbw   mm2,    mm7

        punpckhbw   mm3,    mm7
        paddw       mm2,    mm3

        paddw       mm2,    mm6
        psraw       mm0,    1

        psraw       mm2,    1
        packuswb    mm0,    mm2

        movq        mm2,    mm4
        punpcklbw   mm2,    mm0

        movq        [edi],  mm2
        punpckhbw   mm4,    mm0

        movq        [edi+8], mm4
        add         esi,    8

        add         edi,    16
        sub         ecx,    8

        cmp         ecx,    8
        jg          HS_1_2_Loop

// last eight pixel

        movq        mm0,    [esi]
        movq        mm1,    mm0
        
        movq        mm2,    mm0
        movq        mm3,    mm1

        psrlq       mm1,    8
        psrlq       mm3,    56
        
        psllq       mm3,    56
        por         mm1,    mm3

        movq        mm3,    mm1
        movq        mm4,    mm0

        punpcklbw   mm0,    mm7
        punpcklbw   mm1,    mm7

        paddw       mm0,    mm1
        paddw       mm0,    mm6

        punpckhbw   mm2,    mm7
        punpckhbw   mm3,    mm7

        paddw       mm2,    mm3
        paddw       mm2,    mm6

        psraw       mm0,    1
        psraw       mm2,    1

        packuswb    mm0,    mm2
        movq        mm2,    mm4

        punpcklbw   mm2,    mm0
        movq        [edi],  mm2

        punpckhbw   mm4,    mm0
        movq        [edi+8], mm4
    }
}  
    
#if defined(__cplusplus)
extern "C" {
#endif
