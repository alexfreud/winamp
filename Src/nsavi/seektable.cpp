#include "seektable.h"
#include "demuxer.h"

static int GetStreamNumber(uint32_t id)
{
	char *stream_data = (char *)(&id); 
	if (!isxdigit(stream_data[0]) || !isxdigit(stream_data[1]))
		return -1;

	stream_data[2] = 0;
	int stream_number = strtoul(stream_data, 0, 16);
	return stream_number;
}

nsavi::SeekTable::SeekTable(int stream_number, bool require_keyframes, const nsavi::HeaderList *header_list)
: header_list(header_list), stream_number(stream_number), require_keyframes(require_keyframes), super_index(0), indices_processed(0), super_index_duration(0)
{
	stream = header_list->stream_list[stream_number].stream_header;
	const nsavi::INDX *index = header_list->stream_list[stream_number].stream_index;
	if (index)
	{
		if (index->index_type == nsavi::indx_type_master)
		{
			super_index = (nsavi::AVISUPERINDEX *)index;
		}
		else if (index->index_type == nsavi::indx_type_chunk)
		{
			AddIndex(index, 0);
		}
	}
}

nsavi::SeekTable::~SeekTable()
{
}

const nsavi::SeekEntry *nsavi::SeekTable::GetSeekPoint(int &timestamp_ms, int current_ms, int seek_direction)
{
	int last_time = 0;
	const nsavi::SeekEntry *last_entry = 0;

	// TODO: binary search
	for (SeekEntries::iterator itr=seek_entries.begin();itr!=seek_entries.end();itr++)
	{
		if (itr->first > timestamp_ms)
		{
			if (seek_direction == SEEK_FORWARD && current_ms >= last_time)
			{
				itr++;
				if (itr != seek_entries.end())
				{
					last_entry = &itr->second;
					last_time = itr->first;
				}
				else
				{
					return 0;
				}
			}

			timestamp_ms = last_time;
			return last_entry;

		}
		last_entry = &itr->second;
		last_time = itr->first;

	}
	if (seek_direction == SEEK_FORWARD && current_ms >= last_time)
	{
		return 0;
	}
	else
	{
		timestamp_ms = last_time;
		return last_entry;
	}

}

void nsavi::SeekTable::AddIndex(const IDX1 *index)
{
	// TODO: calculate total bytes at the same time, so we can get a more accurate bitrate
	data_processed[index] = true;
	uint64_t total_time = 0;
	uint64_t stream_rate = (uint64_t)stream->rate;
	
	//if (!require_keyframes)
	//		seek_entries.reserve(seek_entries.size() + index->index_count);

	for (uint32_t i=0;i!=index->index_count;i++)
	{
		const nsavi::IDX1_INDEX &this_index = index->indices[i];
		int this_stream_number = GetStreamNumber(this_index.chunk_id);
		if (this_stream_number == stream_number && (!require_keyframes || this_index.flags & nsavi::idx1_flags_keyframe))
		{
			int timestamp = (int)(total_time * 1000ULL / stream_rate);
			SeekEntry &entry = seek_entries[timestamp];
#ifdef SEEK_TABLE_STORE_CHUNK_HEADER
			entry.chunk_id = this_index.chunk_id;
			entry.chunk_size = this_index.chunk_size;
#endif
			entry.file_position = this_index.offset;
			entry.stream_time = total_time;
			entry.timestamp = timestamp;
			entry.absolute = false;
		}
		if (this_stream_number == stream_number && !(this_index.flags & nsavi::idx1_flags_no_duration))
		{
			if (stream->sample_size)
			{
				uint64_t samples = this_index.chunk_size / stream->sample_size; 
				total_time += stream->scale * samples;
			}
			else
				total_time += stream->scale;
		}
	}
}

void nsavi::SeekTable::AddIndex(const INDX *index, uint64_t start_time)
{
	if (index->index_type	== nsavi::indx_type_chunk)
	{
		const nsavi::AVISTDINDEX *chunk_index = (nsavi::AVISTDINDEX *)index;
		
		//if (!require_keyframes)
		//	seek_entries.reserve(seek_entries.size() + chunk_index->indx.number_of_entries);


		start_time *= stream->scale;

		uint64_t stream_rate = (uint64_t)stream->rate;
		for (uint32_t i=0;i!=chunk_index->indx.number_of_entries;i++)
		{
			const INDX_CHUNK_ENTRY &this_index = chunk_index->entries[i];
			if  (!require_keyframes || !(this_index.size & 0x80000000))
			{
				int timestamp = (int)(start_time * 1000ULL / stream_rate);
				SeekEntry &entry = seek_entries[timestamp];
#ifdef SEEK_TABLE_STORE_CHUNK_HEADER
				entry.chunk_id = chunk_index->indx.chunk_id;
				entry.chunk_size = this_index.size & ~0x80000000;
#endif
				entry.file_position = chunk_index->base_offset + this_index.offset;
				entry.stream_time = start_time;
				entry.timestamp = timestamp;
				entry.absolute = true;
			}

			if (stream->sample_size)
			{
				uint64_t samples = this_index.size / stream->sample_size; 
				start_time += stream->scale * samples;
			}
			else
				start_time += stream->scale;
		}
	}
}

bool nsavi::SeekTable::GetIndexLocation(int timestamp, uint64_t *position, uint64_t *start_time)
{
	// TODO: use timestamp more effectively
	if (super_index)
	{
		for (uint32_t i=indices_processed;i!=super_index->indx.number_of_entries;i++)
		{
			indices_processed++;
			*start_time = super_index_duration;
			super_index_duration += super_index->entries[i].duration;
			*position = super_index->entries[i].offset;
			return true;
		}
	}
	return false;
}