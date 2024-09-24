.686
.XMM
.model FLAT


PUBLIC	_inv_level_coefficients
_TEXT	SEGMENT
_blocks$ = 8						; size = 4
_InvLevelScale$ = 12					; size = 4
_qp_per$ = 16						; size = 4
_inv_level_coefficients PROC

	mov	eax, DWORD PTR _blocks$[esp-4]
	mov	ecx, DWORD PTR _qp_per$[esp-4]
	mov	edx, DWORD PTR _InvLevelScale$[esp-4]
	push	esi
	push	edi
	mov	edi, 4
$LL10@inv_level_:

; 3870 : 	{
; 3871 : 		h264_short_block_row_t *block = blocks[b];
; 3872 : 		for (j = 0; j < 4; ++j)
; 3873 : 		{
; 3874 : 				if (block[j][0]) block[j][0]= rshift_rnd_sf((block[j][0] * InvLevelScale[j][0]) << qp_per, 4);

	movsx esi, WORD PTR [eax+4-4]
	test esi, esi
	je	SHORT $LN4@inv_level_
	imul	esi, DWORD PTR [edx]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4-4], si
$LN4@inv_level_:

; 3875 : 				if (block[j][1]) block[j][1]= rshift_rnd_sf((block[j][1] * InvLevelScale[j][1]) << qp_per, 4);

	movsx esi, WORD PTR [eax+4-2]
	test esi, esi
	je	SHORT $LN3@inv_level_
	imul	esi, DWORD PTR [edx+4]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4-2], si
$LN3@inv_level_:

; 3876 : 				if (block[j][2]) block[j][2]= rshift_rnd_sf((block[j][2] * InvLevelScale[j][2]) << qp_per, 4);

	movsx esi, WORD PTR [eax+4]
	test esi, esi
	je	SHORT $LN2@inv_level_
	imul	esi, DWORD PTR [edx+8]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4], si
$LN2@inv_level_:

; 3877 : 				if (block[j][3]) block[j][3]= rshift_rnd_sf((block[j][3] * InvLevelScale[j][3]) << qp_per, 4);

	movsx esi, WORD PTR [eax+4+2]
	test esi, esi
	je	SHORT $LN6@inv_level_
	imul	esi, DWORD PTR [edx+12]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+2], si
$LN6@inv_level_:
	movsx esi, WORD PTR [eax+4+4]
	test esi, esi
	je	SHORT $LN27@inv_level_
	imul	esi, DWORD PTR [edx+16]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+4], si
$LN27@inv_level_:
	movsx esi, WORD PTR [eax+4+6]
	test esi, esi
	je	SHORT $LN28@inv_level_
	imul	esi, DWORD PTR [edx+20]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+6], si
$LN28@inv_level_:
	movsx esi, WORD PTR [eax+4+8]
	test esi, esi
	je	SHORT $LN29@inv_level_
	imul	esi, DWORD PTR [edx+24]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+8], si
$LN29@inv_level_:
	movsx esi, WORD PTR [eax+4+10]
	test esi, esi
	je	SHORT $LN30@inv_level_
	imul	esi, DWORD PTR [edx+28]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+10], si
$LN30@inv_level_:
	movsx esi, WORD PTR [eax+4+12]
	test esi, esi
	je	SHORT $LN32@inv_level_
	imul	esi, DWORD PTR [edx+32]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+12], si
$LN32@inv_level_:
	movsx esi, WORD PTR [eax+4+14]
	test esi, esi
	je	SHORT $LN33@inv_level_
	imul	esi, DWORD PTR [edx+36]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+14], si
$LN33@inv_level_:
	movsx esi, WORD PTR [eax+4+16]
	test esi, esi
	je	SHORT $LN34@inv_level_
	imul	esi, DWORD PTR [edx+40]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+16], si
$LN34@inv_level_:
	movsx esi, WORD PTR [eax+4+18]
	test esi, esi
	je	SHORT $LN35@inv_level_
	imul	esi, DWORD PTR [edx+44]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+18], si
$LN35@inv_level_:
	movsx esi, WORD PTR [eax+4+20]
	test esi, esi
	je	SHORT $LN37@inv_level_
	imul	esi, DWORD PTR [edx+48]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+20], si
$LN37@inv_level_:
	movsx esi, WORD PTR [eax+4+22]
	test esi, esi
	je	SHORT $LN38@inv_level_
	imul	esi, DWORD PTR [edx+52]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+22], si
$LN38@inv_level_:
	movsx esi, WORD PTR [eax+4+24]
	test esi, esi
	je	SHORT $LN39@inv_level_
	imul	esi, DWORD PTR [edx+56]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+24], si
$LN39@inv_level_:
	movsx esi, WORD PTR [eax+4+26]
	test esi, esi
	je	SHORT $LN9@inv_level_
	imul	esi, DWORD PTR [edx+60]
	shl	esi, cl
	add	esi, 8
	sar	esi, 4
	mov	WORD PTR [eax+4+26], si
$LN9@inv_level_:
	add	eax, 32					; 00000020H
	sub	edi, 1
	jne	$LL10@inv_level_
	pop	edi
	pop	esi

	ret	0
_inv_level_coefficients ENDP

END