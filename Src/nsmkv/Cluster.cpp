#include "Cluster.h"
#include "read.h"
#include "global_elements.h"
#include <winsock.h>

// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadBlockGroup(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Block &block, uint64_t *allowed_track_numbers, size_t num_allowed_track_numbers)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		uint64_t ebml_start_position = reader->Tell(); // need this for recording the block binary start position
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_blockgroup_referenceblock:
			{
				int64_t val;
				if (read_signed(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Reference Block: %I64d\n", val);
#endif
				block.reference_block = val;
			}
			break;
		case mkv_blockgroup_block:
			{
#ifdef WA_VALIDATE
				printf("      Block: binary size %I64u\n", node.size);
#endif
				if (ReadBlockBinary(reader, node.size, block.binary, allowed_track_numbers, num_allowed_track_numbers) == 0)
					return 0;
			}
			break;
		case mkv_blockgroup_blockduration:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Block Duration: %I64u\n", val);
#endif
				block.block_duration = val;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadCluster(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Cluster &cluster)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_cluster_timecode:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Time Code: %I64u\n", val);
				cluster.time_code_found = true;
#endif
				cluster.time_code = val;
			}
			break;
		case mkv_cluster_blockgroup:
			{
#ifdef WA_VALIDATE
				printf("    Block Group\n");
#endif
				Block block;
				if (ReadBlockGroup(reader, node.size, block, 0, 0) == 0)
					return 0;
			}
			break;
		case mkv_cluster_simpleblock:
			{
#ifdef WA_VALIDATE
				printf("    Simple Block, size: %I64u\n", node.size);
#endif
				BlockBinary bbinary;
				if (ReadBlockBinary(reader, node.size, bbinary, 0, 0) == 0)
					return 0;
//				fseek64(f, node.size, SEEK_CUR);
			}
			break;
		default:
			ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}


uint64_t nsmkv::ReadBlockBinary(nsmkv::MKVReader *reader, uint64_t size, nsmkv::BlockBinary &binary, uint64_t *allowed_track_numbers, size_t num_allowed_track_numbers)
{
	uint64_t orig_size = size;
	size_t bytes_read = (size_t)read_vint(reader, &binary.track_number);
	if (!bytes_read)
		return 0;
	size-=bytes_read;
	bool allowed_track=false;
	if (num_allowed_track_numbers == (size_t)-1)
	{
		allowed_track=true;
	}
	else
	{
		for (size_t i=0;i!=num_allowed_track_numbers;i++)
		{
			if (binary.track_number == allowed_track_numbers[i])
			{
				allowed_track=true;
				break;
			}
		}
	}
#ifdef WA_VALIDATE
	// if we are validating the file, force all tracks to go thru
	allowed_track = true;
#endif
	if (allowed_track && size >= 3)
	{
		uint16_t time_code_endian;
		size_t bytes_read;
		reader->Read(&time_code_endian, sizeof(uint16_t), &bytes_read);
		if (bytes_read != sizeof(uint16_t))
			return 0;
		binary.time_code = htons(time_code_endian);
		size-=sizeof(uint16_t);

		uint8_t flags;
		reader->Read(&flags, 1, &bytes_read);
		if (bytes_read != 1)
			return 0;

		binary.flags = flags;
		size -= sizeof(uint8_t);

		binary.data_size = (size_t)size;
#ifndef WA_VALIDATE
		binary.data = malloc((size_t)size);
		if (!binary.data)
			return 0;
		reader->Read(binary.data, (size_t)size, &bytes_read);
		if (bytes_read != size)
		{
			free(binary.data);
			return 0;
		}
#else
		printf("        track number = %I64u\n",binary.track_number);
		printf("        timecode = %u\n",binary.time_code);
		printf("        flags = %u\n",binary.flags);  
		printf("        data_size = %I64u\n",size);
		// if we are validating the nmk file we don't need to 
		// actually read the data, just skip past it
		//fseek(reader, size, SEEK_CUR);
#endif
		return orig_size;
	}
	else
	{
		reader->Skip(size);
		return orig_size;
	}
	return 0;
}

