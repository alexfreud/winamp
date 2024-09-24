#ifndef NULLSOFT_IN_MIDI_MIDIFILE_H
#define NULLSOFT_IN_MIDI_MIDIFILE_H
#include <dsound.h>

#ifndef MF_NO_DMCRAP
#define MF_USE_DMCRAP
#endif

#ifdef MF_USE_DMCRAP
#include <dmusici.h>
#include <dmusicf.h>
#endif

typedef struct
{
	UINT fmt,ntrax,tix;
	UINT channels;
	const char* e_type;
	string copyright;
	string * traxnames;
} MIDIINFO;

#define FLAG_INCOMPLETE 1

typedef struct tagINSDESC
{
	tagINSDESC * next;
	UINT bank_hi,bank_lo,patch,count,note_max,note_min,channels,user;
	BOOL drum;
} INSTRUMENT_DESC;

class MIDI_file
{
public:
	string path;
	string title;
	int flags;
	int format;
	int len,tix;
	int size;
	const BYTE* data;
#ifdef MF_USE_DMCRAP
	IDirectMusicSegment *pSeg;
	IDirectMusicCollection *pDLS;
	BYTE* pDLSdata;
	int DLSsize;
#endif
	MIDIINFO info;
	int loopstart,loopend;
	int loopstart_t;
	void * rmi_data;//extra RMI crap
	int rmi_size;
	void * bmp_data;//RMI-style bitmap data w/o BITMAPFILEHEADER
	int bmp_size;
	int kar_track;
	CTempoMap * tmap;
	CSysexMap * smap;

	void GetTitle(char *buf, int maxlen);
	inline int GetLength(void) {return len;}

	static MIDI_file* Create(const char* fn,const void * data, size_t size);

	void Free() {if (--refcount==0) delete this;}
	MIDI_file * AddRef() {refcount++;return this;}

	static int HeaderTest(const void * data,int total_size);//test first 256 bytes of file

private:
	int refcount;
	MIDI_file(const char * fn);
	int Load(const void * data,int size);
	~MIDI_file();	
};

#define CLEAN_DM 1
#define CLEAN_1TRACK 2
#define CLEAN_NOSYSEX 4
#define CLEAN_NOTEMPO 8
#define CLEAN_DLS 16

int DoCleanUp(MIDI_file*,DWORD,void** out_data,int * out_size);
INSTRUMENT_DESC* GetInstruments(MIDI_file*,BOOL do_lsb);

#ifdef MF_USE_DMCRAP
IDirectMusicSegment * LoadSegment(MIDI_file*);
void LoadDLS(MIDI_file* mf);
#endif
#endif