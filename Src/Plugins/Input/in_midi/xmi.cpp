#include "main.h"
#include "cvt.h"

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	DWORD mthd,hdsize;
	MIDIHEADER mhd;
} FILEHEADER;

typedef struct
{
	DWORD mtrk,size;
} TRACKHEADER;

#pragma pack(pop)

struct XMI_cvt
{
public:
	grow_buf out;
	bool _end;
	DWORD tr_sz;
	DWORD cur_time,wr_time;
//	DWORD loopstart;
	bool hasevents;
	void q_add(BYTE ch,BYTE note,DWORD tm);
	void WriteDelta(DWORD _d);
	void DoQueue();
	DWORD ProcessNote(const BYTE* e);
	DWORD ProcessDelta(const BYTE* d);
	DWORD WriteEvent(const BYTE* e);
	bool run(MIDI_file* mf,const BYTE*,DWORD);
#pragma pack(push)
#pragma pack(1)
#define Q_MAX 512

struct
{
	DWORD time;
	BYTE note;
	BYTE channel;
} ch_q[Q_MAX];

#pragma pack(pop)

};


#define WriteBuf(A,B) out.write(A,B)
#define WriteBufB(A) out.write_byte(A)
#define WriteBufD(A) out.write_dword(A)

//WORD _fastcall rev16(WORD);
DWORD _fastcall rev32(DWORD);


#define FixHeader(H) {(H).fmt=rev16((H).fmt);(H).trax=rev16((H).trax);(H).dtx=rev16((H).dtx);}
#define MThd 'dhTM'
#define MTrk 'krTM'
#define EVNT 'TNVE'

void XMI_cvt::q_add(BYTE ch,BYTE note,DWORD tm)
{
	UINT n,_n=-1;
	for(n=0;n<Q_MAX;n++)
	{
		if (ch_q[n].note==note && ch_q[n].channel==ch && ch_q[n].time!=-1)
		{
			/*if (ch_q[n].time>tm) */ch_q[n].time=tm;
//			q_notes++;
			return;
		}
		else if (ch_q[n].time==-1) _n=n;
	}
	if (_n!=-1)
	{
		ch_q[_n].channel=ch;
		ch_q[_n].time=tm;
		ch_q[_n].note=note;
//		q_notes++;
	}
}

void XMI_cvt::WriteDelta(DWORD _d)
{
	DWORD d=_d-wr_time;
	wr_time=_d;
	int st=out.get_size();
	gb_write_delta(out,d);
	tr_sz+=out.get_size()-st;
}

DWORD _inline ReadDelta1(const BYTE* d,DWORD* _l)
{
	DWORD r=d[0],l=0;
	while(!(d[l+1]&0x80))
	{
		r+=d[++l];
	}
	*_l=l+1;
	return r;
}

void XMI_cvt::DoQueue()
{
	while(1)
	{
		DWORD _i=-1;
		DWORD _mt=-1;
		UINT i;
		for(i=0;i<Q_MAX;i++)
		{
			if (ch_q[i].time<_mt) {_i=i;_mt=ch_q[i].time;}
		}
		if (_mt<=cur_time)
		{
			WriteDelta(_mt);
			BYTE t[3]={(BYTE)(0x80|ch_q[_i].channel),ch_q[_i].note,0};
			WriteBuf(t,3);
			ch_q[_i].time=-1;
			tr_sz+=3;
//			q_notes--;
		}
		else break;
	}
}

DWORD XMI_cvt::ProcessNote(const BYTE* e)
{
	DoQueue();
	WriteDelta(cur_time);

	WriteBuf(e,3);
	tr_sz+=3;
	DWORD l=3;
	unsigned int _d;
	l+=DecodeDelta(e+l,&_d);


	if (e[2]) q_add(e[0]&0xF,e[1],cur_time+_d);
		
	return l;
}

DWORD XMI_cvt::ProcessDelta(const BYTE* d)
{
	DWORD l;
	cur_time+=ReadDelta1(d,&l);
	return l;
}

DWORD XMI_cvt::WriteEvent(const BYTE* e)
{
	if ((e[0]&0xF0)==0xF0 && e[0]!=0xFF && e[0]!=0xF0)//hack
	{
		UINT l=1;
		while(!(e[l]&0x80)) l++;
		return l;
	}
	else if (e[0]==0xFF)
	{
		if (e[1]==0x2F)
		{
			DWORD _ct=cur_time;
			cur_time=-2;
			DoQueue();
			
//			loopstart=_ct-wr_time;
			cur_time=_ct;
			DWORD _ev=0x002FFF00;
			WriteBuf(&_ev,4);
			tr_sz+=4;
			_end=1;
			return 3;
		}
		else
		{
			UINT l=e[2];
			if (l&0x80)
			{
				l=((l<<7)|e[3])+1;
			}
			return 3+l;
		}
	}
	DoQueue();
	WriteDelta(cur_time);
	if (e[0]==0xF0)
	{
		unsigned int d;
		UINT l = 1 + DecodeDelta(e+1,&d);
		l+=d;
		WriteBuf(e,l);
		tr_sz+=l;
		return l;
	}
	DWORD l;
	//hasevents=1;
	if ((e[0]&0xF0)==0xC0 || (e[0]&0xF0)==0xD0) l=2;
	else l=3;
	WriteBuf(e,l);
	tr_sz+=l;
	return l;	
}

BYTE xmi_track0[19]={'M','T','r','k',0,0,0,11,0,0xFF,0x51,0x03,0x20,0x8d,0xb7,0,0xFF,0x2F,0};

void ReleaseObject(IUnknown* o);

bool XMI_cvt::run(MIDI_file * mf,const BYTE* buf,DWORD sz)
{
//	q_notes=0;
	FILEHEADER fhd=
	{
		MThd,
		0x06000000,
		0x0100,0x0000,0x0001
	};
	WriteBuf(&fhd,sizeof(fhd));
	UINT ptr=0x22;
	DWORD _sz;
	DWORD sp;
	UINT ntrax=1;
	WriteBuf(xmi_track0,sizeof(xmi_track0));
	while(ptr<sz-4 && *(DWORD*)(buf+ptr)!=EVNT) ptr++;
	if (*(DWORD*)(buf+ptr)!=EVNT) goto fail;
//	loopstart=0;
_track:
	cur_time=wr_time=0;
	WriteBufD(_rv('MTrk'));
	sp=out.get_size();
	WriteBufD(0);
	ntrax++;
	tr_sz=0;
	ptr+=4;
	_sz=ptr+4+rev32(*(DWORD*)(buf+ptr));
	if (_sz>sz+1) goto fail;
	ptr+=4;
	_end=0;
	{
		UINT n;
		for(n=0;n<Q_MAX;n++) ch_q[n].time=-1;
	}
	hasevents=0;
	while(ptr<sz-1 && !_end)
	{
		if ((buf[ptr]&0x80)==0)
		{
			ptr+=ProcessDelta(buf+ptr);
		}
		if ((buf[ptr]&0xF0)==0x90)
		{
			hasevents=1;
			ptr+=ProcessNote(buf+ptr);
		}
		else ptr+=WriteEvent(buf+ptr);
	}
	if (!hasevents) {out.truncate(sp-4);ntrax--;}
	else out.write_dword_ptr(rev32(tr_sz),sp);
	if (ptr&1) ptr++;
	if (ptr>=sz) goto _e;
	if (*(DWORD*)(buf+ptr)==_rv('FORM') && *(DWORD*)(buf+ptr+8)==_rv('XMID'))
	{
		ptr+=12;
_te:	if (ptr&1) ptr++;
		if (*(DWORD*)(buf+ptr)==_rv('EVNT')) goto _track;
		else if (*(DWORD*)(buf+ptr)==_rv('TIMB'))
		{
			ptr+=8+rev32(*(DWORD*)(buf+ptr+4));
			if (ptr<sz) goto _te;
		}
	}
_e:
	{
		WORD w=rev16(ntrax);
		out.write_ptr(&w,2,10);
		if (ntrax>1)
			w=0x200;
		out.write_ptr(&w,2,8);
	}
	
	mf->size = out.get_size();
	mf->data = (BYTE*)out.finish();
	if (!mf->data) return 0;

//	if (loopstart>0x10) mf->loopstart=loopstart;
#ifdef MF_USE_DMCRAP
	if (sz>ptr+0x20 && *(DWORD*)(buf+ptr)==_rv('FORM') && *(DWORD*)(buf+ptr+8)==_rv('XDLS'))
	{
		DWORD rs=rev32(*(DWORD*)(buf+_sz+4));
		if (rs+12+_sz>sz) goto _ret;
		if (*(DWORD*)(buf+ptr+0x14)!=_rv('DLS ')) goto _ret;
		mf->DLSsize=rs;
		mf->pDLSdata=(BYTE*)malloc(rs);
		memcpy(mf->pDLSdata,buf+_sz+12,rs);
	}
#endif
_ret:
	return 1;
fail:
	return 0;
}

bool load_xmi(MIDI_file * mf,const BYTE* buf,size_t sz)
{
	XMI_cvt c;
	return c.run(mf,buf,sz);
}