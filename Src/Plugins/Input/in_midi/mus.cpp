#include "main.h"
#include "cvt.h"

bool is_mus(const BYTE* buf,size_t s)
{
	if (s>0x20 && *(DWORD*)buf == '\x1ASUM')
	{
		int ofs = ((WORD*)buf)[3];
		int n_ins = ((WORD*)buf)[6];
		if (ofs>=16+(n_ins<<1) && ofs<16+(n_ins<<2) && ofs<s)
		{
			return 1;
		}
	}
	return 0;
}

static BYTE tempodat[] = {0x00,0xFF,0x51,0x03,0x09,0xA3,0x1A};

static BYTE controllers[15] = {0,0,1,7,10,11,91,93,64,67,120,123,126,127,121};

#define abort _abort_

struct MUS_cvt
{
public:
	bool abort;
	DWORD ct;
	DWORD lt;
	grow_buf out;
	
	void AddEvent(DWORD ev,int l);
	bool run(MIDI_file* mf,const BYTE* ptr,DWORD sz);
};

void MUS_cvt::AddEvent(DWORD ev,int l)
{
	DWORD dt = ct - lt;
	int tl=3;
	gb_write_delta(out,dt);
	lt = ct;
	BYTE ec=(BYTE)(ev&0xF0);
	out.write(&ev,l);
}

bool MUS_cvt::run(MIDI_file* mf,const BYTE* ptr,DWORD sz)
{
#pragma pack(push)
#pragma pack(1)
	struct
	{
		char id[4];
		WORD len;
		WORD ofs;
		WORD ch1,ch2;
		WORD n_ins;
		WORD dummy;
	} hdr;
#pragma pack(pop)
	DWORD _pt=0;
	memcpy(&hdr,ptr,sizeof(hdr));
	const BYTE* score = ptr+sizeof(hdr)+2*hdr.n_ins;
	long x;
	
	static BYTE _hd_[]={'M','T','h','d',0,0,0,6, 0,0,0,1,0,0x59,'M','T','r','k'};
	out.write(_hd_,sizeof(_hd_));
	DWORD ts_ofs=out.get_size();
	out.write_dword(0);

	lt=0;
	abort = 0;
	ct = 0;
	out.write(tempodat,sizeof(tempodat));

	x=0;
	bool t;
	BYTE ch;
	BYTE vols[16];
	ZeroMemory(vols,sizeof(vols));
	union
	{
		BYTE b[4];
		DWORD dw;
	} ev;
	while(x<hdr.len && score[x]!=0x60)
	{
		ev.dw = 0;
		t=(score[x]&0x80)?1:0;
		ch = score[x]&0xF;
		if (ch == 0xF) ch = 9;//hdr.ch1+1;
		else if (ch>=9) ch++;
		switch(score[x]&0x70)
		{
		case 0:	//release note
			ev.b[0]=0x80|ch;
			ev.b[1]=score[x+1];
			ev.b[2]=0;//vols[ch];
			AddEvent(ev.dw,3);
			x+=2;
			break;
		case 0x10:	//play note
			ev.b[0]=0x90|ch;
			ev.b[1]=score[x+1]&0x7F;
			if (score[x+1]&0x80)
			{
				vols[ch]=score[x+2];
				x+=3;
			}
			else
			{
				x+=2;
			}
			ev.b[2]=vols[ch];
			AddEvent(ev.dw,3);
			break;
		case 0x20:	//pitch wheel
			ev.b[0]=0xE0|ch;
			ev.b[1]=0;
			ev.b[2]=score[x+1]>>1;				
			AddEvent(ev.dw,3);
			x+=2;
			break;
		case 0x30:	//system event
			if (score[x+1]>=10 && score[x+1]<=14)
			{
				ev.b[0]=0xB0|ch;
				ev.b[1]=controllers[score[x+1]];
				ev.b[2]=1;
				AddEvent(ev.dw,3);
				x+=2;
				break;
			}
			else return 0;
		case 0x40:	//change controller
			if (score[x+1])
			{
				if (score[x+1]<10)
				{
					ev.b[0]=0xB0|ch;
					ev.b[1]=controllers[score[x+1]];
					ev.b[2]=score[x+2];
					AddEvent(ev.dw,3);
					x+=3;
				}
				else return 0;
			}
			else
			{
				ev.b[0]=0xC0|ch;
				ev.b[1]=score[x+2];
				AddEvent(ev.dw,2);
				x+=3;
			};				
			break;
		case 0x50:
		case 0x70:
		case 0x60:
			return 0;
		}
		if (abort) return 0;
		if (t)
		{
			DWORD dt=0;
			do
			{
				dt = (dt<<7) + (score[x]&0x7F);
			} while(score[x++]&0x80);
			ct+=dt;
		}
	}

	out.write_dword(0x002FFF00);
	out.write_dword_ptr(rev32(out.get_size()-(ts_ofs+4)),ts_ofs);

	mf->size = out.get_size();
	mf->data = (BYTE*)out.finish();
	return !!mf->data;
}

bool load_mus(MIDI_file* mf,const BYTE* ptr,size_t sz)
{
	MUS_cvt c;
	return c.run(mf,ptr,sz);
}