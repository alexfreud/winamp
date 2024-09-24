#if !defined(_UTILS_H_INCLUDED_)
#define _UTILS_H_INCLUDED_


#include "../pfc/pfc.h"


class NOVTABLE CStream
{
public:
	virtual UINT ReadData(void*,UINT,bool*)=0;
	virtual void Flush()=0;
	virtual ~CStream() {};

	//for sampling
	virtual void Pause(int) {};
	virtual void Eof() {}
};

class CPipe : public CStream
{
	BYTE* buf;
	volatile UINT buf_s,buf_n,buf_rp,buf_wp;
	critical_section sec;
	UINT align;
	volatile bool closed;
public:	
	void WriteData(void*,UINT);
	UINT CanWrite() {return buf_s-buf_n;}
	UINT ReadData(void*,UINT,bool*);
	void Flush()
	{
		sec.enter();
		buf_n=0;
		sec.leave();

	}
	CPipe(UINT _align=4,UINT freq=44100)
	{
		buf_s=MulDiv(1024*256,freq,22050);
		buf=(BYTE*)malloc(buf_s);
		buf_wp=buf_rp=0;
		buf_n=0;
		align=_align;
		closed=0;
	}
	~CPipe()
	{
		if (buf) free(buf);
	}
	void Eof() {closed=1;}
};


class MIDI_file;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	WORD fmt,trax,dtx;
} MIDIHEADER;
#pragma pack(pop)

WORD _inline rev16(WORD x) {return (x>>8)|(x<<8);}
//#define rev16(X) (((X)&0xFF)<<8)|(((X)>>8)&0xFF)
DWORD _fastcall rev32(DWORD);

#define _rv(X) ((((DWORD)(X)&0xFF)<<24)|(((DWORD)(X)&0xFF00)<<8)|(((DWORD)(X)&0xFF0000)>>8)|(((DWORD)(X)&0xFF000000)>>24))

#define FixHeader(H) {(H).fmt=rev16((H).fmt);(H).trax=rev16((H).trax);(H).dtx=rev16((H).dtx);}

struct write_buf;

typedef struct
{
	int pos,tm;
} TMAP_ENTRY;

struct CTempoMap
{
public:
	TMAP_ENTRY *data;
	int pos,size;
	void AddEntry(int _p,int tm);
	~CTempoMap() {if (data) free(data);}
	int BuildTrack(grow_buf & out);
};

CTempoMap* tmap_merge(CTempoMap*,CTempoMap*);

typedef struct
{
	int pos,ofs,len;
} SYSEX_ENTRY;

struct CSysexMap
{
public:
	DWORD d_size,e_size;
	SYSEX_ENTRY *events;
	BYTE* data;
	int pos,d_pos;
	void CleanUp();
	void AddEvent(const BYTE* e,DWORD s,DWORD t);
	~CSysexMap();
	CSysexMap* Translate(MIDI_file* _mf);//MIDI_file* mf
	int BuildTrack(grow_buf & out);
	const char* GetType();
};


typedef struct tagKAR
{
	UINT time;
	UINT start,end;
	BOOL foo;
} KAR_ENTRY;


KAR_ENTRY * kmap_create(MIDI_file* mf,UINT prec,UINT * num,char** text);


CTempoMap* tmap_create();
CSysexMap* smap_create();


int EncodeDelta(BYTE* dst,int d);
unsigned int DecodeDelta(const BYTE* src,unsigned int* _d, unsigned int limit=-1);
int ReadSysex(const BYTE* src,int ml);

char* BuildFilterString(UINT res_id, char* ext, int* len);
BOOL DoOpenFile(HWND w,char* fn,UINT res_id,char* ext,BOOL save);

typedef void (*SYSEXFUNC)(void*,BYTE*,UINT);
void sysex_startup(SYSEXFUNC,void*);
void sysex_startup_midiout(UINT m_id);
bool need_sysex_start();

typedef struct
{
	DWORD tm;
	DWORD ev;
} MIDI_EVENT;

MIDI_EVENT* do_table(MIDI_file* mf,UINT prec,UINT * size,UINT* _lstart,DWORD cflags);

void gb_write_delta(grow_buf & gb,DWORD d);

void do_messages(HWND w,bool* br);
ATOM do_callback_class(WNDPROC p);
HWND create_callback_wnd(ATOM cl,void* p);

class sysex_table
{
private:
	struct entry
	{
		entry * next;
		int size,time;
		BYTE * data;
	};
	entry * entries;
	enum {MHP_MAGIC='0PHM'};
public:
	sysex_table() {entries=0;}
	~sysex_table() {reset();}
	int num_entries() const;
	int get_entry(int idx,BYTE ** p_data,int * p_size,int * p_time) const;
	void insert_entry(int idx,BYTE * data,int size,int time);
	int remove_entry(int idx);
	
	inline void add_entry(BYTE * data,int size,int time) {insert_entry(num_entries(),data,size,time);}
	inline void modify_entry(int idx,BYTE * data,int size,int time) {remove_entry(idx);insert_entry(idx,data,size,time);}
	inline void reset() {while(entries) remove_entry(0);}
	inline int get_time(int idx) const {int time;return get_entry(idx,0,0,&time) ? time : 0;}

	int file_read(const char * path);
	int file_write(const char * path) const;
	void * memblock_write(int * size) const;
	int memblock_read(const void * ptr,int size);

	int print_preview(int idx,char * out) const;
	void print_edit(int idx,HWND wnd) const;
	
	void copy(const sysex_table & src);
	sysex_table(const sysex_table & src) {entries=0;copy(src);}
	sysex_table& operator=(const sysex_table & src) {copy(src);return *this;}
	int is_empty() {return !entries;}
};


#endif
