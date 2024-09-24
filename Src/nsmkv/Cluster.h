#pragma once
#include "mkv_reader.h"
#include <bfc/platform/types.h>

const uint32_t mkv_segment_cluster = 0xf43b675;
const uint32_t mkv_cluster_timecode = 0x67;
const uint32_t mkv_cluster_blockgroup = 0x20;
const uint32_t mkv_cluster_simpleblock = 0x23;
const uint32_t mkv_blockgroup_referenceblock = 0x7b;
const uint32_t mkv_blockgroup_block = 0x21;
const uint32_t mkv_blockgroup_blockduration = 0x1b;

/* TODO: benski>
need to think this whole thing through.
Ideally, we would be able to enumerate through the clusters/blocks,
but the output plugin might have a different audio buffer size
than the size that the encoder assumed.
so the first attempt is going to be random access
and we'll see if there is a better way after implementation
*/
namespace nsmkv
{

	class BlockBinary
	{
	public:
		BlockBinary()
		{
			data=0;
			data_size=0;
			track_number=0;
			flags=0;
			time_code=0;
		}
		~BlockBinary()
		{
			free(data);
		}
		uint64_t track_number;
		int16_t time_code;
		uint8_t flags;
		size_t data_size;
		void *data; // maybe optionally allow the user to pass in the buffer to reduce mallocs
		
		enum
		{
			LACE_MASK = 0x6,
			XIPH_LACING= 0x2,
			FIXED_LACING = 0x4,
			EBML_LACING = 0x6,
			NO_LACING = 0x0,			
		};
	};

	class Block
	{
		public:
			Block()
#ifdef WA_VALIDATE
				:
				reference_block_found(false),
				block_duration_found(false)
#endif
			{
				reference_block=0;
				block_duration=0;
			}
			BlockBinary binary;
			uint64_t reference_block;
			uint64_t block_duration;

#ifdef WA_VALIDATE
			bool reference_block_found;
			bool block_duration_found;
#endif
	};

	class Cluster
	{
	public:
		Cluster()
#ifdef WA_VALIDATE
			:
			time_code_found(false)
#endif
		{
			time_code = 0;
			position = 0;
			previous_size = 0;
		}
		uint64_t time_code;
		uint64_t position;
		uint64_t previous_size;
#ifdef WA_VALIDATE
		bool time_code_found;
#endif
	};

	class Clusters
	{

	};
	uint64_t ReadCluster(MKVReader *reader, uint64_t size, nsmkv::Cluster &cluster);
	uint64_t ReadBlockBinary(MKVReader *reader, uint64_t size, nsmkv::BlockBinary &binary, uint64_t *allowed_track_numbers, size_t num_allowed_track_numbers);
	uint64_t ReadBlockGroup(MKVReader *reader, uint64_t size, nsmkv::Block &block, uint64_t *allowed_track_numbers, size_t num_allowed_track_numbers);
}