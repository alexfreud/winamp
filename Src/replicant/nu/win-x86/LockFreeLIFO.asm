.686
.model FLAT

PUBLIC _lifo_push
_TEXT SEGMENT
lifo = 4      ; size = 4
entry = 8      ; size = 4
_lifo_push PROC
	mov ecx, DWORD PTR 4[esp] ; ecx holds lifo
	mov edx, DWORD PTR 8[esp] ; edx holds the new entry
again:
	mov eax, DWORD PTR [ecx] ; eax holds the old head
	mov DWORD PTR[edx], eax ; new node's 'next' is set to the old head
	lock cmpxchg DWORD PTR [ecx], edx
	jnz again
	ret	0
_lifo_push ENDP

PUBLIC _lifo_pop
_TEXT SEGMENT
lifo = 4      ; size = 4
_lifo_pop PROC
	push esi
	push ebx
	mov	esi, DWORD PTR 12[esp] ; esi holds lifo
again:
    ; if re-ordered loads become an issue, we could use cmpxchg8b to read in (after zeroing ebx/ecx) or maybe use movq
	mov edx, DWORD PTR [esi+4] ; counter
	; or we could put an LFENCE here
	mov eax, DWORD PTR [esi] ; pointer
	test eax, eax
	jz bail

	mov ecx, edx ; counter
	mov ebx, DWORD PTR [eax] ; pointer->next
	inc ecx

	lock cmpxchg8b QWORD PTR [esi]
	jnz again

bail:
	pop ebx
	pop esi
	ret 0
_lifo_pop ENDP

_TEXT ENDS

END

