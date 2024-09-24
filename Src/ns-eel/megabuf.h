#ifndef _MEGABUF_H_
#define _MEGABUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MEGABUF_BLOCKS 64
#define MEGABUF_ITEMSPERBLOCK 16384

void _asm_megabuf(void);
void _asm_megabuf_end(void);
void megabuf_ppproc(void *data, int data_size, void **userfunc_data);
void megabuf_cleanup(NSEEL_VMCTX);

#ifdef __cplusplus
}
#endif


#endif