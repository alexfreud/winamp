#ifndef IN_VORBIS_MAIN_H
#define IN_VORBIS_MAIN_H

#define WINSOCK_API_LINKAGE

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>

extern int (*warand)();
extern float (*warandf)();

inline void * z_malloc(int x)
{
	void* foo=malloc(x);
	if (foo) memset(foo,0,x);
	return foo;
}
#include <shlwapi.h>
#include <malloc.h>
#define uitoa(x,y) _itoa(x,y,10)
#define atoui atoi

#include <vorbis\vorbisfile.h>
#include "c_string.h"
#include "../Winamp/in2.h"

extern In_Module mod;

#include "resource.h"

#define VER L"1.79"
#define _NAME "Nullsoft Vorbis Decoder"

extern "C"
{
	extern const char *INI_FILE;
	extern const wchar_t *INI_DIRECTORY;
}
class CfgVar
{
private:
	String name;
	CfgVar * next;
	static CfgVar * list;
public:
	
	static void ReadConfig();
	static void WriteConfig();

	//helpers
	static bool read_struct(const char *inifile, const char *section, const char * name,void * ptr,UINT size);
	static void write_struct(const char *inifile, const char *section, const char * name,void * ptr,UINT size);
	static void write_int(const char *inifile, const char *section, const char * name,int val);
	static int read_int(const char *inifile, const char *section,const char * name,int def);

protected:
	CfgVar(const char * n) : name(n) {next=list;list=this;}
	virtual void Read(const char * name)=0;
	virtual void Write(const char * name)=0;
};

class CfgInt : private CfgVar
{
private:
	int def,value;
public:
	CfgInt(const char * name,int _def) : CfgVar(name) {value=def=_def;}
	inline int operator=(int x) {value=x;return value;}
	inline operator int() {return value;}
private:
	virtual void Read(const char * name);
	virtual void Write(const char * name);
};

class CfgString : private CfgVar, public StringW
{
private:
	StringW def;
public:
	CfgString(const char * name,const char * _def) : CfgVar(name), StringW(_def), def(_def) {}
private:
	virtual void Read(const char * name);
	virtual void Write(const char * name);
};

template<class T>
class CfgStructT : private CfgVar
{
public:
	T data;
	CfgStructT(const char * name) : CfgVar(name) {}
private:
	void Read(const char * name) { read_struct(INI_FILE, "in_vorbis",name,&data,sizeof(data));}
	void Write(const char * name) {if (IsValueDefault()) WritePrivateProfileStringA("in_vorbis", name, 0, INI_FILE); else write_struct(INI_FILE, "in_vorbis", name, &data, sizeof(data));}
protected:
	virtual bool IsValueDefault() {return 0;}
};


class CfgFont : public CfgStructT<LOGFONT>
{
private:
	void get_def(LOGFONT * f) {memset(f,0,sizeof(LOGFONT));GetObject(GetStockObject(DEFAULT_GUI_FONT),sizeof(LOGFONT),f);}
	virtual bool IsValueDefault()
	{
		LOGFONT t;
		get_def(&t);
		return !memcmp(&data,&t,sizeof(LOGFONT));
	}
public:
	CfgFont(const char * name) : CfgStructT<LOGFONT>(name)
	{
		get_def(&data);
	}
};

extern int32_t priority_tab[7];
extern HINSTANCE hIns;

extern CfgString cfg_ssave_format,cfg_dumpdir;

int is_http(const char* url);

class VorbisFile
{
protected:
	virtual int f_seek(__int64 offset,int whence)=0;
	virtual size_t f_read(UINT siz,void * ptr)=0;
	virtual UINT f_tell()=0;
	static int _f_close(void *);
	static int _f_seek(void* rs,__int64 offset,int whence);
	static size_t _f_read(void* ptr,size_t size,size_t nmemb,void * rs);
	static long _f_tell(void* rs);
	static ov_callbacks oc;
	static VorbisFile * Create_HTTP(const char * url,bool is_info);
	VorbisFile(const wchar_t * u, bool is_info) : url(u) {memset(&vf,0,sizeof(vf));stopping=0;abort_prebuf=0;avg_kbps=0;use_prebuf=0;primary=!is_info; baseoffs=0;}
	bool init();
	virtual void post_init() {};
	UINT avg_kbps;
	bool Aborting();

  __int64 baseoffs;
public:
	enum {TYPE_LOCAL,TYPE_HTTP};
	virtual int GetType()=0;
	virtual bool IsLive() {return 0;}
	virtual void do_prebuf() {use_prebuf=1;abort_prebuf=0;};
	StringW url;
	String withlp;
	String stream_title;
	bool stopping,abort_prebuf,use_prebuf;
	bool primary;//display status messages or not
	OggVorbis_File vf;
	UINT get_avg_bitrate()
	{
		if (avg_kbps>0) return avg_kbps;
		vorbis_info * vi=ov_info(&vf,-1);
		if (!vi) return 0;
		return vi->bitrate_nominal/1000;
	}	

	const char* get_meta(const char* tag,UINT c);
	void set_meta(const vorbis_comment * vc,int links);

	static VorbisFile * Create(const wchar_t * url,bool is_info);

	double Length() {return ov_time_total(&vf,-1);}
	double GetPos() {return ov_time_tell(&vf);}
	int Seek(double p);
	void Status(const wchar_t * zzz);
	virtual UINT FileSize()=0;
	
	virtual ~VorbisFile() {ov_clear(&vf);}
	virtual void Idle() {Sleep(10);}

	virtual void setBaseOffset(__int64 offs) { baseoffs=offs; }

	float GetGain();
};

extern VorbisFile * theFile;

extern StringW cur_file;

extern CRITICAL_SECTION sync;

BOOL modify_file(const wchar_t* url,const vorbis_comment * comments,int links);
void winampGetExtendedFileInfoW_Cleanup(void);
void UpdateFileTimeChanged(const wchar_t *fn);
void do_cfg(int s);
bool KeywordMatch(const char *mainString, const char *keyword);

class Info
{
public:
	Info(const wchar_t *filename);
	~Info();
	bool Save();
	int Error() { return vc==0?1:0; }
	int GetNumMetadataItems();
	void EnumMetadata(int n,wchar_t *key,int keylen, wchar_t *val, int vallen);
	void RemoveMetadata(wchar_t * key);
	void RemoveMetadata(int n);
	void SetMetadata(wchar_t *key, wchar_t *val);
	void SetMetadata(int n, wchar_t *key, wchar_t *val);
	void SetTag(int n,wchar_t *key); // changes the key name
private:
	const wchar_t *filename;
	vorbis_comment * vc;
	int numstreams, stream;
};

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
    { 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#endif //IN_VORBIS_MAIN_H