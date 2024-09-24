#include "rle.h"
static bool CheckOverflow(size_t total_size, int current_position, int read_size)
{
	if (read_size > (int)total_size) // check separate to avoid overflow
		return true;
	if (((int)total_size - read_size) < current_position)
		return true;
	return false;
}


void RLE16(const uint8_t *rle, size_t rle_size_bytes, uint16_t *video_frame, size_t video_frame_size, int stride)
{
	int input = 0;
	int output = 0;
	video_frame_size >>= 1; // divide by 2 since we're indexing as uint16_t
	int next_line = output + stride;
	while (input < (int)rle_size_bytes && output < (int)video_frame_size)
	{
		if (CheckOverflow(rle_size_bytes, input, 2)) // we always read at least two bytes
			break;

		uint8_t b0 = rle[input++];
		if (b0)
		{
			if (CheckOverflow(rle_size_bytes, input, 2))
				break;

			if (CheckOverflow(video_frame_size, output, b0))
			{
				b0 = (uint8_t)(video_frame_size - output);
			}

			uint16_t pixel = *(uint16_t *)(&rle[input]);
			input += 2;
			while (b0--)
			{
				memcpy(&video_frame[output], &pixel, 2);
				output++;
			}
		}
		else
		{
			uint8_t b1 = rle[input++];
			if (b1 == 0)
			{
				output = next_line;
				next_line = output + stride;
			}
			else if (b1 == 1)
			{
				return;
			}
			else if (b1 == 2)
			{
				if (CheckOverflow(rle_size_bytes, input, 2)) 
					break;

				uint8_t p1 = rle[input++];
				uint8_t p2 = rle[input++];
				output += p1;
				output += p2*stride;
				next_line += p2*stride;
			}
			else
			{
				if (CheckOverflow(rle_size_bytes, input, b1*2))
					break;

				if (CheckOverflow(video_frame_size, output, b1))
					break;
				for (uint8_t i=0;i!=b1;i++)
				{
					video_frame[output++] = *(uint16_t *)(&rle[input]);
					input+=2;
				}
			}
		}
	}
}

void RLE8(const uint8_t *rle, size_t rle_size_bytes, uint8_t *video_frame, size_t video_frame_size, int stride)
{
	int input = 0;
	int output = 0;
	int next_line = output + stride;
	while (input < (int)rle_size_bytes && output < (int)video_frame_size)
	{
		if (CheckOverflow(rle_size_bytes, input, 2)) // we always read at least two bytes
			break;
		uint8_t b0 = rle[input++];
		if (b0)
		{
			if (CheckOverflow(video_frame_size, output, b0))
			{
				b0 = (uint8_t)(video_frame_size - output);
			}

			uint8_t pixel = rle[input++];
			memset(&video_frame[output], pixel, b0);
			output+=b0;
		}
		else
		{
			uint8_t b1 = rle[input++];
			if (b1 == 0)
			{
				output = next_line;
				next_line = output + stride;
			}
			else if (b1 == 1)
			{
				break;
			}
			else if (b1 == 2)
			{
				if (CheckOverflow(rle_size_bytes, input, 2)) 
					break;

				uint8_t p1 = rle[input++];
				uint8_t p2 = rle[input++];
				output += p1;
				output += p2*stride;
				next_line += p2*stride;
			}
			else
			{
				if (CheckOverflow(rle_size_bytes, input, b1))
					break;

				if (CheckOverflow(video_frame_size, output, b1))
					break;
				memcpy(&video_frame[output], &rle[input], b1);
				input += b1;
				output += b1;
				if (b1 & 1)
					input++;
			}
		}
	}
}