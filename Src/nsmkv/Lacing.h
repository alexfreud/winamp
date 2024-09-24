#pragma once
#include <bfc/platform/types.h>

namespace nsmkv
{

	struct LacingState
	{
		uint16_t laces;
		int64_t sizes[256]; // max 256 laces (stored as 8bit number and 1 added)
		uint64_t positions[256];
	};

	namespace Lacing
	{
		bool GetState(uint8_t flags, const uint8_t *data, size_t data_len, LacingState *state);
		bool GetFrame(uint16_t frame_number, const uint8_t *data, size_t data_len, const uint8_t **frame, size_t *frame_len, const LacingState *state);
	}
}
