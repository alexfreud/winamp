#pragma once
#include "foundation/types.h"

enum Agave_PositionType
{
	AGAVE_PLAYPOSITION_100NANOECONDS = 0,
	AGAVE_PLAYPOSITION_MILLISECONDS = 1,
	AGAVE_PLAYPOSITION_SECONDS = 2,
	AGAVE_PLAYPOSITION_HMSF= 3,
	AGAVE_PLAYPOSITION_SAMPLE_FRAMES = 4,
	AGAVE_PLAYPOSITION_BYTES = 5,
	AGAVE_PLAYPOSITION_PACKETS = 6,
};

struct Agave_HMSF
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t frames;
};

union Agave_Position
{
	uint64_t nanoseconds100; // in increments of 100 nanoseconds (microsoft style)
	uint64_t milliseconds;
	double seconds;
	Agave_HMSF hmsf;
	uint64_t sample_frames; 
	uint64_t bytes;
	uint64_t packets;
};

struct Agave_Seek
{
	Agave_PositionType position_type;
	Agave_Position position;
};

static void Agave_Seek_SetBytes(Agave_Seek *seek, uint64_t bytes)
{
	seek->position_type=AGAVE_PLAYPOSITION_BYTES;
	seek->position.bytes = bytes;
}
