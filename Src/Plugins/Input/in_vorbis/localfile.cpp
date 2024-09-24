#include "main.h"
#include "../nu/AutoChar.h"

extern CfgInt cfg_fullbuf;

int VorbisFile::_f_close(void *) {return 0;}

int VorbisFile::_f_seek(void* rs,__int64 offset,int whence)
{
	return ((VorbisFile*)rs)->f_seek(offset,whence);
}

size_t VorbisFile::_f_read(void* ptr,size_t size,size_t nmemb,void * rs)
{
	return ((VorbisFile*)rs)->f_read((UINT)(size*nmemb),ptr);
}

long VorbisFile::_f_tell(void* rs) 
{
	return ((VorbisFile*)rs)->f_tell();
}

ov_callbacks VorbisFile::oc={_f_read,_f_seek,_f_close,_f_tell};

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

static __int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

class VorbisFile_Local : public VorbisFile
{
private:
	HANDLE hFile;
protected:
	int f_seek(__int64 offset,int whence)
	{
	    if(whence==SEEK_SET) offset+=baseoffs;
		if (Seek64(hFile,offset,whence) != INVALID_SET_FILE_POINTER) return 0;
		else return -1;
	}

	size_t f_read(UINT siz,void * ptr)
	{
	    DWORD bw=0;
		ReadFile(hFile,ptr,siz,&bw,0);
		return bw;
	}

	UINT f_tell()
	{
	    return (UINT)(SetFilePointer(hFile,0,0,FILE_CURRENT)-baseoffs);
	}

	UINT FileSize()
	{
		return (UINT)(FileSize64(hFile)-baseoffs);
	}

public:
	virtual int GetType() {return TYPE_LOCAL;}
	VorbisFile_Local(HANDLE f,const wchar_t * u,bool is_info) : VorbisFile(u,is_info) {hFile=f;}
	~VorbisFile_Local() {CloseHandle(hFile);}
};

class VorbisFile_Mem : public VorbisFile
{
	BYTE * block;
	UINT size,ptr;
protected:
	int f_seek(__int64 offset,int whence)
	{
		switch(whence)
		{
			case SEEK_SET:
				ptr=(UINT)(offset+baseoffs);
				break;
			case SEEK_CUR:
				ptr+=(UINT)offset;
				break;
			case SEEK_END:
				ptr=size+whence;
				break;
		}
		if (ptr<=size) return 0;
		else {ptr=size;return -1;}
	}

	size_t f_read(UINT siz,void * out)
	{
		UINT d=size-ptr;
		if (d>siz) d=siz;
		memcpy(out,block+ptr,d);
		ptr+=d;
		return d;
	}

	UINT f_tell()
	{
	    return (UINT)(ptr-baseoffs);
	}

	UINT FileSize()
	{
		return (UINT)(size-baseoffs);
	}

public:
	virtual int GetType() {return TYPE_LOCAL;}

	VorbisFile_Mem(HANDLE f,const wchar_t * u,bool is_info) : VorbisFile(u,is_info)
	{
		size=GetFileSize(f,0);
		ptr=0;
		block=(BYTE*)malloc(size);
		DWORD br = 0;
		ReadFile(f,block,size,&br,0);
		CloseHandle(f);
	}

	~VorbisFile_Mem() {free(block);}
};

VorbisFile * VorbisFile::Create(const wchar_t *url, bool is_info)
{
	VorbisFile * r;
	if (PathIsURLW(url))
	{
	    if (is_info) return 0;
		r=Create_HTTP(AutoChar(url),is_info);
	}
	else
	{
		__int64 baseoffs=0;
		HANDLE f=CreateFileW(url,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
		if (f==INVALID_HANDLE_VALUE) return 0;
	    {
			DWORD dw = 0, br = 0;
			ReadFile(f,&dw,4,&br,0);
			if(br==4 && dw=='SggO')
			{
				SetFilePointer(f,0,0,FILE_BEGIN);
			} 
			else if(br==4 && dw=='FFIR')
			{
				//RIFF file
				DWORD wavhdr = 0, nb = 0;
				SetFilePointer(f,4,0,FILE_CURRENT);
				ReadFile(f,&wavhdr,4,&nb,0);
				if(nb!=4 || wavhdr!='EVAW')
				{
					goto abort;
				}

				//find data starting point
				char tmp[1024] = {0};
				ReadFile(f,&tmp,1024,&nb,0);
				for(int i=0;i<1020;i++)
				if(tmp[i]=='d'&&tmp[i+1]=='a'&&tmp[i+2]=='t'&&tmp[i+3]=='a')
				{
					baseoffs=i+12+8;
					Seek64(f, baseoffs, FILE_BEGIN);
				}

				if(!baseoffs) goto abort;
			}
			else
			{
		abort:
				CloseHandle(f);
				return 0;
			}
		}

		r=cfg_fullbuf ? (VorbisFile*)new VorbisFile_Mem(f,url,is_info) : (VorbisFile*)new VorbisFile_Local(f,url,is_info);
		r->setBaseOffset(baseoffs);
	}
	if (r && !r->init())
	{
	    delete r;
		r=0;
	}
	return r;
}

bool VorbisFile::init()
{
	if (ov_open_callbacks(this,&vf,0,0,oc)) return 0;
	//TODO bitrate
	UINT siz=FileSize();
	double len=Length();
	if (siz>0 && len>0)
	{
		UINT divisor = (UINT)(len*125.0);
		if (divisor)
			avg_kbps=siz/divisor;
	}

	post_init();
	return 1;
}

int is_http(const char* url)
{
	return (!_strnicmp(url,"http://",7) || !_strnicmp(url,"https://",8));
}

void VorbisFile::set_meta(const vorbis_comment * vc,int links)
{
	if (links == vf.links)
	{
		int n;
		for(n=0;n<links;n++)
		{
			vorbis_comment_clear(vf.vc+n);
			/*
			extern void vorbis_comment_init(vorbis_comment *vc);
			extern void vorbis_comment_add(vorbis_comment *vc, char *comment); 
			extern void vorbis_comment_add_tag(vorbis_comment *vc,char *tag, char *contents);
			extern char *vorbis_comment_query(vorbis_comment *vc, char *tag, int count);
			extern int vorbis_comment_query_count(vorbis_comment *vc, char *tag);
			extern void vorbis_comment_clear(vorbis_comment *vc);
			*/
		}
		_ogg_free(vf.vc);
		vf.vc = (vorbis_comment*) _ogg_calloc(links,sizeof(vorbis_comment));
		for(n=0;n<links;n++)
		{
			vorbis_comment_init(vf.vc+n);
			int c;
			for(c=0;c<vc[n].comments;c++)
			{
				vorbis_comment_add(vf.vc+n,vc[n].user_comments[c]);
			}
			vf.vc[n].vendor = _strdup(vc[n].vendor);
		}
	}
}