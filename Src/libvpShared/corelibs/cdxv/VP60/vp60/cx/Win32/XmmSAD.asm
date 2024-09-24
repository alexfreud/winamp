
.686P
.387
.MODEL  flat, SYSCALL, os_dos
.XMM

; macros

.DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

ALIGN 32


.CODE

NAME XmmGetSAD

PUBLIC XMMGetSAD_
PUBLIC _XMMGetSAD

INCLUDE XmmSAD.ash

;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0


;------------------------------------------------
;INT32 XMMGetSAD( UINT8 * NewDataPtr, INT32 PixelsPerLine,
;                 UINT8 * RefDataPtr, INT32 RefPixelsPerLine,
;                 INT32 ErrorSoFar, INT32 BestSoFar )
;
XMMGetSAD_:
_XMMGetSAD:

push    ecx
push    ebx 
push    edx

push    esi
push    edi
push    ebp

mov     ecx,	(XMMGetSADParams PTR [esp]).PixelsPerLine
mov     eax,	(XMMGetSADParams PTR [esp]).NewDataPtr	
mov     ebx,	(XMMGetSADParams PTR [esp]).RefDataPtr

movq	mm0,	[eax]					; Copy eight bytes to mm0
;
; ESP = Stack Pointer                      MM0 = Free
; ESI = Free                               MM1 = Free
; EDI = Free                               MM2 = Free
; EBP = Free                               MM3 = Free
; EBX = RefDataPtr                         MM4 = Free
; ECX = PixelsPerLine                      MM5 = Free
; EDX = RefPixelsPerLine                   MM6 = Free
; EAX = NewDataPtr                         MM7 = Free


; Row 1
mov         edx, (XMMGetSADParams PTR [esp]).RefPixelsPerLine
lea		esi, [eax+2*ecx];			; Calculate the source ptr for row4
psadbw      mm0, [ebx]

; Row 2
movq		mm1, [eax+ecx]				; Copy eight bytes to mm1
lea		edi, [ebx+2*edx]			; Calculate the source ptr for row4
psadbw      mm1, [ebx+edx]



; Row 3
movq		mm2, [eax+2*ecx]			; Copy eight bytes to mm2
add		esi, ecx;					; Calculate the source ptr for row4
psadbw      mm2, [ebx+2*edx]


add		edi, edx;					; Calculate the source ptr for row4

; Row 4
movq		mm3, [esi]					; Copy eight bytes to mm3
psadbw      mm3, [edi]



; Row 5
movq		mm4, [eax+4*ecx]			; Copy eight bytes to mm4
paddd       mm0,mm1						
psadbw      mm4, [ebx+4*edx]



; Row 6
movq		mm5, [esi+2*ecx]			; Copy eight bytes to mm5
lea		eax, [esi+2*ecx]
psadbw      mm5, [edi+2*edx]


lea		ebx, [edi+2*edx]

; Row 7
movq		mm6, [eax+ecx]					; Copy eight bytes to mm0
psadbw      mm6, [ebx+edx]
paddd       mm2,mm3



; Row 8
movq		mm7, [esi+4*ecx]					; Copy eight bytes to mm0
psadbw      mm7, [edi+4*edx]

; start accumulating differences

mov		eax,	(XMMGetSADParams PTR [esp]).ErrorSoFar

pop		ebp
paddd       mm4,mm5
paddd       mm6,mm7

pop		edi
paddd       mm0,mm2
paddd       mm4,mm6

pop		esi
paddd       mm0,mm4
movd        ecx,mm0

theExit:
pop		edx
add         eax,ecx                     ; add in calculated error

pop     ebx
pop     ecx


ret

;************************************************
END

