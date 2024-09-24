#include "SeekTable.h"
#include "read.h"
#include "global_elements.h"
#include "vint.h"
#include <stdio.h>
#include <assert.h>

#ifdef WA_VALIDATE
extern uint32_t num_seekhead_elements_found;
extern uint32_t num_seek_elements_found;
#endif

bool nsmkv::SeekTable::GetEntry(uint64_t id, uint64_t *position)
{
	return EnumEntry(0, id, position);
}

bool nsmkv::SeekTable::EnumEntry(size_t i, uint64_t id, uint64_t *position)
{
	SeekMap::iterator found = seekMap.find(id);
	if (found != seekMap.end())
	{
		SeekEntries *entries = found->second;
		if (entries && entries->size() > i)
		{
			*position = entries->at(i).position;
			return true;
		}
	}
	return false;
}

bool nsmkv::SeekTable::EnumEntry(size_t i, uint64_t id, SeekEntry **seek_entry)
{
	SeekMap::iterator found = seekMap.find(id);
	if (found != seekMap.end())
	{
		SeekEntries *entries = found->second;
		if (entries && entries->size() > i)
		{
			*seek_entry = &entries->at(i);
			return true;
		}
	}
	return false;
}

void nsmkv::SeekTable::AddEntry(nsmkv::SeekEntry &entry, int flags)
{
	// check for duplicates
	size_t i=0;
	SeekEntry *seek_entry;
	while (EnumEntry(i++, entry.id, &seek_entry))
	{
		if (flags & ADDENTRY_SINGLE)
		{
			if (flags & ADDENTRY_FOUND)
				seek_entry->position = entry.position;
			return;
		}

		if (entry.position == seek_entry->position)
		{
			return;
		}
	}

	SeekEntries *&entries = seekMap[entry.id];
	if (!entries)
	{
		entries = new SeekEntries;
	}
	entries->push_back(entry);
#ifdef WA_VALIDATE
	num_seek_elements_found++;
#endif
}

void nsmkv::SeekTable::Dump()
{
	SeekMap::iterator itr;
	for (itr=seekMap.begin();itr!=seekMap.end();itr++)
	{
		SeekEntries *entries = itr->second;
		if (entries)
		{
			SeekEntries::iterator seekItr;
			for (seekItr=entries->begin();seekItr!=entries->end();seekItr++)
			{
				SeekEntry &entry = *seekItr;
				printf("Seek Entry ->  id=%I64x, position=%I64u\n", entry.id, entry.position);
			}
		}
	}
}

// returns bytes read.  0 means EOF
static uint64_t ReadSeek(nsmkv::MKVReader *reader, uint64_t size, nsmkv::SeekTable &seekTable)
{
	uint64_t total_bytes_read=0;
	nsmkv::SeekEntry entry;

	enum
	{
		ID_FIELD_PARSED = 0x1,
		POSITION_FIELD_PARSED = 0x2,
	};
	int fields_parsed=0;
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
		case mkv_metaseek_seekid:
			{
				uint8_t binary[9] = {0};
				if (!node.size || node.size > 9)
				{
#ifdef WA_VALIDATE
					printf("        SeekID size invalid, size=%d\n",node.size);
#endif
					assert(node.size<9);
					return 0;
				}

				size_t bytes_read;
				reader->Read(binary, (size_t)node.size, &bytes_read);
				if (bytes_read != (size_t)node.size)
					return 0;
#ifdef WA_VALIDATE
				printf("        SeekID: %I64x\n", vint_read_ptr_len(node.size-1, binary));
#endif
				entry.id = vint_read_ptr_len((uint8_t)node.size-1, binary);
				fields_parsed |= ID_FIELD_PARSED;
			}
			break;
		case mkv_metaseek_seekposition:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;
#ifdef WA_VALIDATE
				printf("        SeekPosition: %I64u\n", val);
#endif
				entry.position = val;
				fields_parsed |= POSITION_FIELD_PARSED;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	if (fields_parsed == 0x3)
	{
		//entry.state = nsmkv::NOT_READ;
		seekTable.AddEntry(entry);
	}
#ifdef WA_VALIDATE
	else if (fields_parsed == 0x2)
	{
		printf("        Seek only contains SeekPosition\n");
	} else if (fields_parsed == 0x01)
	{
		printf("        Seek element only contains SeekID\n");
	}
#endif
	return total_bytes_read;
}


// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadSeekHead(nsmkv::MKVReader *reader, uint64_t size, nsmkv::SeekTable &seekTable)
{
	uint64_t total_bytes_read=0;
#ifdef WA_VALIDATE
	num_seekhead_elements_found++;
#endif

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
		case mkv_metaseek_seek:
			{
#ifdef WA_VALIDATE
				printf("      Seek\n");
#endif
				ReadSeek(reader, node.size, seekTable);
			}
			break;
		default:
			ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

