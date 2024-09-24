#include "main.h"
#include "cvt.h"

bool is_gmf(const BYTE* p,size_t s)
{
	return s>0x20 && *(DWORD*)p==_rv('GMF\x01');
}

bool load_gmf(MIDI_file * mf,const BYTE* buf,size_t siz)
{
	grow_buf wb;
	wb.write_dword(_rv('MThd'));
	wb.write_dword(_rv(6));
	MIDIHEADER h={0x000,0x100,0xC000};
	wb.write(&h,6);
	wb.write_dword(_rv('MTrk'));
	int tempo=100000*rev16(*(WORD*)(buf+4));
	wb.write_dword(rev32(siz-9+8+3));//MTrk size
	BYTE tempo_event[8]={0,0xFF,0x51,0x03,(BYTE)((tempo>>16)&0xFF),(BYTE)((tempo>>8)&0xFF),(BYTE)(tempo&0xFF),0};
	wb.write(tempo_event,8);
	wb.write(buf+8,siz-9);
	wb.write("\xFF\x2F\x00",3);
	mf->size = wb.get_size();
	mf->data = (BYTE*)wb.finish();

	return !!mf->data;
}
