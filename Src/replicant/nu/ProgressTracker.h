#pragma once
#include "foundation/types.h"
#include <map>
#include "nu/AutoLock.h"

/* Helper class for managing valid chunks in non-sequential scenarios 
e.g. progressive downloading */

class ProgressTracker
{
public:
	static const uint64_t null_position;
	ProgressTracker();
	void Write(uint64_t bytes_written);
	bool Valid(uint64_t requested_position, uint64_t requested_end, uint64_t *available=0);
	bool Seek(uint64_t requested_position, uint64_t requested_end, uint64_t *new_start, uint64_t *new_end);
	void Dump();

	typedef std::map<uint64_t, uint64_t> ChunkList;
	ChunkList chunks;
	uint64_t current_chunk;
	uint64_t current_position;
	nu::LockGuard list_guard;
};
