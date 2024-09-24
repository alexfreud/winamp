#include "Lacing.h"
#include "Cluster.h"
#include "vint.h"



bool nsmkv::Lacing::GetState(uint8_t flags, const uint8_t *data, size_t data_len, nsmkv::LacingState *state)
{
	// TODO: error check by making sure data_len doesn't go below 0
	switch(flags & BlockBinary::LACE_MASK)
	{
	case BlockBinary::NO_LACING:
		state->laces = 1;
		state->positions[0] = 0;
		state->sizes[0] = data_len;
#ifdef WA_VALIDATE
		printf("        Lacing = NO_LACING\n");
#endif
		return true;
	case BlockBinary::EBML_LACING:
		{
			uint16_t number_of_frames = state->laces = *data++ + 1;
			uint64_t delta = vint_get_number_bytes(data[0]);
			state->sizes[0] = vint_read_ptr_len((uint8_t)delta, data);
			delta++;
			state->positions[0] = 1;
			state->positions[1] = 1+state->sizes[0];
			for (uint16_t i=1;i<number_of_frames-1;i++)
			{
				uint8_t this_len = vint_get_number_bytes(data[delta]);
				state->sizes[i] = vsint_read_ptr_len(this_len, data+delta);
				state->sizes[i] += state->sizes[i-1];
				state->positions[i+1] = state->positions[i] + state->sizes[i];
				delta+=this_len + 1;
			}
			state->sizes[number_of_frames-1] = data_len - state->positions[number_of_frames-1] - delta;

			for (uint16_t i=0;i<number_of_frames;i++)
			{
				state->positions[i] += delta;
			}
#ifdef WA_VALIDATE
			printf("        Lacing = EBML_LACING\n");
#endif
		}
		return true;
	case BlockBinary::XIPH_LACING:
		{
			const uint8_t *orig_data = data;
			uint16_t number_of_frames=state->laces = *data++ + 1;
			for (uint16_t i=0;i!=number_of_frames-1;i++)
			{
				size_t frame_len = 0;
				do
				{
					frame_len += *data;
				} while (data && *data++ == 255);
				state->sizes[i] = frame_len;
			}
			uint64_t delta = data - orig_data;
			uint64_t last_position = delta;
			uint64_t last_size = 0;
			for (uint16_t i=0;i!=number_of_frames;i++)
			{
				state->positions[i] = last_position + last_size;
				last_position = state->positions[i];
				last_size = state->sizes[i];
			}
			state->sizes[number_of_frames-1] = data_len - state->positions[number_of_frames-1];
#ifdef WA_VALIDATE
			printf("        Lacing = XIPH LACING\n");
#endif
		}
		return true;
	case BlockBinary::FIXED_LACING:
		{
			uint16_t number_of_frames=state->laces=data[0]+1;
			uint32_t size_per_frame = (uint32_t)(data_len-1) / number_of_frames;
			for (uint16_t i=0;i<number_of_frames;i++)
			{
				state->positions[i] = (uint64_t)(1 + size_per_frame*i);
				state->sizes[i] = size_per_frame;
			}
#ifdef WA_VALIDATE
			printf("        Lacing = FIXED LACING\n");
#endif
		}
		return true;

	default:
		return false;
	}
}

bool nsmkv::Lacing::GetFrame(uint16_t frame_number, const uint8_t *data, size_t data_len, const uint8_t **frame, size_t *frame_len, const LacingState *state)
{
	if (frame_number >= state->laces)
		return false;

	const uint8_t *lace = data + state->positions[frame_number];
	size_t lace_len = (size_t)state->sizes[frame_number];
	if (lace < data  // if the lace starts before our data
		|| (state->positions[frame_number] + lace_len) > data_len) // or extends out past our data
	{
		return false;
	}

	*frame = lace;
	*frame_len = lace_len;
	return true;
}
