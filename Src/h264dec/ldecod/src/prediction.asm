.686
.XMM
.model FLAT

copy_image_data_16x16_stride@OptimizedFunctions = 32
dec_picture@VideoParameters = 698192
p_Slice@MacroBlock = 0
plane_images@StorablePicture = 158512
mb_rec@Slice = 1696
mb_pred@Slice = 928
cof@Slice = 2464

CONST SEGMENT
align 16
const32	DW 020H, 020H, 020H, 020H, 020H, 020H, 020H, 020H
CONST ENDS

;
;
;
;

PUBLIC _weighted_bi_prediction4x4
_TEXT	SEGMENT
mb_pred = 4						
block_l0 = 8						
wp_scale_l0 = 12					
wp_scale_l1 = 16					
wp_offset = 20
weight_denom = 24
_weighted_bi_prediction4x4 PROC				; COMDAT
  mov eax, DWORD PTR weight_denom[esp]
  pxor mm0, mm0
  pshufw mm1, MMWORD PTR wp_scale_l0[esp], 0
  test eax, eax
  pshufw mm2, MMWORD PTR wp_scale_l1[esp], 0
  pshufw mm3, MMWORD PTR wp_offset[esp], 0
  jle	BI_PRED4x4@LEFT_SHIFT

  movd mm4, eax	
  lea	ecx, DWORD PTR [eax-1] ; 
  mov	edx, 1
  shl	edx, cl
  movd mm5, edx
  mov eax, mb_pred[esp]
  mov edx, block_l0[esp]
  pshufw mm5, mm5, 0
  movd mm6, DWORD PTR 0[edx] ; block_l0
  movd mm7, DWORD PTR 0[eax] ; mb_pred
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 16[eax] ; mb_pred
  paddw mm6, mm5
  psraw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 0[eax], mm6

  movd mm6, DWORD PTR 16[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 32[eax] ; mb_pred
  paddw mm6, mm5
  psraw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 16[eax], mm6

  movd mm6, DWORD PTR 32[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 48[eax] ; mb_pred
  paddw mm6, mm5
  psraw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 0[eax], mm6

  movd mm6, DWORD PTR 48[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  paddw mm6, mm5
  psraw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 48[eax], mm6
  ret 0	

BI_PRED4x4@LEFT_SHIFT:
  neg eax
  movd mm4, eax	
  mov eax, mb_pred[esp]
  mov edx, block_l0[esp]
  movd mm6, DWORD PTR 0[edx] ; block_l0
  movd mm7, DWORD PTR 0[eax] ; mb_pred
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 16[eax] ; mb_pred
  psllw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 0[eax], mm6

  movd mm6, DWORD PTR 16[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 32[eax] ; mb_pred
  psllw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 16[eax], mm6

  movd mm6, DWORD PTR 32[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  movd mm7, DWORD PTR 48[eax] ; mb_pred
  psllw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 0[eax], mm6

  movd mm6, DWORD PTR 48[edx] ; block_l0
  punpcklbw mm6, mm0
  punpcklbw mm7, mm0
  pmullw	mm6, mm1
  pmullw	mm7, mm2
  paddw mm6, mm7
  psllw	mm6, mm4
  paddw mm6, mm3
  packuswb mm6, mm6
  movd DWORD PTR 48[eax], mm6
  ret 0	
_weighted_bi_prediction4x4 ENDP
_TEXT	ENDS

PUBLIC	_itrans4x4_mmx
_TEXT	SEGMENT
_tblock$ = 4						; size = 4
_mb_pred$ = 8						; size = 4
_mb_rec$ = 12						; size = 4
_pos_x$ = 16						; size = 4
_pos_y$ = 20						; size = 4
_itrans4x4_mmx PROC					; COMDAT

	mov	edx, DWORD PTR _pos_y$[esp]
	shl	edx, 4
	add	edx, DWORD PTR _pos_x$[esp]
	mov eax, DWORD PTR _tblock$[esp]
	mov	ecx, DWORD PTR _mb_pred$[esp]
	add	ecx, edx
	add edx, DWORD PTR _mb_rec$[esp]
_itrans4x4_mmx_direct PROC					; COMDAT
			; load 4x4 matrix
			movq mm0, MMWORD PTR 0[eax]
			movq mm1, MMWORD PTR 8[eax]
			movq mm2, MMWORD PTR 16[eax]
			movq mm3, MMWORD PTR 24[eax]

			; rotate 4x4 matrix
			movq mm4, mm0 ; p0 = mm4 (copy)
			punpcklwd mm0, mm2 ; r0 = mm0
			punpckhwd mm4, mm2 ; r2 = mm4
			movq mm5, mm1 ; p1 = mm5 (copy)
			punpcklwd mm1, mm3 ; r1 = mm1
			punpckhwd mm5, mm3 ; r3 = mm5
			movq mm6, mm0 ; r0 = mm6 (copy)
			punpcklwd mm0, mm1 ; t0 = mm0
			punpckhwd mm6, mm1 ; t1 = mm6
			movq mm1, mm4 ; r2 = mm1 (copy)
			punpcklwd mm1, mm5 ; t2 = mm1
			punpckhwd mm4, mm5 ; t3 = mm4

			movq mm2, mm0 ; mm2 = t0 (copy)
			paddw mm0, mm1 ; mm0 = p0
			psubw mm2, mm1 ; mm2 = p1, mm1 available
			movq mm5, mm6 ; mm5 = t1 (copy)
			psraw mm5, 1 ; mm5 = (t1 >> 1)
			psubw mm5, mm4 ; mm5 = p2
			psraw mm4, 1 ; mm4 = (t3 >> 1)
			paddw mm6, mm4 ; mm6 = p3

			movq mm3, mm0 ; mm3 = p0 (copy)
			paddw mm0, mm6 ; mm0 = r0
			movq mm1, mm2 ; mm1 = p1 (copy)
			paddw mm1, mm5 ; mm1 = r1
			psubw mm2, mm5 ; mm2 = r2, mm5 available
			psubw mm3, mm6 ; mm3 = r3

			; rotate 4x4 matrix to set up for vertical
			movq mm4, mm0 ; r0 = mm4 (copy)
			punpcklwd mm0, mm2 ; p0 = mm0
			punpckhwd mm4, mm2 ; p2 = mm4
			movq mm5, mm1 ; r1 = mm5 (copy)
			punpcklwd mm1, mm3 ; p1 = mm1
			punpckhwd mm5, mm3 ; p3 = mm5
			movq mm6, mm0 ; p0 = mm6 (copy)
			punpcklwd mm0, mm1 ; t0 = mm0
			punpckhwd mm6, mm1 ; t1 = mm6
			movq mm1, mm4 ; p2 = mm1 (copy)
			punpcklwd mm1, mm5 ; t2 = mm1
			punpckhwd mm4, mm5 ; t3 = mm4

			movq mm2, mm0 ; mm2 = t0 (copy)
			paddw mm0, mm1 ; mm0 = p0
			psubw mm2, mm1 ; mm2 = p1, mm1 available
			movq mm5, mm6 ; mm5 = t1 (copy)
			psraw mm5, 1 ; mm5 = (t1 >> 1)
			psubw mm5, mm4 ; mm5 = p2
			psraw mm4, 1 ; mm4 = (t3 >> 1)
			paddw mm6, mm4 ; mm6 = p3
			movq mm3, mm0 ; mm3 = p0 (copy)
			paddw mm0, mm6 ; mm0 = r0
			movq mm1, mm2 ; mm1 = p1 (copy)
			paddw mm1, mm5 ; mm1 = r1
			psubw mm2, mm5 ; mm2 = r2, mm5 available
			psubw mm3, mm6 ; mm3 = r3


; --- 4x4 iDCT done, now time to combine with mpr --- 

			movq	mm7, MMWORD PTR const32

			paddw mm0, mm7 ; rres + 32
			psraw mm0, 6 ; (rres + 32) >> 6
			paddw mm1, mm7 ; rres + 32
			psraw mm1, 6 ; (rres + 32) >> 6
			paddw mm2, mm7 ; rres + 32
			psraw mm2, 6 ; (rres + 32) >> 6
			paddw mm3, mm7 ; rres + 32
			psraw mm3, 6 ; (rres + 32) >> 6

			pxor mm7, mm7

			; convert mpr from unsigned char to short
			movd mm4, DWORD PTR 0[ecx]
			movd mm5, DWORD PTR 16[ecx]
			movd mm6, DWORD PTR 32[ecx]
			punpcklbw mm4, mm7
			punpcklbw mm5, mm7
			punpcklbw mm6, mm7
			paddsw mm4, mm0 ; pred_row + rres_row
			movd mm0, DWORD PTR 48[ecx] ; reuse mm0 for mpr[3]
			paddsw mm5, mm1 ; pred_row + rres_row
			punpcklbw mm0, mm7
			paddsw mm6, mm2 ; pred_row + rres_row			
			paddsw mm0, mm3 ; pred_row + rres_row
			; results in mm4, mm5, mm6, mm0
			
			; move back to 8 bit
			packuswb mm4, mm7
			packuswb mm5, mm7
			packuswb mm6, mm7
			packuswb mm0, mm7
			movd DWORD PTR 0[edx], mm4
			movd DWORD PTR 16[edx], mm5
			movd DWORD PTR 32[edx], mm6
			movd DWORD PTR 48[edx], mm0
	ret	0

_itrans4x4_mmx_direct ENDP
_itrans4x4_mmx ENDP
_TEXT	ENDS

EXTRN	_itrans_sp:PROC
EXTRN	_Inv_Residual_trans_4x4:PROC
PUBLIC	_iMBtrans4x4
EXTRN	_opt:BYTE
_TEXT	SEGMENT
_currSlice$ = -4					; size = 4
_mb_rec$166704 = 8					; size = 4
_currMB$ = 8						; size = 4
_curr_img$ = 12						; size = 4
_pl$ = 8 ; second parameter
_smb$ = 16						; size = 4
_iMBtrans4x4 PROC
	push	ecx
	push	ebx
	push	ebp
	push	esi
STACKOFFSET = 16
; 408  :   VideoImage *curr_img = pl ? dec_picture->imgUV[pl - 1]: dec_picture->imgY;

	mov	esi, DWORD PTR _pl$[esp+STACKOFFSET]
	push	edi
STACKOFFSET = STACKOFFSET + 4
	mov	edi, DWORD PTR _currMB$[esp+16]
	mov	ebp, DWORD PTR [edi+p_Slice@MacroBlock] ; ebp: currMB->p_Slice
	mov	eax, DWORD PTR [edi+4]
	mov	eax, DWORD PTR [eax+dec_picture@VideoParameters] ; eax: p_Vid->dec_picture;
	mov	DWORD PTR _currSlice$[esp+20], ebp
	mov	ecx, DWORD PTR [eax+esi*4+plane_images@StorablePicture]
	mov	DWORD PTR _curr_img$[esp+16], ecx
	
	cmp	DWORD PTR _smb$[esp+16], 0 ; if (smb)
; 413  : 	{
; 414  : 		h264_short_block_t *blocks = currSlice->cof4[pl];
; 415  : 		const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];
; 416  : 
; 417  : 		itrans_sp(blocks[0], mb_pred, currMB, pl, 0, 0);

	je	$LN4@iMBtrans4x
	push	0
	push	0
	mov	eax, esi
	shl	eax, 9
	lea	ebx, DWORD PTR [eax+ebp+cof@Slice]
	mov	ecx, esi
	shl	ecx, 8
	lea	ebp, DWORD PTR [ecx+ebp+mb_pred@Slice]
	push	esi
	push	ebp
	push	ebx
	mov	eax, edi
	call	_itrans_sp

; 418  : 		itrans_sp(blocks[1], mb_pred, currMB, pl, 4, 0);

	push	0
	push	4
	push	esi
	lea	edx, DWORD PTR [ebx+32]
	push	ebp
	push	edx
	mov	eax, edi
	call	_itrans_sp

; 419  : 		itrans_sp(blocks[2], mb_pred, currMB, pl, 0, 4);

	push	4
	push	0
	push	esi
	lea	eax, DWORD PTR [ebx+64]
	push	ebp
	push	eax
	mov	eax, edi
	call	_itrans_sp

; 420  : 		itrans_sp(blocks[3], mb_pred, currMB, pl, 4, 4);

	push	4
	push	4
	push	esi
	lea	ecx, DWORD PTR [ebx+96]
	push	ebp
	push	ecx
	mov	eax, edi
	call	_itrans_sp
	add	esp, 80					; 00000050H

; 421  : 		itrans_sp(blocks[4], mb_pred, currMB, pl, 8, 0);

	push	0
	push	8
	push	esi
	lea	edx, DWORD PTR [ebx+128]
	push	ebp
	push	edx
	mov	eax, edi
	call	_itrans_sp

; 422  : 		itrans_sp(blocks[5], mb_pred, currMB, pl, 12, 0);

	push	0
	push	12					; 0000000cH
	push	esi
	lea	eax, DWORD PTR [ebx+160]
	push	ebp
	push	eax
	mov	eax, edi
	call	_itrans_sp

; 423  : 		itrans_sp(blocks[6], mb_pred, currMB, pl, 8, 4);

	push	4
	push	8
	push	esi
	lea	ecx, DWORD PTR [ebx+192]
	push	ebp
	push	ecx
	mov	eax, edi
	call	_itrans_sp

; 424  : 		itrans_sp(blocks[7], mb_pred, currMB, pl, 12, 4);

	push	4
	push	12					; 0000000cH
	push	esi
	lea	edx, DWORD PTR [ebx+224]
	push	ebp
	push	edx
	mov	eax, edi
	call	_itrans_sp
	add	esp, 80					; 00000050H

; 425  : 		itrans_sp(blocks[8], mb_pred, currMB, pl, 0, 8);

	push	8
	push	0
	push	esi
	lea	eax, DWORD PTR [ebx+256]
	push	ebp
	push	eax
	mov	eax, edi
	call	_itrans_sp

; 426  : 		itrans_sp(blocks[9], mb_pred, currMB, pl, 4, 8);

	push	8
	push	4
	push	esi
	push	ebp
	lea	ecx, DWORD PTR [ebx+288]
	push	ecx
	mov	eax, edi
	call	_itrans_sp

; 427  : 		itrans_sp(blocks[10], mb_pred, currMB, pl, 0, 12);

	push	12					; 0000000cH
	push	0
	push	esi
	lea	edx, DWORD PTR [ebx+320]
	push	ebp
	push	edx
	mov	eax, edi
	call	_itrans_sp

; 428  : 		itrans_sp(blocks[11], mb_pred, currMB, pl, 4, 12);

	push	12					; 0000000cH
	push	4
	push	esi
	lea	eax, DWORD PTR [ebx+352]
	push	ebp
	push	eax
	mov	eax, edi
	call	_itrans_sp
	add	esp, 80					; 00000050H

; 429  : 		itrans_sp(blocks[12], mb_pred, currMB, pl, 8, 8);

	push	8
	push	8
	push	esi
	lea	ecx, DWORD PTR [ebx+384]
	push	ebp
	push	ecx
	mov	eax, edi
	call	_itrans_sp

; 430  : 		itrans_sp(blocks[13], mb_pred, currMB, pl, 12, 8);

	push	8
	push	12					; 0000000cH
	push	esi
	lea	edx, DWORD PTR [ebx+416]
	push	ebp
	push	edx
	mov	eax, edi
	call	_itrans_sp

; 431  : 		itrans_sp(blocks[14], mb_pred, currMB, pl, 8, 12);

	push	12					; 0000000cH
	push	8
	push	esi
	lea	eax, DWORD PTR [ebx+448]
	push	ebp
	push	eax
	mov	eax, edi
	call	_itrans_sp

; 432  : 		itrans_sp(blocks[15], mb_pred, currMB, pl, 12, 12);

	push	12					; 0000000cH
	push	12					; 0000000cH
	push	esi
	add	ebx, 480				; 000001e0H
	push	ebp
	push	ebx
	mov	eax, edi
	call	_itrans_sp
		mov	ebp, DWORD PTR _currSlice$[esp+100]
	add	esp, 80					; 00000050H
	jmp	COPY_16x16
	
$LN4@iMBtrans4x:

; 433  : 	}
; 434  : 	else if (currMB->is_lossless)

	cmp	DWORD PTR [edi+84], 0
	je	$LN2@iMBtrans4x

	push	0
	push	0

; 435  : 	{
; 436  : 		Inv_Residual_trans_4x4(currMB, pl, 0, 0);

	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 437  : 		Inv_Residual_trans_4x4(currMB, pl, 4, 0);

	push	0
	push	4
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 438  : 		Inv_Residual_trans_4x4(currMB, pl, 0, 4);

	push	4
	push	0
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 439  : 		Inv_Residual_trans_4x4(currMB, pl, 4, 4);

	push	4
	push	4
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4
	add	esp, 64					; 00000040H

; 440  : 		Inv_Residual_trans_4x4(currMB, pl, 8, 0);

	push	0
	push	8
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 441  : 		Inv_Residual_trans_4x4(currMB, pl, 12, 0);

	push	0
	push	12					; 0000000cH
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 442  : 		Inv_Residual_trans_4x4(currMB, pl, 8, 4);

	push	4
	push	8
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 443  : 		Inv_Residual_trans_4x4(currMB, pl, 12, 4);

	push	4
	push	12					; 0000000cH
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4
	add	esp, 64					; 00000040H

; 444  : 		Inv_Residual_trans_4x4(currMB, pl, 0, 8);

	push	8
	push	0
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 445  : 		Inv_Residual_trans_4x4(currMB, pl, 4, 8);

	push	8
	push	4
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 446  : 		Inv_Residual_trans_4x4(currMB, pl, 0, 12);

	push	12					; 0000000cH
	push	0
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 447  : 		Inv_Residual_trans_4x4(currMB, pl, 4, 12);

	push	12					; 0000000cH
	push	4
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4
	add	esp, 64					; 00000040H

; 448  : 		Inv_Residual_trans_4x4(currMB, pl, 8, 8);

	push	8
	push	8
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 449  : 		Inv_Residual_trans_4x4(currMB, pl, 12, 8);

	push	8
	push	12					; 0000000cH
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 450  : 		Inv_Residual_trans_4x4(currMB, pl, 8, 12);

	push	12					; 0000000cH
	push	8
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4

; 451  : 		Inv_Residual_trans_4x4(currMB, pl, 12, 12);

	push	12					; 0000000cH
	push	12					; 0000000cH
	push	esi
	push	edi
	call	_Inv_Residual_trans_4x4
	add	esp, 64					; 00000040H

; 452  : 	}
; 453  : 	else

	jmp	COPY_16x16
$LN2@iMBtrans4x:

; 454  : 	{
; 455  : 			const h264_short_block_t *blocks = currSlice->cof4[pl];
; 456  : 			const h264_imgpel_macroblock_row_t *mb_pred=currSlice->mb_pred[pl];

	mov	edx, esi
	mov	ecx, esi
	shl	edx, 8
	shl	ecx, 9
	lea	eax, DWORD PTR [edx+ebp]
	lea	ebx, DWORD PTR [ecx+ebp+cof@Slice]

; 457  : 			h264_imgpel_macroblock_row_t *mb_rec = currSlice->mb_rec[pl];
	
	; put things in registers that itrans4x4_mmx_direct wants
	lea edx, [eax + mb_rec@Slice]; mb_rec
	lea ecx, [eax + mb_pred@Slice] ; mb_pred
	mov eax, ebx ; blocks
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[0], mb_pred, mb_rec, 0, 0);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+32]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[1], mb_pred, mb_rec, 4, 0);

	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+128]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[4], mb_pred, mb_rec, 8, 0);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+160]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[5], mb_pred, mb_rec, 12, 0);

	; second row
	lea edx, [edx+52]
	lea ecx, [ecx+52]
	lea eax, [ebx+64]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[2], mb_pred, mb_rec, 0, 4);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+96]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[3], mb_pred, mb_rec, 4, 4);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+192]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[6], mb_pred, mb_rec, 8, 4);

	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+224]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[7], mb_pred, mb_rec, 12, 4);

	; third row
	lea edx, [edx+52]
	lea ecx, [ecx+52]
	lea eax, [ebx+256]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[8], mb_pred, mb_rec, 0, 8);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+288]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[9], mb_pred, mb_rec, 4, 8);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+384]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[12], mb_pred, mb_rec, 8, 8);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+416]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[13], mb_pred, mb_rec, 12, 8);
	
	; fourth row
	lea edx, [edx+52]
	lea ecx, [ecx+52]
	lea eax, [ebx+320]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[10], mb_pred, mb_rec, 0, 12);

	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+352]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[11], mb_pred, mb_rec, 4, 12);
	
	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+448]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[14], mb_pred, mb_rec, 8, 12);

	lea edx, [edx+4]
	lea ecx, [ecx+4]
	lea eax, [ebx+480]
	call	_itrans4x4_mmx_direct ; opt_itrans4x4(blocks[15], mb_pred, mb_rec, 12, 12);
COPY_16x16:

; construct picture from 4x4 blocks
; opt_copy_image_data_16x16_stride(curr_img, currMB->pix_x, currMB->pix_y, currSlice->mb_rec[pl]);

	mov	eax, DWORD PTR [edi+40]
	mov	ecx, DWORD PTR [edi+36]
	shl	esi, 8
	lea	edx, DWORD PTR [esi+ebp+mb_rec@Slice]
	push	edx
	mov	edx, DWORD PTR _curr_img$[esp+20]
	push	eax
	push	ecx
	push	edx
	call	DWORD PTR _opt+copy_image_data_16x16_stride@OptimizedFunctions
	add	esp, 16					; 00000010H
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	ecx
	ret	0
_iMBtrans4x4 ENDP
_TEXT	ENDS

_TEXT	SEGMENT 
       ALIGN     2
	PUBLIC _itrans8x8_sse2
_itrans8x8_sse2	PROC NEAR
; parameter 1(mb_rec): 8 + ebp
; parameter 2(mb_pred): 12 + ebp
; parameter 3(block): 16 + ebp
; parameter 4(pos_x): 20 + ebp
        push      ebp
        mov       ebp, esp
        and       esp, -16
        sub       esp, 176
        mov edx, DWORD PTR [ebp+20]
        mov ecx, DWORD PTR [ebp+8] ; ecx: mb_rec
        add ecx, edx
        add edx, DWORD PTR [ebp+12] ; edx: mb_pred
        mov eax, DWORD PTR [ebp+16] ; eax: block

;;; 		__m128i a0, a1, a2, a3;
;;; 		__m128i p0, p1, p2, p3, p4, p5 ,p6, p7;
;;; 		__m128i b0, b1, b2, b3, b4, b5, b6, b7;
;;; 		__m128i r0, r1, r2, r3, r4, r5, r6, r7;
;;; 		__m128i const32, zero;
;;; 		__declspec(align(32)) static const int16_t c32[8] = {32, 32, 32, 32, 32, 32, 32, 32};
;;; 		__m128i pred0, pred1;
;;; 
;;; 		const32 = _mm_load_si128((const __m128i *)c32);

        movdqa    xmm0, XMMWORD PTR const32

;;; 		zero = _mm_setzero_si128();
;;; 
;;; 				// Horizontal
;;; 			b0 = _mm_load_si128((__m128i *)(block[0])); 

        movdqa    xmm4, XMMWORD PTR [eax]

;;; 			b1 = _mm_load_si128((__m128i *)(block[1])); 

        movdqa    xmm7, XMMWORD PTR [eax+16]

;;; 			b2 = _mm_load_si128((__m128i *)(block[2])); 

        movdqa    xmm5, XMMWORD PTR [eax+32]

;;; 			b3 = _mm_load_si128((__m128i *)(block[3])); 

        movdqa    xmm3, XMMWORD PTR [eax+48]

;;; 			b4 = _mm_load_si128((__m128i *)(block[4])); 

        movdqa    xmm6, XMMWORD PTR [eax+64]

;;; 			b5 = _mm_load_si128((__m128i *)(block[5])); 
;;; 			b6 = _mm_load_si128((__m128i *)(block[6])); 

        movdqa    xmm1, XMMWORD PTR [eax+96]

;;; 			b7 = _mm_load_si128((__m128i *)(block[7])); 

        movdqa    xmm2, XMMWORD PTR [eax+112]
        movdqa    XMMWORD PTR [esp], xmm0
        movdqa    xmm0, XMMWORD PTR [eax+80]
        movdqa    XMMWORD PTR [esp+16], xmm2

;;; 
;;; 			/* rotate 8x8 (ugh) */
;;; 			r0 = _mm_unpacklo_epi16(b0, b2); 

        movdqa    xmm2, xmm4
        punpcklwd xmm2, xmm5

;;; 			r1 = _mm_unpacklo_epi16(b1, b3); 
;;; 			r2 = _mm_unpackhi_epi16(b0, b2); 

        punpckhwd xmm4, xmm5

;;; 			r3 = _mm_unpackhi_epi16(b1, b3); 
;;; 			r4 = _mm_unpacklo_epi16(b4, b6); 
;;; 			r5 = _mm_unpacklo_epi16(b5, b7); 

        movdqa    xmm5, xmm0
        movdqa    XMMWORD PTR [esp+32], xmm2
        movdqa    xmm2, xmm7
        punpcklwd xmm2, xmm3
        punpckhwd xmm7, xmm3
        movdqa    xmm3, xmm6
        punpcklwd xmm3, xmm1
        movdqa    XMMWORD PTR [esp+48], xmm3
        movdqa    xmm3, XMMWORD PTR [esp+16]
        punpcklwd xmm5, xmm3

;;; 			r6 = _mm_unpackhi_epi16(b4, b6); 

        punpckhwd xmm6, xmm1
;;; 			r7 = _mm_unpackhi_epi16(b5, b7); 

        punpckhwd xmm0, xmm3

;;; 
;;; 			b0 = _mm_unpacklo_epi16(r0, r1); 

        movdqa    xmm3, XMMWORD PTR [esp+32]
        movdqa    xmm1, xmm3
        punpcklwd xmm1, xmm2

;;; 			b1 = _mm_unpackhi_epi16(r0, r1); 

        punpckhwd xmm3, xmm2

;;; 			b2 = _mm_unpacklo_epi16(r2, r3); 

        movdqa    xmm2, xmm4
        punpcklwd xmm2, xmm7

;;; 			b3 = _mm_unpackhi_epi16(r2, r3); 

        punpckhwd xmm4, xmm7
        movdqa    XMMWORD PTR [esp+64], xmm4

;;; 			b4 = _mm_unpacklo_epi16(r4, r5); 

        movdqa    xmm4, XMMWORD PTR [esp+48]
        movdqa    xmm7, xmm4
        punpcklwd xmm7, xmm5

;;; 			b5 = _mm_unpackhi_epi16(r4, r5); 

        punpckhwd xmm4, xmm5

;;; 			b6 = _mm_unpacklo_epi16(r6, r7); 

        movdqa    xmm5, xmm6
        punpcklwd xmm5, xmm0

;;; 			b7 = _mm_unpackhi_epi16(r6, r7); 

        punpckhwd xmm6, xmm0

;;; 
;;; 			p0 = _mm_unpacklo_epi64(b0, b4);

        movdqa    xmm0, xmm1
        punpcklqdq xmm0, xmm7

;;; 			p1 = _mm_unpackhi_epi64(b0, b4);

        punpckhqdq xmm1, xmm7
        movdqa    XMMWORD PTR [esp+16], xmm1

;;; 			p2 = _mm_unpacklo_epi64(b1, b5);

        movdqa    xmm1, xmm3
        punpcklqdq xmm1, xmm4

;;; 			p3 = _mm_unpackhi_epi64(b1, b5);
;;; 			p4 = _mm_unpacklo_epi64(b2, b6);
;;; 			p5 = _mm_unpackhi_epi64(b2, b6);
;;; 			p6 = _mm_unpacklo_epi64(b3, b7);
;;; 			p7 = _mm_unpackhi_epi64(b3, b7);
;;; 
;;; 			/* perform approx DCT */
;;; 						a0 = _mm_add_epi16(p0, p4); // p0 + p4
;;; 			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
;;; 			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1

        movdqa    xmm7, xmm1
        psraw     xmm7, 1
        punpckhqdq xmm3, xmm4
        movdqa    XMMWORD PTR [esp+32], xmm3
        movdqa    xmm3, xmm2
        punpcklqdq xmm3, xmm5
        punpckhqdq xmm2, xmm5
        movdqa    xmm5, XMMWORD PTR [esp+64]
        movdqa    xmm4, xmm5
        punpcklqdq xmm4, xmm6
        punpckhqdq xmm5, xmm6
        movdqa    xmm6, xmm0
        paddw     xmm6, xmm3
        psubw     xmm0, xmm3

;;; 			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)

        movdqa    xmm3, xmm4

;;; 			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1

        psraw     xmm4, 1
        psubw     xmm3, xmm7

;;; 			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

        paddw     xmm1, xmm4

;;; 
;;; 			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;

        movdqa    xmm4, xmm6

;;; 			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;

        movdqa    xmm7, xmm0
        paddw     xmm4, xmm1
        psubw     xmm7, xmm3
        movdqa    XMMWORD PTR [esp+48], xmm7

;;; 			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;

        paddw     xmm0, xmm3
        movdqa    XMMWORD PTR [esp+80], xmm0

;;; 			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;
;;; 
;;; 			//-p3 + p5 - p7 - (p7 >> 1);
;;; 			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
;;; 			a0 = _mm_sub_epi16(p5, p3); // p5 - p3

        movdqa    xmm0, XMMWORD PTR [esp+32]
        psubw     xmm6, xmm1
        movdqa    xmm1, xmm5
        psraw     xmm1, 1
        movdqa    xmm3, xmm2

;;; 			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7
;;; 			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)
;;; 
;;; 			//p1 + p7 - p3 - (p3 >> 1);
;;; 			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)

        movdqa    xmm7, xmm0
        movdqa    XMMWORD PTR [esp+96], xmm6

;;; 			a1 = _mm_add_epi16(p1, p7); // p1 + p7

        movdqa    xmm6, XMMWORD PTR [esp+16]
        psubw     xmm3, xmm0
        psubw     xmm3, xmm5
        psraw     xmm7, 1
        psubw     xmm3, xmm1
        movdqa    xmm1, xmm6
        paddw     xmm1, xmm5

;;; 			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3

        psubw     xmm1, xmm0

;;; 			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

        psubw     xmm1, xmm7

;;; 
;;; 			// -p1 + p7 + p5 + (p5 >> 1);
;;; 			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)

        movdqa    xmm7, xmm2
        psraw     xmm7, 1

;;; 			a2 = _mm_sub_epi16(p7, p1); // p7 - p1

        psubw     xmm5, xmm6

;;; 			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5

        paddw     xmm5, xmm2

;;; 			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

        paddw     xmm5, xmm7

;;; 
;;; 			// p3 + p5 + p1 + (p1 >> 1);
;;; 			a3 = _mm_add_epi16(p3, p5); // p3+p5

        paddw     xmm0, xmm2

;;; 			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
;;; 			p1 = _mm_srai_epi16(p1, 1); // p1 >> 1
;;; 			a3 = _mm_add_epi16(a3, p1); //p3 + p5 + p1 + (p1 >> 1)
;;; 
;;; 			r0 = _mm_srai_epi16(a3, 2); // a3>>2
;;; 			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);
;;; 			r0 = _mm_srai_epi16(a2, 2); // a2>>2
;;; 			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);
;;; 			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
;;; 			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
;;; 			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
;;; 			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);
;;; 
;;; 			p0 = _mm_add_epi16(b0, b7); // b0 + b7;
;;; 			p1 = _mm_sub_epi16(b2, b5); // b2 - b5;
;;; 			p2 = _mm_add_epi16(b4, b3); // b4 + b3;
;;; 			p3 = _mm_add_epi16(b6, b1); // b6 + b1;

        movdqa    xmm2, XMMWORD PTR [esp+96]
        paddw     xmm0, xmm6
        psraw     xmm6, 1
        paddw     xmm0, xmm6
        movdqa    xmm7, xmm0
        movdqa    xmm6, xmm5
        psraw     xmm7, 2
        paddw     xmm7, xmm3
        psraw     xmm6, 2
        paddw     xmm6, xmm1
        psraw     xmm1, 2
        psubw     xmm5, xmm1
        movdqa    xmm1, xmm4
        psraw     xmm3, 2
        psubw     xmm0, xmm3
        movdqa    xmm3, XMMWORD PTR [esp+80]
        movdqa    XMMWORD PTR [esp+32], xmm0

;;; 			p4 = _mm_sub_epi16(b6, b1); // b6 - b1;
;;; 			p5 = _mm_sub_epi16(b4, b3); // b4 - b3;
;;; 			p6 = _mm_add_epi16(b2, b5); // b2 + b5;
;;; 			p7 = _mm_sub_epi16(b0, b7); // b0 - b7;

        psubw     xmm4, XMMWORD PTR [esp+32]
        paddw     xmm1, xmm0
        movdqa    XMMWORD PTR [esp+112], xmm1
        movdqa    xmm1, XMMWORD PTR [esp+48]
        movdqa    xmm0, xmm1
        psubw     xmm0, xmm5
        movdqa    XMMWORD PTR [esp+16], xmm0
        movdqa    xmm0, xmm3
        paddw     xmm0, xmm6
        psubw     xmm3, xmm6
        movdqa    XMMWORD PTR [esp+128], xmm0

;;; 
;;; 						/* rotate 8x8 (ugh) */
;;; 			r0 = _mm_unpacklo_epi16(p0, p2); 

        movdqa    xmm6, XMMWORD PTR [esp+128]
        movdqa    xmm0, xmm2
        paddw     xmm0, xmm7
        psubw     xmm2, xmm7
        paddw     xmm1, xmm5
        movdqa    xmm5, XMMWORD PTR [esp+112]
        movdqa    XMMWORD PTR [esp+144], xmm4
        movdqa    xmm4, xmm5
        punpcklwd xmm4, xmm6

;;; 			r1 = _mm_unpacklo_epi16(p1, p3); 
;;; 			r2 = _mm_unpackhi_epi16(p0, p2); 

        punpckhwd xmm5, xmm6

;;; 			r3 = _mm_unpackhi_epi16(p1, p3); 
;;; 			r4 = _mm_unpacklo_epi16(p4, p6); 
;;; 			r5 = _mm_unpacklo_epi16(p5, p7); 

        movdqa    xmm6, xmm3
        movdqa    XMMWORD PTR [esp+64], xmm4
        movdqa    xmm4, XMMWORD PTR [esp+16]
        movdqa    xmm7, xmm4
        punpcklwd xmm7, xmm0
        punpckhwd xmm4, xmm0
        movdqa    xmm0, xmm2
        punpcklwd xmm0, xmm1
        movdqa    XMMWORD PTR [esp+128], xmm0
        movdqa    xmm0, XMMWORD PTR [esp+144]
        punpcklwd xmm6, xmm0

;;; 			r6 = _mm_unpackhi_epi16(p4, p6); 

        punpckhwd xmm2, xmm1

;;; 			r7 = _mm_unpackhi_epi16(p5, p7); 
;;; 
;;; 			b0 = _mm_unpacklo_epi16(r0, r1); 

        movdqa    xmm1, XMMWORD PTR [esp+64]
        punpckhwd xmm3, xmm0
        movdqa    xmm0, xmm1
        punpcklwd xmm0, xmm7

;;; 			b1 = _mm_unpackhi_epi16(r0, r1); 

        punpckhwd xmm1, xmm7

;;; 			b2 = _mm_unpacklo_epi16(r2, r3); 

        movdqa    xmm7, xmm5
        punpcklwd xmm7, xmm4

;;; 			b3 = _mm_unpackhi_epi16(r2, r3); 

        punpckhwd xmm5, xmm4
        movdqa    XMMWORD PTR [esp+112], xmm5

;;; 			b4 = _mm_unpacklo_epi16(r4, r5); 

        movdqa    xmm5, XMMWORD PTR [esp+128]
        movdqa    xmm4, xmm5
        punpcklwd xmm4, xmm6

;;; 			b5 = _mm_unpackhi_epi16(r4, r5); 

        punpckhwd xmm5, xmm6

;;; 			b6 = _mm_unpacklo_epi16(r6, r7); 

        movdqa    xmm6, xmm2
        punpcklwd xmm6, xmm3

;;; 			b7 = _mm_unpackhi_epi16(r6, r7); 

        punpckhwd xmm2, xmm3

;;; 
;;; 			p0 = _mm_unpacklo_epi64(b0, b4);

        movdqa    xmm3, xmm0
        punpcklqdq xmm3, xmm4

;;; 			p1 = _mm_unpackhi_epi64(b0, b4);

        punpckhqdq xmm0, xmm4
        movdqa    XMMWORD PTR [esp+144], xmm0

;;; 			p2 = _mm_unpacklo_epi64(b1, b5);
;;; 			p3 = _mm_unpackhi_epi64(b1, b5);
;;; 			p4 = _mm_unpacklo_epi64(b2, b6);
;;; 			p5 = _mm_unpackhi_epi64(b2, b6);
;;; 			p6 = _mm_unpacklo_epi64(b3, b7);

        movdqa    xmm0, XMMWORD PTR [esp+112]
        movdqa    xmm4, xmm1
        punpcklqdq xmm4, xmm5
        punpckhqdq xmm1, xmm5
        movdqa    XMMWORD PTR [esp+64], xmm1
        movdqa    xmm1, xmm7
        movdqa    xmm5, xmm0
        punpcklqdq xmm1, xmm6
        punpckhqdq xmm7, xmm6

;;; 			p7 = _mm_unpackhi_epi64(b3, b7);
;;; 
;;; 
;;; 		/*  Vertical  */
;;; 
;;; 			a0 = _mm_add_epi16(p0, p4); // p0 + p4
;;; 			a1 = _mm_sub_epi16(p0, p4); // p0 - p4
;;; 			r0 = _mm_srai_epi16(p2, 1); // p2 >> 1

        movdqa    xmm6, xmm4
        psraw     xmm6, 1
        punpcklqdq xmm5, xmm2
        punpckhqdq xmm0, xmm2
        movdqa    xmm2, xmm3
        paddw     xmm2, xmm1
        psubw     xmm3, xmm1

;;; 			a2 = _mm_sub_epi16(p6, r0); // p6 - (p2 >> 1)

        movdqa    xmm1, xmm5

;;; 			r0 = _mm_srai_epi16(p6, 1); // p6 >> 1

        psraw     xmm5, 1
        psubw     xmm1, xmm6

;;; 			a3 = _mm_add_epi16(p2, r0); //p2 + (p6 >> 1)

        paddw     xmm4, xmm5

;;; 
;;; 			b0 =  _mm_add_epi16(a0, a3); // a0 + a3;

        movdqa    xmm5, xmm2

;;; 			b2 =  _mm_sub_epi16(a1, a2);  // a1 - a2;

        movdqa    xmm6, xmm3
        paddw     xmm5, xmm4
        psubw     xmm6, xmm1
        movdqa    XMMWORD PTR [esp+128], xmm6

;;; 			b4 =  _mm_add_epi16(a1, a2);    // a1 + a2;
;;; 			b6 =  _mm_sub_epi16(a0, a3);  // a0 - a3;
;;; 
;;; 			//-p3 + p5 - p7 - (p7 >> 1);
;;; 			r0 = _mm_srai_epi16(p7, 1); // p7 >> 1
;;; 			a0 = _mm_sub_epi16(p5, p3); // p5 - p3

        movdqa    xmm6, XMMWORD PTR [esp+64]
        paddw     xmm3, xmm1
        movdqa    XMMWORD PTR [esp+80], xmm3
        psubw     xmm2, xmm4
        movdqa    xmm1, xmm0
        psraw     xmm1, 1
        movdqa    xmm3, xmm7
        movdqa    XMMWORD PTR [esp+96], xmm2
        psubw     xmm3, xmm6

;;; 			a0 = _mm_sub_epi16(a0, p7); // (-p3 + p5) - p7

        psubw     xmm3, xmm0

;;; 			a0 = _mm_sub_epi16(a0, r0); // (-p3 + p5 - p7) - (p7 >> 1)
;;; 
;;; 			//p1 + p7 - p3 - (p3 >> 1);
;;; 			r0 =  _mm_srai_epi16(p3, 1); // (p3 >> 1)

        movdqa    xmm2, xmm6
        psraw     xmm2, 1
        psubw     xmm3, xmm1

;;; 			a1 = _mm_add_epi16(p1, p7); // p1 + p7

        movdqa    xmm1, XMMWORD PTR [esp+144]
        movdqa    xmm4, xmm1
        paddw     xmm4, xmm0

;;; 			a1 = _mm_sub_epi16(a1, p3); // (p1 + p7) - p3

        psubw     xmm4, xmm6

;;; 			a1 = _mm_sub_epi16(a1, r0); // (p1 + p7 - p3) - (p3>>1)

        psubw     xmm4, xmm2

;;; 
;;; 			// -p1 + p7 + p5 + (p5 >> 1);
;;; 			r0 =  _mm_srai_epi16(p5, 1); // (p5 >> 1)

        movdqa    xmm2, xmm7
        psraw     xmm2, 1

;;; 			a2 = _mm_sub_epi16(p7, p1); // p7 - p1

        psubw     xmm0, xmm1

;;; 			a2 = _mm_add_epi16(a2, p5); // -p1 + p7 + p5

        paddw     xmm0, xmm7

;;; 			a2 = _mm_add_epi16(a2, r0); // (-p1 + p7 + p5) + (p5 >> 1)

        paddw     xmm0, xmm2

;;; 
;;; 			// p3 + p5 + p1 + (p1 >> 1);
;;; 			r0 = _mm_srai_epi16(p1, 1); // p1 >> 1

        movdqa    xmm2, xmm1
        psraw     xmm2, 1

;;; 			a3 = _mm_add_epi16(p3, p5); // p3+p5

        paddw     xmm6, xmm7

;;; 			a3 = _mm_add_epi16(a3, p1); // p3 + p5 + p1
;;; 			a3 = _mm_add_epi16(a3, r0); //p3 + p5 + p1 + (p1 >> 1)
;;; 
;;; 			r0 = _mm_srai_epi16(a3, 2); // a3>>2
;;; 			b1 = _mm_add_epi16(a0, r0); //a0 + (a3>>2);
;;; 			r0 = _mm_srai_epi16(a2, 2); // a2>>2
;;; 			b3 = _mm_add_epi16(a1, r0); // a1 + (a2>>2);
;;; 			a1 = _mm_srai_epi16(a1, 2); // all done with a1, so this is safe
;;; 			b5 = _mm_sub_epi16(a2, a1); //a2 - (a1>>2);
;;; 			a0 = _mm_srai_epi16(a0, 2); // all done with a0, so this is safe
;;; 			b7 = _mm_sub_epi16(a3, a0); //a3 - (a0>>2);
;;; 
;;; 			r0 = _mm_add_epi16(b0, b7); // b0 + b7;
;;; 			r1 = _mm_sub_epi16(b2, b5); // b2 - b5;

        movdqa    xmm7, XMMWORD PTR [esp+128]
        paddw     xmm6, xmm1
        paddw     xmm6, xmm2
        movdqa    xmm1, xmm6
        psraw     xmm1, 2
        movdqa    xmm2, xmm0
        paddw     xmm1, xmm3
        psraw     xmm2, 2
        paddw     xmm2, xmm4
        psraw     xmm4, 2
        psubw     xmm0, xmm4
        psraw     xmm3, 2
        psubw     xmm6, xmm3
        movdqa    XMMWORD PTR [esp+64], xmm6
        movdqa    xmm3, xmm5

;;; 			r2 = _mm_add_epi16(b4, b3); // b4 + b3;
;;; 			r3 = _mm_add_epi16(b6, b1); // b6 + b1;
;;; 			r4 = _mm_sub_epi16(b6, b1); // b6 - b1;
;;; 			r5 = _mm_sub_epi16(b4, b3); // b4 - b3;
;;; 			r6 = _mm_add_epi16(b2, b5); // b2 + b5;
;;; 			r7 = _mm_sub_epi16(b0, b7); // b0 - b7;

        psubw     xmm5, XMMWORD PTR [esp+64]
        paddw     xmm3, xmm6
        movdqa    XMMWORD PTR [esp+144], xmm3
        movdqa    xmm3, xmm7
        psubw     xmm3, xmm0
        movdqa    XMMWORD PTR [esp+48], xmm3
        movdqa    xmm3, XMMWORD PTR [esp+80]
        movdqa    xmm4, xmm3
        paddw     xmm4, xmm2
        psubw     xmm3, xmm2

;;; 
;;; 
;;; 			// add in prediction values
;;; 			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[0][pos_x]));
;;; 			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[1][pos_x]));
;;; 			// (x + 32) >> 6
;;; 			r0 = _mm_adds_epi16(r0, const32);

        movdqa    xmm2, XMMWORD PTR const32
        movdqa    XMMWORD PTR [esp+16], xmm4
        movdqa    xmm4, XMMWORD PTR [esp+96]
        movdqa    xmm6, xmm4
        paddw     xmm6, xmm1
        psubw     xmm4, xmm1

;;; 			r0 = _mm_srai_epi16(r0, 6);
;;; 			r1 = _mm_adds_epi16(r1, const32);

        movdqa    xmm1, XMMWORD PTR [esp+48]
        paddw     xmm7, xmm0
        movdqa    xmm0, XMMWORD PTR [esp+144]
        movdqa    XMMWORD PTR [esp+128], xmm7

;;; 			r1 = _mm_srai_epi16(r1, 6);
;;; 			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short
;;; 			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short

        movq      xmm7, QWORD PTR [edx+16]
        movdqa    XMMWORD PTR [esp+32], xmm5
        paddsw    xmm0, xmm2
        psraw     xmm0, 6
        paddsw    xmm1, xmm2
        pxor      xmm2, xmm2
        punpcklbw xmm7, xmm2
        movq      xmm5, QWORD PTR [edx] 
        punpcklbw xmm5, xmm2
        psraw     xmm1, 6

;;; 			pred0 = _mm_adds_epi16(pred0, r0);
;;; 			pred1 = _mm_adds_epi16(pred1, r1);

        paddsw    xmm7, xmm1
        paddsw    xmm5, xmm0

;;; 
;;; 			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

        packuswb  xmm5, xmm7

;;; 
;;; 			// store
;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[0][pos_x]), pred0);

        movdqa    xmm0, XMMWORD PTR [esp+32]
        movdqa    xmm2, XMMWORD PTR [esp+128]
        movq      QWORD PTR [ecx], xmm5 

;;; 			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
;;; 			pred0 = _mm_srli_si128(pred0, 8);

        psrldq    xmm5, 8

;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[1][pos_x]), pred0);

        movq      QWORD PTR [ecx+16], xmm5

;;; 
;;; 			/* --- */
;;; 
;;; 			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[2][pos_x]));

        movq      xmm1, QWORD PTR [edx+32]

;;; 			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[3][pos_x]));
;;; 			// (x + 32) >> 6
;;; 			r2 = _mm_adds_epi16(r2, const32);

        movdqa    xmm5, XMMWORD PTR [esp]
        movdqa    XMMWORD PTR [esp+32], xmm0                    ;

;;; 			r2 = _mm_srai_epi16(r2, 6);
;;; 			r3 = _mm_adds_epi16(r3, const32);

        paddsw    xmm6, xmm5

;;; 			r3 = _mm_srai_epi16(r3, 6);

        psraw     xmm6, 6

;;; 			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short

        pxor      xmm7, xmm7
        punpcklbw xmm1, xmm7
        movdqa    xmm0, XMMWORD PTR [esp+16]
        paddsw    xmm0, xmm5
        psraw     xmm0, 6

;;; 			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short
;;; 			pred0 = _mm_adds_epi16(pred0, r2);

        paddsw    xmm1, xmm0

;;; 			pred1 = _mm_adds_epi16(pred1, r3);
;;; 
;;; 			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char
;;; 
;;; 			// store
;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[2][pos_x]), pred0);

        movdqa    xmm0, XMMWORD PTR [esp+32]
        movq      xmm5, QWORD PTR [edx+48]
        punpcklbw xmm5, xmm7
        paddsw    xmm5, xmm6
        packuswb  xmm1, xmm5
        movq      QWORD PTR [ecx+32], xmm1

;;; 			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
;;; 			pred0 = _mm_srli_si128(pred0, 8);

        psrldq    xmm1, 8

;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[3][pos_x]), pred0);

        movq      QWORD PTR [ecx+48], xmm1

;;; 
;;; 			/* --- */
;;; 
;;; 			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[4][pos_x]));

        movq      xmm7, QWORD PTR [edx+64]

;;; 			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[5][pos_x]));

        movq      xmm6, QWORD PTR [edx+80]

;;; 			// (x + 32) >> 6
;;; 			r4 = _mm_adds_epi16(r4, const32);
;;; 			r4 = _mm_srai_epi16(r4, 6);
;;; 			r5 = _mm_adds_epi16(r5, const32);
;;; 			r5 = _mm_srai_epi16(r5, 6);
;;; 			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short

        pxor      xmm5, xmm5
        punpcklbw xmm7, xmm5

;;; 			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short

        punpcklbw xmm6, xmm5
        movdqa    xmm1, XMMWORD PTR [esp]
        paddsw    xmm4, xmm1
        psraw     xmm4, 6
        paddsw    xmm3, xmm1
        psraw     xmm3, 6

;;; 			pred0 = _mm_adds_epi16(pred0, r4);

        paddsw    xmm7, xmm4

;;; 			pred1 = _mm_adds_epi16(pred1, r5);

        paddsw    xmm6, xmm3

;;; 
;;; 			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

        packuswb  xmm7, xmm6

;;; 
;;; 			// store
;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[4][pos_x]), pred0);

        movq      QWORD PTR [ecx+64], xmm7

;;; 			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
;;; 			pred0 = _mm_srli_si128(pred0, 8);

        psrldq    xmm7, 8

;;; 			_mm_storel_epi64((__m128i *)(&mb_rec[5][pos_x]), pred0);

        movq      QWORD PTR [ecx+80], xmm7


;;; 
;;; 			/* --- */
;;; 
;;; 			pred0 = _mm_loadl_epi64((__m128i *)(&mb_pred[6][pos_x]));

        movq      xmm5, QWORD PTR [edx+96]

;;; 			pred1 = _mm_loadl_epi64((__m128i *)(&mb_pred[7][pos_x]));

        movq      xmm4, QWORD PTR [edx+112] 

;;; 			// (x + 32) >> 6
;;; 			r6 = _mm_adds_epi16(r6, const32);
;;; 			r6 = _mm_srai_epi16(r6, 6);
;;; 			r7 = _mm_adds_epi16(r7, const32);
;;; 			r7 = _mm_srai_epi16(r7, 6);
;;; 			pred0 = _mm_unpacklo_epi8(pred0, zero); // convert to short

        pxor      xmm3, xmm3
        punpcklbw xmm5, xmm3

;;; 			pred1 = _mm_unpacklo_epi8(pred1, zero); // convert to short

        punpcklbw xmm4, xmm3
        movdqa    xmm1, XMMWORD PTR [esp]
        paddsw    xmm2, xmm1
        psraw     xmm2, 6
        paddsw    xmm0, xmm1
        psraw     xmm0, 6

;;; 			pred0 = _mm_adds_epi16(pred0, r6);

        paddsw    xmm5, xmm2

;;; 			pred1 = _mm_adds_epi16(pred1, r7);

        paddsw    xmm4, xmm0

;;; 
;;; 			pred0 = _mm_packus_epi16(pred0, pred1); // convert to unsigned char

        packuswb  xmm5, xmm4

;;; 
;;; 			// store
;;; 			_mm_storel_epi64((__m128i *)&mb_rec[6][pos_x], pred0);

        movq      QWORD PTR [ecx+96], xmm5

;;; 			// TODO: if mb_pred was converted to 4 8x8 blocks, we could store more easily.
;;; 			pred0 = _mm_srli_si128(pred0, 8);

        psrldq    xmm5, 8

;;; 			_mm_storel_epi64((__m128i *)&mb_rec[7][pos_x], pred0);

        movq      QWORD PTR [ecx+112], xmm5 
        mov       esp, ebp
        pop       ebp
        ret
        ALIGN     2
_itrans8x8_sse2 ENDP


END