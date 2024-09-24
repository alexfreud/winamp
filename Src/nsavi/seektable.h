#pragma once
#include <map>
#include "../nsavi/avi_header.h"

//#define SEEK_TABLE_STORE_CHUNK_HEADER
namespace nsavi
{
	struct HeaderList;
	struct SeekEntry
	{
		SeekEntry(int dummy=0) : timestamp(0), absolute(false) {}
		int timestamp;  // in miliseconds
		bool absolute;
		uint64_t file_position = 0;
#ifdef SEEK_TABLE_STORE_CHUNK_HEADER
		uint32_t chunk_id; // chunk ID at file_position, used to verify seeking
		uint32_t chunk_size; // chunk size at file_position, used to verify seeking
#endif
		uint64_t stream_time = 0; // actually allocated to header_list->stream_list_size		
	};

class SeekTable
{
public:
	SeekTable(int stream_number, bool require_keyframes, const nsavi::HeaderList *header_list);
	~SeekTable();
	enum
	{
		SEEK_WHATEVER=0,
		SEEK_FORWARD=1,
		SEEK_BACKWARD=2,
	};
	const SeekEntry *GetSeekPoint(int &timestamp_ms, int current_ms=0, int seek_direction=SEEK_WHATEVER);
	void AddIndex(const IDX1 *index);
	void AddIndex(const INDX *index, uint64_t start_time);

	// returns a recommend place to go hunting for an idx1, indx or ix## chunk
	// usually based on indx super chunks already found in header_list
	bool GetIndexLocation(int timestamp, uint64_t *position, uint64_t *start_time);
	
	typedef std::map<int, SeekEntry> SeekEntries; // mapped to timestamp in milliseconds
	SeekEntries seek_entries; 
	std::map<const void *, bool> data_processed; // TODO: make nu::Set
	const nsavi::HeaderList *header_list;
	int stream_number; // which stream number is this Seek Table associated with
	bool require_keyframes; // whether or not the master stream requires keyframes
	const nsavi::STRH *stream;
	const nsavi::AVISUPERINDEX *super_index;
	uint32_t indices_processed;
	uint64_t super_index_duration;
};



}