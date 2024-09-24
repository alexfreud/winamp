#include <windows.h>
#include "resource.h"
#include "..\..\..\winamp\dsp.h"
#include "sps_common.h"

void NSEEL_HOSTSTUB_EnterMutex() {}
void NSEEL_HOSTSTUB_LeaveMutex() {}

extern winampDSPModule mod;

char *SPSHELP_gethelptext( int sel )
{
	int help_id[] = { IDR_GENERAL,IDR_OPERATORS,IDR_FUNCTIONS,IDR_CONSTANTS };
	void *data = 0;

	HRSRC rsrc = FindResource( WASABI_API_LNG_HINST, MAKEINTRESOURCE( help_id[ sel ] ), "TEXT" );
	if ( rsrc )
	{
		HGLOBAL resourceHandle = LoadResource( WASABI_API_LNG_HINST, rsrc );
		data = LockResource( resourceHandle );

		return (char *) data;
	}

	return 0;
}

void SPS_initcontext( SPSEffectContext *ctx )
{
	memset( ctx, 0, sizeof( SPSEffectContext ) );
	InitializeCriticalSection( &ctx->code_cs );
	ctx->vm_ctx = NSEEL_VM_alloc();
	ctx->code_needrecompile = 7;
}

void SPS_quitcontext( SPSEffectContext *ctx )
{
	if ( ctx->sample_buffer ) GlobalFree( ctx->sample_buffer );
	ctx->sample_buffer_len = 0;
	ctx->sample_buffer = NULL;

	NSEEL_code_free( ctx->code[ 0 ] );
	NSEEL_code_free( ctx->code[ 1 ] );
	NSEEL_code_free( ctx->code[ 2 ] );
	memset( &ctx->code, 0, sizeof( ctx->code ) );

	//megabuf_cleanup( ctx->vm_ctx );
	NSEEL_VM_free( ctx->vm_ctx );

	ctx->vm_ctx = 0;
	memset( &ctx->vars, 0, sizeof( ctx->vars ) );

	DeleteCriticalSection( &ctx->code_cs );
}

static __inline int double2int( double v )
{
#if 0
	return (int) v;
#else
	int a;
	__asm
	{
		fld v;
		fistp a;
	}
	return a;
#endif
}

static __inline double int24todouble( unsigned char *int24 )
{
	unsigned int a = ( int24[ 0 ] ) | ( int24[ 1 ] << 8 ) | ( int24[ 2 ] << 16 );

	if ( a & 0x800000 )
		a |= 0xFF000000;
	else
		a &= 0xFFFFFF;

	return (double) ( ( (signed int) a ) + 0.5 ) / 8388607.5;
}

static __inline void doubletoint24( double v, unsigned char *int24 )
{
	v = ( v * 8388607.5 ) - 0.5;
	if ( v <= -8388608.0 )
	{
		int24[ 0 ] = int24[ 1 ] = 0; // 0x800000 is lowest possible value
		int24[ 2 ] = 0x80;
	}
	else if ( v >= 8388607.0 )
	{
		int24[ 0 ] = int24[ 1 ] = 0xff; // 0x7fffff is highest possible value
		int24[ 2 ] = 0x7f;
	}
	else
	{
		int a = (int) v;
		int24[ 0 ] = a & 0xff;
		int24[ 1 ] = ( a >> 8 ) & 0xff;
		int24[ 2 ] = ( a >> 16 ) & 0xff;
	}
}

int SPS_process_samples( SPSEffectContext *ctx,
						 void *samples, int numsamples,
						 int isfloat, int bps, int nch,
						 int srate, int maxout, int minout )
{
	// can only do 1 or 2ch for now
	if ( nch != 1 && nch != 2 )
		return numsamples;

	int samplepair_size = ( bps / 8 ) * nch;
	if ( ctx->bypass )
	{
		memset( ctx->triggers, 0, sizeof( ctx->triggers ) );

		return numsamples;
	}

	if ( !samples || numsamples < 1 )
		return numsamples;

	int rlen = numsamples * samplepair_size;

	if ( !ctx->sample_buffer || ctx->sample_buffer_len < rlen )
	{
		ctx->sample_buffer_len = rlen * 2;

		if ( ctx->sample_buffer )
			GlobalFree( ctx->sample_buffer );

		ctx->sample_buffer = (void *) GlobalAlloc( GMEM_FIXED, ctx->sample_buffer_len );
	}

	if ( !ctx->sample_buffer )
		return numsamples;

	int needinit = ctx->last_nch != nch || ctx->last_srate != srate;
	ctx->last_nch = nch;
	ctx->last_srate = srate;

	if ( ctx->code_needrecompile )
	{
		EnterCriticalSection( &ctx->code_cs );
		if ( ctx->code_needrecompile & 1 )
		{
			//NSEEL_VM_resetvars( ctx->vm_ctx );
			ctx->vars.spl0     = NSEEL_VM_regvar( ctx->vm_ctx, "spl0" );
			ctx->vars.spl1     = NSEEL_VM_regvar( ctx->vm_ctx, "spl1" );
			ctx->vars.skip     = NSEEL_VM_regvar( ctx->vm_ctx, "skip" );
			ctx->vars.repeat   = NSEEL_VM_regvar( ctx->vm_ctx, "repeat" );
			ctx->vars.nch      = NSEEL_VM_regvar( ctx->vm_ctx, "nch" );
			ctx->vars.srate    = NSEEL_VM_regvar( ctx->vm_ctx, "srate" );
			ctx->vars.sliders1 = NSEEL_VM_regvar( ctx->vm_ctx, "slider1" );
			ctx->vars.sliders2 = NSEEL_VM_regvar( ctx->vm_ctx, "slider2" );
			ctx->vars.sliders3 = NSEEL_VM_regvar( ctx->vm_ctx, "slider3" );
			ctx->vars.sliders4 = NSEEL_VM_regvar( ctx->vm_ctx, "slider4" );
			ctx->vars.trigger1 = NSEEL_VM_regvar( ctx->vm_ctx, "trig1" );
			ctx->vars.trigger2 = NSEEL_VM_regvar( ctx->vm_ctx, "trig2" );
			ctx->vars.trigger3 = NSEEL_VM_regvar( ctx->vm_ctx, "trig3" );
			ctx->vars.trigger4 = NSEEL_VM_regvar( ctx->vm_ctx, "trig4" );

			needinit = 1;
			NSEEL_code_free( ctx->code[ 0 ] );
			ctx->code[ 0 ] = NSEEL_code_compile( ctx->vm_ctx, ctx->curpreset.code_text[ 0 ], 0 );
		}

		if ( ctx->code_needrecompile & 2 )
		{
			NSEEL_code_free( ctx->code[ 1 ] );
			ctx->code[ 1 ] = NSEEL_code_compile( ctx->vm_ctx, ctx->curpreset.code_text[ 1 ], 0 );
#ifdef DEBUG
			if ( !ctx->code[ 1 ] && NSEEL_code_getcodeerror( ctx->vm_ctx ) && *NSEEL_code_getcodeerror( ctx->vm_ctx ) )
				OutputDebugString( NSEEL_code_getcodeerror( ctx->vm_ctx ) );
#endif
		}
		if ( ctx->code_needrecompile & 4 )
		{
			NSEEL_code_free( ctx->code[ 2 ] );
			ctx->code[ 2 ] = NSEEL_code_compile( ctx->vm_ctx, ctx->curpreset.code_text[ 2 ], 0 );
			ctx->sliderchange = 1;
		}
		ctx->code_needrecompile = 0;
		LeaveCriticalSection( &ctx->code_cs );
	}

	if ( !ctx->vars.spl0 )
		return numsamples;

	*( ctx->vars.nch ) = (double) nch;
	*( ctx->vars.srate ) = (double) srate;

	int slidech = ctx->sliderchange;
	ctx->sliderchange = 0;

	if ( ctx->triggers[ 0 ] )
	{
		ctx->triggers[ 0 ]--;
		*( ctx->vars.trigger1 ) = 1.0;
	}

	if ( ctx->triggers[ 1 ] )
	{
		ctx->triggers[ 1 ]--;
		*( ctx->vars.trigger2 ) = 1.0;
	}

	if ( ctx->triggers[ 2 ] )
	{
		ctx->triggers[ 2 ]--;
		*( ctx->vars.trigger3 ) = 1.0;
	}

	if ( ctx->triggers[ 3 ] )
	{
		ctx->triggers[ 3 ]--;
		*( ctx->vars.trigger4 ) = 1.0;
	}

	if ( needinit || slidech )
	{
		*( ctx->vars.sliders1 ) = (double) ctx->curpreset.sliderpos[ 0 ] / 1000.0;
		*( ctx->vars.sliders2 ) = (double) ctx->curpreset.sliderpos[ 1 ] / 1000.0;
		*( ctx->vars.sliders3 ) = (double) ctx->curpreset.sliderpos[ 2 ] / 1000.0;
		*( ctx->vars.sliders4 ) = (double) ctx->curpreset.sliderpos[ 3 ] / 1000.0;
	}

	if ( needinit )
	{
		//megabuf_cleanup( ctx->vm_ctx );
		NSEEL_code_execute( ctx->code[ 0 ] );
	}

	if ( needinit || slidech )
	{
		NSEEL_code_execute( ctx->code[ 2 ] );
	}


	if ( !maxout )
		maxout = numsamples;

	memcpy( ctx->sample_buffer, samples, rlen );
	int x = 0;
	int outpos = 0;

	while ( x < numsamples && outpos < maxout )
	{
		*( ctx->vars.skip ) = 0;
		*( ctx->vars.repeat ) = 0;

		if ( isfloat )
		{
			if ( bps == 32 )
			{
				float *sbuf_float = (float *) ctx->sample_buffer;
				if ( nch == 2 )
				{
					*( ctx->vars.spl0 ) = (double) sbuf_float[ x + x ];
					*( ctx->vars.spl1 ) = (double) sbuf_float[ x + x + 1 ];
				}
				else
				{
					*( ctx->vars.spl1 ) =
						*( ctx->vars.spl0 ) = (double) sbuf_float[ x ];
				}
			}
			else if ( bps == 64 )
			{
				double *sbuf_dbl = (double *) ctx->sample_buffer;
				if ( nch == 2 )
				{
					*( ctx->vars.spl0 ) = sbuf_dbl[ x + x ];
					*( ctx->vars.spl1 ) = sbuf_dbl[ x + x + 1 ];
				}
				else
				{
					*( ctx->vars.spl1 ) =
						*( ctx->vars.spl0 ) = sbuf_dbl[ x ];
				}
			}
			else
			{
				*( ctx->vars.spl1 ) =
					*( ctx->vars.spl0 ) = 0.0;
			}
		}
		else
		{
			if ( bps == 16 )
			{
				short *sbuf_short = (short *) ctx->sample_buffer;
				if ( nch == 2 )
				{
					*( ctx->vars.spl0 ) = ( sbuf_short[ x + x ] + 0.5 ) / 32767.5;
					*( ctx->vars.spl1 ) = ( sbuf_short[ x + x + 1 ] + 0.5 ) / 32767.5;
				}
				else
				{
					*( ctx->vars.spl1 ) =
						*( ctx->vars.spl0 ) = ( sbuf_short[ x ] + 0.5 ) / 32767.5;
				}
			}
			else if ( bps == 8 )
			{
				unsigned char *sbuf_char = (unsigned char *) ctx->sample_buffer;
				if ( nch == 2 )
				{
					*( ctx->vars.spl0 ) = ( sbuf_char[ x + x ] - 127.5 ) / 127.5;
					*( ctx->vars.spl1 ) = ( sbuf_char[ x + x + 1 ] - 127.5 ) / 127.5;
				}
				else
				{
					*( ctx->vars.spl1 ) =
						*( ctx->vars.spl0 ) = ( sbuf_char[ x ] - 127.5 ) / 127.5;
				}
			}
			else if ( bps == 24 )
			{
				unsigned char *sbuf_char = (unsigned char *) ctx->sample_buffer;
				if ( nch == 2 )
				{
					int x6 = ( x << 2 ) + x + x;
					*( ctx->vars.spl0 ) = int24todouble( sbuf_char + x6 );
					*( ctx->vars.spl1 ) = int24todouble( sbuf_char + x6 + 3 );
				}
				else
				{
					*( ctx->vars.spl1 ) =
						*( ctx->vars.spl0 ) = int24todouble( sbuf_char + x + x + x );
				}
			}
			else
			{
				// todo: 32 bit mode?
				*( ctx->vars.spl1 ) =
					*( ctx->vars.spl0 ) = 0.0;
			}
		}

		NSEEL_code_execute( ctx->code[ 1 ] );

		if ( *( ctx->vars.skip ) < 0.0001 || (/*samples out*/ outpos + /* samples left */ ( numsamples - x ) ) < minout )
		{
			if ( isfloat )
			{
				if ( bps == 32 )
				{
					float *samples_float = (float *) samples;
					if ( nch == 2 )
					{
						samples_float[ outpos + outpos ] = (float) *ctx->vars.spl0;
						samples_float[ outpos + outpos + 1 ] = (float) *ctx->vars.spl1;
					}
					else
					{
						samples_float[ outpos ] = (float) *ctx->vars.spl0;
					}
				}
				else if ( bps == 64 )
				{
					double *samples_double = (double *) samples;
					if ( nch == 2 )
					{
						samples_double[ outpos + outpos ] = *ctx->vars.spl0;
						samples_double[ outpos + outpos + 1 ] = *ctx->vars.spl1;
					}
					else
					{
						samples_double[ outpos ] = *ctx->vars.spl0;
					}
				}
			}
			else
			{
				if ( bps == 16 )
				{
					short *samples_short = (short *) samples;
					if ( nch == 2 )
					{
						double d = *( ctx->vars.spl0 ) * 32767.5 - 0.5;
						if ( d <= -32768.0 )
							samples_short[ outpos + outpos ] = -32768;
						else if ( d >= 32767.0 )
							samples_short[ outpos + outpos ] = 32767;
						else
							samples_short[ outpos + outpos ] = double2int( d );

						d = *( ctx->vars.spl1 ) * 32767.5 - 0.5;
						if ( d <= -32768.0 )
							samples_short[ outpos + outpos + 1 ] = -32768;
						else if ( d >= 32767.0 )
							samples_short[ outpos + outpos + 1 ] = 32767;
						else
							samples_short[ outpos + outpos + 1 ] = double2int( d );
					}
					else
					{
						double d = *( ctx->vars.spl0 ) * 32767.5 - 0.5;
						if ( d <= -32768.0 )
							samples_short[ outpos ] = -32768;
						else if ( d >= 32767.0 )
							samples_short[ outpos ] = 32767;
						else
							samples_short[ outpos ] = double2int( d );
					}
				}
				else if ( bps == 8 )
				{
					unsigned char *samples_char = (unsigned char *) samples;
					if ( nch == 2 )
					{
						double d = *( ctx->vars.spl0 ) * 127.5 + 127.5;
						if ( d <= 0.0 )
							samples_char[ outpos + outpos ] = 0;
						else if ( d >= 255.0 )
							samples_char[ outpos + outpos ] = 255;
						else
							samples_char[ outpos + outpos ] = double2int( d );

						d = *( ctx->vars.spl1 ) * 127.5 + 127.5;
						if ( d <= 0.0 )
							samples_char[ outpos + outpos + 1 ] = 0;
						else if ( d >= 255.0 )
							samples_char[ outpos + outpos + 1 ] = 255;
						else
							samples_char[ outpos + outpos + 1 ] = double2int( d );
					}
					else
					{
						double d = *( ctx->vars.spl0 ) * 127.5 + 127.5;
						if ( d <= 0.0 )
							samples_char[ outpos ] = 0;
						else if ( d >= 255.0 )
							samples_char[ outpos ] = 255;
						else
							samples_char[ outpos ] = double2int( d );
					}
				}
				else if ( bps == 24 )
				{
					unsigned char *samples_char = (unsigned char *) samples;
					if ( nch == 2 )
					{
						int op6 = outpos + outpos + ( outpos << 2 );
						doubletoint24( *ctx->vars.spl0, samples_char + op6 );
						doubletoint24( *ctx->vars.spl1, samples_char + op6 + 3 );
					}
					else
					{
						doubletoint24( *ctx->vars.spl0, samples_char + outpos + outpos + outpos );
					}
				}
				else
				{
					memcpy( (char *) samples + outpos * samplepair_size, (char *) ctx->sample_buffer + x * samplepair_size,
							samplepair_size );
					// todo: 24/32 bit modes
				}
			}
			outpos++;

			if ( *( ctx->vars.repeat ) < 0.0001 )
				x++;
		}
		else x++;
	}

	return outpos;
}

static void WriteInt( char *section, char *name, int value, char *fn )
{
	char str[ 128 ];
	wsprintf( str, "%d", value );
	WritePrivateProfileString( section, name, str, fn );
}

void SPS_save_preset( SPSEffectContext *ctx, char *filename, char *section )
{
	SPSPresetConfig *cfg = &ctx->curpreset;

	WriteInt( section, "slider1", cfg->sliderpos[ 0 ], filename );
	WriteInt( section, "slider2", cfg->sliderpos[ 1 ], filename );
	WriteInt( section, "slider3", cfg->sliderpos[ 2 ], filename );
	WriteInt( section, "slider4", cfg->sliderpos[ 3 ], filename );
	int x;
	for ( x = 0; x < 4; x++ )
	{
		int y;
		for ( y = 0; y < 3; y++ )
		{
			char buf[ 64 ];
			wsprintf( buf, "labels_%d_%d", x, y );
			WritePrivateProfileString( section, buf, cfg->slider_labels[ x ][ y ], filename );
		}
	}

	int s = strlen( cfg->code_text[ 0 ] );
	WriteInt( section, "code0_size", s, filename );
	WritePrivateProfileStruct( section, "code0_data", cfg->code_text[ 0 ], s, filename );
	s = strlen( cfg->code_text[ 1 ] );
	WriteInt( section, "code1_size", s, filename );
	WritePrivateProfileStruct( section, "code1_data", cfg->code_text[ 1 ], s, filename );
	s = strlen( cfg->code_text[ 2 ] );
	WriteInt( section, "code2_size", s, filename );
	WritePrivateProfileStruct( section, "code2_data", cfg->code_text[ 2 ], s, filename );
}

void SPS_load_preset( SPSEffectContext *ctx, char *filename, char *section )
{
	SPSPresetConfig *cfg = &ctx->curpreset;
	EnterCriticalSection( &ctx->code_cs );
	cfg->sliderpos[ 0 ] = GetPrivateProfileInt( section, "slider1", 0, filename );
	cfg->sliderpos[ 1 ] = GetPrivateProfileInt( section, "slider2", 0, filename );
	cfg->sliderpos[ 2 ] = GetPrivateProfileInt( section, "slider3", 0, filename );
	cfg->sliderpos[ 3 ] = GetPrivateProfileInt( section, "slider4", 0, filename );
	int x;
	for ( x = 0; x < 4; x++ )
	{
		int y;
		for ( y = 0; y < 3; y++ )
		{
			char buf[ 64 ];
			wsprintf( buf, "labels_%d_%d", x, y );
			GetPrivateProfileString( section, buf, "", cfg->slider_labels[ x ][ y ], MAX_LABEL_LEN, filename );
		}
	}

	int s = GetPrivateProfileInt( section, "code0_size", 0, filename );
	cfg->code_text[ 0 ][ 0 ] = 0;
	if ( s > 0 && s < MAX_CODE_LEN - 1 )
	{
		if ( GetPrivateProfileStruct( section, "code0_data", cfg->code_text[ 0 ], s, filename ) )
			cfg->code_text[ 0 ][ s ] = 0;
		else
			cfg->code_text[ 0 ][ 0 ] = 0;
	}

	s = GetPrivateProfileInt( section, "code1_size", 0, filename );
	cfg->code_text[ 1 ][ 0 ] = 0;
	if ( s > 0 && s < MAX_CODE_LEN - 1 )
	{
		if ( GetPrivateProfileStruct( section, "code1_data", cfg->code_text[ 1 ], s, filename ) )
			cfg->code_text[ 1 ][ s ] = 0;
		else
			cfg->code_text[ 1 ][ 0 ] = 0;
	}
	s = GetPrivateProfileInt( section, "code2_size", 0, filename );
	cfg->code_text[ 2 ][ 0 ] = 0;
	if ( s > 0 && s < MAX_CODE_LEN - 1 )
	{
		if ( GetPrivateProfileStruct( section, "code2_data", cfg->code_text[ 2 ], s, filename ) )
			cfg->code_text[ 2 ][ s ] = 0;
		else
			cfg->code_text[ 2 ][ 0 ] = 0;
	}
	ctx->code_needrecompile = 7;
	lstrcpyn( ctx->curpreset_name, filename, sizeof( ctx->curpreset_name ) );
	LeaveCriticalSection( &ctx->code_cs );
}

void SPS_initapp()
{
	NSEEL_init();
	//NSEEL_addfunctionex( "megabuf", 1, (int) _asm_megabuf, (int) _asm_megabuf_end - (int) _asm_megabuf, megabuf_ppproc );
}

void SPS_quitapp()
{
	NSEEL_quit();
}