#include "LAMEInfo.h"
#include "MPEGHeader.h"
#include "foundation/error.h"
#include <string.h>
#include "nu/ByteReader.h"
#include "nu/BitReader.h"
// Xing header -
// 4   Xing
// 4   flags
// 4   frames
// 4   bytes
// 100 toc
// 4   bytes VBR quality

// Lame tag
// 9 bytes - release name
// 11

// Lame extended info tag

// http://gabriel.mp3-tech.org/mp3infotag.html



LAMEInfo::LAMEInfo() 
{
	memset(this, 0, sizeof(LAMEInfo));
}

bool LAMEInfo::Flag(int flag) const
{
	return flags & flag;
}

int LAMEInfo::GetGaps(size_t *pregap, size_t *postgap)
{
	if (!encoder_delay)
		return NErr_Empty;

	*pregap = encoder_delay;
	*postgap = padding;
	return NErr_Success;
}

uint64_t LAMEInfo::GetSeekPoint(double percent) const
{
	// interpolate in TOC to get file seek point in bytes
	int a;
	uint64_t seekpoint;
	double fa, fb, fx;

	percent*=100.0;
	if (percent < 0.0)
		percent = 0.0;
	if (percent > 100.0)
		percent = 100.0;

	a = (int)(percent);
	if (a > 99) a = 99;
	fa = toc[a];
	if (a < 99)
	{
		fb = toc[a + 1];
	}
	else
	{
		fb = 256.0;
	}

	fx = fa + (fb - fa) * (percent - a);
	seekpoint = (uint64_t) ((1.0 / 256.0) * fx * bytes);
	return seekpoint;
}

uint64_t LAMEInfo::GetSamples() const
{
	if (flags&FRAMES_FLAG) 
	{
		uint64_t samples = frames * samples_per_frame;
		samples -= (encoder_delay + padding);
		return samples;
	}
	return 0;
}

uint32_t LAMEInfo::GetFrames() const
{
	if (flags&FRAMES_FLAG) 
		return frames;
	else
		return 0;
}

double LAMEInfo::GetLengthSeconds() const
{
	if (flags&FRAMES_FLAG) 
	{
		return (double)GetSamples() / (double)sample_rate;
	}
	return 0;
}

int LAMEInfo::Read(const MPEGHeader &frame, const uint8_t *buffer, size_t buffer_length)
{
	int flags;
	bool crc_hack_applied=false;
	bytereader_value_t byte_reader;

	/* maybe toolame writes these things also, I dunno.  we'll just abort for now */
	if (frame.layer != MPEGHeader::Layer3)
		return 0;


	bytereader_init(&byte_reader, buffer, buffer_length);

	sample_rate = frame.GetSampleRate();
	version = frame.mpeg_version;
	samples_per_frame = frame.GetSamplesPerFrame();

	// skip sideinfo
	if (frame.mpeg_version == MPEGHeader::MPEG1) // MPEG 1
	{        
		if (frame.channel_mode == MPEGHeader::Mono)
			bytereader_advance(&byte_reader, 17);
		else
			bytereader_advance(&byte_reader, 32);
	}
	else if (frame.mpeg_version == MPEGHeader::MPEG2) // MPEG 2
	{  
		if (frame.channel_mode == MPEGHeader::Mono)
			bytereader_advance(&byte_reader, 9);
		else
			bytereader_advance(&byte_reader, 17);
	}
	else if (frame.mpeg_version == MPEGHeader::MPEG2_5) // MPEG 2
	{  
		if (frame.channel_mode == MPEGHeader::Mono)
			bytereader_advance(&byte_reader, 9);
		else
			bytereader_advance(&byte_reader, 17);
	}

	if (bytereader_size(&byte_reader) > buffer_length /* check for wraparound */
			|| bytereader_size(&byte_reader) < 8)
		return NErr_Insufficient;

again:
	if (bytereader_show_u32_be(&byte_reader) == 'Info')
		cbr=1;
	else if (bytereader_show_u32_be(&byte_reader) != 'Xing' && bytereader_show_u32_be(&byte_reader) != 'Lame')
	{
		// if there's CRC data, LAME sometimes writes to the wrong position
		if (frame.IsCRC() && !crc_hack_applied)
		{
			crc_hack_applied=true;
			bytereader_advance(&byte_reader, 2);
			goto again;
		}
		return NErr_False;
	}

	bytereader_advance(&byte_reader, 4);  // skip Xing tag
	flags = this->flags = bytereader_read_u32_be(&byte_reader);

	if (flags & FRAMES_FLAG)
	{
		if (bytereader_size(&byte_reader) < 4)
			return NErr_Insufficient;

		frames = bytereader_read_u32_be(&byte_reader);
	}
	if (flags & BYTES_FLAG)
	{
		if (bytereader_size(&byte_reader) < 4)
			return NErr_Insufficient;
		bytes = bytereader_read_u32_be(&byte_reader);
	}
	if (flags & TOC_FLAG)
	{
		if (bytereader_size(&byte_reader) < 100)
			return NErr_Insufficient;

		int i;
		memcpy(toc, bytereader_pointer(&byte_reader), 100);

		// verify that TOC isn't empty
		for (i = 0; i < 100; i++)
			if (toc[i]) break;
		if (i == 100)
			flags &= ~TOC_FLAG;

		bytereader_advance(&byte_reader, 100);
	}

	vbr_scale = -1;
	if (flags & VBR_SCALE_FLAG)
	{
		if (bytereader_size(&byte_reader) < 4)
			return NErr_Insufficient;
		vbr_scale = bytereader_read_u32_be(&byte_reader);
	}

	if (bytereader_size(&byte_reader) < 27)
		return NErr_Success; // stop here if we have to, we have at least some data

	if (bytereader_show_u32_be(&byte_reader) == 'LAME')
	{
		for (int i=0;i<9;i++)
			encoder[i]=bytereader_read_u8(&byte_reader);
		encoder[9]=0; // null terminate in case tag used all 9 characters

		if (bytereader_show_u8(&byte_reader) == '(')
		{
			// read 11 more characters
			for (int i=9;i<20;i++)
				encoder[i]=bytereader_read_u8(&byte_reader);
			encoder[20]=0;
		}
		else
		{
			tag_revision = bytereader_show_u8(&byte_reader)>>4;
			if (tag_revision == 0)
			{
				encoding_method = bytereader_read_u8(&byte_reader)&0xF; // VBR method
				lowpass = bytereader_read_u8(&byte_reader)*100; // lowpass value
				peak=bytereader_read_f32_be(&byte_reader); // read peak value

				// read track gain
				int16_t gain_word = bytereader_read_s16_be(&byte_reader);
				if ((gain_word & 0xFC00) == 0x2C00)
				{
					replaygain_track_gain = (float)(gain_word & 0x01FF);
					replaygain_track_gain /= 10;
					if (gain_word & 0x0200)
						replaygain_track_gain = -replaygain_track_gain;
				}

				// read album gain
				gain_word = bytereader_read_s16_be(&byte_reader);
				if ((gain_word & 0xFC00) == 0x4C00)
				{
					replaygain_album_gain = (float)(gain_word & 0x01FF);
					replaygain_album_gain /= 10;
					if (gain_word & 0x0200)
						replaygain_album_gain = -replaygain_album_gain;
				}

				bytereader_advance(&byte_reader, 1); // skip encoding flags + ATH type
				abr_bitrate = bytereader_read_u8(&byte_reader); // bitrate

				// get the encoder delay and padding, annoyingly as 12 bit values packed into 3 bytes
				BitReader bit_reader;
				bit_reader.data = (const uint8_t *)bytereader_pointer(&byte_reader);
				bit_reader.numBits = 24;
				const uint8_t *temp = (const uint8_t *)bytereader_pointer(&byte_reader);
				encoder_delay = bit_reader.getbits(12);
				padding = bit_reader.getbits(12);
				bytereader_advance(&byte_reader, 3);

				bytereader_advance(&byte_reader, 4);
				// skip misc
				// skip MP3Gain reconstruction info
				// skip surround info and preset info

				music_length = bytereader_read_u32_be(&byte_reader);
				music_crc = bytereader_read_u16_be(&byte_reader);
				tag_crc = bytereader_read_u16_be(&byte_reader);

			}
		}
	}
	else if (!memcmp(bytereader_pointer(&byte_reader), "iTunes", 6))
	{
		int i=0;
		while (bytereader_size(&byte_reader) && i < 31)
		{
			encoder[i] = bytereader_read_u8(&byte_reader);
			if (!encoder[i])
				break;
			i++;
		}
		encoder[31]=0;
	}
	else if (!memcmp(bytereader_pointer(&byte_reader), "\0\0\0\0mp3HD", 9))
	{
		bytereader_advance(&byte_reader, 4);
		for (int i=0;i<5;i++)
			encoder[i] = bytereader_read_u8(&byte_reader);

		encoder[5]=0;
	}
	return NErr_Success;
}
