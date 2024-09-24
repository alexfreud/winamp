#include "amg.h"
#include "api__jpeg.h"
#include <setjmp.h>
extern "C"
{
#undef FAR
#include "jpeglib.h"
// our reader...

	static void init_source( j_decompress_ptr cinfo )
	{}
	static const JOCTET jpeg_eof[] = { (JOCTET)0xFF, (JOCTET)JPEG_EOI };
	static boolean fill_input_buffer( j_decompress_ptr cinfo )
	{
		cinfo->src->next_input_byte = jpeg_eof;
		cinfo->src->bytes_in_buffer = 2;
		return TRUE;
	}
	static void skip_input_data( j_decompress_ptr cinfo, long num_bytes )
	{
		//my_src_ptr src = (my_src_ptr) cinfo->src;
		if ( num_bytes > 0 )
		{
			if ( num_bytes > (long)cinfo->src->bytes_in_buffer )
			{
				fill_input_buffer( cinfo );
			}
			else
			{
				cinfo->src->next_input_byte += (size_t)num_bytes;
				cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
			}
		}
	}
	static void term_source( j_decompress_ptr cinfo )
	{}


	typedef struct
	{
		jpeg_destination_mgr pub;
		uint8_t *buf;
		int len;
	} my_destination_mgr;

	void init_destination( j_compress_ptr cinfo );
	boolean empty_output_buffer( j_compress_ptr cinfo );
	void term_destination( j_compress_ptr cinfo );
	static void wasabi_jpgload_error_exit( j_common_ptr cinfo )
	{
		jmp_buf *stack_env = (jmp_buf *)cinfo->client_data;
		longjmp( *stack_env, 1 );
	}
};



int AMGSucks::WriteAlbumArt(const void *data, size_t data_len, void **out, int *out_len)
{
	*out = 0;
	jmp_buf stack_env;

	jpeg_decompress_struct cinfo_dec={0};
	jpeg_compress_struct cinfo={0};
	cinfo.client_data=&stack_env;
	cinfo_dec.client_data=&stack_env;

	if (setjmp(stack_env))
	{
		// longjmp will goto here
		jpeg_destroy_decompress(&cinfo_dec);
		jpeg_destroy_compress(&cinfo);
		return 1;
	}

	jpeg_error_mgr jerr;
	jpeg_source_mgr src = {(const JOCTET *)data,data_len,init_source,fill_input_buffer,skip_input_data,jpeg_resync_to_restart,term_source};
	cinfo_dec.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo_dec);
	cinfo_dec.err->error_exit = wasabi_jpgload_error_exit;
	cinfo_dec.src = &src;

	if (jpeg_read_header(&cinfo_dec, TRUE) == JPEG_HEADER_OK)
	{
		cinfo.err = cinfo_dec.err;
		jpeg_create_compress(&cinfo);
		cinfo.err->error_exit = wasabi_jpgload_error_exit;

		my_destination_mgr dest = {{0,65536,init_destination,empty_output_buffer,term_destination},0,65536};
		dest.buf = (uint8_t *)WASABI_API_MEMMGR->sysMalloc(65536);
		dest.pub.next_output_byte = dest.buf;
		cinfo.dest = (jpeg_destination_mgr*)&dest;
		cinfo.image_width = cinfo_dec.image_width;
		cinfo.image_height = cinfo_dec.image_height;
		cinfo.input_components = 4;
		cinfo.in_color_space = JCS_RGB;

		jpeg_copy_critical_parameters(&cinfo_dec, &cinfo);
		jvirt_barray_ptr *stuff = jpeg_read_coefficients(&cinfo_dec);
		jpeg_write_coefficients(&cinfo, stuff);
		const char *blah = "AMG/AOL";
		jpeg_write_marker(&cinfo, JPEG_COM, (JOCTET *)blah, (unsigned int)strlen(blah));

		jpeg_finish_compress(&cinfo);

		*out = dest.buf;
		*out_len = dest.len - (int)dest.pub.free_in_buffer;

		jpeg_destroy_decompress(&cinfo_dec);
		jpeg_destroy_compress(&cinfo);

		return 0;
	}
	return 1;
}

#define CBCLASS AMGSucks
START_DISPATCH;
CB(WRITEALBUMART, WriteAlbumArt);
END_DISPATCH;
#undef CBCLASS