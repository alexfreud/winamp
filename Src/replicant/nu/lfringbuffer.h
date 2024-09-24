#pragma once
#include "foundation/types.h"
/* lock free floating point ring buffer
  generic implementation
		
	*/

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct lfringbuffer_s { } *lfringbuffer_t;

	/* create a ring buffer with the desired number of elements */
	int lfringbuffer_create(lfringbuffer_t *out_ring_buffer, size_t number_of_elements);
	int lfringbuffer_destroy(lfringbuffer_t ring_buffer);

	/* ----- Read functions ----- */
	/* get how many elements can currently be read */
	size_t lfringbuffer_read_available(lfringbuffer_t ring_buffer);

	/* retrieve a pointer that can be read from.  
	   you might have to call this twice, because of ring buffer wraparound
		 call lfringbuffer_read_update() when you are done */
	int lfringbuffer_read_get(lfringbuffer_t ring_buffer, size_t elements_requested, const float **out_buffer, size_t *elements_available);

	/* call to acknowledge that you have read the bytes */
	void lfringbuffer_read_update(lfringbuffer_t ring_buffer, size_t elements_read);

	/* ----- Write functions ----- */
		/* get how many elements can currently be written */
	size_t lfringbuffer_write_available(lfringbuffer_t ring_buffer);

	/* retrieve a pointer that can be written to.  
	   you might have to call this twice, because of ring buffer wraparound
		 call lfringbuffer_write_update() when you are done */
	int lfringbuffer_write_get(lfringbuffer_t ring_buffer, size_t elements_requested, float **out_buffer, size_t *elements_available);

	/* call to acknowledge that you have written the bytes */
	void lfringbuffer_write_update(lfringbuffer_t ring_buffer, size_t elements_read);

	size_t lfringbuffer_write_get_position(lfringbuffer_t ring_buffer);

#ifdef __cplusplus
}
#endif
