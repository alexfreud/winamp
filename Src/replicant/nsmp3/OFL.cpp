#include "OFL.h"
#include "foundation/error.h"

static void crcofl(unsigned short crcPoly, unsigned short crcMask, unsigned long *crc, unsigned char byte)
{
	int i;
	for (i=0; i<8; i++) 
	{
		unsigned short flag = (*crc) & crcMask ? 1:0;
		flag ^= (byte & 0x80 ? 1 : 0);
		(*crc)<<=1;
		byte <<= 1;
		if(flag)
			(*crc) ^= crcPoly;
	}
}

int OFL::GetGaps(size_t *pregap, size_t *postgap)
{
	/* TODO: verify the postgap calculation */
	if (codec_delay >= 529)
	{
		*pregap = codec_delay;
		size_t endcut;
		endcut = samples_per_frame - ((total_length + codec_delay) % samples_per_frame); // how many 0 samples had to be added?
		*postgap = endcut;
		return NErr_Success;
	}
	return NErr_Empty;
}


double OFL::GetLengthSeconds() const
{
	return (double)GetSamples() / (double)sample_rate;
}

uint64_t OFL::GetSamples() const
{
	return total_length;
}

uint32_t OFL::GetFrames() const
{
	uint64_t real_samples = (total_length+codec_delay)*samples_per_frame;
	return (uint32_t) (real_samples/samples_per_frame);
}

int OFL::Read(const MPEGHeader &header, const uint8_t *buffer, size_t buffer_len)
{
	if (header.layer != MPEGHeader::Layer3)
		return NErr_False;

	sample_rate = header.GetSampleRate();
	samples_per_frame = header.GetSamplesPerFrame();

	if (header.channel_mode == MPEGHeader::Mono)
	{
		if (header.mpeg_version == MPEGHeader::MPEG1)
		{
			// 0-9 : main_data_end
			int16_t main_data_end = (buffer[0] << 1) | (buffer[1] >> 7);

			// read the 2 part2_3_lengths out so we know how big the main data section is
			uint16_t part2_3_length = ((buffer[2] & 0x3F) << 6) | (buffer[3]>>2); // bits 18-30
			part2_3_length += ((buffer[9] & 0x7) << 9) | (buffer[10] << 1) | (buffer[11] >> 7) ; // bits 77-89

			size_t offset = 17 + (part2_3_length+7)/8;
			if (offset+9 < buffer_len && buffer[offset] == 0xb4)
			{
				unsigned long crc=255;
				for (int i=0;i<9;i++)
					crcofl(0x0045, 0x0080, &crc, buffer[offset+i]);

				if ((crc & 0xFF) == buffer[offset+9])
				{
					total_length = (buffer[offset+3] << 24) | (buffer[offset+4] << 16) | (buffer[offset+5] << 8) | (buffer[offset+6]);
					codec_delay =     (buffer[offset+1] << 8) | (buffer[offset+2]);
					additional_delay=     (buffer[offset+7] << 8) | (buffer[offset+8]);
					return NErr_Success;
				}
			}
		}
		else
		{ // MPEG2 and 2.5
			// 0-8 : main_data_end
			uint16_t main_data_end = buffer[0];

			// read the 2 part2_3_lengths out so we know how big the main data section is
			uint16_t part2_3_length = ((buffer[1] & 0x7F) << 5) | (buffer[2]>>3); // bits 9-21

			size_t offset = 9 + (part2_3_length+7)/8;
			if (offset+9 < buffer_len && buffer[offset] == 0xb4)
			{
				unsigned long crc=255;
				for (int i=0;i<9;i++)
					crcofl(0x0045, 0x0080, &crc, buffer[offset+i]);

				if ((crc & 0xFF) == buffer[offset+9])
				{
					total_length = (buffer[offset+3] << 24) | (buffer[offset+4] << 16) | (buffer[offset+5] << 8) | (buffer[offset+6]);
					codec_delay =     (buffer[offset+1] << 8) | (buffer[offset+2]);
					additional_delay=     (buffer[offset+7] << 8) | (buffer[offset+8]);
					return NErr_Success;
				}
			}
		}
	}
	else
	{
		if (header.mpeg_version == MPEGHeader::MPEG1)
		{
			// 0-9 : main_data_end
			uint16_t main_data_end = (buffer[0] << 1) | (buffer[1] >> 7);

			// read the 4 part2_3_lengths out so we know how big the main data section is
			uint16_t part2_3_length = ((buffer[2] & 0xF) << 8) | buffer[3]; // bits 20-32 
			part2_3_length += ((buffer[9] & 0x1) << 11) | (buffer[10] << 3) | (buffer[11] >> 5) ; // bits 79-91
			part2_3_length += ((buffer[17] & 0x3F) << 6) | (buffer[18] >> 2); // bits 138-150
			part2_3_length += ((buffer[24] & 0x7) << 9) | (buffer[25] << 1) | (buffer[26] >> 7); // bits 197-209

			size_t offset = 32 + (part2_3_length+7)/8;
			if (offset+9 < buffer_len && buffer[offset] == 0xb4)
			{
				unsigned long crc=255;
				for (int i=0;i<9;i++)
					crcofl(0x0045, 0x0080, &crc, buffer[offset+i]);

				if ((crc & 0xFF) == buffer[offset+9])
				{
					total_length = (buffer[offset+3] << 24) | (buffer[offset+4] << 16) | (buffer[offset+5] << 8) | (buffer[offset+6]);
					codec_delay =     (buffer[offset+1] << 8) | (buffer[offset+2]);
					additional_delay=     (buffer[offset+7] << 8) | (buffer[offset+8]);
					return NErr_Success;
				}
			}
		}
		else
		{ // MPEG2 and 2.5
			// 0-8 : main_data_end
			uint16_t main_data_end = buffer[0];

			// read the 4 part2_3_lengths out so we know how big the main data section is
			uint16_t part2_3_length = ((buffer[1] & 0x3F) << 6) | (buffer[2] >> 2); // bits 10-22
			part2_3_length += ((buffer[8] & 0x7) << 9) | (buffer[9] << 1) | (buffer[10] >> 7) ; // bits 69-81

			size_t offset = 17 + (part2_3_length+7)/8;
			if (offset+9 < buffer_len && buffer[offset] == 0xb4)
			{
				unsigned long crc=255;
				for (int i=0;i<9;i++)
					crcofl(0x0045, 0x0080, &crc, buffer[offset+i]);

				if ((crc & 0xFF) == buffer[offset+9])
				{
					total_length = (buffer[offset+3] << 24) | (buffer[offset+4] << 16) | (buffer[offset+5] << 8) | (buffer[offset+6]);
					codec_delay =     (buffer[offset+1] << 8) | (buffer[offset+2]);
					additional_delay=     (buffer[offset+7] << 8) | (buffer[offset+8]);
					return NErr_Success;
				}
			}
		}
	}
	return NErr_False;
}
