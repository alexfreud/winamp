#include "CompressionUtility.h"
#include "malloc.h"
#include <cstring>
#include "zlib.h"
#include "minizip/unzip.h"


#define dir_delimter '/'
#define MAX_FILENAME 512
#define READ_SIZE 8192

/// <summary>
///  Compress given buffer as GZIP.
///  Dont forget to free out buffer!!!!
/// </summary>
/// <param name="input"></param>
/// <param name="input_size"></param>
/// <param name="ppvOut"></param>
/// <param name="out_size"></param>
/// <returns></returns>
int CompressionUtility::CompressAsGZip(const void* input, size_t input_size, void** ppvOut, size_t& out_size)
{
	z_stream zlib_stream;
	zlib_stream.next_in = (Bytef*)input;
	zlib_stream.avail_in = (uInt)input_size;
	zlib_stream.next_out = Z_NULL;
	zlib_stream.avail_out = Z_NULL;
	zlib_stream.zalloc = (alloc_func)0;
	zlib_stream.zfree = (free_func)0;
	zlib_stream.opaque = 0;
	int ret = deflateInit2(&zlib_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (16 + MAX_WBITS), 8, Z_DEFAULT_STRATEGY);

	unsigned full_length = 2000;
	unsigned half_length = input_size / 2;

	unsigned compLength = full_length;
	unsigned char* comp = (unsigned char*)malloc(compLength);

	bool done = false;

	while (!done)
	{
		if (zlib_stream.total_out >= compLength)
		{
			// Increase size of output buffer  
			unsigned char* uncomp2 = (unsigned char*)malloc(compLength + half_length);
			memcpy(uncomp2, comp, compLength);
			compLength += half_length;
			free(comp);
			comp = uncomp2;
		}

		zlib_stream.next_out = (Bytef*)(comp + zlib_stream.total_out);
		zlib_stream.avail_out = compLength - zlib_stream.total_out;

		// Deflate another chunk.  
		ret = deflate(&zlib_stream, Z_FINISH);
		if (Z_STREAM_END == ret)
		{
			done = true;
		}
		else if (Z_OK != ret)
		{
			break;
		}
	}
	if (Z_OK != deflateEnd(&zlib_stream))
	{
		free(comp);
		return ret;
	}

	*ppvOut = (void*)comp;
	out_size = zlib_stream.total_out;
	
	return ret;
}
/// <summary>
///  Decompress given buffer.
///  Dont forget to free out buffer!!!!
/// </summary>
/// <param name="input"></param>
/// <param name="input_size"></param>
/// <param name="ppvOut"></param>
/// <param name="out_size"></param>
/// <returns></returns>
int CompressionUtility::DecompressGZip(const void* input, size_t input_size, void** ppvOut, size_t& out_size)
{
	z_stream zlib_stream;
	zlib_stream.next_in = (Bytef*)input;
	zlib_stream.avail_in = (uInt)input_size;
	zlib_stream.next_out = Z_NULL;
	zlib_stream.avail_out = Z_NULL;
	zlib_stream.zalloc = (alloc_func)0;
	zlib_stream.zfree = (free_func)0;
	zlib_stream.opaque = Z_NULL;

	int ret = inflateInit2(&zlib_stream, (16 + MAX_WBITS));
	if (Z_OK != ret)
	{
		return ret;
	}
	unsigned full_length = input_size;
	unsigned half_length = input_size / 2;

	unsigned uncompLength = full_length;
	unsigned char* uncomp = (unsigned char*)malloc(uncompLength);

	bool done = false;

	while (!done)
	{
		if (zlib_stream.total_out >= uncompLength) 
		{
			// Increase size of output buffer  
			unsigned char* uncomp2 = (unsigned char*)malloc(uncompLength + half_length);
			memcpy(uncomp2, uncomp, uncompLength);
			uncompLength += half_length;
			free(uncomp);
			uncomp = uncomp2;
		}

		zlib_stream.next_out = (Bytef*)(uncomp + zlib_stream.total_out);
		zlib_stream.avail_out = uncompLength - zlib_stream.total_out;

		// Inflate another chunk.  
		ret = inflate(&zlib_stream, Z_SYNC_FLUSH);
		if (Z_STREAM_END == ret)
		{
			done = true;
		}
		else if (Z_OK != ret) 
		{
			break;
		}
	}
	if (Z_OK != inflateEnd(&zlib_stream)) 
	{
		free(uncomp);
		return ret;
	}

	*ppvOut = (void*)uncomp;
	out_size = zlib_stream.total_out;
	return ret;
}

/// <summary>
///		Returns inflated first file inside the ZIP container,
///		rest are ignored!!!!!
/// </summary>
/// <param name="input"></param>
/// <param name="input_size"></param>
/// <param name="ppvOut"></param>
/// <param name="out_size"></param>
/// <returns></returns>
int CompressionUtility::DecompressPKZip(const char* fn, void** ppvOut, size_t& out_size)
{
	// Open the zip file
	unzFile zipfile = unzOpen(fn);
	if (nullptr == zipfile)
	{
		// file not found
		return -1;
	}

	// Get info about the zip file
	unz_global_info global_info;
	if (UNZ_OK != unzGetGlobalInfo(zipfile, &global_info))
	{
		// could not read file global info
		unzClose(zipfile);
		return -1;
	}

	// Buffer to hold data read from the zip file.
	//char read_buffer[READ_SIZE];

	// Loop to extract all files
	for (uLong i = 0; i < global_info.number_entry; ++i)
	{
		// Get info about current file.
		unz_file_info file_info;
		char filename[MAX_FILENAME];
		if (unzGetCurrentFileInfo(
			zipfile,
			&file_info,
			filename,
			MAX_FILENAME,
			NULL, 0, NULL, 0) != UNZ_OK)
		{
			// could not read file info
			unzClose(zipfile);
			return -1;
		}

		// Check if this entry is a directory or file.
		const size_t filename_length = strlen(filename);
		if (filename[filename_length - 1] == dir_delimter)
		{
			// Entry is a directory, skip it
		}
		else
		{
			// Entry is a file, so extract it.
			if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			{
				// could not open file
				unzClose(zipfile);
				return -1;
			}

			unsigned full_length = READ_SIZE * 2;
			unsigned half_length = READ_SIZE;

			unsigned uncompLength = full_length;
			unsigned char* uncomp = (unsigned char*)malloc(uncompLength);

			size_t total_out = 0;
			int error = UNZ_OK;
			do
			{
				if (total_out >= uncompLength)
				{
					// Increase size of output buffer  
					unsigned char* uncomp2 = (unsigned char*)malloc(uncompLength + half_length);
					memcpy(uncomp2, uncomp, uncompLength);
					uncompLength += half_length;
					free(uncomp);
					uncomp = uncomp2;
				}

				error = unzReadCurrentFile(zipfile, uncomp + total_out, uncompLength - total_out);
				if (error < 0)
				{
					// something happened
					unzCloseCurrentFile(zipfile);
					unzClose(zipfile);
					return -1;
				}

				// Write data to buffer.
				if (error > 0)
				{
					total_out += error;
				}
			} while (error > 0);

			*ppvOut = (void*)uncomp;
			out_size = total_out;
		}

		unzCloseCurrentFile(zipfile);

		// Go the the next entry listed in the zip file.
		//if ((i + 1) < global_info.number_entry)
		//{
		//	if (unzGoToNextFile(zipfile) != UNZ_OK)
		//	{
		//		printf("cound not read next file\n");
		//		unzClose(zipfile);
		//		return -1;
		//	}
		//}
	}

	unzClose(zipfile);

	return UNZ_OK;
}
