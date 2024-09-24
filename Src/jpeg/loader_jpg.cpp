#include "main.h"

#include "loader_jpg.h"

#include "api__jpeg.h"
#include "../xml/ifc_xmlreaderparams.h"
#include <api/memmgr/api_memmgr.h>
#include <shlwapi.h>
#include <setjmp.h>

#include <wingdi.h>
#include <intsafe.h>

/*BIG BIG THING TO NOTE
I have modified jmorecfg.h line 319 to specify 4 bytes per pixel with RGB. it is normally three.
*/
extern "C"
{
#undef FAR
#include "jpeglib.h"
};

int JpgLoad::isMine( const wchar_t *filename )
{
	if ( !filename )
		return 0;

	const wchar_t *ext = PathFindExtensionW( filename );

	if ( !ext )
		return 0;

	if ( !_wcsicmp( ext, L".jpg" ) )
		return 1;

	if ( !_wcsicmp( ext, L".jpeg" ) )
		return 1;

	return 0;
}

const wchar_t *JpgLoad::mimeType()
{
	return L"image/jpeg";
}

int JpgLoad::getHeaderSize()
{
	return 3;
}

int JpgLoad::testData( const void *data, int datalen )
{
	if ( datalen < 3 )
		return 0;

	const unsigned __int8 *text = static_cast<const unsigned __int8 *>( data );
	if ( text[ 0 ] == 0xFF && text[ 1 ] == 0xD8 && text[ 2 ] == 0xFF )
		return 1;

	return 0;
}
/*
struct jpeg_source_mgr {
  const JOCTET * next_input_byte; // => next byte to read from buffer
  size_t bytes_in_buffer;	// # of bytes remaining in buffer

  JMETHOD(void, init_source, (j_decompress_ptr cinfo));
  JMETHOD(boolean, fill_input_buffer, (j_decompress_ptr cinfo));
  JMETHOD(void, skip_input_data, (j_decompress_ptr cinfo, long num_bytes));
  JMETHOD(boolean, resync_to_restart, (j_decompress_ptr cinfo, int desired));
  JMETHOD(void, term_source, (j_decompress_ptr cinfo));
};
*/

// our reader...
extern "C"
{
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
};

static void wasabi_jpgload_error_exit( j_common_ptr cinfo )
{
	jmp_buf *stack_env = (jmp_buf *)cinfo->client_data;
	longjmp( *stack_env, 1 );
}

static bool IsAMG( jpeg_saved_marker_ptr marker_list )
{
	while ( marker_list )
	{
		if ( marker_list->marker == JPEG_COM && marker_list->data_length == 7 && memcmp( (const char *)marker_list->data, "AMG/AOL", 7 ) == 0 )
		{
			return true;
		}

		marker_list = marker_list->next;
	}

	return false;
}

ARGB32 *JpgLoad::loadImage( const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params )
{
	int fail_on_amg = 0;

	if ( params ) // epic failness
		fail_on_amg = params->getItemValueInt( L"AMG", 0 );

	ARGB32 *buf = 0;
	jpeg_error_mgr jerr;
	jpeg_decompress_struct cinfo;
	jpeg_source_mgr src = { (const JOCTET *)data,(size_t)datalen,init_source,fill_input_buffer,skip_input_data,jpeg_resync_to_restart,term_source };

	cinfo.err = jpeg_std_error( &jerr );

	jpeg_create_decompress( &cinfo );
	cinfo.src = &src;

	/* set up error handling.  basically C style exceptions :) */
	jmp_buf stack_env;
	cinfo.client_data = &stack_env;
	cinfo.err->error_exit = wasabi_jpgload_error_exit;
	if ( setjmp( stack_env ) )
	{
		// longjmp will goto here
		jpeg_destroy_decompress( &cinfo );
		if ( buf )
			WASABI_API_MEMMGR->sysFree( buf );

		return 0;
	}

	if ( fail_on_amg )
		jpeg_save_markers( &cinfo, JPEG_COM, 10 );

	if ( jpeg_read_header( &cinfo, TRUE ) == JPEG_HEADER_OK )
	{
		cinfo.out_color_space = JCS_RGB;
		/*int ret = */jpeg_start_decompress( &cinfo );
		if ( !fail_on_amg || !IsAMG( cinfo.marker_list ) )
		{
			size_t image_size = 0;
			if ( SizeTMult( cinfo.output_width, cinfo.output_height, &image_size ) == S_OK && SizeTMult( image_size, 4, &image_size ) == S_OK )
			{
				buf = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc( image_size );
				int row_stride = cinfo.output_width * cinfo.output_components;

				ARGB32 *p = buf;// + (cinfo.output_width * cinfo.output_height);

				void* line = malloc(row_stride);

				while ( cinfo.output_scanline < cinfo.output_height )
				{
					//p -= cinfo.output_width;
					jpeg_read_scanlines( &cinfo, (JSAMPARRAY)&line, 1 );

					unsigned char* rgb = (unsigned char*)line;
					unsigned char* argb = (unsigned char*)p;
					for (size_t i = 0; i < cinfo.output_width; i++)
					{
						argb[4 * i]     = rgb[3 * i + 2];
						argb[4 * i + 1] = rgb[3 * i + 1];
						argb[4 * i + 2] = rgb[3 * i];
						argb[4 * i + 3] = 0xff;
					}


					p += cinfo.output_width;
				}
				free(line);

				if ( w )
					*w = cinfo.output_width;

				if ( h )
					*h = cinfo.output_height;

				jpeg_finish_decompress( &cinfo );
			}
		}
	}

	jpeg_destroy_decompress( &cinfo );

	return buf;
}

#define CBCLASS JpgLoad
START_DISPATCH;
CB( ISMINE, isMine );
CB( MIMETYPE, mimeType );
CB( TESTDATA, testData );
CB( GETHEADERSIZE, getHeaderSize );
CB( LOADIMAGE, loadImage );
END_DISPATCH;
#undef CBCLASS