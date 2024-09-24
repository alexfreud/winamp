#include "nsv_vlb.h"

///////////////////////////////////////////////////////////////////////
//
// NSV VLB Decoder
//
///////////////////////////////////////////////////////////////////////

VLB_Decoder::VLB_Decoder()
{
	aacdec = new CAacDecoderApi( &datain );
	fused = 0;
	needsync = 1;
	packets_in_since_flush = 0;
	packets_decoded_since_flush = 0;
}

VLB_Decoder::~VLB_Decoder()
{
	delete aacdec;
}

void VLB_Decoder::flush()
{
	datain.Empty();
	dataout.Empty();

	//OutputDebugString("FLUSH\n");


	// JF> this seems to be necessary for me at least, to have aacdec's internal buffer and state get reset :/
	//   (especially for AAC files, but I got it to do weird stuff on VLB files too :/)
	// might be cleaner to see if we can just clear it somehow manually.. hmm..
	delete aacdec;
	aacdec = new CAacDecoderApi( &datain );
	fused = 0;
	needsync = 1;
	packets_in_since_flush = 0;
	packets_decoded_since_flush = 0;
}


int VLB_Decoder::decode( void *in, int in_len,
                         void *out, int *out_len,
                         unsigned int out_fmt[8] )
{
	// This function gets called with 1 nsv frame's worth of audio data.
	// That could mean 1 OR MORE audio packets.  (or zero?)
	// Just process the smallest amount (1 packet) and return 0 if you
	//   finished processing this big chunk, or 1 if you need to work on
	//   extracting more audio (from this same nsv frame) on the next call.

	// RETURN VALUES:
	// 1: call me again w/same buffer (and contents) next time
	// 0: give me a new buffer (w/new frame contents) next time

	AUDIO_FORMATINFO info;

	int rval = 1;

	if (!dataout.BytesAvail())
	{
		int l = datain.GetInputFree();  // the # of bytes that datain still has room for
		if ( l > in_len - fused ) l = in_len - fused;

		if ( l > 0)
		{
			datain.Fill( (unsigned char *)in + fused, l );
			fused += l;
		}

		/*********************************/
		if (in_len > 0)
			packets_in_since_flush++;
#define PACKETS_TO_PREBUFFER_BEFORE_SYNCHRONIZE 2
		if (needsync && packets_in_since_flush < PACKETS_TO_PREBUFFER_BEFORE_SYNCHRONIZE)
		{
			// Don't allow ourselves to call Synchronize() until we've actually received
			// TWO audio packets.  We need two because Synchronize() peeks ahead beyond
			// the first packet, and throws an exception if the second one isn't also available.
			// (note that if we were worried about getting partial frames from the nsv decoder,
			// we'd want to set PACKETS_TO_PREBUFFER==3...)
			fused = 0;
			*out_len = 0;
			return 0;
		}
		/*********************************/

		if (!datain.GetSize())
		{
			if ( fused >= in_len ) rval = fused = 0; // get more data
			*out_len = 0;
			return rval;
		}

		if (needsync)
		{
			int status = aacdec->Synchronize( &params ); // returns # of bytes skipped forward through 'datain' in params.frame_length
			if (status)
			{
				// ERROR
				*out_len = 0;
				if ( fused >= in_len ) rval = fused = 0; // get more data
				return rval;
			}

			needsync = 0;

			// NOTE: as long as the NSV file was encoded properly (i.e. all vlb audio packets were
			//  sent to the encoder intact, never split among two nsv frames), we don't have to worry
			//  about ever getting back a partial frame from the NSV decoder; it will sync for us,
			//  and hand us the 1st complete audio packet it can find.
			// In short: Synchronize() should always return with params.frame_length==0.
			/*
			int bytes_skipped = params.frame_length;
			if (bytes_skipped)
			{
			// here we assume that the first packet was partially or entirely skipped through
			// (but that the 2nd and 3rd packets were 100% okay), so we won't try to decode
			// that first packet.
			packets_in_since_flush--;
			}
			*/

			info.ucNChannels = (unsigned char) params.num_channels;
			info.uiSampleRate = params.sampling_frequency;

			dataout.SetFormatInfo( &info );
		}

		while (packets_decoded_since_flush < packets_in_since_flush)
		{
			int status = aacdec->DecodeFrame( &dataout, &params ); // returns # of bytes consumed from 'datain' in params.frame_length
			packets_decoded_since_flush++;
			if ( status > ERR_NO_ERROR && status != ERR_END_OF_FILE)
			{
				// ERROR
				flush();
				break;
			}
		}

		if (packets_decoded_since_flush > 64)
		{
			// avoid overflow, but don't let either of these vars drop back to 0, 1, or 2!
			packets_in_since_flush -= 32;
			packets_decoded_since_flush -= 32;
		}
	}

	int l = dataout.BytesAvail();
	if (l > 0)
	{
		if ( l > *out_len ) l = *out_len;
		else *out_len = l;

		dataout.PullBytes( (unsigned char *)out, l );

		info = dataout.GetFormatInfo();

		out_fmt[0] = NSV_MAKETYPE( 'P', 'C', 'M', ' ' );
		out_fmt[1] = info.uiSampleRate;
		out_fmt[2] = info.ucNChannels;
		out_fmt[3] = 16;
		out_fmt[4] = params.bitrate;
	}
	else *out_len = 0;

	return rval;
}