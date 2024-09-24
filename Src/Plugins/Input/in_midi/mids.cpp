#include "main.h"
#include "cvt.h"

bool is_mids(const BYTE* buf,size_t s)
{
	return s>0x20 && *(DWORD*)buf==_rv('RIFF') && *(DWORD*)(buf+8)==_rv('MIDS') && *(DWORD*)(buf+12)==_rv('fmt ');
}

typedef struct
{
    DWORD           dwTimeFormat;
    DWORD           cbMaxBuffer;
    DWORD           dwFlags;
} MIDSFMT;


#define WRITE(X,Y) out.write(X,Y)

#define WRITE_DELTA(X) gb_write_delta(out,X)

#define D_WRITE {WRITE_DELTA(ct-tw);tw=ct;}

bool load_mids(MIDI_file* mf,const BYTE* buf,size_t sz)
{
	if (sz<*(long*)(buf+4)+8) return 0;
	MIDSFMT* fmt=(MIDSFMT*)(buf+0x14);
	DWORD ofs;
	ofs=*(DWORD*)(buf+0x10)+0x14;
	if (*(DWORD*)(buf+ofs)!=_rv('data')) return 0;
	//ofs+=8+*(DWORD*)(buf+ofs+4);
	ofs+=8;
	DWORD ss=*(DWORD*)(buf+ofs-4);
	DWORD nc=*(DWORD*)(buf+ofs);
	DWORD* ptr=(DWORD*)(buf+ofs);
	grow_buf out;
	ss>>=2;
	DWORD mhdr[2];
	mhdr[0]=_rv('MThd');
	mhdr[1]=_rv(6);
	WRITE(mhdr,8);
	WORD w=0;
	WRITE(&w,2);
	w=0x100;
	WRITE(&w,2);
	w=rev16((WORD)fmt->dwTimeFormat);
	WRITE(&w,2);
	mhdr[0]=_rv('MTrk');
	WRITE(mhdr,8);
	DWORD tw=0,ct=0;
	DWORD cc=0;
	DWORD cs;
	DWORD pos=1;
	while(cc<nc)
	{
		cs = (ptr[pos+1]>>2)+pos;
		if (cs>ss) break;
		pos+=2;
		while(pos<cs)
		{
			ct+=ptr[pos];
			pos+=2;
			DWORD e=ptr[pos];
			if (e&MEVT_F_LONG)
			{
				pos+=e&0xFFFFFF;
			}
			else
			{
				if (e>>24==MEVT_TEMPO)
				{
					D_WRITE;
					BYTE tmp[6]={0xFF,0x51,0x03,(BYTE)((e>>16)&0xFF),(BYTE)((e>>8)&0xFF),(BYTE)(e&0xFF)};
					WRITE(tmp,6);
				}
				else if (!(e>>24))
				{
					BYTE c=(BYTE)(e&0xF0);
					if (c!=0xF0)
					{
						D_WRITE;
						DWORD l=(c==0xC0 || c==0xD0) ? 2 : 3;
						WRITE(&e,l);
					}
				}
				pos++;
			}
		}
	}
	WRITE("\x00\xFF\x2F",4);

	out.write_dword_ptr(rev32(out.get_size()-(8+6+8)),8+6+4);

	mf->size = out.get_size();
	mf->data = (BYTE*)out.finish();
	return !!mf->data;
}