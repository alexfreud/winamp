#include "main.h"
#include "cvt.h"
#include <intsafe.h>

#define _MThd 'dhTM'
#define _MTrk 'krTM'

static DWORD ProcessTrack(const BYTE* track,grow_buf & out,int size)
{
	UINT s_sz=out.get_size();
	const BYTE *pt = track;
	BYTE lc1 = 0,lastcom = 0;
	DWORD t=0,d;
	bool run = 0;
	int n1,n2;
	while(track < pt + size)
	{		
		if (track[0]&0x80)
		{
			BYTE b=track[0]&0x7F;
			out.write_byte(b);			
			t+=b;
		}
		else
		{
			d = (track[0])&0x7F;
			n1 = 0;
			while((track[n1]&0x80)==0)
			{
				n1++;
				d+=(track[n1]&0x7F)<<(n1*7);
			}
			t+=d;
			
			n1 = 1;
			while((track[n1]&0x80)==0)
			{
				n1++;
				if (n1==4) return 0;
			}
			for(n2=0;n2<=n1;n2++)
			{
				BYTE b=track[n1-n2]&0x7F;
				
				if (n2!=n1) b|=0x80;
				out.write_byte(b);
			}
			track+=n1;
		}
		track++;
		if (*track == 0xFF)//meta
		{
			unsigned int _d;
			UINT s=DecodeDelta(track+2,&_d);
			UINT result;
			if (UIntAdd(2, s, &result) || UIntAdd(result, _d, &result) == S_OK || !out.write(track,result))
				return 0;
			if (track[1]==0x2F) break;			
		}
		else 
		{
			lc1=track[0];
			if ((lc1&0x80) == 0) return 0;
			switch(lc1&0xF0)
			{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
				if (lc1!=lastcom)
				{
					out.write_byte(lc1);
				}
				out.write(track+1,2);
				track+=3;
				break;
			case 0xC0:
			case 0xD0:
				if (lc1!=lastcom)
				{
					out.write_byte(lc1);
				}
				out.write_byte(track[1]);
				track+=2;
				break;
			default:
				return 0;
			}
			lastcom=lc1;
		}
	}
	return out.get_size()-s_sz;
}

#define FixHeader(H) {(H).fmt=rev16((H).fmt);(H).trax=rev16((H).trax);(H).dtx=rev16((H).dtx);}

BYTE hmp_track0[19]={'M','T','r','k',0,0,0,11,0,0xFF,0x51,0x03,0x18,0x80,0x00,0,0xFF,0x2F,0};

bool load_hmp(MIDI_file* mf,const BYTE* buf, size_t br)
{
	MIDIHEADER mhd = {1,0,0xC0};
	const BYTE * max = buf+br;
	const BYTE* ptr = buf;
	BOOL funky=0;
	if (!memcmp(buf,"HMIMIDIR",8)) funky=1;
	grow_buf dst;
	DWORD n1,n2;
	dst.write_dword(_rv('MThd'));
	dst.write_dword(_rv(6));
	dst.write(0,sizeof(mhd));
	ptr = buf+(funky ? 0x1a: 0x30);
	mhd.trax = *ptr;
	if (funky) mhd.dtx=rev16(*(WORD*)(buf+0x4c))/6;
	dst.write(hmp_track0,sizeof(hmp_track0));

	while(*(WORD*)ptr != 0x2FFF && ptr < max - 4-7) ptr++;
	ptr+=funky ? 5 : 7;
	if (ptr == max-4) return 0;
	UINT n;


	for(n=1;n<mhd.trax;n++)
	{
		n1 = funky ? *(WORD*)ptr : *(DWORD*)ptr - 12;
		if (ptr + n1 > max)
		{
			mhd.trax=n;
			break;
		}
		dst.write_dword(_rv('MTrk'));
		if (!funky) ptr += 8;
		
		UINT ts_ofs=dst.get_size();
		dst.write_dword(0);
		if (!(n2=ProcessTrack(funky ? ptr+4 : ptr,dst,n1))) return 0;
		
		dst.write_dword_ptr(rev32(n2),ts_ofs);
		if (funky) ptr+=n1;
		else ptr += n1 + 4;
	}
	FixHeader(mhd);
	dst.write_ptr(&mhd,sizeof(mhd),8);


	mf->size = dst.get_size();
	mf->data = (BYTE*)dst.finish();
	return !!mf->data;
}
