#include "main.h"
#include "cvt.h"

#define _MThd 'dhTM'
#define _MTrk 'krTM'

#define tempo 0x188000

#define Q_MAX 128

struct HMI_cvt
{
public:
	struct
	{
		DWORD tm;
		BYTE ch,n;
	} q_rel[Q_MAX];

	void inline q_add(BYTE ch,BYTE nt,DWORD t)
	{
		UINT n=0;
		while(q_rel[n].tm!=-1) n++;
		q_rel[n].tm=t;
		q_rel[n].ch=ch;
		q_rel[n].n=nt;
	}
	grow_buf buf;

	UINT DoTrack(const BYTE* t,UINT *_bw);
	void DoQueue(DWORD ct,DWORD& tw,BYTE& _run);
	bool run(MIDI_file * mf,const BYTE* _buf,DWORD sz);
};


#define FixHeader(H) {(H).fmt=rev16((H).fmt);(H).trax=rev16((H).trax);(H).dtx=rev16((H).dtx);}

void HMI_cvt::DoQueue(DWORD ct,DWORD& tw,BYTE& _run)
{
	UINT n,mt,_n;
_t:	
	mt=-1;
	for(n=0;n<Q_MAX;n++)
	{
		if (q_rel[n].tm<mt) {_n=n;mt=q_rel[n].tm;}
	}
	if (mt>ct) return;
	gb_write_delta(buf,mt-tw);
	tw=mt;
	BYTE _e=q_rel[_n].ch|0x90;
	if (_e!=_run) buf.write_byte(_run=_e);
	buf.write_byte(q_rel[_n].n);
	buf.write_byte(0);
	q_rel[_n].tm=-1;
	goto _t;
}

extern BYTE ff7loopstart[12];

UINT HMI_cvt::DoTrack(const BYTE* t,UINT *_bw)
{
	{
		UINT n;
		for(n=0;n<Q_MAX;n++) q_rel[n].tm=-1;
	}
	DWORD pt=0;
	DWORD ct=0,tw=0;
	BYTE run=0;
	BYTE _run=0;
	DWORD bw_s=buf.get_size();
	while(1)
	{
		{
			unsigned int _d;
			pt+=DecodeDelta(t+pt,&_d);
			ct+=_d;
		}
		DoQueue(ct,tw,_run);
		BYTE c=t[pt];
		if (c==0xFF)
		{
			DoQueue(-2,tw,_run);
			if (t[pt+1]==0x2f)
			{
				pt+=3;
				buf.write_dword(0x002FFF00);
				break;
			}
			return -1;
			
		}
		else if (c==0xF0)
		{
			gb_write_delta(buf,ct-tw);
			tw=ct;
			UINT _p=pt;
			while(t[pt]!=0xF7) pt++;
			pt++;
			buf.write(t+_p,pt-_p);
		}
		else if (c==0xFE)
		{
			c=t[pt+1];
			if (c==0x10)
			{
				pt+=t[pt+4]+9;
			}
			else if (c==0x14)
			{
				pt+=4;
				gb_write_delta(buf,ct-tw);
				tw=ct;
				buf.write(ff7loopstart,12);
			}
			else if (c==0x15) pt+=8;
			else return -1;
		}
		else
		{
			gb_write_delta(buf,ct-tw);
			tw=ct;
			if (c&0x80) {pt++;run=c;}
			else c=run;
			if (c!=_run) buf.write_byte(_run=c);
			buf.write_byte(t[pt++]);
			BYTE c1=c&0xF0;
			if (c1!=0xC0 && c1!=0xD0) buf.write_byte(t[pt++]);
			if (c1==0x90)
			{
				BYTE b=t[pt-2];
				unsigned int _t;
				pt+=DecodeDelta(t+pt,&_t);
				q_add(c&0xF,b,_t+ct);
			}
		}
	}
	(*_bw)+=buf.get_size()-bw_s;
	return pt;
}

extern BYTE hmp_track0[19];	//hmp.cpp

bool HMI_cvt::run(MIDI_file* mf,const BYTE* _buf,DWORD sz)
{
	const BYTE *ptr=_buf;

	while(*(DWORD*)ptr!='CART')
	{
		ptr++;
		if (ptr==_buf+sz) {return 0;}
	}
	
	buf.write(0,14);

	ptr-=8;
	UINT ntrax=1;
	UINT nft=*(DWORD*)(_buf+0xE4);

	buf.write(hmp_track0,sizeof(hmp_track0));

	UINT n;
	for(n=0;n<nft;n++)
	{
		if (ptr>_buf+sz) return 0;
		UINT _b=0;
		ntrax++;
		buf.write_dword(_rv('MTrk'));
		DWORD _s=buf.get_size();
		buf.write(0,4);
		{
			const BYTE* p1=ptr+ptr[0x4B];
			const BYTE* _p=p1+p1[1];
			p1+=2;
			while(_p[-1]==' ') _p--;
			_b=(_p-p1)+4;

			BYTE tmp[3]={0,0xFF,1};
			buf.write(tmp,3);
			gb_write_delta(buf,_p-p1);
			buf.write(p1,_p-p1);
			p1=_p;
		}
		ptr+=ptr[0x57];
		{
			DWORD d=DoTrack(ptr,&_b);
			if (d==-1) return 0;
			ptr+=d;
		}
		buf.write_dword_ptr(rev32(_b),_s);
	}
	buf.write_dword_ptr(_rv('MThd'),0);
	buf.write_dword_ptr(_rv(6),4);

	MIDIHEADER mhd={0x0100,rev16(ntrax),0xC000};
	buf.write_ptr(&mhd,sizeof(mhd),8);
	
	mf->size = buf.get_size();
	mf->data = (BYTE*)buf.finish();
	return !!mf->data;
}

bool load_hmi(MIDI_file* mf,const BYTE* _buf,size_t sz)
{
	HMI_cvt c;
	return c.run(mf,_buf,sz);
}