;***********************************************************************
;	File:			fdct_m.asm
;
;	Description:
;					This function perform 2-D Forward DCT on a 8x8 block
;					
;
;	Input:			Pointers to input source data buffer and destination 
;					buffer.
;
;	Note:			none
;
;	Special Notes:	We try to do the truncation right to match the result 
;					of the c version. 
;
;************************************************************************
;	Revision History:
;   
;	1.00 YWX 08/05/00  Configuration Baseline 
;

 
        .586
        .387
        .MODEL  flat, SYSCALL, os_dos
        .MMX
;
; macro functions
;
Fdct MACRO ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7
    ; execute stage 1 of forward DCT
    
	
	movq        mm0,ip0             ; mm0 = ip0
    movq        mm1,ip1             ; mm1 = ip1
    movq        mm2,ip3             ; mm2 = ip3
    movq        mm3,ip5             ; mm3 = ip5
    movq        mm4,mm1             ; mm4 = ip1
    movq        mm5,mm3             ; mm5 = ip5
    movq        mm6,mm0             ; mm0 = ip0
    movq        mm7,mm2             ; mm7 = ip3

    paddsw      mm0,ip7             ; mm0 = ip0 + ip7 = is07
    paddsw      mm1,ip2             ; mm1 = ip1 + ip2 = is12
    paddsw      mm2,ip4             ; mm2 = ip3 + ip4 = is34
    paddsw      mm3,ip6             ; mm3 = ip5 + ip6 = is56
    psubsw      mm6,ip7             ; mm6 = ip0 - ip7 = id07
    psubsw      mm7,ip4             ; mm7 = ip3 - ip4 = id34
    psubsw      mm4,ip2             ; mm4 = ip1 - ip2 = id12
    psubsw      mm5,ip6             ; mm5 = ip5 - ip6 = id56

    movq        TID07,mm6           ; save id07
    movq        TID34,mm7           ; save id34

    ; free = mm6, mm7

    movq        mm6,mm4             ; mm6 = id12
    psubsw      mm4,mm5             ; mm4 = id12 - id56 = irot_input_x

	movq        TIRX,mm4            ; save irot_input_x    
    paddsw      mm6,mm5             ; mm6 = id12 + id56
	movq		mm5,mm6				; 

    pmulhw      mm6,xC4S4           ; (xC4S4 * (id12 + id56)) - (id12 + id56) 
	paddw		mm6,mm5				; (xC4S4 * (id12 + id56))
	psrlw		mm5,15				;

	paddw		mm6,mm5;			;
	

    ; free = mm4 ,mm5, mm7

    movq        mm4,mm0             ; mm4 = is07
    psubsw      mm0,mm2             ; mm0 = is07 - is34 = irot_input_y

    movq        TIRY,mm0            ; save irot_input_y

    ; free = mm0, mm5, mm7

    movq        mm0,mm1             ; mm0 = is12
    psubsw      mm1,mm3             ; mm1 = is12 - is56

    movq        TIC2,mm6            ; save icommon_product2
	movq		mm7, mm1

    pmulhw      mm1,xC4S4           ; mm1 = (xC4S4 * (is12 - is56)) - (is12 - is56)
	paddw		mm1, mm7			; mm1 = (xC4S4 * (is12 - is56))	
	psrlw		mm7, 15				;
	
	paddw		mm1, mm7
    movq        TIC1,mm1            ; save icommon_product1

    ; free = mm1, mm5, mm6, mm7

    paddsw      mm4,mm2             ; mm4 = is07 + is34 = is0734
    paddsw      mm0,mm3             ; mm0 = is12 + is56 = is1256
    movq        mm1,mm4             ; mm1 = is07 + is34 = is0734

    paddsw      mm4,mm0             ; mm4 = is0734 + is1256
    psubsw      mm1,mm0             ; mm1 = is0734 - is1256

	movq		mm7,mm4
	movq		mm6,mm1

    pmulhw      mm4,xC4S4           ; mm4 = (xC4S4 * (is0734 + is1256)) - (is0734 + is1256)
    pmulhw      mm1,xC4S4           ; mm1 = (xC4S4 * (is0734 - is1256)) - (is0734 - is1256)
	paddw		mm4,mm7				; mm4 = (xC4S4 * (is0734 + is1256))
	paddw       mm1,mm6				; mm1 = (xC4S4 * (is0734 - is1256))

	psrlw		mm7, 15
	psrlw		mm6, 15

	paddw		mm4, mm7
    movq        ip0,mm4             ; write out ip0

	paddw		mm1, mm6
    movq        ip4,mm1             ; write out ip4

    ; free = mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7

    movq        mm0,TIRY            ; mm0 = irot_input_y
    movq        mm1,TIRX            ; mm1 = irot_input_x

    movq        mm2,mm0             ; mm2 = irot_input_y
	movq		mm3,mm1				; mm3 = irot_input_x

	movq		mm4,mm0				;
	movq		mm5,mm1				;
	
	movq		mm6,xC2S6			;
	movq		mm7,xC6s2			;

    pmulhw      mm0,mm6             ; mm0 = xC2S6*irot_input_y - irot_input_y
    pmulhw      mm3,mm6             ; mm3 = xC2S6*irot_input_x - irot_input_x
	psrlw		mm4, 15
	psrlw		mm5, 15
	paddw		mm0,mm2				; mm0 = xC2S6*irot_input_y
	paddw		mm3,mm1				; mm3 = xC2S6*irot_input_x
	paddw		mm0,mm4
	paddw		mm3,mm5;

	pmulhw      mm1,mm7             ; mm1 = xC6S2*irot_input_x
    pmulhw      mm2,mm7             ; mm2 = xC6S2*irot_input_y 

	paddw		mm1,mm5				;
	paddw		mm2,mm4				;
    
    paddsw      mm0,mm1             ; mm0 = xC2S6(irot_input_y * 2) + xC6S2(irot_input_x * 2) = ip2
    psubsw      mm2,mm3             ; mm2 = xC6S2(irot_input_y * 2) - xC2S6(irot_input_x * 2) = ip6

    movq        ip2,mm0             ; write out ip2
    movq        ip6,mm2             ; write out ip6

    ;

    movq        mm6,TIC1            ; mm6 = icommon_product1
    movq        mm4,TID07           ; mm4 = id07

    movq        mm5,TID34           ; mm5 = id34
    movq        mm7,TIC2            ; mm7 = icommon_product2

    movq        mm1,mm6             ; mm1 = icommon_product1
    movq        mm3,mm7             ; mm3 = icommon_product2

    pxor        mm0,mm0             ; clear mm0
    paddsw      mm7,mm5             ; mm7 = icommon_product2 + id34
    
	paddsw      mm6,mm4             ; mm6 = icommon_product1 + id07 = irot_input_x
    psubsw      mm0,mm7             ; mm0 = -(icommon_product2 + id34) = irot_input_y


    ; free = mm2, mm7, mm4, mm5;

    movq        mm2,mm6             ; mm2 = irot_input_x 
    movq        mm7,mm0             ; mm7 = irot_input_y 

	movq		mm4,mm6;
	movq		mm5,mm0;			

    pmulhw      mm6,xC1S7           ; mm6 = xC1S7*irot_input_x -irot_input_x
	psrlw		mm4,15;

	psrlw		mm5,15;			
   	pmulhw      mm7,xC1S7           ; mm7 = xC1S7*irot_input_y -irot_input_y

	paddw		mm6,mm2				; mm6 = xC1S7*irot_input_x 
	paddw		mm7,mm0				; mm7 = xC1S7*irot_input_y 

    pmulhw      mm0,xC7S1           ; mm0 = xC7S1*irot_input_y 
	paddw		mm6,mm4				;

	paddw		mm7,mm5				;
    pmulhw      mm2,xC7S1           ; mm2 = xC7S1*irot_input_x 

	paddw		mm0,mm5				;
	paddw		mm2,mm4				;

    psubsw      mm6,mm0             ; mm6 = xC1S7*irot_input_x - xC7S1*irot_input_y = ip1
    paddsw      mm2,mm7             ; mm2 = xC7S1*irot_input_x + xC1S7*irot_input_y = ip7

    movq        ip1,mm6             ; write out ip1

    movq        mm4,TID07           ; mm4 = id07
    movq        mm5,TID34           ; mm5 = id34

    movq        ip7,mm2             ; write out ip7


    psubsw      mm4,mm1             ; mm4 = id07 - icommon_product1 = irot_input_x
    psubsw      mm5,mm3             ; mm5 = id34 - icommon_product2 = irot_input_y

    movq        mm6,mm4             ; mm6 = irot_input_x 
	movq		mm0,mm4				; mm0 = irot_input_x 

    movq        mm7,mm5             ; mm7 = irot_input_y 
	movq		mm2,mm5				; mm2 = irot_input_y 

	movq		mm1,xC3S5
	movq		mm3,xC5S3

    pmulhw      mm4,mm1             ; mm4 = xC3S5*irot_input_x - irot_input_x
    pmulhw      mm6,mm3             ; mm6 = xC5S3*irot_input_x - irot_input_x
	pmulhw      mm5,mm3             ; mm5 = xC5S3*irot_input_y - irot_input_y
    pmulhw      mm7,mm1             ; mm7 = xC3S5*irot_input_y - irot_input_y

	paddw		mm4, mm0			; mm4 = xC3S5*irot_input_x
	paddw       mm6, mm0			; mm6 = xC5S3*irot_input_x
	paddw		mm5, mm2			; mm5 = xC5S3*irot_input_y
	paddw		mm7, mm2			; mm7 = xC3S5*irot_input_y

	
	psrlw		mm0, 15				;
	psrlw		mm2, 15				;
	
	paddw		mm4, mm0			;
	paddw		mm6, mm0			;
	paddw		mm5, mm2			;
	paddw		mm7, mm2			;

    psubsw      mm4,mm5             ; mm4 = xC3S4*irot_input_x - xC5S3*irot_input_y  = ip3
    paddsw      mm6,mm7             ; mm6 = xC5S3*irot_input_x + xC3S5*irot_input_y  = ip5

    movq        ip3,mm4             ; write out ip3
    movq        ip5,mm6             ; write out ip5


ENDM

Fdct_new MACRO ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7
    ; execute stage 1 of forward DCT
    
	
	movq        mm0,ip0             ; mm0 = ip0
    movq        mm1,ip1             ; mm1 = ip1
    movq        mm2,ip3             ; mm2 = ip3
    movq        mm3,ip5             ; mm3 = ip5
	movq        mm4,ip0             ; mm0 = ip0
    movq        mm5,ip1             ; mm1 = ip1
    movq        mm6,ip3             ; mm2 = ip3
    movq        mm7,ip5             ; mm3 = ip5


    paddsw      mm0,ip7             ; mm0 = ip0 + ip7 = is07
    paddsw      mm1,ip2             ; mm1 = ip1 + ip2 = is12
    paddsw      mm2,ip4             ; mm2 = ip3 + ip4 = is34
    paddsw      mm3,ip6             ; mm3 = ip5 + ip6 = is56
    psubsw      mm4,ip7             ; mm4 = ip0 - ip7 = id07
    psubsw      mm5,ip2             ; mm5 = ip1 - ip2 = id12

	 psubsw		mm0,mm2				; mm0 = is07 - is34

	 paddsw		mm2,mm2				

    psubsw      mm6,ip4             ; mm6 = ip3 - ip4 = id34
	 
     paddsw		mm2,mm0				; mm2 = is07 + is34 = is0734
	 psubsw		mm1,mm3				; mm1 = is12 - is56
	 movq		TIRY,mm0			; Save is07 - is34 to free mm0;
	 paddsw		mm3,mm3				
     paddsw		mm3,mm1				; mm3 = is12 + 1s56	= is1256

    psubsw      mm7,ip6             ; mm7 = ip5 - ip6 = id56

;--------------------------------------------------------------------
;

	psubsw		mm5,mm7				; mm5 = id12 - id56
	paddsw		mm7,mm7				
	paddsw		mm7,mm5				; mm7 = id12 + id56

									; mm4 = id07
									
									; mm6 = id34	
;---------------------------------------------------------------------
; ip[0], ip[4]
;	mm0			Free
;	mm2			is0734
;	mm3			is1256
	

	psubsw		mm2,mm3				; mm2 = is0734 - is1256
	paddsw		mm3,mm3				

	movq		mm0,mm2				; make a copy 
	paddsw		mm3,mm2				; mm3 = is0734 + is1256

	pmulhw		mm0,xC4S4			; mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 )
	paddw		mm0,mm2				; mm0 = xC4S4 * ( is0734 - is1256 )
	psrlw		mm2,15				;
	paddw		mm0,mm2				; Truncate mm0, now it is op[4]

	movq		mm2,mm3				;
	movq		ip4,mm0				; save ip4, now mm0,mm2 are free

	movq		mm0,mm3				;
	pmulhw		mm3,xC4S4			; mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 )

	psrlw		mm2,15				; 
	paddw		mm3,mm0				; mm3 = xC4S4 * ( is0734 +is1256 )	
	paddw		mm3,mm2				; Truncate mm3, now it is op[0]

	movq		ip0,mm3				;

;----------------------------------------------------------------------
; ip[2], ip[6]
;	mm0			Free
;	mm2			Free
;	mm3			Free
;	mm5			id12 - id56			irot_input_x
;	TIRY		is07 - is34			irot_input_y

	movq		mm3,TIRY			; mm3 = irot_input_y
	pmulhw		mm3,xC2S6			; mm3 = xC2S6 * irot_input_y - irot_input_y

	movq		mm2,TIRY			;
	movq		mm0,mm2				;
	
	psrlw		mm2,15				; mm3 = xC2S6 * irot_input_y
	paddw		mm3,mm0
	
	paddw       mm3,mm2				; Truncated
	movq		mm0, mm5;			;


	movq		mm2, mm5;
	pmulhw		mm0, xC6S2			; mm0 = xC6S2 * irot_input_x

	psrlw		mm2, 15			
	paddw		mm0, mm2			; Truncated

	paddsw		mm3, mm0			; ip[2]
	movq		ip2, mm3			; Save ip2


	movq		mm0, mm5			;
	movq		mm2, mm5			;
	
	pmulhw		mm5, xC2S6			; mm5 = xC2S6 * irot_input_x - irot_input_x
	psrlw		mm2, 15				;

	movq		mm3, TIRY			;
	paddw		mm5, mm0		    ; mm5 = xC2S6 * irot_input_x

	paddw		mm5, mm2			; Truncated
	movq		mm2, mm3			
	
	pmulhw		mm3, xC6S2			; mm3 = xC6S2 * irot_input_y
	psrlw		mm2, 15

	paddw		mm3, mm2			; Truncated
	psubsw		mm3, mm5			;

	movq		ip6, mm3			;



;-----------------------------------------------------------------------
; icommon_product1, icommon_product2
;	mm0			Free	
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm1			is12 - is56
;	mm7			id12 + id56

	movq		mm0, xC4S4
	movq		mm2, mm1
	movq		mm3, mm1

	pmulhw		mm1, mm0			; mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 )
	psrlw		mm2, 15				

	paddw		mm1, mm3			; mm0 = xC4S4 * ( is12 - is56 )
	paddw		mm1, mm2			; Truncate mm1, now it is icommon_product1

	movq		mm2, mm7
	movq		mm3, mm7			

	pmulhw		mm7, mm0			; mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 )
	psrlw		mm2, 15			

	paddw		mm7, mm3			; mm7 = xC4S4 * ( id12 + id56 )
	paddw		mm7, mm2			; Truncate mm7, now it is icommon_product2

;------------------------------------------------------------------------
;	mm0			Free	
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm1			icommon_product1
;	mm7			icommon_product2
;   mm4			id07
;	mm6			id34

	
	pxor		mm0, mm0			; Clear mm0
	psubsw		mm0, mm6			; mm0 = - id34

	psubsw		mm0, mm7			; mm0 = - ( id34 + idcommon_product2 )
	paddsw		mm6, mm6			;
	paddsw		mm6, mm0			; mm6 = id34 - icommon_product2

	psubsw		mm4, mm1			; mm4 = id07 - icommon_product1
	paddsw		mm1, mm1			;
	paddsw		mm1, mm4			; mm1 = id07 + icommon_product1


;-------------------------------------------------------------------------
; ip1, ip7
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm7			Free	
;	mm1			irot_input_x
;	mm0			irot_input_y

	movq		mm7, xC1S7
	movq		mm2, mm1

	movq		mm3, mm1;
	pmulhw		mm1, mm7			; mm1 = xC1S7 * irot_input_x - irot_input_x

	movq		mm7, xC7S1			;
	psrlw		mm2, 15				
	
	paddw		mm1, mm3			; mm1 = xC1S7 * irot_input_x
	paddw		mm1, mm2			; Trucated

	pmulhw		mm3, mm7			; mm3 = xC7S1 * irot_input_x
	paddw		mm3, mm2			; Truncated

	movq		mm5, mm0			
	movq	    mm2, mm0

	movq		mm7, xC1S7			
	pmulhw		mm0, mm7			; mm0 = xC1S7 * irot_input_y - irot_input_y
	
	movq		mm7, xC7S1			
	psrlw		mm2, 15			
	
	paddw		mm0, mm5			; mm0 = xC1S7 * irot_input_y
	paddw		mm0, mm2			; Truncated

	pmulhw		mm5, mm7			; mm5 = xC7S1 * irot_input_y
	paddw		mm5, mm2			; Truncated

	psubsw		mm1, mm5			; mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = ip1
	paddsw		mm3, mm0			; mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = ip7
	
	movq		ip1, mm1
	movq		ip7, mm3
;-----------------------------------------------------------------------------
; ip3, ip5
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm7			Free	
;	mm1			Free
;	mm0			Free
;   mm4         id07 - icommon_product1 = irot_input_x
;   mm6			id34 - icommon_product2 = irot_input_y

	movq		mm0, xC3S5
	movq		mm1, xC5S3

	movq		mm5, mm6
	movq		mm7, mm6

	movq		mm2, mm4
	movq		mm3, mm4

	pmulhw		mm4, mm0			; mm4 = xC3S5 * irot_input_x - irot_input_x
	pmulhw		mm6, mm1			; mm6 = xC5S3 * irot_input_y - irot_input_y

	psrlw		mm2, 15
	psrlw		mm5, 15

	paddw		mm4, mm3			; mm4 = xC3S5 * irot_input_x
	paddw		mm6, mm7			; mm6 = xC5S3 * irot_input_y

	paddw		mm4, mm2			; Truncated
	paddw		mm6, mm5			; Truncated

	psubsw		mm4, mm6			; ip3
	movq		ip3, mm4			;

	movq		mm4, mm3			;
	movq		mm6, mm7			;

	pmulhw		mm3, mm1			; mm3 = xC5S3 * irot_input_x - irot_input_x
	pmulhw		mm7, mm0			; mm7 = xC3S5 * irot_input_y - irot_input_y

	paddw		mm4, mm2
	paddw		mm6, mm5

	paddw		mm3, mm4			; mm3 = xC5S3 * irot_input_x
	paddw		mm7, mm6			; mm7 = xC3S5 * irot_input_y

	paddw		mm3, mm7			; ip5
	movq		ip5, mm3			;

ENDM

Transpose MACRO ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7,
                op0, op1, op2, op3, op4, op5, op6, op7
    movq        mm0,ip0             ; mm0 = a0 a1 a2 a3
    movq        mm4,ip4             ; mm4 = e4 e5 e6 e7
    movq        mm1,ip1             ; mm1 = b0 b1 b2 b3
    movq        mm5,ip5             ; mm5 = f4 f5 f6 f7
    movq        mm2,ip2             ; mm2 = c0 c1 c2 c3
    movq        mm6,ip6             ; mm6 = g4 g5 g6 g7
    movq        mm3,ip3             ; mm3 = d0 d1 d2 d3
    movq        op1,mm1             ; save  b0 b1 b2 b3
    movq        mm7,ip7             ; mm7 = h0 h1 h2 h3

    ; Transpose 2x8 block
    movq		mm1, mm4            ; mm1 = e3 e2 e1 e0      
	 punpcklwd	mm4, mm5            ; mm4 = f1 e1 f0 e0      
	movq		op0, mm0            ; save a3 a2 a1 a0      
	 punpckhwd	mm1, mm5            ; mm1 = f3 e3 f2 e2      
	movq		mm0, mm6            ; mm0 = g3 g2 g1 g0      
	 punpcklwd	mm6, mm7            ; mm6 = h1 g1 h0 g0      
	movq		mm5, mm4            ; mm5 = f1 e1 f0 e0      
	 punpckldq	mm4, mm6            ; mm4 = h0 g0 f0 e0 = MM4 
	punpckhdq	mm5, mm6            ; mm5 = h1 g1 f1 e1 = MM5 
	 movq		mm6, mm1            ; mm6 = f3 e3 f2 e2      
	movq		op4, mm4            ;                           
	 punpckhwd	mm0, mm7            ; mm0 = h3 g3 h2 g2      
	movq		op5, mm5            ;                           
	 punpckhdq	mm6, mm0            ; mm6 = h3 g3 f3 e3 = MM7 
	movq		mm4, op0            ; mm4 = a3 a2 a1 a0      
	 punpckldq	mm1, mm0            ; mm1 = h2 g2 f2 e2 = MM6 
	movq		mm5, op1            ; mm5 = b3 b2 b1 b0      
	 movq		mm0, mm4            ; mm0 = a3 a2 a1 a0      
	movq		op7, mm6            ;                           
	 punpcklwd	mm0, mm5            ; mm0 = b1 a1 b0 a0      
	movq		op6, mm1            ;                           
	 punpckhwd	mm4, mm5            ; mm4 = b3 a3 b2 a2      
	movq		mm5, mm2            ; mm5 = c3 c2 c1 c0      
	 punpcklwd	mm2, mm3            ; mm2 = d1 c1 d0 c0      
	movq		mm1, mm0            ; mm1 = b1 a1 b0 a0      
	 punpckldq	mm0, mm2            ; mm0 = d0 c0 b0 a0 = MM0 
	punpckhdq	mm1, mm2            ; mm1 = d1 c1 b1 a1 = MM1 
	 movq		mm2, mm4            ; mm2 = b3 a3 b2 a2      
	movq		op0, mm0            ;                           
	 punpckhwd	mm5, mm3            ; mm5 = d3 c3 d2 c2      
	movq		op1, mm1            ;                           
	 punpckhdq	mm4, mm5            ; mm4 = d3 c3 b3 a3 = MM3 
	punpckldq	mm2, mm5            ; mm2 = d2 c2 b2 a2 = MM2 
	movq		op3, mm4
	movq		op2, mm2	 
ENDM

;------------------------------------------------
fdctParams  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    InputPtr        dd  ?
    OutputPtr       dd  ?
fdctParams  ENDS
;------------------------------------------------



        .DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

        ALIGN 32

xC1S7  QWORD   0fb15fb15fb15fb15h
xC2S6  QWORD   0ec83ec83ec83ec83h
xC3S5  QWORD   0d4dbd4dbd4dbd4dbh
xC4S4  QWORD   0b505b505b505b505h
xC5S3  QWORD   08e3a8e3a8e3a8e3ah
xC6S2  QWORD   061f861f861f861f8h
xC7S1  QWORD   031f131f131f131f1h
TIRX   QWORD   00000000000000000h
TIRY   QWORD   00000000000000000h
TIC1   QWORD   00000000000000000h
TIC2   QWORD   00000000000000000h
TID07  QWORD   00000000000000000h
TID34  QWORD   00000000000000000h

; data goes here

        .CODE

NAME fdct

PUBLIC fdct_MMX_
PUBLIC _fdct_MMX
 
; includes go here


;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0


;------------------------------------------------
; void fdct_MMX ( INT16 * InputData, INT16 * OutputData )
;
fdct_MMX_:
_fdct_MMX:
    push    esi
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

;
; ESP = Stack Pointer                      MM0 = Free
; ESI = Free                               MM1 = Free
; EDI = Free                               MM2 = Free
; EBP = Free                               MM3 = Free
; EBX = Free                               MM4 = Free
; ECX = Free                               MM5 = Free
; EDX = Free                               MM6 = Free
; EAX = Free                               MM7 = Free
;

    mov         eax,(fdctParams PTR [esp]).InputPtr             ; load pointer to input data
    mov         edx,(fdctParams PTR [esp]).OutputPtr            ; load pointer to output data
    
    ;
    ; Input data is an 8x8 block.  To make processing of the data more efficent
    ; we will transpose the block of data to two 4x8 blocks???
    ;

    Transpose [eax], [eax+16], [eax+32], [eax+48], [eax+8], [eax+24], [eax+40], [eax+56], [edx], [edx+16], [edx+32], [edx+48], [edx+8], [edx+24], [edx+40], [edx+56]
	Fdct_new [edx], [edx+16], [edx+32], [edx+48], [edx+8], [edx+24], [edx+40], [edx+56]	

    Transpose [eax+64], [eax+80], [eax+96], [eax+112], [eax+72], [eax+88], [eax+104], [eax+120], [edx+64], [edx+80], [edx+96], [edx+112], [edx+72], [edx+88], [edx+104], [edx+120]
    Fdct_new [edx+64], [edx+80], [edx+96], [edx+112], [edx+72], [edx+88], [edx+104], [edx+120]

    Transpose [edx+0], [edx+16], [edx+32], [edx+48], [edx+64], [edx+80], [edx+96], [edx+112], [edx+0], [edx+16], [edx+32], [edx+48], [edx+64], [edx+80], [edx+96], [edx+112] 
    Fdct_new [edx+0], [edx+16], [edx+32], [edx+48], [edx+64], [edx+80], [edx+96], [edx+112]

    Transpose [edx+8], [edx+24], [edx+40], [edx+56], [edx+72], [edx+88], [edx+104], [edx+120], [edx+8], [edx+24], [edx+40], [edx+56], [edx+72], [edx+88], [edx+104], [edx+120]
    Fdct_new [edx+8], [edx+24], [edx+40], [edx+56], [edx+72], [edx+88], [edx+104], [edx+120]
    
    
theExit:

    emms

    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi

    ret


NAME FDct1D4Mmx

PUBLIC FDct1D4Mmx_
PUBLIC _FDct1D4Mmx
 
; includes go here


;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0

;------------------------------------------------
; void FDct1D4Mmx ( INT16 * InputData, INT16 * OutputData )
;
FDct1D4Mmx_:
_FDct1D4Mmx:
    push    esi
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

;
; ESP = Stack Pointer                      MM0 = Free
; ESI = Free                               MM1 = Free
; EDI = Free                               MM2 = Free
; EBP = Free                               MM3 = Free
; EBX = Free                               MM4 = Free
; ECX = Free                               MM5 = Free
; EDX = Free                               MM6 = Free
; EAX = Free                               MM7 = Free
;

    mov         eax,(fdctParams PTR [esp]).InputPtr             ; load pointer to input data
    mov         edx,(fdctParams PTR [esp]).OutputPtr            ; load pointer to output data


	movq        mm0,[eax]           ; mm0 = ip0
    movq        mm1,[eax + 8]       ; mm1 = ip1
    movq        mm2,[eax + 24]      ; mm2 = ip3
    movq        mm3,[eax + 40]      ; mm3 = ip5
	movq        mm4,[eax]           ; mm0 = ip0
    movq        mm5,[eax + 8]       ; mm1 = ip1
    movq        mm6,[eax + 24]      ; mm2 = ip3
    movq        mm7,[eax + 40]      ; mm3 = ip5


    paddsw      mm0,[eax + 56]      ; mm0 = ip0 + ip7 = is07
    paddsw      mm1,[eax + 16]      ; mm1 = ip1 + ip2 = is12
    paddsw      mm2,[eax + 32]      ; mm2 = ip3 + ip4 = is34
    paddsw      mm3,[eax + 48]      ; mm3 = ip5 + ip6 = is56
    psubsw      mm4,[eax + 56]      ; mm4 = ip0 - ip7 = id07
    psubsw      mm5,[eax + 16]      ; mm5 = ip1 - ip2 = id12

	 psubsw		mm0,mm2				; mm0 = is07 - is34

	 paddsw		mm2,mm2				

     psubsw      mm6,[eax + 32]     ; mm6 = ip3 - ip4 = id34
	 
     paddsw		mm2,mm0				; mm2 = is07 + is34 = is0734
	 psubsw		mm1,mm3				; mm1 = is12 - is56
	 movq		TIRY,mm0			; Save is07 - is34 to free mm0;
	 paddsw		mm3,mm3				
     paddsw		mm3,mm1				; mm3 = is12 + 1s56	= is1256

	 psubsw      mm7,[eax + 48]     ; mm7 = ip5 - ip6 = id56

;--------------------------------------------------------------------
;

	psubsw		mm5,mm7				; mm5 = id12 - id56
	paddsw		mm7,mm7				
	paddsw		mm7,mm5				; mm7 = id12 + id56

									; mm4 = id07
									
									; mm6 = id34	
;---------------------------------------------------------------------
; ip[0], ip[4]
;	mm0			Free
;	mm2			is0734
;	mm3			is1256
	

	psubsw		mm2,mm3				; mm2 = is0734 - is1256
	paddsw		mm3,mm3				

	movq		mm0,mm2				; make a copy 
	paddsw		mm3,mm2				; mm3 = is0734 + is1256

	pmulhw		mm0,xC4S4			; mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 )
	paddw		mm0,mm2				; mm0 = xC4S4 * ( is0734 - is1256 )
	psrlw		mm2,15				;
	paddw		mm0,mm2				; Truncate mm0, now it is op[4]

	movq		mm2,mm3				;
	movq		[edx + 32],mm0		; save op4, now mm0,mm2 are free

	movq		mm0,mm3				;
	pmulhw		mm3,xC4S4			; mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 )

	psrlw		mm2,15				; 
	paddw		mm3,mm0				; mm3 = xC4S4 * ( is0734 +is1256 )	
	paddw		mm3,mm2				; Truncate mm3, now it is op[0]

	movq		[edx],mm3				;

;----------------------------------------------------------------------
; ip[2], ip[6]
;	mm0			Free
;	mm2			Free
;	mm3			Free
;	mm5			id12 - id56			irot_input_x
;	TIRY		is07 - is34			irot_input_y

	movq		mm3,TIRY			; mm3 = irot_input_y
	pmulhw		mm3,xC2S6			; mm3 = xC2S6 * irot_input_y - irot_input_y

	movq		mm2,TIRY			;
	movq		mm0,mm2				;
	
	psrlw		mm2,15				; mm3 = xC2S6 * irot_input_y
	paddw		mm3,mm0
	
	paddw       mm3,mm2				; Truncated
	movq		mm0, mm5;			;


	movq		mm2, mm5;
	pmulhw		mm0, xC6S2			; mm0 = xC6S2 * irot_input_x

	psrlw		mm2, 15			
	paddw		mm0, mm2			; Truncated

	paddsw		mm3, mm0			; ip[2]
	movq		[edx + 16], mm3			; Save ip2


	movq		mm0, mm5			;
	movq		mm2, mm5			;
	
	pmulhw		mm5, xC2S6			; mm5 = xC2S6 * irot_input_x - irot_input_x
	psrlw		mm2, 15				;

	movq		mm3, TIRY			;
	paddw		mm5, mm0		    ; mm5 = xC2S6 * irot_input_x

	paddw		mm5, mm2			; Truncated
	movq		mm2, mm3			
	
	pmulhw		mm3, xC6S2			; mm3 = xC6S2 * irot_input_y
	psrlw		mm2, 15

	paddw		mm3, mm2			; Truncated
	psubsw		mm3, mm5			;

	movq		[edx + 48], mm3			;



;-----------------------------------------------------------------------
; icommon_product1, icommon_product2
;	mm0			Free	
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm1			is12 - is56
;	mm7			id12 + id56

	movq		mm0, xC4S4
	movq		mm2, mm1
	movq		mm3, mm1

	pmulhw		mm1, mm0			; mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 )
	psrlw		mm2, 15				

	paddw		mm1, mm3			; mm0 = xC4S4 * ( is12 - is56 )
	paddw		mm1, mm2			; Truncate mm1, now it is icommon_product1

	movq		mm2, mm7
	movq		mm3, mm7			

	pmulhw		mm7, mm0			; mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 )
	psrlw		mm2, 15			

	paddw		mm7, mm3			; mm7 = xC4S4 * ( id12 + id56 )
	paddw		mm7, mm2			; Truncate mm7, now it is icommon_product2

;------------------------------------------------------------------------
;	mm0			Free	
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm1			icommon_product1
;	mm7			icommon_product2
;   mm4			id07
;	mm6			id34

	
	pxor		mm0, mm0			; Clear mm0
	psubsw		mm0, mm6			; mm0 = - id34

	psubsw		mm0, mm7			; mm0 = - ( id34 + idcommon_product2 )
	paddsw		mm6, mm6			;
	paddsw		mm6, mm0			; mm6 = id34 - icommon_product2

	psubsw		mm4, mm1			; mm4 = id07 - icommon_product1
	paddsw		mm1, mm1			;
	paddsw		mm1, mm4			; mm1 = id07 + icommon_product1


;-------------------------------------------------------------------------
; ip1, ip7
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm7			Free	
;	mm1			irot_input_x
;	mm0			irot_input_y

	movq		mm7, xC1S7
	movq		mm2, mm1

	movq		mm3, mm1;
	pmulhw		mm1, mm7			; mm1 = xC1S7 * irot_input_x - irot_input_x

	movq		mm7, xC7S1			;
	psrlw		mm2, 15				
	
	paddw		mm1, mm3			; mm1 = xC1S7 * irot_input_x
	paddw		mm1, mm2			; Trucated

	pmulhw		mm3, mm7			; mm3 = xC7S1 * irot_input_x
	paddw		mm3, mm2			; Truncated

	movq		mm5, mm0			
	movq	    mm2, mm0

	movq		mm7, xC1S7			
	pmulhw		mm0, mm7			; mm0 = xC1S7 * irot_input_y - irot_input_y
	
	movq		mm7, xC7S1			
	psrlw		mm2, 15			
	
	paddw		mm0, mm5			; mm0 = xC1S7 * irot_input_y
	paddw		mm0, mm2			; Truncated

	pmulhw		mm5, mm7			; mm5 = xC7S1 * irot_input_y
	paddw		mm5, mm2			; Truncated

	psubsw		mm1, mm5			; mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = ip1
	paddsw		mm3, mm0			; mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = ip7
	
	movq		[edx + 8], mm1
	movq		[edx + 56], mm3
;-----------------------------------------------------------------------------
; ip3, ip5
;	mm2			Free
;	mm3			Free
;	mm5			Free
;	mm7			Free	
;	mm1			Free
;	mm0			Free
;   mm4         id07 - icommon_product1 = irot_input_x
;   mm6			id34 - icommon_product2 = irot_input_y

	movq		mm0, xC3S5
	movq		mm1, xC5S3

	movq		mm5, mm6
	movq		mm7, mm6

	movq		mm2, mm4
	movq		mm3, mm4

	pmulhw		mm4, mm0			; mm4 = xC3S5 * irot_input_x - irot_input_x
	pmulhw		mm6, mm1			; mm6 = xC5S3 * irot_input_y - irot_input_y

	psrlw		mm2, 15
	psrlw		mm5, 15

	paddw		mm4, mm3			; mm4 = xC3S5 * irot_input_x
	paddw		mm6, mm7			; mm6 = xC5S3 * irot_input_y

	paddw		mm4, mm2			; Truncated
	paddw		mm6, mm5			; Truncated

	psubsw		mm4, mm6			; ip3
	movq		[edx + 24], mm4			;

	movq		mm4, mm3			;
	movq		mm6, mm7			;

	pmulhw		mm3, mm1			; mm3 = xC5S3 * irot_input_x - irot_input_x
	pmulhw		mm7, mm0			; mm7 = xC3S5 * irot_input_y - irot_input_y

	paddw		mm4, mm2
	paddw		mm6, mm5

	paddw		mm3, mm4			; mm3 = xC5S3 * irot_input_x
	paddw		mm7, mm6			; mm7 = xC3S5 * irot_input_y

	paddw		mm3, mm7			; ip5
	movq		[edx + 40], mm3			;

    
    emms

    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi

    ret


;************************************************
        END

