#include "nxfile.h"
#include "foundation/error.h"
#include <sys/stat.h>
ns_error_t NXFile_move(nx_uri_t destination, nx_uri_t source)
{
	if (!ReplaceFile(destination->string, source->string, 0, 0, 0, 0))
	{
		if (!MoveFile(source->string, destination->string))
		{
			if (!CopyFile(source->string, destination->string, FALSE))
			{
				return NErr_Error;
			}
			DeleteFile(source->string);
		}
	}
	return NErr_Success;
}

ns_error_t NXFile_unlink(nx_uri_t filename)
{
	if (DeleteFile(filename->string))
		return NErr_Success;
	else
		return NErr_Error;
}

ns_error_t NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats)
{
	struct __stat64 buffer; 

	if (_wstat64(filename->string, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

ns_error_t NXFile_statFILE(FILE *f, nx_file_stat_t file_stats)
{
	int fd = _fileno(f);	
	if (fd == -1)
		return NErr_Error;

	return NXFile_fstat(fd, file_stats);
}

ns_error_t NXFile_fstat(int file_descriptor, nx_file_stat_t file_stats)
{
	struct __stat64 buffer; 

	if (_fstat64(file_descriptor, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

