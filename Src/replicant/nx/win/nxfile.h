#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../../nx/nxapi.h"
#include <stdio.h> // for FILE

#include "../../nx/nxuri.h"
#include <io.h>
#include <fcntl.h>
#include "../../nx/nxtime.h"
#include "../../foundation/error.h"
#include "../../jnetlib/jnetlib.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct nx_file_stat_s
	{
		nx_time_unix_64_t creation_time;
		nx_time_unix_64_t access_time;
		nx_time_unix_64_t modified_time;
		uint64_t file_size;
	} nx_file_stat_s, *nx_file_stat_t;

	typedef enum
	{
		nx_file_FILE_none = 0,
		nx_file_FILE_binary = (1 << 0),
		nx_file_FILE_read_text= (1 << 1),
		nx_file_FILE_read_binary=nx_file_FILE_read_text|nx_file_FILE_binary,
		nx_file_FILE_write_text=(1 << 2),
		nx_file_FILE_write_binary=nx_file_FILE_write_text|nx_file_FILE_binary,
		nx_file_FILE_update_text=(1 << 3),
		nx_file_FILE_update_binary=nx_file_FILE_update_text|nx_file_FILE_binary,
		nx_file_FILE_readwrite_text=(1 << 4),
		nx_file_FILE_readwrite_binary=nx_file_FILE_readwrite_text|nx_file_FILE_binary,

		nx_file_FILE_writable_mask = nx_file_FILE_write_text|nx_file_FILE_update_text|nx_file_FILE_readwrite_text,
	} nx_file_FILE_flags_t;

	static const int nx_file_O_BINARY=_O_BINARY;
	static const int nx_file_O_WRONLY=_O_WRONLY;
	static const int nx_file_O_RDONLY=_O_RDONLY;

	static FILE *NXFile_fopen(nx_uri_t filename, nx_file_FILE_flags_t flags)
	{
		if (filename)
			{
			if (flags == nx_file_FILE_read_binary)
			{
				return _wfopen(filename->string, L"rb");
			}
			else if (flags == nx_file_FILE_write_binary)
			{
				return _wfopen(filename->string, L"wb");
			}
			else if (flags == nx_file_FILE_update_binary)
			{
				return _wfopen(filename->string, L"r+b");
			}
			else if (flags == nx_file_FILE_readwrite_binary)
			{
				return _wfopen(filename->string, L"w+b");
			}
		}
		return 0;
	}

	/* returns a file descriptor */
	static int NXFile_open(nx_uri_t filename, int flags)
	{
		return _wopen(filename->string, flags);
	}

	NX_API ns_error_t NXFile_move(nx_uri_t destination, nx_uri_t source);
	NX_API ns_error_t NXFile_unlink(nx_uri_t filename);
	NX_API ns_error_t NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats);
	NX_API ns_error_t NXFile_statFILE(FILE *f, nx_file_stat_t file_stats);
	NX_API ns_error_t NXFile_fstat(int file_descriptor, nx_file_stat_t file_stats);

	/* --------------------------------------------------------------------------- */
	typedef struct nx_file_s { size_t dummy; } *nx_file_t;
	NX_API ns_error_t NXFileOpenFile(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags);
	NX_API ns_error_t NXFileOpenProgressiveDownloader(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags, jnl_http_t http, const char *user_agent);
	NX_API ns_error_t NXFileOpenZip(nx_file_t *out_file, nx_uri_t filename, nx_string_t extension_hint);
	NX_API nx_file_t NXFileRetain(nx_file_t f);
	NX_API void NXFileRelease(nx_file_t f);
	/* the implementation of this function will only return NErr_EndOfFile if 0 bytes were read. 
	   when *bytes_read < bytes_requested, it's likely that the file is at the end, but it will still return NErr_Success
	   until the next call */
	NX_API ns_error_t NXFileRead(nx_file_t f, void *buffer, size_t bytes_requested, size_t *bytes_read);
	NX_API ns_error_t NXFileWrite(nx_file_t f, const void *buffer, size_t bytes);
	NX_API ns_error_t NXFileSeek(nx_file_t f, uint64_t position);
	NX_API ns_error_t NXFileTell(nx_file_t f, uint64_t *position);
	NX_API ns_error_t NXFileLockRegion(nx_file_t _f, uint64_t start_position, uint64_t end_position);
	NX_API ns_error_t NXFileUnlockRegion(nx_file_t _f);
	/* file_stats does _not_ take into account the current region */
	NX_API ns_error_t NXFileStat(nx_file_t f, nx_file_stat_t file_stats);
	/* returns the length of the file given the current region */
	NX_API ns_error_t NXFileLength(nx_file_t f, uint64_t *length);
	/* returns NErr_True, NErr_False, or possibly some error */
	NX_API ns_error_t NXFileEndOfFile(nx_file_t f);
	/* this exists as a one-off for nsmp4. hopefully we can get rid of it */
	NX_API ns_error_t NXFilePeekByte(nx_file_t f, uint8_t *byte);
	NX_API ns_error_t NXFileSync(nx_file_t f);
	NX_API ns_error_t NXFileTruncate(nx_file_t f);
	
	/* only valid for Progressive Downloader objects */
	NX_API ns_error_t NXFileProgressiveDownloaderAvailable(nx_file_t f, uint64_t size, uint64_t *available);
	
#ifdef __cplusplus
}
#endif