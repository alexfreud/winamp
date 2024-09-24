#pragma once
#include "foundation/types.h"
#include "foundation/error.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "nu/nodelist.h"

struct NXFileRegion : public queue_node_t
{
	uint64_t start;
	uint64_t end;
};

class NXFileObject
{
public:
	size_t Retain();
	size_t Release();
	ns_error_t LockRegion(uint64_t start, uint64_t end);
	ns_error_t UnlockRegion();
	ns_error_t Stat(nx_file_stat_t out_stats);
	ns_error_t Length(uint64_t *length);

	virtual ns_error_t Read(void *buffer, size_t bytes_requested, size_t *bytes_read)=0;
	virtual ns_error_t Write(const void *buffer, size_t bytes)=0;
	virtual ns_error_t PeekByte(uint8_t *byte)=0;
	virtual ns_error_t Seek(uint64_t position)=0;
	virtual ns_error_t Tell(uint64_t *position)=0;
	virtual ns_error_t Sync()=0;
	virtual ns_error_t Truncate()=0;
	
	virtual ns_error_t EndOfFile();

protected:
	NXFileObject();
	virtual ~NXFileObject();

	nx_file_stat_s file_stats;
	uint64_t position; /* note: this represents absolute position, _not_ position within the region */
	ns_error_t Initialize(nx_uri_t uri);
	NXFileRegion region;
	nodelist_s region_stack;
	nx_uri_t uri;
	volatile size_t reference_count;
};