#pragma once
#include <map>
#include <vector>
#include "mkv_reader.h"

const uint32_t mkv_metaseek_seekhead = 0x14d9b74;
const uint32_t mkv_metaseek_seek=0xdbb;
const uint32_t mkv_metaseek_seekid = 0x13ab;
const uint32_t mkv_metaseek_seekposition=0x13ac;

/* this represents a seek table of other nodes in EBML 
 be careful, CuePoints (CuePoints.h) is for seeking to a certain time in the song
the SeekTable (SeekTable.h) is for fast indexing of the mkv file structure 
*/
namespace nsmkv
{
	class SeekEntry
	{
	public:
		SeekEntry()
		{
			id=0;
			position=0;
		}
		SeekEntry(uint64_t id, uint64_t position) : id(id), position(position)
		{
		}
		uint64_t id; // ID of the EBML node
		uint64_t position;
	};

	class SeekTable
	{
	public:
		void AddEntry(SeekEntry &entry, int flags=0);
		void Dump();
		bool GetEntry(uint64_t id, uint64_t *position);
		bool EnumEntry(size_t i, uint64_t id, uint64_t *position);

		enum // flags for AddEntry
		{
			ADDENTRY_SINGLE = 0x1, // if there can only be one
			ADDENTRY_FOUND = 0x2, // pass this is you physically found the entry in the file - this takes priority over the SeekHead
		};
	private:
		bool EnumEntry(size_t i, uint64_t id, SeekEntry **seek_entry);
		typedef std::vector<SeekEntry> SeekEntries;
		typedef std::map<uint64_t, SeekEntries*> SeekMap;
		SeekMap seekMap;
	};

	// returns bytes read.  0 means EOF
	uint64_t ReadSeekHead(nsmkv::MKVReader *reader, uint64_t size, nsmkv::SeekTable &seekTable);
}