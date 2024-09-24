.686
.XMM
.model FLAT

; Slice
tex_ctx@Slice = 100
coeff@Slice = 15632
coeff_ctr@Slice = 15760
pos@Slice = 15764
last_dquant@Slice = 88
mot_ctx@Slice = 96
slice_type@Slice = 64

; VideoParameters
structure@VideoParameters = 697200
dec_picture@VideoParameters = 698192
bitdepth_chroma_qp_scale@VideoParameters = 697456

; Macroblock
p_Slice@Macroblock = 0
p_Vid@Macroblock = 4
qp@macroblock = 60
qp_scaled@Macroblock = 72
mb_field@Macroblock = 344
read_and_store_CBP_block_bit@Macroblock = 400

; StorablePicture
structure@StorablePicture = 0
chroma_qp_offset@StorablePicture = 158688

; TextureInfoContexts
map_contexts@TextureInfoContexts = 436
last_contexts@TextureInfoContexts = 3252
one_contexts@TextureInfoContexts = 6068
abs_contexts@TextureInfoContexts = 6508

_DATA	SEGMENT
_pos2ctx_map DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map2x4c
	DD	FLAT:_pos2ctx_map4x4c
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map8x4
	DD	FLAT:_pos2ctx_map4x4
_pos2ctx_map_int DD FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8i
	DD	FLAT:_pos2ctx_map8x4i
	DD	FLAT:_pos2ctx_map4x8i
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map2x4c
	DD	FLAT:_pos2ctx_map4x4c
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8i
	DD	FLAT:_pos2ctx_map8x4i
	DD	FLAT:_pos2ctx_map8x4i
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map4x4
	DD	FLAT:_pos2ctx_map8x8i
	DD	FLAT:_pos2ctx_map8x4i
	DD	FLAT:_pos2ctx_map8x4i
	DD	FLAT:_pos2ctx_map4x4
_pos2ctx_last DD FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last8x8
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last2x4c
	DD	FLAT:_pos2ctx_last4x4c
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last8x8
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last4x4
	DD	FLAT:_pos2ctx_last8x8
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last8x4
	DD	FLAT:_pos2ctx_last4x4
_DATA	ENDS

CONST SEGMENT
_rLPS_table_64x4 DB 080H
	DB	080H
	DB	080H
	DB	07bH
	DB	074H
	DB	06fH
	DB	069H
	DB	064H
	DB	05fH
	DB	05aH
	DB	055H
	DB	051H
	DB	04dH
	DB	049H
	DB	045H
	DB	042H
	DB	03eH
	DB	03bH
	DB	038H
	DB	035H
	DB	033H
	DB	030H
	DB	02eH
	DB	02bH
	DB	029H
	DB	027H
	DB	025H
	DB	023H
	DB	021H
	DB	020H
	DB	01eH
	DB	01dH
	DB	01bH
	DB	01aH
	DB	018H
	DB	017H
	DB	016H
	DB	015H
	DB	014H
	DB	013H
	DB	012H
	DB	011H
	DB	010H
	DB	0fH
	DB	0eH
	DB	0eH
	DB	0dH
	DB	0cH
	DB	0cH
	DB	0bH
	DB	0bH
	DB	0aH
	DB	0aH
	DB	09H
	DB	09H
	DB	08H
	DB	08H
	DB	07H
	DB	07H
	DB	07H
	DB	06H
	DB	06H
	DB	06H
	DB	02H
	DB	0b0H
	DB	0a7H
	DB	09eH
	DB	096H
	DB	08eH
	DB	087H
	DB	080H
	DB	07aH
	DB	074H
	DB	06eH
	DB	068H
	DB	063H
	DB	05eH
	DB	059H
	DB	055H
	DB	050H
	DB	04cH
	DB	048H
	DB	045H
	DB	041H
	DB	03eH
	DB	03bH
	DB	038H
	DB	035H
	DB	032H
	DB	030H
	DB	02dH
	DB	02bH
	DB	029H
	DB	027H
	DB	025H
	DB	023H
	DB	021H
	DB	01fH
	DB	01eH
	DB	01cH
	DB	01bH
	DB	01aH
	DB	018H
	DB	017H
	DB	016H
	DB	015H
	DB	014H
	DB	013H
	DB	012H
	DB	011H
	DB	010H
	DB	0fH
	DB	0eH
	DB	0eH
	DB	0dH
	DB	0cH
	DB	0cH
	DB	0bH
	DB	0bH
	DB	0aH
	DB	09H
	DB	09H
	DB	09H
	DB	08H
	DB	08H
	DB	07H
	DB	07H
	DB	02H
	DB	0d0H
	DB	0c5H
	DB	0bbH
	DB	0b2H
	DB	0a9H
	DB	0a0H
	DB	098H
	DB	090H
	DB	089H
	DB	082H
	DB	07bH
	DB	075H
	DB	06fH
	DB	069H
	DB	064H
	DB	05fH
	DB	05aH
	DB	056H
	DB	051H
	DB	04dH
	DB	049H
	DB	045H
	DB	042H
	DB	03fH
	DB	03bH
	DB	038H
	DB	036H
	DB	033H
	DB	030H
	DB	02eH
	DB	02bH
	DB	029H
	DB	027H
	DB	025H
	DB	023H
	DB	021H
	DB	020H
	DB	01eH
	DB	01dH
	DB	01bH
	DB	01aH
	DB	019H
	DB	017H
	DB	016H
	DB	015H
	DB	014H
	DB	013H
	DB	012H
	DB	011H
	DB	010H
	DB	0fH
	DB	0fH
	DB	0eH
	DB	0dH
	DB	0cH
	DB	0cH
	DB	0bH
	DB	0bH
	DB	0aH
	DB	0aH
	DB	09H
	DB	09H
	DB	08H
	DB	02H
	DB	0f0H
	DB	0e3H
	DB	0d8H
	DB	0cdH
	DB	0c3H
	DB	0b9H
	DB	0afH
	DB	0a6H
	DB	09eH
	DB	096H
	DB	08eH
	DB	087H
	DB	080H
	DB	07aH
	DB	074H
	DB	06eH
	DB	068H
	DB	063H
	DB	05eH
	DB	059H
	DB	055H
	DB	050H
	DB	04cH
	DB	048H
	DB	045H
	DB	041H
	DB	03eH
	DB	03bH
	DB	038H
	DB	035H
	DB	032H
	DB	030H
	DB	02dH
	DB	02bH
	DB	029H
	DB	027H
	DB	025H
	DB	023H
	DB	021H
	DB	01fH
	DB	01eH
	DB	01cH
	DB	01bH
	DB	019H
	DB	018H
	DB	017H
	DB	016H
	DB	015H
	DB	014H
	DB	013H
	DB	012H
	DB	011H
	DB	010H
	DB	0fH
	DB	0eH
	DB	0eH
	DB	0dH
	DB	0cH
	DB	0cH
	DB	0bH
	DB	0bH
	DB	0aH
	DB	09H
	DB	02H
_AC_next_state_MPS_64 DB 01H
 DB 02H
 DB 03H
 DB 04H
 DB 05H
 DB 06H
 DB 07H
 DB 08H
 DB 09H
 DB 0aH
 DB 0bH
 DB 0cH
 DB 0dH
 DB 0eH
 DB 0fH
 DB 010H
 DB 011H
 DB 012H
 DB 013H
 DB 014H
 DB 015H
 DB 016H
 DB 017H
 DB 018H
 DB 019H
 DB 01aH
 DB 01bH
 DB 01cH
 DB 01dH
 DB 01eH
 DB 01fH
 DB 020H
 DB 021H
 DB 022H
 DB 023H
 DB 024H
 DB 025H
 DB 026H
 DB 027H
 DB 028H
 DB 029H
 DB 02aH
 DB 02bH
 DB 02cH
 DB 02dH
 DB 02eH
 DB 02fH
 DB 030H
 DB 031H
 DB 032H
 DB 033H
 DB 034H
 DB 035H
 DB 036H
 DB 037H
 DB 038H
 DB 039H
 DB 03aH
 DB 03bH
 DB 03cH
 DB 03dH
 DB 03eH
 DB 03eH
 DB 03fH
_AC_next_state_LPS_64 DB 00H
 DB 00H
 DB 01H
 DB 02H
 DB 02H
 DB 04H
 DB 04H
 DB 05H
 DB 06H
 DB 07H
 DB 08H
 DB 09H
 DB 09H
 DB 0bH
 DB 0bH
 DB 0cH
 DB 0dH
 DB 0dH
 DB 0fH
 DB 0fH
 DB 010H
 DB 010H
 DB 012H
 DB 012H
 DB 013H
 DB 013H
 DB 015H
 DB 015H
 DB 016H
 DB 016H
 DB 017H
 DB 018H
 DB 018H
 DB 019H
 DB 01aH
 DB 01aH
 DB 01bH
 DB 01bH
 DB 01cH
 DB 01dH
 DB 01dH
 DB 01eH
 DB 01eH
 DB 01eH
 DB 01fH
 DB 020H
 DB 020H
 DB 021H
 DB 021H
 DB 021H
 DB 022H
 DB 022H
 DB 023H
 DB 023H
 DB 023H
 DB 024H
 DB 024H
 DB 024H
 DB 025H
 DB 025H
 DB 025H
 DB 026H
 DB 026H
 DB 03fH
_renorm_table_32 DB 06H
 DB 05H
 DB 04H
 DB 04H
 DB 03H
 DB 03H
 DB 03H
 DB 03H
 DB 02H
 DB 02H
 DB 02H
 DB 02H
 DB 02H
 DB 02H
 DB 02H
 DB 02H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 DB 01H
 _renorm_table_256 DB 06H
	DB	06H
	DB	06H
	DB	06H
	DB	06H
	DB	06H
	DB	06H
	DB	06H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
_maxpos	DB	0fH
	DB	0eH
	DB	03fH
	DB	01fH
	DB	01fH
	DB	0fH
	DB	03H
	DB	0eH
	DB	07H
	DB	0fH
	DB	0fH
	DB	0eH
	DB	03fH
	DB	01fH
	DB	01fH
	DB	0fH
	DB	0fH
	DB	0eH
	DB	03fH
	DB	01fH
	DB	01fH
	DB	0fH
	ORG $+2
_c1isdc	DB	01H
	DB	00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	00H
	DB	01H
	DB	01H
	DB	01H
	DB	00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	ORG $+2
_type2ctx_bcbp DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	05H
	DB	05H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0dH
	DB	0eH
	DB	010H
	DB	011H
	DB	012H
	DB	013H
	DB	013H
	DB	014H
	ORG $+2
_type2ctx_map DW 00H
DW 010H
DW 020H
DW 030H
DW 040H
DW 050H
DW 060H
DW 070H
DW 060H
DW 060H
DW 0A0H
DW 0B0H
DW 0C0H
DW 0D0H
DW 0E0H
DW 0F0H
DW 0100H
DW 0110H
DW 0120H
DW 0130H
DW 0140H
DW 0150H
	ORG $+2
_type2ctx_last DW 00H
DW 010H
DW 020H
DW 030H
DW 040H
DW 050H
DW 060H
DW 070H
DW 060H
DW 060H
DW 0A0H
DW 0B0H
DW 0C0H
DW 0D0H
DW 0E0H
DW 0F0H
DW 0100H
DW 0110H
DW 0120H
DW 0130H
DW 0140H
DW 0150H
	ORG $+2
_type2ctx_one DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	05H
	DB	05H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0dH
	DB	0eH
	DB	010H
	DB	011H
	DB	012H
	DB	013H
	DB	013H
	DB	014H
	ORG $+2
_type2ctx_abs DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	05H
	DB	05H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0dH
	DB	0eH
	DB	010H
	DB	011H
	DB	012H
	DB	013H
	DB	013H
	DB	014H
	ORG $+2
plus_one_clip4 DD 1,2,3,4,4
plus_one_clip3 DD 1,2,3,3
_max_c2	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip3
	DD	plus_one_clip4
	DD	plus_one_clip3
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	DD	plus_one_clip4
	ORG $+6
_pos2ctx_map8x8 DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	04H
	DB	05H
	DB	05H
	DB	04H
	DB	04H
	DB	03H
	DB	03H
	DB	04H
	DB	04H
	DB	04H
	DB	05H
	DB	05H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	03H
	DB	03H
	DB	06H
	DB	07H
	DB	07H
	DB	07H
	DB	08H
	DB	09H
	DB	0aH
	DB	09H
	DB	08H
	DB	07H
	DB	07H
	DB	06H
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0bH
	DB	06H
	DB	07H
	DB	08H
	DB	09H
	DB	0eH
	DB	0aH
	DB	09H
	DB	08H
	DB	06H
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0bH
	DB	06H
	DB	09H
	DB	0eH
	DB	0aH
	DB	09H
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0bH
	DB	0eH
	DB	0aH
	DB	0cH
	DB	0eH
_pos2ctx_map8x4 DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	04H
	DB	05H
	DB	07H
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	09H
	DB	08H
	DB	06H
	DB	07H
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	09H
	DB	08H
	DB	06H
	DB	0cH
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	09H
	DB	0dH
	DB	0dH
	DB	0eH
	DB	0eH
_pos2ctx_map4x4 DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	07H
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0eH
	DB	0eH
_pos2ctx_map2x4c DB 00H
	DB	00H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
_pos2ctx_map4x4c DB 00H
	DB	00H
	DB	00H
	DB	00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
_pos2ctx_map8x8i DB 00H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	03H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	07H
	DB	07H
	DB	07H
	DB	08H
	DB	04H
	DB	05H
	DB	06H
	DB	09H
	DB	0aH
	DB	0aH
	DB	08H
	DB	0bH
	DB	0cH
	DB	0bH
	DB	09H
	DB	09H
	DB	0aH
	DB	0aH
	DB	08H
	DB	0bH
	DB	0cH
	DB	0bH
	DB	09H
	DB	09H
	DB	0aH
	DB	0aH
	DB	08H
	DB	0bH
	DB	0cH
	DB	0bH
	DB	09H
	DB	09H
	DB	0aH
	DB	0aH
	DB	08H
	DB	0dH
	DB	0dH
	DB	09H
	DB	09H
	DB	0aH
	DB	0aH
	DB	08H
	DB	0dH
	DB	0dH
	DB	09H
	DB	09H
	DB	0aH
	DB	0aH
	DB	0eH
	DB	0eH
	DB	0eH
	DB	0eH
	DB	0eH
	DB	0eH
_pos2ctx_map8x4i DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	03H
	DB	04H
	DB	07H
	DB	06H
	DB	08H
	DB	09H
	DB	07H
	DB	06H
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0cH
	DB	0aH
	DB	0bH
	DB	0dH
	DB	0dH
	DB	0eH
	DB	0eH
	DB	0eH
_pos2ctx_map4x8i DB 00H
	DB	01H
	DB	01H
	DB	01H
	DB	02H
	DB	03H
	DB	03H
	DB	04H
	DB	04H
	DB	04H
	DB	05H
	DB	06H
	DB	02H
	DB	07H
	DB	07H
	DB	08H
	DB	08H
	DB	08H
	DB	05H
	DB	06H
	DB	09H
	DB	0aH
	DB	0aH
	DB	0bH
	DB	0bH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0dH
	DB	0eH
	DB	0eH
	DB	0eH
_pos2ctx_last8x8 DB 00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	05H
	DB	05H
	DB	05H
	DB	05H
	DB	06H
	DB	06H
	DB	06H
	DB	06H
	DB	07H
	DB	07H
	DB	07H
	DB	07H
	DB	08H
	DB	08H
	DB	08H
	DB	08H
_pos2ctx_last8x4 DB 00H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	03H
	DB	03H
	DB	03H
	DB	03H
	DB	04H
	DB	04H
	DB	04H
	DB	04H
	DB	05H
	DB	05H
	DB	06H
	DB	06H
	DB	07H
	DB	07H
	DB	08H
	DB	08H
_pos2ctx_last4x4 DB 00H
	DB	01H
	DB	02H
	DB	03H
	DB	04H
	DB	05H
	DB	06H
	DB	07H
	DB	08H
	DB	09H
	DB	0aH
	DB	0bH
	DB	0cH
	DB	0dH
	DB	0eH
	DB	0fH
_pos2ctx_last2x4c DB 00H
	DB	00H
	DB	01H
	DB	01H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
	DB	02H
_pos2ctx_last4x4c DB 00, 00, 00, 00, 01, 01, 01, 01, 02, 02, 02, 02, 02, 02, 02, 02
plus_one_clip0_4 DD 0,2,3,4,4

align 16
_QP_SCALE_CR DD	00H
	DD	01H
	DD	02H
	DD	03H
	DD	04H
	DD	05H
	DD	06H
	DD	07H
	DD	08H
	DD	09H
	DD	0aH
	DD	0bH
	DD	0cH
	DD	0dH
	DD	0eH
	DD	0fH
	DD	010H
	DD	011H
	DD	012H
	DD	013H
	DD	014H
	DD	015H
	DD	016H
	DD	017H
	DD	018H
	DD	019H
	DD	01aH
	DD	01bH
	DD	01cH
	DD	01dH
	DD	01dH
	DD	01eH
	DD	01fH
	DD	020H
	DD	020H
	DD	021H
	DD	022H
	DD	022H
	DD	023H
	DD	023H
	DD	024H
	DD	024H
	DD	025H
	DD	025H
	DD	025H
	DD	026H
	DD	026H
	DD	026H
	DD	027H
	DD	027H
	DD	027H
	DD	027H
	align 16
_51 DD 51
CONST ENDS


PUBLIC _biari_decode_symbol
_TEXT SEGMENT
dep = 4      ; size = 4
bi_ct = 8      ; size = 4
_biari_decode_symbol PROC
 STACKOFFSET=0
 mov edx, DWORD PTR dep[esp+STACKOFFSET] ; edx = dep
 STACKOFFSET=STACKOFFSET+4
 push ebx 
 mov ebx, DWORD PTR bi_ct[esp+STACKOFFSET] ; ebx = bi_ct
 movzx eax, WORD PTR [ebx] ; eax = state
 push ebp 
 push edi 
 STACKOFFSET = STACKOFFSET+8

 mov edi, DWORD PTR [edx] ; edi = range
 mov ecx, edi ; ecx = range
 and ecx, 0C0H ; range >>= 6
 movzx ebp, BYTE PTR _rLPS_table_64x4[ecx+eax] ; ebp = rLPS
 
 ; register state:
 ; eax: state (bi_ct->state)
 ; ebx: bi_ct
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 mov ecx, DWORD PTR [edx+8] ; ecx = bitsleft
 sub edi, ebp ; range -= rLPS
 shl edi, cl ; range << bitsleft
 cmp DWORD PTR [edx+4], edi ; value < (range << bitsleft)
 jge SHORT CABAC@LPS

 movzx ax, BYTE PTR _AC_next_state_MPS_64[eax] ; eax = state = AC_next_state_MPS_64[state]
 shr edi, cl ; undo earlier shift
 mov WORD PTR [ebx], ax ; bi_ct->MPS = state
 cmp edi, 256    ; 00000100H
 setb cl
 
 ; register state
 ; eax: state
 ; ebx: bi_ct
 ; ecx: state (old)
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 shl edi, cl
 sub DWORD PTR [edx+8], ecx ; dep->DbitsLeft--
 mov DWORD PTR [edx], edi ; dep->Drange = range 
 movzx eax, BYTE PTR [ebx+2] ; return bit
 jz SHORT READ_TWO_BYTES; if (dep->DbitsLeft==0)
 
 pop edi
 pop ebp
 pop ebx
 ret 0
align 16
CABAC@LPS:
 sub DWORD PTR [edx+4], edi
 movzx cx, BYTE PTR _AC_next_state_LPS_64[eax] ; cx: state = AC_next_state_LPS_64[state]
 mov WORD PTR [ebx], cx ; store state back to bi_ct->MPS

 ; register state:
 ; eax: state (old)
 ; ebx: bi_ct
 ; ecx: state (new)
 ; edx: dep
 ; edi: range
 ; ebp: rLPS
 
 mov edi, ebx
 test eax, eax ; if state(old) == 0
 movzx ecx, BYTE PTR _renorm_table_256[ebp] ; ecx = renorm_table_32[rLPS>>3]
 sete bl ; bl = 1  [ if state(old) == 0 ]
 movzx eax, BYTE PTR [edi+2]
 xor eax, 1
 xor BYTE PTR [edi+2], bl  ; al ^= bi_ct->state
 
 ; register state:
 ; eax: !state
 ; ebx: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 shl ebp, cl ; ebp = range = rLPS <<= renorm
 sub DWORD PTR [edx+8], ecx ; dep->DbitsLeft -= renorm;
 mov DWORD PTR [edx], ebp ; dep->Drange = range;
 jle SHORT READ_TWO_BYTES ; if( dep->DbitsLeft <= 0 )

 ; register state:
 ; eax: !state
 ; ebx: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 ; ebp: range = rLPS <<= renorm
 
 pop edi
 pop ebp
 pop ebx
 ret 0
align 16
READ_TWO_BYTES:

 ; register state:
 ; eax: !state
 ; ebx: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 
 mov ebx, DWORD PTR [edx+16] ; eax = dep->Dcodestrm_len
 mov ecx, DWORD PTR [ebx] ; ecx = *dep->Dcodestrm_len
 lea edi, DWORD PTR [ecx+2] ; edi = *dep->Dcodestrm_len + 2
 mov DWORD PTR [ebx], edi ; *dep->Dcodestrm_len += 2
 mov ebx, DWORD PTR [edx+12] ; edx = dep->Dcodestrm
 movzx ecx, WORD PTR [ebx+ecx]
 xchg cl, ch
 shl DWORD PTR [edx+4], 16
 mov WORD PTR [edx+4], cx
 
 add DWORD PTR [edx+8], 16   ; dep->DbitsLeft += 16
 ;mov eax, DWORD PTR _bit$[esp+STACKOFFSET] ; eax = bit = return value
 pop edi
 pop ebp
 pop ebx
 ret 0
_biari_decode_symbol ENDP
_TEXT	ENDS

;
;
; a version of biari_decode_symbol slightly optimized
; pass dep in edx and ctx in eax.  edx retains dep on exit

_TEXT SEGMENT
_biari_decode_symbol_map PROC NEAR
 STACKOFFSET=0
 push ebx 
 STACKOFFSET=4
 movzx ebx, WORD PTR [eax] ; ebx = state
 push ebp 
 push edi 
 STACKOFFSET = 12

 mov edi, DWORD PTR [edx] ; edi = range
 mov ecx, edi ; ecx = range
 and ecx, 0C0H ; range >>= 6
 movzx ebp, BYTE PTR _rLPS_table_64x4[ecx+ebx] ; ebp = rLPS
 
 ; register state:
 ; ebx: state (bi_ct->state)
 ; eax: bi_ct
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 mov ecx, DWORD PTR [edx+8] ; ecx = bitsleft
 sub edi, ebp ; range -= rLPS
 shl edi, cl ; range << bitsleft
 cmp DWORD PTR [edx+4], edi ; value < (range << bitsleft)
 jge SHORT CABAC_OPT@LPS
; MPS 
 movzx bx, BYTE PTR _AC_next_state_MPS_64[ebx] ; ebx = state = AC_next_state_MPS_64[state]
 shr edi, cl ; undo earlier shift
 mov WORD PTR [eax], bx ; bi_ct->MPS = state
 cmp edi, 256    ; 00000100H
 setb cl
 
 ; register state
 ; ebx: state
 ; eax: bi_ct
 ; ecx: state (old)
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 shl edi, cl
 sub DWORD PTR [edx+8], ecx ; dep->DbitsLeft--
 mov DWORD PTR [edx], edi ; dep->Drange = range
 movzx eax, BYTE PTR [eax+2] ; return bit
 jz SHORT READ_TWO_BYTES ; if (dep->DbitsLeft==0)
 
 ; register state
 ; ebx: state
 ; eax: bi_ct
 ; ecx: range<<1
 ; edx: dep
 ; edi: range
 ; ebp: rLPS
 
 pop edi
 pop ebp
 pop ebx
 ret 0
align 16
CABAC_OPT@LPS:
 sub DWORD PTR [edx+4], edi
 movzx cx, BYTE PTR _AC_next_state_LPS_64[ebx] ; cx: state = AC_next_state_LPS_64[state]
 mov WORD PTR [eax], cx ; store state back to bi_ct->MPS

 ; register state:
 ; ebx: state (old)
 ; eax: bi_ct
 ; ecx: state (new)
 ; edx: dep
 ; edi: range
 ; ebp: rLPS
 
 mov edi, eax
 movzx eax, BYTE PTR [eax+2]
 xor eax, 1
 test ebx, ebx ; if state(old) == 0
 movzx ecx, BYTE PTR _renorm_table_256[ebp] ; ecx = renorm_table_32[rLPS>>3]
 sete bl ; bl = 1  [ if state(old) == 0 ]
 xor BYTE PTR [edi+2], bl  ; bl ^= bi_ct->state
  
 ; register state:
 ; ebx: !state
 ; eax: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 ; ebp: rLPS

 shl ebp, cl ; ebp = range = rLPS <<= renorm
 sub DWORD PTR [edx+8], ecx ; dep->DbitsLeft -= renorm;
 mov DWORD PTR [edx], ebp ; dep->Drange = range;
 jle SHORT READ_TWO_BYTES ; if( dep->DbitsLeft <= 0 )

 ; register state:
 ; ebx: !state
 ; eax: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 ; ebp: range = rLPS <<= renorm
 
 pop edi
 pop ebp
 pop ebx
 ret 0
align 16
READ_TWO_BYTES:

 ; register state:
 ; ebx: !state
 ; eax: bi_ct
 ; ecx: renorm
 ; edx: dep
 ; edi: range
 
 mov ebx, DWORD PTR [edx+16] ; ebx = dep->Dcodestrm_len
 mov ecx, DWORD PTR [ebx] ; ecx = *dep->Dcodestrm_len
 lea edi, DWORD PTR [ecx+2] ; edi = *dep->Dcodestrm_len + 2
 mov DWORD PTR [ebx], edi ; *dep->Dcodestrm_len += 2
 mov ebx, DWORD PTR [edx+12] ; edx = dep->Dcodestrm
 movzx ecx, WORD PTR [ebx+ecx]
 xchg cl, ch
 shl DWORD PTR [edx+4], 16
 mov WORD PTR [edx+4], cx
 
 add DWORD PTR [edx+8], 16   ; dep->DbitsLeft += 16
 pop edi
 pop ebp
 pop ebx
 ret 0

_biari_decode_symbol_map ENDP
_TEXT	ENDS


; ebx, ebp and edi are NOT preserved
; pass tex_ctx in ebp
; pass type in ebx
; pass dep in edx
; pass coeff in edi
SigCoefFunction MACRO MaxC2, TypeCtxOne, TypeCtxAbs, MaxPos
_abs_contexts$ = 28 ; local variable (safe because of how the function is called)
_one_contexts$ = 32	; local variable (safe because of how the function is called)
STACKOFFSET=0
	lea	eax, DWORD PTR [ebp+TypeCtxOne*20+6068] ; 6068 = offsetof(tex_ctx, one_contexts)
	mov	DWORD PTR _one_contexts$[esp+STACKOFFSET], eax ; one_contexts = tex_ctx->one_contexts[type2ctx_one[type]];
	;push	esi
	STACKOFFSET=STACKOFFSET+0

	;esi: i (loop variable) = maxpos[type]
	lea	ecx, DWORD PTR [ebp+TypeCtxAbs*20+6508]
	mov	ebp, 1 ; ebp: c1	
	xor	ebx, ebx ; ebx: c2
	mov	DWORD PTR _abs_contexts$[esp+STACKOFFSET], ecx ; abs_contexts = tex_ctx->abs_contexts[type2ctx_abs[type]];
SIGN_COEFF@LOOP_AGAIN:
	cmp	WORD PTR [edi+esi*2], 0 ;if (coeff[i]!=0)
	je	SHORT SIGN_COEFF@LOOP_ITR
	mov	ecx, DWORD PTR _one_contexts$[esp+STACKOFFSET]
	lea	eax, DWORD PTR [ecx+ebp*4]
	mov	ebp, DWORD PTR plus_one_clip0_4[ebp*4] ; c1 = plus_one_clip0_4[c1];
	call	_biari_decode_symbol_map ; biari_decode_symbol (dep_dp, one_contexts + c1);
	test eax, eax
	jz SHORT SIGN_COEFF@DECODE_EQ_PROB
	;add	WORD PTR [edi+esi*2], ax ; coeff[i] += 
	mov	ecx, DWORD PTR _abs_contexts$[esp+STACKOFFSET]
	lea	eax, DWORD PTR [ecx+ebx*4]
	call _unary_exp_golomb_level_decode ;unary_exp_golomb_level_decode (dep_dp, abs_contexts + c2);
	inc eax
	add	WORD PTR [edi+esi*2], ax ; coeff[i] += return val
	xor	ebp, ebp ; c1 = 0
	mov	ebx, DWORD PTR MaxC2[ebx*4]	
SIGN_COEFF@DECODE_EQ_PROB:
	call	_biari_decode_symbol_eq_prob_asm ; biari_decode_symbol_eq_prob(dep_dp)
	js	SHORT SIGN_COEFF@LOOP_ITR
	neg WORD PTR [edi+esi*2]
SIGN_COEFF@LOOP_ITR:
	sub	esi, 1
	jns	SHORT SIGN_COEFF@LOOP_AGAIN
	pop	esi
	ret	0
ENDM

_TEXT SEGMENT
_read_significant_coefficients0 PROC
SigCoefFunction plus_one_clip4, 0, 0, 15
_read_significant_coefficients0 ENDP
_read_significant_coefficients1 PROC
SigCoefFunction plus_one_clip4, 1, 1, 14
_read_significant_coefficients1 ENDP
_read_significant_coefficients2 PROC
SigCoefFunction plus_one_clip4, 2, 2, 63
_read_significant_coefficients2 ENDP
_read_significant_coefficients3 PROC
SigCoefFunction plus_one_clip4, 3, 3, 31
_read_significant_coefficients3 ENDP
_read_significant_coefficients4 PROC
SigCoefFunction plus_one_clip4, 3, 3, 31
_read_significant_coefficients4 ENDP
_read_significant_coefficients5 PROC
SigCoefFunction plus_one_clip4, 4, 4, 15
_read_significant_coefficients5 ENDP
_read_significant_coefficients6 PROC
SigCoefFunction plus_one_clip3, 5, 5, 3
_read_significant_coefficients6 ENDP
_read_significant_coefficients7 PROC
SigCoefFunction plus_one_clip4, 6, 6, 14
_read_significant_coefficients7 ENDP
_read_significant_coefficients8 PROC
SigCoefFunction plus_one_clip3, 5, 5, 7
_read_significant_coefficients8 ENDP
_read_significant_coefficients9 PROC
SigCoefFunction plus_one_clip4, 5, 5, 15
_read_significant_coefficients9 ENDP
_read_significant_coefficients10 PROC
SigCoefFunction plus_one_clip4, 10, 10, 15
_read_significant_coefficients10 ENDP
_read_significant_coefficients11 PROC
SigCoefFunction plus_one_clip4, 11, 11, 14
_read_significant_coefficients11 ENDP
_read_significant_coefficients12 PROC
SigCoefFunction plus_one_clip4, 12, 12, 63
_read_significant_coefficients12 ENDP
_read_significant_coefficients13 PROC
SigCoefFunction plus_one_clip4, 13, 13, 31
_read_significant_coefficients13 ENDP
_read_significant_coefficients14 PROC
SigCoefFunction plus_one_clip4, 13, 13, 31
_read_significant_coefficients14 ENDP
_read_significant_coefficients15 PROC
SigCoefFunction plus_one_clip4, 14, 14, 15
_read_significant_coefficients15 ENDP
_read_significant_coefficients16 PROC
SigCoefFunction plus_one_clip4, 16, 16, 15
_read_significant_coefficients16 ENDP
_read_significant_coefficients17 PROC
SigCoefFunction plus_one_clip4, 17, 17, 14
_read_significant_coefficients17 ENDP
_read_significant_coefficients18 PROC
SigCoefFunction plus_one_clip4, 18, 18, 63
_read_significant_coefficients18 ENDP
_read_significant_coefficients19 PROC
SigCoefFunction plus_one_clip4, 19, 19, 31
_read_significant_coefficients19 ENDP
_read_significant_coefficients20 PROC
SigCoefFunction plus_one_clip4, 19, 19, 31
_read_significant_coefficients20 ENDP
_read_significant_coefficients21 PROC
SigCoefFunction plus_one_clip4, 20, 20, 15
_read_significant_coefficients21 ENDP
_TEXT ENDS


;
; push	eax ; currSlice->coeff
; push	ecx ; tex_ctx
; edi is NOT preserved
; pass currMB in edi
; pass dep in ebp
; pass type in ebx
; on return, edi contains coeff, edx contains dep


SigMapFunction MACRO  PosCtxMap, TypeCtxLast, IsDC, MaxPos, PosCtxLast, TypeCtxMap, Func
last_ctx$ = 24 ; local variable (cheating and using stack space from _readRunLevel_CABAC)
coeff_ctr$ = 28	 ; local variable (cheating and using stack space from _readRunLevel_CABAC)
	STACKOFFSET=0
	mov	edx, DWORD PTR [edi+p_Vid@Macroblock] ; edx: p_Vid
	push	esi 
	xor esi, esi
	STACKOFFSET=STACKOFFSET+4
	mov edx, DWORD PTR [edx+structure@VideoParameters]
	add edx, DWORD PTR [edi+mb_field@Macroblock] ; currMB->mb_field
	mov edi, eax ; edi: coeff
	mov eax, 1408 ; 16 * 22 * sizeof(BiContextType)
	cmovz eax, esi
	mov	edx, OFFSET PosCtxMap
	cmovnz edx, DWORD PTR _pos2ctx_map_int[ebx*4]
	IF IsDC EQ 0 
	lea ebx, [edx + 1]
	ELSE
	mov ebx, edx ; pos2ctx_Map = (fld) ? pos2ctx_map_int[type] : pos2ctx_map[type];
	ENDIF
	mov	edx, ebp 	
	lea ebp, [eax+ecx+TypeCtxMap*64+map_contexts@TextureInfoContexts] ; map_ctx   = tex_ctx->map_contexts[fld][type2ctx_map [type]];
	lea	ecx, DWORD PTR [eax+ecx+TypeCtxLast*64+last_contexts@TextureInfoContexts]
	mov	DWORD PTR last_ctx$[esp+STACKOFFSET], ecx ; last_ctx  = tex_ctx->last_contexts[fld][type2ctx_last[type]];
	mov	DWORD PTR coeff_ctr$[esp+STACKOFFSET], esi; coeff_ctr = 0
	;jne	LOOP_AGAIN

	; esi: i
	; ebx: i1 (loop end)
	; ebp: dep_dp
	; edi: coeff
; for (i=i0; i < i1; ++i) // if last coeff is reached, it has to be significant
LOOP_AGAIN:

; --- read significance symbol ---
; if (biari_decode_symbol   (dep_dp, map_ctx + pos2ctx_Map[i]))

	movzx	eax, BYTE PTR [esi+ebx]	
	lea	eax, DWORD PTR [ebp+eax*4]
	call	_biari_decode_symbol_map
	test	eax, eax
	mov	WORD PTR [edi+esi*2], ax ; coeff[i] = biari_decode_symbol()
	je	SHORT LOOP_ITR

; --- read last coefficient symbol ---
; if (biari_decode_symbol (dep_dp, last_ctx + last[i]))

	inc	DWORD PTR coeff_ctr$[esp+STACKOFFSET] ; coeff_ctr++
	IF IsDC EQ 0 
      movzx ecx, BYTE PTR PosCtxLast[esi+1]
	ELSE
	  movzx ecx, BYTE PTR PosCtxLast[esi]
	ENDIF
	mov	eax, DWORD PTR last_ctx$[esp+STACKOFFSET]	
	lea	eax, DWORD PTR [eax+ecx*4]
	call	_biari_decode_symbol_map
	test	eax, eax
	je	SHORT LOOP_ITR
	
	mov	eax, DWORD PTR coeff_ctr$[esp+STACKOFFSET]; return coeff_ctr;
	mov ecx, DWORD PTR [esp]
	mov	ebp, DWORD PTR [ecx+tex_ctx@Slice] ; ; edx:  currSlice->tex_ctx
	mov	DWORD PTR [ecx+coeff_ctr@Slice], eax ; currSlice->coeff_ctr = return value (read_significance_map)
	jmp Func
	align 16
LOOP_ITR:
	inc	esi
	cmp	esi, MaxPos
	jl	SHORT LOOP_AGAIN
	mov	eax, DWORD PTR coeff_ctr$[esp+STACKOFFSET]
	mov	WORD PTR [edi+esi*2], 1
	inc	eax
	mov ecx, DWORD PTR [esp]
	mov	ebp, DWORD PTR [ecx+tex_ctx@Slice] ; ; edx:  currSlice->tex_ctx
	mov	DWORD PTR [ecx+coeff_ctr@Slice], eax ; currSlice->coeff_ctr = return value (read_significance_map)
	jmp Func
ENDM

_TEXT SEGMENT
_read_significance_map0 PROC
SigMapFunction _pos2ctx_map4x4, 0, 1, 15, _pos2ctx_last4x4, 0, _read_significant_coefficients0
_read_significance_map0 ENDP
_read_significance_map1 PROC
SigMapFunction _pos2ctx_map4x4, 1, 0, 14, _pos2ctx_last4x4, 1, _read_significant_coefficients1
_read_significance_map1 ENDP
_read_significance_map2 PROC
SigMapFunction _pos2ctx_map8x8, 2, 1, 63, _pos2ctx_last8x8, 2, _read_significant_coefficients2
_read_significance_map2 ENDP
_read_significance_map3 PROC
SigMapFunction _pos2ctx_map8x4, 3, 1, 31, _pos2ctx_last8x4, 3, _read_significant_coefficients3
_read_significance_map3 ENDP
_read_significance_map4 PROC
SigMapFunction _pos2ctx_map8x4, 4, 1, 31, _pos2ctx_last8x4, 4, _read_significant_coefficients4
_read_significance_map4 ENDP
_read_significance_map5 PROC
SigMapFunction _pos2ctx_map4x4, 5, 1, 15, _pos2ctx_last4x4, 5, _read_significant_coefficients5
_read_significance_map5 ENDP
_read_significance_map6 PROC
SigMapFunction _pos2ctx_map4x4, 6, 1, 3, _pos2ctx_last4x4, 6, _read_significant_coefficients6
_read_significance_map6 ENDP
_read_significance_map7 PROC
SigMapFunction _pos2ctx_map4x4, 7, 0, 14, _pos2ctx_last4x4, 7, _read_significant_coefficients7
_read_significance_map7 ENDP
_read_significance_map8 PROC
SigMapFunction _pos2ctx_map2x4c, 6, 1, 7, _pos2ctx_last2x4c, 6, _read_significant_coefficients8
_read_significance_map8 ENDP
_read_significance_map9 PROC
SigMapFunction _pos2ctx_map4x4c, 6, 1, 15, _pos2ctx_last4x4c, 6, _read_significant_coefficients9
_read_significance_map9 ENDP
_read_significance_map10 PROC
SigMapFunction _pos2ctx_map4x4, 10, 1, 15, _pos2ctx_last4x4, 10, _read_significant_coefficients10
_read_significance_map10 ENDP
_read_significance_map11 PROC
SigMapFunction _pos2ctx_map4x4, 11, 0, 14, _pos2ctx_last4x4, 11, _read_significant_coefficients11
_read_significance_map11 ENDP
_read_significance_map12 PROC
SigMapFunction _pos2ctx_map8x8, 12, 1, 63, _pos2ctx_last8x8, 12, _read_significant_coefficients12
_read_significance_map12 ENDP
_read_significance_map13 PROC
SigMapFunction _pos2ctx_map8x4, 13, 1, 31, _pos2ctx_last8x4, 13, _read_significant_coefficients13
_read_significance_map13 ENDP
_read_significance_map14 PROC
SigMapFunction _pos2ctx_map8x4, 14, 1, 31, _pos2ctx_last8x4, 14, _read_significant_coefficients14
_read_significance_map14 ENDP
_read_significance_map15 PROC
SigMapFunction _pos2ctx_map4x4, 15, 1, 15, _pos2ctx_last4x4, 15, _read_significant_coefficients15
_read_significance_map15 ENDP
_read_significance_map16 PROC
SigMapFunction _pos2ctx_map4x4, 16, 1, 15, _pos2ctx_last4x4, 16, _read_significant_coefficients16
_read_significance_map16 ENDP
_read_significance_map17 PROC
SigMapFunction _pos2ctx_map4x4, 17, 0, 14, _pos2ctx_last4x4, 17, _read_significant_coefficients17
_read_significance_map17 ENDP
_read_significance_map18 PROC
SigMapFunction _pos2ctx_map8x8, 18, 1, 63, _pos2ctx_last8x8, 18, _read_significant_coefficients18
_read_significance_map18 ENDP
_read_significance_map19 PROC
SigMapFunction _pos2ctx_map8x4, 19, 1, 31, _pos2ctx_last8x4, 19, _read_significant_coefficients19
_read_significance_map19 ENDP
_read_significance_map20 PROC
SigMapFunction _pos2ctx_map8x4, 20, 1, 31, _pos2ctx_last8x4, 20, _read_significant_coefficients20
_read_significance_map20 ENDP
_read_significance_map21 PROC
SigMapFunction _pos2ctx_map4x4, 21, 1, 15, _pos2ctx_last4x4, 21, _read_significant_coefficients21
_read_significance_map21 ENDP
_TEXT ENDS


_TEXT	SEGMENT
; edx: dep - unchanged by function
; SF holds the return value
_biari_decode_symbol_eq_prob_asm PROC
	mov ecx, DWORD PTR [edx+8]; dep->DbitsLeft
	dec ecx ; dep->DbitsLeft--
	mov	eax, DWORD PTR [edx+4] ; eax: dep->DValue
	push	esi
	jnz	SHORT $LN3@biari_deco; if(--(dep->DbitsLeft) == 0)  

	mov	ecx, DWORD PTR [edx+16] ; ebp: dep->Dcodestrm_len
	mov	esi, DWORD PTR [ecx] ; esi: *dep->Dcodestrm_len
	add	DWORD PTR [ecx], 2 ; *dep->Dcodestrm_len += 2
	mov	ecx, DWORD PTR [edx+12] ; ebp: dep->Dcodestrm
	shl eax, 16
	mov	ax, WORD PTR [ecx+esi]  ; value = (value << 16) | getword( dep )
	xchg ah, al
	mov	ecx, 16			; dep->DbitsLeft = 16;
$LN3@biari_deco:
	mov	esi, DWORD PTR [edx] ; dep->Drange 
	shl	esi, cl ; (dep->Drange << dep->DbitsLeft)
	mov DWORD PTR [edx+8], ecx
	mov	ecx, eax
	sub	ecx, esi
	pop	esi
	
	cmovns eax, ecx ; if (tmp_value <0) value = tmp_value
	mov	DWORD PTR [edx+4], eax ; dep->Dvalue = value;
	ret	0
_biari_decode_symbol_eq_prob_asm ENDP
_TEXT	ENDS

_TEXT	SEGMENT
; edx: dep.  retained on return
; esi and ebp are NOT retained, because the (only) calling function doesn't need them to be
_exp_golomb_decode_eq_prob0 PROC
STACKOFFSET=0
	xor esi, esi ; esi: binary_symbol
	xor	ebp, ebp ; ebp: symbol
	push	edi
	mov edi, 1 ; edi: k
DECODE_EQ@LOOP_AGAIN:
	call	_biari_decode_symbol_eq_prob_asm ; l = biari_decode_symbol_eq_prob(dep_dp);
	js	SHORT DECODE_EQ@LOOP_DONE
	add	ebp, edi ; symbol += k
	shl edi, 1 ; k <<= 1
	jmp	SHORT DECODE_EQ@LOOP_AGAIN
	align 16
DECODE_EQ@LOOP_DONE:
	shr edi, 1
	jz	SHORT DECODE_EQ@RETURN
	call	_biari_decode_symbol_eq_prob_asm ; if (biari_decode_symbol_eq_prob(dep_dp)==1)
	js	SHORT DECODE_EQ@LOOP_DONE
	or	esi, edi ; binary_symbol |= (1<<k);
	jmp SHORT DECODE_EQ@LOOP_DONE
	align 16
DECODE_EQ@RETURN:
	lea	eax, DWORD PTR [esi+ebp+13] ; return (unsigned int) (symbol + binary_symbol);
	pop	edi
	ret	0
_exp_golomb_decode_eq_prob0 ENDP
_TEXT	ENDS

;
;
; pass dep in edx, context in eax
; edx is retained on return
; ebp is destroyed

_TEXT	SEGMENT
ctx = 4 ; second parameter
_unary_exp_golomb_level_decode PROC	
	STACKOFFSET=0
	mov ebp, eax ; eax (and now ebp also) contains the context pointer
	call	_biari_decode_symbol_map
	test	eax, eax ; if (symbol==0)
	jne	SHORT SYMBOL_NOT_ZERO
	ret	0
align 16
SYMBOL_NOT_ZERO:
	push	esi
	xor	esi, esi
LEVEL_DECODE@LOOP_AGAIN:

	mov eax, ebp ; _biari_decode_symbol_map wants ctx in eax
	inc	esi ; ++symbol;
	call	_biari_decode_symbol_map ; l = biari_decode_symbol(dep_dp, ctx);

	test	eax, eax ; if (!l)
	je	SHORT LEVEL_IS_ZERO
	cmp	esi, 12				; exp_start-1
	jb	SHORT LEVEL_DECODE@LOOP_AGAIN

	call _exp_golomb_decode_eq_prob0 ; exp_golomb_decode_eq_prob(dep_dp,0)
	pop	esi
	ret	0
align 16
LEVEL_IS_ZERO:
	mov	eax, esi ; return symbol;
	pop	esi
	ret	0
_unary_exp_golomb_level_decode ENDP
_TEXT	ENDS

CONST SEGMENT
sigmap_functions DD FLAT:_read_significance_map0
DD FLAT:_read_significance_map1
DD FLAT:_read_significance_map2
DD FLAT:_read_significance_map3
DD FLAT:_read_significance_map4
DD FLAT:_read_significance_map5
DD FLAT:_read_significance_map6
DD FLAT:_read_significance_map7
DD FLAT:_read_significance_map8
DD FLAT:_read_significance_map9
DD FLAT:_read_significance_map10
DD FLAT:_read_significance_map11
DD FLAT:_read_significance_map12
DD FLAT:_read_significance_map13
DD FLAT:_read_significance_map14
DD FLAT:_read_significance_map15
DD FLAT:_read_significance_map16
DD FLAT:_read_significance_map17
DD FLAT:_read_significance_map18
DD FLAT:_read_significance_map19
DD FLAT:_read_significance_map20
DD FLAT:_read_significance_map21
CONST ENDS

PUBLIC	_readRunLevel_CABAC
_TEXT	SEGMENT
_currMB$ = 4 ; first parameter
_dep_dp$ = 8 ; second parameter
_context$ = 12 ; third parameter
_readRunLevel_CABAC PROC
	push	esi
	push	edi
STACKOFFSET=8
	mov	edi, DWORD PTR _currMB$[esp+STACKOFFSET] ; edi: currMB
	mov	esi, DWORD PTR [edi] ; esi: currSlice = currMB->p_Slice;

	cmp	DWORD PTR [esi+coeff_ctr@Slice], 0 ; if (currSlice->coeff_ctr >= 0)
	jge	SHORT SET_RUN_AND_LEVEL

; ===== decode CBP-BIT =====
	mov	eax, DWORD PTR [edi+read_and_store_CBP_block_bit@Macroblock] ; eax: currMB->read_and_store_CBP_block_bit
	push	ebx
STACKOFFSET=STACKOFFSET+4
	mov	ebx, DWORD PTR _context$[esp+STACKOFFSET] ; ebx: context
	push	ebp
STACKOFFSET=STACKOFFSET+4
	mov	ebp, DWORD PTR _dep_dp$[esp+STACKOFFSET] ; ebp: dep
	push	ebx ; context
	push	ebp ; dep
	push	edi ; currMB
	call	eax ; currMB->read_and_store_CBP_block_bit(currMB, dep_dp, context)
	add	esp, 12
	mov	DWORD PTR [esi+coeff_ctr@Slice], eax ; currSlice->coeff_ctr = return value
	test	eax, eax ; if (currSlice->coeff_ctr == 0)
	je	SHORT SET_RUN_AND_LEVEL_POP

; ===== decode significance coefficients =====
	mov	ecx, DWORD PTR [esi+tex_ctx@Slice] ; ecx:  currSlice->tex_ctx
	lea	eax, DWORD PTR [esi+coeff@Slice] ; eax: currSlice->coeff
	;push	eax ; currSlice->coeff
	;push	ecx ; tex_ctx
	;call	_read_significance_map ; read_significance_map(currSlice->tex_ctx, currMB, dep_dp, context, currSlice->coeff);
	call sigmap_functions[ebx*4]
SET_RUN_AND_LEVEL_POP:
	pop	ebp
	pop	ebx
STACKOFFSET=STACKOFFSET-8
SET_RUN_AND_LEVEL:

; --- set run and level ---
	xor	edx, edx ; edx: 0

	dec DWORD PTR [esi+coeff_ctr@Slice] ; if (currSlice->coeff_ctr--)
	js 	SHORT EOB

; --- set run and level (coefficient) ---
	mov	ecx, DWORD PTR [esi+pos@Slice] ; ecx: currSlice->pos
	xor edi, edi ; edi: run=0
	cmp	WORD PTR [esi+ecx*2+coeff@Slice], dx ; currSlice->coeff[currSlice->pos] == 0
	jne	SHORT LOOP_END
LOOP_ITR:
	cmp	WORD PTR [esi+ecx*2+1+coeff@Slice], dx ; currSlice->coeff[currSlice->pos] == 0
	lea ecx, [ecx+1]
	lea edi, [edi+1]
	je	SHORT LOOP_ITR
LOOP_END:
	movsx eax, WORD PTR [esi+ecx*2+coeff@Slice] ; eax: value = currSlice->coeff[currSlice->pos]
	inc	ecx ; currSlice->pos++

; --- decrement coefficient counter and re-set position ---

	;cmp	DWORD PTR [esi+coeff_ctr@Slice], edx ; if (currSlice->coeff_ctr == 0) 
	;cmove ecx, edx ; currSlice->pos = 0
	mov edx, edi
	pop	edi
	mov	DWORD PTR [esi+pos@Slice], ecx ; store currSlice->pos
	pop	esi
	ret	0 ; eax contains value
	align 16
EOB:
	xor	eax, eax ; return 0
	mov	DWORD PTR [esi+pos@Slice], edx ; currSlice->pos = 0;
	pop	edi
	pop	esi
	ret	0
_readRunLevel_CABAC ENDP
_TEXT	ENDS

;
; edi is not saved
; pass dep_dp in edx, retained on exit
; pass ctx in edi
; return value in esi

PUBLIC	_unary_exp_golomb_mv_decode3
_TEXT	SEGMENT
_ctx$ = 4 ; second parameter
_unary_exp_golomb_mv_decode3 PROC
STACKOFFSET=0
	mov eax, edi
	call	_biari_decode_symbol_map ; pass dep in edx and ctx in eax.  edx retains dep on exit
	test	eax, eax ; if (symbol)
	jne	SHORT SYMBOL_NOT_ZERO
	xor esi, esi
	ret	0
	align 16
SYMBOL_NOT_ZERO:
	push	ebp
STACKOFFSET=STACKOFFSET+4
	mov	ebp, 3
	add	edi, 4 ; ctx++
	mov	esi, 1 ; esi: symbol
LOOP_START:
	mov eax, edi
	call	_biari_decode_symbol_map ; pass dep in edx and ctx in eax.  edx retains dep on exit
	test	eax, eax
	je	SHORT SYMBOL_ZERO_RETURN

	inc	esi
	cmp	esi, 2 ; if (symbol == 2)
	sete al ; eax will be 1, so this is safe to do
	lea edi, [edi + eax*4] ; ctx += (symbol == 2)

	cmp	esi, ebp ; if (symbol == max_bin)
	sete al ; eax will have nothing set high, so this is safe to do
	lea edi, [edi + eax*4] ; ctx += (symbol != max_bin)

	cmp	esi, 8 ; if (symbol < exp_start)
	jb	SHORT LOOP_START

; return exp_start + exp_golomb_decode_eq_prob(dep_dp,3);
	xor	ebp, ebp ; ebp: symbol
	mov	edi, ebp ; edi: binary_symbol
DECODE_EQ3@LOOP1:
	call	_biari_decode_symbol_eq_prob_asm ; edx holds dep_dp
	js	SHORT DECODE_EQ3@LOOP2
	or ebp, esi; symbol += (l<<k)
	shl esi, 1 ; k <<= 1
	jmp	SHORT DECODE_EQ3@LOOP1
	align 16
DECODE_EQ3@LOOP2:
	shr esi, 1
	jz	SHORT DECODE_EQ3@RETURN
	call	_biari_decode_symbol_eq_prob_asm
	js	SHORT DECODE_EQ3@LOOP2
	or	edi, esi ; binary_symbol |= (1<<k);
	jmp	SHORT DECODE_EQ3@LOOP2
	align 16
DECODE_EQ3@RETURN:
	; return (unsigned int) (symbol + binary_symbol);
	lea esi, [edi+ebp+8]
	pop	ebp
	ret	0
	align 16
SYMBOL_ZERO_RETURN:
	; return symbol is in esi
	pop	ebp
	ret	0
_unary_exp_golomb_mv_decode3 ENDP
_TEXT	ENDS

_TEXT	SEGMENT
_unary_bin_decode1 PROC
; _ctx$ = eax
; _dep_dp$ = edx
	push edi
	mov	edi, eax
	call	_biari_decode_symbol_map ; biari_decode_symbol(dep_dp, ctx );
	test	eax, eax ; if (symbol)
	jne	SHORT $LN5@unary_bin_@2
	mov eax, 2
	shr	eax, 1
	pop	edi
	ret	0
align 16
$LN5@unary_bin_@2:
	xor	esi, esi ; symbol = 0;
$LL3@unary_bin_@2:
	inc	esi ; ++symbol;
	lea	eax, DWORD PTR [edi+4] ; ctx + ctx_offset
	call	_biari_decode_symbol_map ; biari_decode_symbol(dep_dp, ctx);
	test	eax, eax ; while( l != 0 );
	jne	SHORT $LL3@unary_bin_@2
	lea eax, [esi + 2]; return symbol+2;
	shr	eax, 1
	pop edi
	ret	0
_unary_bin_decode1 ENDP
_TEXT	ENDS


PUBLIC	_readDquant_CABAC
_TEXT	SEGMENT
_currSlice$ = 4 ; first parameter
_dep_dp$ = 8 ; second parameter
_readDquant_CABAC PROC
STACKOFFSET=0
; 815  : 	MotionInfoContexts *ctx = currSlice->mot_ctx;
; 816  : 	short dquant;
; 817  : 	int act_ctx = ((currSlice->last_dquant != 0) ? 1 : 0);
; 818  : 	int act_sym = biari_decode_symbol(dep_dp,ctx->delta_qp_contexts + act_ctx );

	mov	edx, DWORD PTR _dep_dp$[esp+STACKOFFSET]
	push	esi
	push	edi
STACKOFFSET = STACKOFFSET + 8
	mov	edi, DWORD PTR _currSlice$[esp+STACKOFFSET]
	mov	esi, DWORD PTR [edi+mot_ctx@Slice]
	xor	eax, eax
	cmp	DWORD PTR [edi+last_dquant@Slice], eax
	setne	al
	lea	eax, DWORD PTR [esi+eax*4+332]
	; pass dep in edx and ctx in eax.  edx retains dep on exit
	call	_biari_decode_symbol_map

	test	eax, eax ; if (!act_sym)
	jz	SHORT $LN2@readDquant

	lea	eax, DWORD PTR [esi+340] ; unary_bin_decode(dep_dp,ctx->delta_qp_contexts + 2,1);
	call	_unary_bin_decode1
	
	jnc	SHORT $LN2@readDquant ; lsb is signed bit

	neg	eax ; dquant = -dquant;
	movzx	eax, ax
$LN2@readDquant:
	movsx	edx, ax
	mov	DWORD PTR [edi+last_dquant@Slice], edx ; currSlice->last_dquant = dquant;
	pop	edi
	pop	esi
	;mov	ax, cx ; return dquant;
	ret	0
_readDquant_CABAC ENDP
_TEXT	ENDS

PUBLIC	_readIntraPredMode_CABAC
_TEXT	SEGMENT
_currSlice$ = 4 ; first parameter
_dep_dp$ = 8 ; second parameter
_readIntraPredMode_CABAC PROC
; 720  : 	TextureInfoContexts *ctx     =
STACKOFFSET=0
	mov	eax, DWORD PTR _currSlice$[esp + STACKOFFSET]
	push	esi
	mov	esi, DWORD PTR [eax+100] ;  currSlice->tex_ctx;
STACKOFFSET=4
; 721  : 	int act_sym;
; 722  : 
; 723  : 	// use_most_probable_mode
; 724  : 	act_sym = biari_decode_symbol(dep_dp, ctx->ipr_contexts);

	mov	edx, DWORD PTR _dep_dp$[esp+STACKOFFSET]
	lea	eax, DWORD PTR [esi+12]
	call	_biari_decode_symbol_map

	; remaining_mode_selector
	test	eax, eax ; if (act_sym == 0)
	jz	SHORT $LN2@readIntraP

	or	eax, -1 ; return -1;
	pop	esi
	ret	0
align 16
$LN2@readIntraP:
	push	ebx
	add	esi, 16					; 00000010H
	mov eax, esi
	call	_biari_decode_symbol_map
	mov	ebx, eax
; 735  : 		pred_mode |= (biari_decode_symbol(dep_dp, ctx->ipr_contexts+1) << 1);

	mov eax, esi
	call	_biari_decode_symbol_map
	lea ebx, [ebx+2*eax]
; 736  : 		pred_mode |= (biari_decode_symbol(dep_dp, ctx->ipr_contexts+1) << 2);

	mov eax, esi
	call	_biari_decode_symbol_map
	lea eax, [ebx+4*eax] ; return pred_mode;

	pop	ebx
	pop	esi
	ret	0
_readIntraPredMode_CABAC ENDP
_TEXT	ENDS

PUBLIC	_readMB_skip_flagInfo_CABAC
_TEXT	SEGMENT
_currMB$ = 4 ; first parameter
_dep_dp$ = 12						; size = 4
_readMB_skip_flagInfo_CABAC PROC

; 406  : 	Slice *currSlice = currMB->p_Slice;
STACKOFFSET=0
	mov	ecx, DWORD PTR _currMB$[esp + STACKOFFSET]
	push	ebp

	xor	eax, eax
	push	esi
	mov	esi, DWORD PTR [ecx + p_Slice@Macroblock] ; esi: currSlice
	cmp	DWORD PTR [esi+slice_type@Slice], 1 ; int bframe=(currSlice->slice_type == B_SLICE);
	push	edi

	mov	edi, DWORD PTR [esi+mot_ctx@Slice] ; edi: ctx = currSlice->mot_ctx;  
	sete	al ; int bframe=(currSlice->slice_type == B_SLICE);

; 409  : 	int a = (currMB->mb_left != NULL) ? (currMB->mb_left->skip_flag == 0) : 0;

	xor	edx, edx
	mov	ebp, eax
	mov	eax, DWORD PTR [ecx+104]
	test	eax, eax
	je	SHORT READ_B
	cmp	DWORD PTR [eax+348], edx
	sete	dl

; 410  : 	int b = (currMB->mb_up   != NULL) ? (currMB->mb_up  ->skip_flag == 0) : 0;

READ_B:
	mov	ecx, DWORD PTR [ecx+100]
	xor	eax, eax
	test	ecx, ecx
	je	SHORT $LN9@readMB_ski
	cmp	DWORD PTR [ecx+348], eax
	sete	al
$LN9@readMB_ski:

; 414  : 	if (bframe)
; 415  : 	{
; 416  : 		act_ctx = 7 + a + b;
; 418  : 		skip = biari_decode_symbol (dep_dp, &ctx->mb_type_contexts[2][act_ctx]);

	add	eax, edx
	test	ebp, ebp
	mov	edx, DWORD PTR _dep_dp$[esp+8]
	je	SHORT $LN3@readMB_ski
	lea	eax, DWORD PTR [edi+eax*4+116]
	jmp	SHORT $LN11@readMB_ski
align 16
$LN3@readMB_ski:

; 422  : 		act_ctx = a + b;
; 424  : 		skip = biari_decode_symbol(dep_dp, &ctx->mb_type_contexts[1][act_ctx]);

	lea	eax, DWORD PTR [edi+eax*4+44]
$LN11@readMB_ski:
	call	_biari_decode_symbol_map

	test	eax, eax ; if (!skip)
	je	SHORT $LN1@readMB_ski

; 429  : 		currSlice->last_dquant = 0;

	mov	DWORD PTR [esi + last_dquant@Slice], 0
$LN1@readMB_ski:
	pop	edi
	pop	esi
	pop	ebp
	ret	0
_readMB_skip_flagInfo_CABAC ENDP
_TEXT	ENDS


PUBLIC	_set_chroma_qp
_TEXT	SEGMENT
_currMB$ = 4 ; first parameter
_set_chroma_qp PROC
	mov	eax, DWORD PTR _currMB$[esp] ; eax: currMB
	mov	ecx, DWORD PTR [eax+4] ; ecx: currMB->p_Vid
	mov	edx, DWORD PTR [ecx+bitdepth_chroma_qp_scale@VideoParameters] ; edx: p_Vid->bitdepth_chroma_qp_scale;
	push	edi
	mov	edi, DWORD PTR [ecx+dec_picture@VideoParameters] ; edi: p_Vid->dec_picture
	mov	ecx, DWORD PTR [edi+chroma_qp_offset@StorablePicture] ; ecx: dec_picture->chroma_qp_offset[0]
	add	ecx, DWORD PTR [eax+qp@macroblock] ; ecx: dec_picture->chroma_qp_offset[0] + currMB->qp
	neg	edx ; edx: -p_Vid->bitdepth_chroma_qp_scale;
	cmp	ecx, edx
	cmovl ecx, edx
	cmp	ecx, 51
	cmovg ecx, DWORD PTR _51 ; cmov doesn't allow for immediates
	test	ecx, ecx
	cmovge	ecx, DWORD PTR _QP_SCALE_CR[ecx*4]
	mov	DWORD PTR [eax+64], ecx

	sub ecx, edx; currMB->qpc[0] + p_Vid->bitdepth_chroma_qp_scale;
	mov	DWORD PTR [eax+qp_scaled@Macroblock + 4], ecx ; currMB->qp_scaled[1]
	mov	ecx, DWORD PTR [edi+chroma_qp_offset@StorablePicture + 4]
	add	ecx, DWORD PTR [eax+qp@macroblock]
	cmp	ecx, edx
	cmovl ecx, edx
	cmp	ecx, 51
	cmovg ecx, DWORD PTR _51 ; cmov doesn't allow for immediates
	test	ecx, ecx
	cmovge	ecx, DWORD PTR _QP_SCALE_CR[ecx*4]
	mov	DWORD PTR [eax+64+4], ecx
	sub ecx, edx
	pop	edi
	mov	DWORD PTR [eax+72 + 8], ecx
	ret	0
_set_chroma_qp ENDP
_TEXT	ENDS

PUBLIC	_decodeMVD_CABAC
_TEXT	SEGMENT
_dep_dp$ = 4 ; first parameter
_mv_ctx$ = 8 ; second parameter
_act_ctx$ = 12; third parameter
_err$ = 16 ; 4th parameter
_decodeMVD_CABAC PROC
STACKOFFSET = 0
	mov	eax, DWORD PTR _act_ctx$[esp+STACKOFFSET]
	push	edi
STACKOFFSET = STACKOFFSET + 4
	mov	edi, DWORD PTR _mv_ctx$[esp+STACKOFFSET]
	lea edi, [edi+eax*4] ; mv_ctx[0][act_ctx]
	mov	eax, DWORD PTR _err$[esp+STACKOFFSET]
	lea	eax, DWORD PTR [edi+eax*4] ; &mv_ctx[0][act_ctx+err]
	mov	edx, DWORD PTR _dep_dp$[esp+STACKOFFSET]
	call	_biari_decode_symbol_map ; int act_sym = biari_decode_symbol(dep_dp,&mv_ctx[0][act_ctx+err] );

	test	eax, eax ; if (act_sym != 0)
	je	SHORT SYMBOL_ZERO
	push	esi
STACKOFFSET = STACKOFFSET + 4
	lea	edi, [edi + 40] ; mv_ctx[1]+act_ctx
	call	_unary_exp_golomb_mv_decode3 ; act_sym = unary_exp_golomb_mv_decode3(dep_dp,mv_ctx[1]+act_ctx);
	inc	esi ; ++act_sym;
	call	_biari_decode_symbol_eq_prob_asm ; mv_sign = biari_decode_symbol_eq_prob(dep_dp);
	js	SHORT SKIP_NEGATE; if(mv_sign)
	neg	esi ; act_sym = -act_sym;
SKIP_NEGATE:
	mov	eax, esi
	pop	esi
SYMBOL_ZERO:
	pop	edi
	ret	0
_decodeMVD_CABAC ENDP
_TEXT	ENDS

END

