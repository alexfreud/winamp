#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

static size_t callback_fread(void *ptr, size_t size, size_t nmemb, HANDLE hFile)
{
	DWORD bw = 0;
	ReadFile(hFile,ptr,(DWORD)(size*nmemb),&bw,0);
	return bw/size;
}

static size_t callback_write(void * ptr, size_t size, size_t nmemb, HANDLE hFile)
{
	DWORD bw = 0;
	WriteFile(hFile,ptr,(DWORD)(size*nmemb),&bw,0);
	return bw/size;
}

static int callback_fseek(HANDLE hFile, __int64 offset, int whence)
{
	__int64 temp = offset;
	SetFilePointer(hFile,*(DWORD*)&temp,((long*)&temp+1),whence);
	return 0;
}

static int callback_fclose(HANDLE f)
{
	return 0;
}

static __int64 callback_ftell(HANDLE hFile)
{
	__int64 ret=0;
	*(DWORD*)&ret = SetFilePointer(hFile,0,((long*)&ret+1),FILE_CURRENT);
	return ret;
}

static void* callbacks[4]=
{
	callback_fread,callback_fseek,callback_fclose,callback_ftell
};

namespace ogg_helper
{
	int num_get_tracks(HANDLE hFile/*track_indexer::callback * out,reader * r*/)
	{
		SetFilePointer(hFile,0,0,FILE_BEGIN);
		OggVorbis_File l_vf;
		memset(&l_vf,0,sizeof(l_vf));
		if (ov_open_callbacks(hFile,&l_vf,0,0,*(ov_callbacks*)callbacks))
		{
			return 0;
		}
		int rv = l_vf.links;
		ov_clear(&l_vf);
		return rv;
	}

	int query_chained_stream_offset(HANDLE hFile,int idx,__int64 * out_beginning,__int64 * out_end)
	{
		SetFilePointer(hFile,0,0,FILE_BEGIN);
		OggVorbis_File l_vf;
		memset(&l_vf,0,sizeof(l_vf));
		if (ov_open_callbacks(hFile,&l_vf,0,0,*(ov_callbacks*)callbacks))
		{
			return 0;
		}
		int retval = 0;
		if (idx>=0 && idx<l_vf.links)
		{
			retval = 1;
			*out_beginning = l_vf.offsets[idx];
			*out_end = l_vf.offsets[idx+1];
		}

		ov_clear(&l_vf);
		return retval;
	}
}