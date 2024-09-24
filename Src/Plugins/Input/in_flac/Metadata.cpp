/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#include "Metadata.h"
#include <FLAC/all.h>
#include "StreamFileWin32.h" // for FileSize64
#include "api__in_flv.h"
#include <strsafe.h>

struct MetadataReader
{
	MetadataReader(HANDLE _handle)
	{
		handle = _handle;
		endOfFile=false;
	}
		HANDLE handle;
	bool endOfFile;	
};

static size_t win32_read(void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	DWORD bytesRead=0;
	BOOL result = ReadFile(reader->handle, ptr, size*nmemb, &bytesRead, NULL);
	if (result == TRUE && bytesRead == 0)
		reader->endOfFile=true;
	return bytesRead/size;
}

static size_t win32_write(const void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	DWORD bytesWritten=0;
	WriteFile(reader->handle, ptr, size*nmemb, &bytesWritten, NULL);
	return bytesWritten/size;
}

static __int64 Seek64(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

static int win32_seek(FLAC__IOHandle handle, FLAC__int64 offset, int whence)
{
		MetadataReader *reader = (MetadataReader *)handle;
	if (Seek64(reader->handle, offset, whence) == -1)
		return -1;
	else
		return 0;
	
}

static FLAC__int64 win32_tell(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	return Seek64(reader->handle, 0, FILE_CURRENT);
}

static int win32_eof(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	return !!reader->endOfFile;
}

static int win32_close(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;

	CloseHandle(reader->handle);
	reader->handle = INVALID_HANDLE_VALUE;
	return 0;
}

static FLAC__IOCallbacks unicodeIO =
{
  win32_read,
  win32_write,
  win32_seek,
  win32_tell,
  win32_eof,
  win32_close,
};

FLACMetadata::FLACMetadata()
{
	chain=FLAC__metadata_chain_new();
	itr = FLAC__metadata_iterator_new();
	block=0;
	streamInfo=0;
	filesize=0;
}

FLACMetadata::~FLACMetadata()
{
	if (chain)
		FLAC__metadata_chain_delete(chain);
	if (itr)
		FLAC__metadata_iterator_delete(itr);
}

void FLACMetadata::Reset()
{
	if (chain)
		FLAC__metadata_chain_delete(chain);
	if (itr)
		FLAC__metadata_iterator_delete(itr);
	chain=FLAC__metadata_chain_new();
	itr = FLAC__metadata_iterator_new();
	block=0;
	streamInfo=0;
	filesize=0;
}

const FLAC__StreamMetadata_StreamInfo *FLACMetadata::GetStreamInfo()
{
	if (streamInfo)
		return &streamInfo->data.stream_info;
	else
		return 0;
}

bool FLACMetadata::Open(const wchar_t *filename, bool optimize)
{
	if (!chain || !itr)
		return false;

	HANDLE hfile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	MetadataReader reader(hfile);
	if (reader.handle == INVALID_HANDLE_VALUE)
		return false;
	filesize = FileSize64(reader.handle);

	FLAC__bool success = FLAC__metadata_chain_read_with_callbacks(chain, &reader, unicodeIO);
	CloseHandle(hfile);
	if (!success)
		return false;

	if (optimize)
	{
		FLAC__metadata_chain_sort_padding(chain);
		FLAC__metadata_chain_merge_padding(chain);
	}

	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		FLAC__MetadataType type=FLAC__metadata_iterator_get_block_type(itr);
		switch (type)
		{
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			block = FLAC__metadata_iterator_get_block(itr);
			break;
		case FLAC__METADATA_TYPE_STREAMINFO:
			streamInfo = FLAC__metadata_iterator_get_block(itr);
			break;
		}
		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}
	return true;
}

const char *FLACMetadata::GetMetadata(const char *tag)
{
	if (!block)
		return 0;

	int pos = FLAC__metadata_object_vorbiscomment_find_entry_from(block, 0, tag);
	if (pos < 0)
	{
		// fail
	}
	else
	{
		const char *entry = (const char *)block->data.vorbis_comment.comments[pos].entry;
		const char *metadata = strchr(entry, '='); // find the first equal
		if (metadata)
		{
			return metadata+1;
		}
	}

	return 0;
}

void FLACMetadata::SetMetadata(const char *tag, const char *value)
{
	if (!block)
	{
		FLAC__metadata_iterator_init(itr, chain);
		do
		{
			if (FLAC__METADATA_TYPE_VORBIS_COMMENT == FLAC__metadata_iterator_get_block_type(itr))
			{
				block = FLAC__metadata_iterator_get_block(itr);
				break;
			}
		}
		while (FLAC__metadata_iterator_next(itr) != 0);
		if (!block)
		{
			block = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
			FLAC__metadata_iterator_insert_block_after(itr, block);
		}		
	}

	if (!block)
		return;

	FLAC__StreamMetadata_VorbisComment_Entry entry;
	size_t totalLen = strlen(tag) + 1 /* = */ + strlen(value);
	entry.entry = (FLAC__byte *)malloc(totalLen + 1);
	entry.length = totalLen;

	StringCchPrintfA((char *)entry.entry, totalLen+1, "%s=%s", tag, value);

	int pos = FLAC__metadata_object_vorbiscomment_find_entry_from(block, 0, tag);
	if (pos < 0)
	{
		//new comment
		FLAC__metadata_object_vorbiscomment_append_comment(block, entry, true);
		// would love to not copy, but we can't guarantee that FLAC links to the same CRT as us
	}
	else
	{
		FLAC__metadata_object_vorbiscomment_set_comment(block, pos, entry, true);
		// would love to not copy, but we can't guarantee that FLAC links to the same CRT as us
	}
	free(entry.entry);
}

void FLACMetadata::RemoveMetadata(const char *tag)
{
	if (!block)
		return;

	FLAC__metadata_object_vorbiscomment_remove_entries_matching(block, tag);
}

void FLACMetadata::RemoveMetadata(int n)
{
	if (!block)
		return;

	FLAC__metadata_object_vorbiscomment_delete_comment(block, (unsigned int)n);
}

static FLAC__StreamMetadata *GetOrMakePadding(FLAC__Metadata_Chain *chain, FLAC__Metadata_Iterator *itr)
{
	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PADDING == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			return block;
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}
	FLAC__StreamMetadata *padding = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
	if (padding)
		FLAC__metadata_iterator_insert_block_after(itr, padding);

	return padding;
}

bool FLACMetadata::Save(const wchar_t *filename)
{
	if (FLAC__metadata_chain_check_if_tempfile_needed(chain, true))
	{
		// since we needed to write a tempfile, let's add some more padding so it doesn't happen again
		FLAC__metadata_chain_sort_padding(chain);

		FLAC__StreamMetadata *padding = GetOrMakePadding(chain, itr);
		if (padding && padding->length < 16384)
			padding->length = 16384; // TODO: configurable padding size

		HANDLE hfile = CreateFileW(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		MetadataReader reader(hfile);
		if (reader.handle == INVALID_HANDLE_VALUE)
			return false;

		wchar_t tempPath[MAX_PATH-14] = {0}, tempFile[MAX_PATH] = {0};
		GetTempPathW(MAX_PATH-14, tempPath);
		GetTempFileNameW(tempPath, L"waf", 0, tempFile);

		HANDLE hTempFile = CreateFileW(tempFile, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		MetadataReader tempReader(hTempFile);

		FLAC__bool res = FLAC__metadata_chain_write_with_callbacks_and_tempfile(chain, false, &reader, unicodeIO, &tempReader, unicodeIO);

		CloseHandle(hfile);
		CloseHandle(hTempFile);
		if (!MoveFileW(tempFile, filename))
		{
			if (CopyFileW(tempFile, filename, FALSE))
			{
				DeleteFileW(tempFile);
			}
			else
			{
				DeleteFileW(tempFile);
				return false;
			}
		}
		return !!res;
	}
	else
	{
		HANDLE hfile = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		MetadataReader reader(hfile);
		if (reader.handle == INVALID_HANDLE_VALUE)
			return false;

		FLAC__bool res = FLAC__metadata_chain_write_with_callbacks(chain, true, &reader, unicodeIO);

		CloseHandle(hfile);
		return !!res;
	}
}

bool FLACMetadata::GetLengthMilliseconds(unsigned __int64 *length)
{
	if (!streamInfo)
		return false;
	*length = (__int64)((double)(FLAC__int64)streamInfo->data.stream_info.total_samples / (double)streamInfo->data.stream_info.sample_rate * 1000.0 + 0.5);
	return true;
}

int FLACMetadata::GetNumMetadataItems()
{
	if (block) return block->data.vorbis_comment.num_comments;
	else return 0;
}

const char* FLACMetadata::EnumMetadata(int n, char *tag, int taglen)
{
	if (tag) tag[0]=0;
	if (!block) return 0;
	const char *entry = (const char *)block->data.vorbis_comment.comments[n].entry;
	const char *metadata = strchr(entry, '='); // find the first equal
	if (metadata)
	{
		if (tag) lstrcpynA(tag,entry,min(metadata-entry+1,taglen));
		return metadata+1;
	}
	else return 0;
}

void FLACMetadata::SetTag(int pos, const char *tag)
{
	char * value = (char*)EnumMetadata(pos,0,0);
	value = _strdup(value?value:"");
	FLAC__StreamMetadata_VorbisComment_Entry entry;
	size_t totalLen = strlen(tag) + 1 /* = */ + strlen(value);
	entry.entry = (FLAC__byte *)malloc(totalLen + 1);
	entry.length = totalLen;
	StringCchPrintfA((char *)entry.entry, totalLen+1, "%s=%s", tag, value);
	FLAC__metadata_object_vorbiscomment_set_comment(block, pos, entry, true);
	free(value);
}

bool FLACMetadata::GetPicture(FLAC__StreamMetadata_Picture_Type type, void **data, size_t *len, wchar_t **mimeType)
{
	if (!chain || !itr)
		return false;

	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			FLAC__StreamMetadata_Picture &picture = block->data.picture;
			if (picture.type == type)
			{
				*len = picture.data_length;
				*data = WASABI_API_MEMMGR->sysMalloc(picture.data_length);
				if (!*data)
					return false;
				memcpy(*data, picture.data, picture.data_length);

				char *type = 0;
				if (picture.mime_type)
					type = strchr(picture.mime_type, '/');

				if (type && *type)
				{
					type++;

					char *type2 = strchr(type, '/');
					if (type2 && *type2) type2++;
					else type2 = type;

					int typelen = MultiByteToWideChar(CP_ACP, 0, type2, -1, 0, 0);
					*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(typelen * sizeof(wchar_t));
					if (*mimeType)
						MultiByteToWideChar(CP_ACP, 0, type, -1, *mimeType, typelen);
				}
				else
					*mimeType = 0; // unknown!

				return true;
			}
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}

	return false;
}

bool FLACMetadata::RemovePicture(FLAC__StreamMetadata_Picture_Type type)
{
	if (!chain || !itr)
		return false;

	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			FLAC__StreamMetadata_Picture &picture = block->data.picture;
			if (picture.type == type)
			{
				FLAC__metadata_iterator_delete_block(itr, false);
				return true;
			}
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}

	return false;
}

bool FLACMetadata::SetPicture(FLAC__StreamMetadata_Picture_Type type, void *data, size_t len, const wchar_t *mimeType, int width, int height)
{
	if (!chain || !itr)
		return false;

	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			FLAC__StreamMetadata_Picture &picture = block->data.picture;
			if (picture.type == type)
			{
				FLAC__metadata_object_picture_set_data(block, (FLAC__byte *)data, len, true);
				picture.width = width;
				picture.height = height;
				picture.depth = 32;
				picture.colors = 0;

				FLAC__metadata_object_picture_set_description(block, (FLAC__byte *)"", true);// TODO?

				char mime[256] = {0};
				if (wcsstr(mimeType, L"/") != 0)
				{
					StringCchPrintfA(mime, 256, "%S", mimeType);
				}
				else
				{
					StringCchPrintfA(mime, 256, "image/%S", mimeType);
				}
				FLAC__metadata_object_picture_set_mime_type(block, mime, true);
				return true;
			}
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}

	// not found. let's add it
	FLAC__StreamMetadata *newBlock = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);

	FLAC__metadata_object_picture_set_data(newBlock, (FLAC__byte *)data, len, true);
	FLAC__StreamMetadata_Picture &picture = newBlock->data.picture;
	picture.type = type;
	picture.width = width;
	picture.height = height;
	picture.depth = 32;
	picture.colors = 0;

	FLAC__metadata_object_picture_set_description(newBlock, (FLAC__byte *)"", true);// TODO?

	char mime[256] = {0};
	StringCchPrintfA(mime, 256, "image/%S", mimeType);
	FLAC__metadata_object_picture_set_mime_type(newBlock, mime, true);

	FLAC__metadata_iterator_insert_block_after(itr, newBlock);
	return true;
}

bool FLACMetadata::GetIndexPicture(int index, FLAC__StreamMetadata_Picture_Type *type, void **data, size_t *len, wchar_t **mimeType)
{
	if (!chain || !itr)
		return false;
	int i=0;
	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			FLAC__StreamMetadata_Picture &picture = block->data.picture;
			if (i++ == index)
			{
				*type = picture.type;
				*len = picture.data_length;
				*data = WASABI_API_MEMMGR->sysMalloc(picture.data_length);
				if (!*data)
					return false;
				memcpy(*data, picture.data, picture.data_length);

				char *type = 0;
				if (picture.mime_type)
					type = strchr(picture.mime_type, '/');

				if (type && *type)
				{
					type++;
					int typelen = MultiByteToWideChar(CP_ACP, 0, type, -1, 0, 0);
					*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(typelen * sizeof(wchar_t));
					if (*mimeType)
						MultiByteToWideChar(CP_ACP, 0, type, -1, *mimeType, typelen);
				}
				else
					*mimeType = 0; // unknown!

				return true;
			}
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}

	return false;
}