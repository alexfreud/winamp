#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "NXFileObject.h"

#include "nx/nxfile.h"
#include "nu/nodelist.h"
#include "foundation/atomics.h"
#include "nu/nodelist.h"
#include <errno.h>
#include <new>

/* Windows implementation
note: for now, we're using FILE.  We will eventually replace with better buffering 

TODO: deal with files opened in "append" mode */

NXFileObject::NXFileObject()
{
	nodelist_init(&region_stack);
	region.start = 0;
	region.end = 0xFFFFFFFFFFFFFFFFULL;
	position=0;
	reference_count=1;
	uri=0;
	memset(&file_stats, 0, sizeof(file_stats));
}

NXFileObject::~NXFileObject()
{
	NXURIRelease(uri);
	NXFileRegion *region = (NXFileRegion *)region_stack.head;
	while (region)
	{
		NXFileRegion *next = (NXFileRegion *)region->Next;
		free(region);
		region = next;
	}
}

ns_error_t NXFileObject::Initialize(nx_uri_t _uri)
{
	uri = NXURIRetain(_uri);
	return NErr_Success;
}

size_t NXFileObject::Retain()
{
	return nx_atomic_inc(&reference_count);
}

size_t NXFileObject::Release()
{
	if (!reference_count)
	{
		return reference_count; 
	}
	size_t r = nx_atomic_dec_release(&reference_count); 
	if (!r)
	{
		delete(this);
	}
	return r; 
}

ns_error_t NXFileObject::LockRegion(uint64_t start, uint64_t end)
{
	// save the old region data
	NXFileRegion *old_region = (NXFileRegion *)calloc(1, sizeof(NXFileRegion));
	if (!old_region)
	{
		return NErr_OutOfMemory;
	}
	old_region->start = region.start;
	old_region->end = region.end;
	nodelist_push_front(&region_stack, old_region);

	// if we're already locked, Lock within our current region.  
	// The weird way the logic is done prevents overflow
	if (start > region.end - region.start)
	{
		start = region.end;
	}
	else
	{
		start = region.start + start;
	}

	if (end > region.end - region.start)
	{
		end = region.end;
	}
	else
	{
		end = region.start + end;
	}

	region.start = start;		
	region.end = end;
	return NErr_Success;		
}

ns_error_t NXFileObject::UnlockRegion()
{
	NXFileRegion *new_region = (NXFileRegion *)nodelist_pop_front(&region_stack);
	if (new_region)
	{
		region.start = new_region->start;
		region.end = new_region->end;
		free(new_region);
		return NErr_Success;
	}
	else
	{
		return NErr_NoAction;
	}
}

ns_error_t NXFileObject::Stat(nx_file_stat_t out_stats)
{
	*out_stats = file_stats;
	return NErr_Success;
}

ns_error_t NXFileObject::Length(uint64_t *length)
{
	*length = region.end - region.start;
	return NErr_Success;
}

ns_error_t NXFileObject::EndOfFile()
{
	if (position >= region.end)
	{
		return NErr_True;
	}
	else
	{
		return NErr_False;
	}
}
/* ----------------------------------------- */

class NXFileObject_FILE : public NXFileObject
{
public:
	NXFileObject_FILE();
	~NXFileObject_FILE();
	ns_error_t Initialize(nx_uri_t uri, FILE *f, bool writable);

private:
	FILE *f;
	bool writable;

	ns_error_t Read(void *buffer, size_t bytes_requested, size_t *bytes_read);
	ns_error_t Write(const void *buffer, size_t bytes);
	ns_error_t Seek(uint64_t position);
	ns_error_t Tell(uint64_t *position);
	ns_error_t PeekByte(uint8_t *byte);
	ns_error_t Sync();
	ns_error_t Truncate();
};

NXFileObject_FILE::NXFileObject_FILE()
{
	f = 0;
	writable = false;
}

NXFileObject_FILE::~NXFileObject_FILE()
{
	if (f)
	{
		fclose(f);
	}
}

ns_error_t NXFileObject_FILE::Initialize(nx_uri_t uri, FILE *_f, bool _writable)
{
	writable = _writable;

	ns_error_t ret = NXFileObject::Initialize(uri);
	if (ret != NErr_Success)
	{
		return ret;
	}

	ret = NXFile_statFILE(_f, &file_stats);
	if (ret != NErr_Success)
	{
		return ret;
	}

	region.end = file_stats.file_size;

	f = _f;
	return NErr_Success;
}

ns_error_t NXFileObject_FILE::Read(void *buffer, size_t bytes_requested, size_t *bytes_read)
{
	// if it's an "empty" read, we need to determine whether or not we're at the end of the file
	if (bytes_requested == 0)
	{
		if (region.end == position || feof(f))
		{
			return NErr_EndOfFile;
		}
		else
		{
			return NErr_Success;
		}
	}

	// don't read into any data after the locked region
	if ((uint64_t)bytes_requested > region.end - position)
	{
		bytes_requested = (size_t)(region.end - position);
	}

	if (bytes_requested == 0)
	{
		return NErr_EndOfFile;
	}

	if (buffer == 0)
	{
		uint64_t old_position=position;
		Seek(position+bytes_requested);
		if (bytes_read)
		{
			*bytes_read = (size_t)(position - old_position);
		}
		return NErr_Success;
	}
	else
	{
		size_t results = fread(buffer, 1, bytes_requested, f);
		if (results == 0)
		{
			if (feof(f))
			{
				return NErr_EndOfFile;
			}
			else
			{
				return NErr_Error;
			}
		}
		if (bytes_read)
		{
			*bytes_read = results;
		}
		position+=results;
		return NErr_Success;
	}	
}

ns_error_t NXFileObject_FILE::Write(const void *buffer, size_t bytes)
{
	// TODO: review this in relation to locked regions
	size_t results = fwrite(buffer, 1, bytes, f);
	if (results == 0)
	{
		return NErr_Error;
	}

	position += results;
	if (region.end < position)
	{
		region.end = position;
	}
	return NErr_Success;
}

ns_error_t NXFileObject_FILE::PeekByte(uint8_t *byte)
{
	if (position == region.end)
	{
		return NErr_EndOfFile;
	}

	int read_byte = fgetc(f);
	if (read_byte != EOF)
	{
		ungetc(read_byte, f);
	}
	else
	{
		return NErr_EndOfFile;
	}

	*byte = (uint8_t)read_byte;
	return NErr_Success;
}

ns_error_t NXFileObject_FILE::Seek(uint64_t new_position)
{
	if (!writable)
	{
		// doing it this way will prevent integer overflow
		if (new_position > (region.end - region.start))
		{
			new_position = region.end - region.start;
		}
	}

	if (_fseeki64(f, region.start+new_position, SEEK_SET) == 0)
	{
		position = region.start+new_position;
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

ns_error_t NXFileObject_FILE::Tell(uint64_t *out_position)
{
	*out_position = position - region.start;
	return NErr_Success;
}

ns_error_t NXFileObject_FILE::Sync()
{
	fflush(f);
	return NErr_Success;
}

ns_error_t NXFileObject_FILE::Truncate()
{
	int fd = _fileno(f);
	_chsize_s(fd, position);
	return NErr_Success;
}

/* ----------------------------------------- */
ns_error_t NXFileOpenFile(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags)
{
	FILE *f = NXFile_fopen(filename, flags);
	if (!f)
	{
		if (errno == ENOENT)
		{
			return NErr_FileNotFound;
		}
		else
		{
			return NErr_Error;
		}
	}

	NXFileObject_FILE *file_object = new (std::nothrow) NXFileObject_FILE;
	if (!file_object)
	{
		fclose(f);
		return NErr_OutOfMemory;
	}

	ns_error_t ret = file_object->Initialize(filename, f, !!(flags & nx_file_FILE_writable_mask));
	if (ret != NErr_Success)
	{
		fclose(f);
		delete file_object;
		return ret;
	}

	*out_file = (nx_file_t)file_object;
	return NErr_Success;
}

nx_file_t NXFileRetain(nx_file_t _f)
{
	if (!_f)
	{
		return 0;
	}

	NXFileObject *f = (NXFileObject *)_f;
	f->Retain();
	return _f;
}

void NXFileRelease(nx_file_t _f)
{
	if (_f)
	{
		NXFileObject *f = (NXFileObject *)_f;
		f->Release();
	}
}

ns_error_t NXFileRead(nx_file_t _f, void *buffer, size_t bytes_requested, size_t *bytes_read)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Read(buffer, bytes_requested, bytes_read);
}

ns_error_t NXFileSeek(nx_file_t _f, uint64_t position)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Seek(position);
}

ns_error_t NXFileTell(nx_file_t _f, uint64_t *position)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Tell(position);
}

ns_error_t NXFileLockRegion(nx_file_t _f, uint64_t start_position, uint64_t end_position)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->LockRegion(start_position, end_position);
}

ns_error_t NXFileUnlockRegion(nx_file_t _f)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->UnlockRegion();
}

ns_error_t NXFileStat(nx_file_t _f, nx_file_stat_t file_stats)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Stat(file_stats);
}

ns_error_t NXFileLength(nx_file_t _f, uint64_t *length)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Length(length);
}

ns_error_t NXFileEndOfFile(nx_file_t _f)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->EndOfFile();
}

ns_error_t NXFilePeekByte(nx_file_t _f, uint8_t *byte)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->PeekByte(byte);
}

ns_error_t NXFileWrite(nx_file_t _f, const void *buffer, size_t bytes)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Write(buffer, bytes);
}

ns_error_t NXFileSync(nx_file_t _f)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Sync();
}

ns_error_t NXFileTruncate(nx_file_t _f)
{
	if (!_f)
	{
		return NErr_BadParameter;
	}

	NXFileObject *f = (NXFileObject *)_f;
	return f->Truncate();
}