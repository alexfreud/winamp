#include "main.h"

#pragma warning(disable:4200)

extern BYTE ff7loopstart[12];
extern BYTE ff7loopend[10];

extern cfg_int cfg_hack_xg_drums, cfg_hack_dls_instruments, cfg_hack_dls_drums, cfg_ff7loopz;

typedef union
{
	BYTE b[4];
	DWORD dw;
} b_dw;

typedef struct
{
	DWORD pos, tm, sz;
	BYTE le;
	BYTE data[];
}
TRACK;

DWORD _fastcall rev32(DWORD);
//WORD _fastcall rev16(WORD);



int test_drum_kit(DWORD no, IDirectMusicCollection* dls);
void do_dls_check(DWORD * i, IDirectMusicCollection * dls);


class CCleaner
{
public:
	INSTRUMENT_DESC* instr, **instr_ptr;
	BYTE ctab[16][128];
	//	UINT dm_vol;
	grow_buf outbuf;
	UINT ntrax, ntrax1;
	UINT maxvol;
	TRACK** in_trax;
	TRACK* out_trax[16];
	DWORD ct;
	UINT tf;
	MIDI_file* mf;
	DWORD vol_set;

	bool drumfix, insfix;
	b_dw ins[16], ins_set[16];

	bool f2, tr1, dm, only_ins, ins_no_lsb;
	bool hasnotes[16];
	void check_ins(UINT msb, UINT lsb, UINT patch, UINT note, BOOL drum, UINT ch) //called on note
	{
		if (ins_no_lsb) lsb = 0;
		INSTRUMENT_DESC * d = instr;
		while (d)
		{
			if (d->bank_hi == msb && d->bank_lo == lsb && d->patch == patch && d->drum == drum) break;
			d = d->next;
		}
		if (d)
		{
			d->count++;
			if (d->note_max < note) d->note_max = note;
			if (d->note_min > note) d->note_min = note;
			d->channels |= 1 << ch;
		}
		else
		{
			d = new INSTRUMENT_DESC;
			*instr_ptr = d;
			instr_ptr = &d->next;
			d->next = 0;
			d->note_min = d->note_max = note;
			d->bank_hi = msb;
			d->bank_lo = lsb;
			d->patch = patch;
			d->count = 1;
			d->drum = drum;
			d->user = 0;
			d->channels = 1 << ch;
		}
	}
	void AdvanceTime(TRACK* t);
	void AddEvent(BYTE ev, BYTE* data);
	void WriteTrack(TRACK* t);
	int Run(MIDI_file* mf, DWORD, void ** out_data, int * out_size);

	void do_shit(UINT n);

	UINT get_next_time()
	{
		UINT t = -1;
		UINT n;
		for (n = 0;n < ntrax;n++)
		{
			UINT t1 = in_trax[n]->tm;
			if (t1 < t) t = t1;
		}
		return t;
	}

	BOOL event_ok(BYTE e, BYTE* p)
	{
		BYTE _c = e & 0xF0;
		BYTE ch = e & 0xF;
		if (_c == 0xB0)
		{
			if (cfg_hack_xg_drums && ch == 9 && p[1] == 0 && (p[0] == 0 || p[0] == 0x20)) 
				return 0;

			if (p[0] > 127)
				return 0;

			ctab[ch][p[0]] = p[1];


			if (p[0] == 0)
			{
				ins[ch].b[2] = p[1];
				if (insfix) return 0;
			}
			if (p[0] == 0x20)
			{
				ins[ch].b[1] = p[1];
				if (insfix) return 0;
			}

			if (dm)	//keep dm drivers happy...
			{
				if (p[0] >= 0x20 && p[0] < 0x40) //lsb values
				{
					return 0;
				}
				else if (p[0] < 0x20)
				{
					BYTE data[2] = {(BYTE)(p[0] + 0x20),ctab[ch][p[0] + 0x20]};
					AddEvent(e, data);
				}
			}

			return 1;

		}

		else if (_c == 0xC0)
		{
			if (ch == 9)
			{
				if (drumfix && !test_drum_kit(p[0], mf->pDLS)) return 0;
				ins[ch].b[0] = p[0];
			}
			else
			{
				ins[ch].b[0] = p[0];
				if (insfix) return 0;
			}
		}
		else if (_c == 0x90 && p[1])
		{
			if (only_ins)
				check_ins(ins[ch].b[2], ins[ch].b[1], ins[ch].b[0], p[0], ch == 9, ch);
			if (ch != 9 && insfix)
			{
				if (ins_set[ch].dw != ins[ch].dw)
				{
					do_dls_check(&ins[ch].dw, mf->pDLS);


					if (ins_set[ch].b[1] != ins[ch].b[1])
					{
						BYTE t[2] = {0x20, ins[ch].b[1]};
						AddEvent(0xB0 | ch, t);
					}

					if (ins_set[ch].b[2] != ins[ch].b[2])
					{
						BYTE t[2] = {0, ins[ch].b[2]};
						AddEvent(0xB0 | ch, t);
					}
					AddEvent(0xC0 | ch, ins[ch].b);

					ins_set[ch].dw = ins[ch].dw;
				}
			}
		}

		return 1;
	}

	CCleaner()
	{
		memset(ins, 0, sizeof(ins));
		memset(ins_set, -1, sizeof(ins_set));
		memset(hasnotes, 0, sizeof(hasnotes));
		memset(out_trax, 0, sizeof(out_trax));
		in_trax = 0;
	}
	~CCleaner()
	{
		UINT n;
		if (in_trax)
		{
			for (n = 0;n < ntrax;n++)
			if (in_trax[n]) {free(in_trax[n]);in_trax[n] = 0;}
			free(in_trax);
		}
		for (n = 0;n < 16;n++)
		{
			if (out_trax[n])
			{
				free(out_trax[n]);
				out_trax[n] = 0;
			}
		}
	}
};

void CCleaner::do_shit(UINT n)
{
	BYTE ce = 0;
	TRACK* t = in_trax[n];
	if (!t) return ;
	while (t->tm == ct)
	{

		if (t->pos >= t->sz)
		{
			t->pos = -1;
			t->tm = -1;
			tf++;
			break;
		}
		BYTE c0 = t->data[t->pos];
		if (c0 == 0xFF) //Meta-events
		{

			if (cfg_ff7loopz
			        && (t->sz - t->pos) >= sizeof(ff7loopend) // bounds check
			        && !memcmp(t->data + t->pos, ff7loopend, sizeof(ff7loopend)))
			{
				//				MessageBox(GetActiveWindow(),"blah",0,0);
				//				AdvanceTime(t);
				tf = ntrax;
				//				return;
			}
			BYTE c1 = t->data[t->pos + 1];
			if (c1 == 0x2F)
			{
				t->pos += 3;
				t->tm = -1;
				tf++;
			}
			{
				t->pos += 2;
				if (t->pos < t->sz)
				{

					unsigned int _d;
					t->pos += DecodeDelta(t->data + t->pos, &_d, t->sz - t->pos);
					t->pos += _d;
				}
			}
		} else if (c0 == 0xF0)
		{
			t->pos += ReadSysex(&t->data[t->pos], t->sz - t->pos);
		}
		else if (c0 == 0xF7) t->pos++;
		else if ((c0&0xF0) == 0xF0) //WTF?
		{
			t->pos = -1;
			t->tm = -1;
			tf++;
			break;
		}
		else
		{
			if (c0&0x80)
			{
				ce = c0;
				t->pos++;
			}
			else ce = t->le;

			if (event_ok(ce, &t->data[t->pos])) AddEvent(ce, &t->data[t->pos]);

			if ((ce&0xF0) == 0xC0 || (ce&0xF0) == 0xD0) t->pos++;
			else t->pos += 2;
			t->le = ce;
		}

		if (t->tm != -1 && t->pos >= t->sz)
		{
			t->pos = -1;
			t->tm = -1;
			tf++;
			break;
		}
		AdvanceTime(t);
	}
}

#define WriteBuf(A,B) outbuf.write(A,B)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	WORD t, n, d;
}
MHD;
typedef struct
{
	DWORD c, s;
}
CHD;
#pragma pack(pop)


void CCleaner::AdvanceTime(TRACK* t)
{
	if (t->tm != -1)
	{
		unsigned int d;
		UINT _n = DecodeDelta(t->data + t->pos, &d, t->sz - t->pos);
		if (_n < 4) t->tm += d;
		t->pos += _n;
	}
}

void CCleaner::AddEvent(BYTE ev, BYTE* data)
{
	if (only_ins) return ;
	BYTE nt = ev & 0xF;
	BYTE ec = ev & 0xF0;
	if (tr1) nt = 0;
	TRACK *t = out_trax[nt];

	ZeroMemory(ctab, sizeof(ctab));


	if (!t)
	{
		t = out_trax[nt] = (TRACK*)malloc(sizeof(TRACK) + 0x1000);
		if (!t) return ;
		ZeroMemory(t, 16);
		t->sz = 0x1000;
		t->tm = 0;

	}
	else if (t->pos > t->sz - 0x10)
	{
		t->sz *= 2;
		out_trax[nt] = (TRACK*)realloc(t, sizeof(TRACK) + t->sz);
		if (!out_trax[nt])
		{
			free(t);
			return ;
		}
		t = out_trax[nt];
	}

	if (t->tm < ct)
	{
		t->pos += EncodeDelta(&t->data[t->pos], ct - t->tm);
		t->tm = ct;
	}
	else
	{
		t->data[t->pos++] = 0;
	}
	if (ec == 0x90)
	{
		hasnotes[nt] = 1;
		data[0] &= 0x7F; /* don't allow 8bit note numbers */
	}
	else if (ec == 0x80)
	{
		data[0] &= 0x7F; /* don't allow 8bit note numbers */
	}
	/*if (ev!=t->le) */{t->data[t->pos++] = ev;t->le = ev;}
	t->data[t->pos++] = data[0];
	if (ec != 0xC0 && ec != 0xD0) t->data[t->pos++] = data[1];
}

void CCleaner::WriteTrack(TRACK* t)
{
	CHD chd;
	chd.c = 'krTM';
	chd.s = rev32(t->pos);
	WriteBuf(&chd, 8);
	WriteBuf(&t->data, t->pos);
	ntrax1++;
}

int DoCleanUp(MIDI_file* mf, DWORD mode, void** out_data, int * out_size)
{
	CCleaner c;
	c.only_ins = 0;
	return c.Run(mf, mode, out_data, out_size);
}

int CCleaner::Run(MIDI_file* _mf, DWORD _md, void ** out_data, int * out_size)
{
	f2 = *(WORD*)(_mf->data + 8) == 0x0200;
	maxvol = 90;
	vol_set = 0;
	dm = (_md & CLEAN_DM) ? 1 : 0;
	tr1 = (_md & CLEAN_1TRACK) ? 1 : 0;

	if (_md&CLEAN_DLS)
	{
		drumfix = dm && cfg_hack_dls_drums;
		insfix = dm && cfg_hack_dls_instruments;
	}
	else
	{
		drumfix = insfix = 0;
	}



	mf = _mf;

	instr_ptr = &instr;
	instr = 0;

	UINT n;

	ct = 0;
	tf = 0;
	ntrax = ntrax1 = 0;
	CHD chd;
	MHD mhd;
	DWORD ptr = 8;



	mhd = *(MHD*)(mf->data + 8);

	ptr += 6;

	mhd.t = rev16(mhd.t);
	mhd.n = rev16(mhd.n);

	if (mhd.t > 2)
		goto fail;
	ntrax = mhd.n;
	n = 0;
	in_trax = (TRACK**)malloc(sizeof(void*) * ntrax);
	for (;n < ntrax && ptr < (UINT)mf->size;n++)
	{
		chd = *(CHD*)(mf->data + ptr);
		ptr += 8;
		if (chd.c != 'krTM' || ptr > (UINT)mf->size)
		{
			ntrax = n;
			break;
		}
		chd.s = rev32(chd.s);
		//if (ptr+chd.s>(UINT)mf->size)
		if (chd.s > ((UINT)mf->size - ptr))
		{
			chd.s = mf->size - ptr;
		}
		//goto fail;
		in_trax[n] = (TRACK*)malloc(16 + chd.s);
		in_trax[n]->sz = chd.s;
		in_trax[n]->tm = 0;
		in_trax[n]->le = 0;
		in_trax[n]->pos = 0;
		memcpy(in_trax[n]->data, mf->data + ptr, chd.s);
		ptr += chd.s;
		AdvanceTime(in_trax[n]);
	}
	if (f2)
	{
		for (n = 0;n < ntrax;n++)
		{
			in_trax[n]->tm = ct;
			while (tf <= n)
			{
				do_shit(n);
				if (in_trax[n]->tm != -1) ct = in_trax[n]->tm;
			}
		}
	}
	else
	{
		while (tf < ntrax)
		{
			UINT nt = get_next_time(); //ct++;
			if (nt == -1) break;
			ct = nt;
			for (n = 0;n < ntrax && tf < ntrax;n++)
			{
				do_shit(n);
			}
		}
	}

	if (!only_ins)
	{


		mhd.t = 0x0100;
		mhd.n = 0; //rev16(ntrax1);
		chd.c = 'dhTM';
		chd.s = 0x06000000;
		WriteBuf(&chd, 8);
		WriteBuf(&mhd, 6);
		if (!(_md&CLEAN_NOTEMPO) && mf->tmap)
		{
			/*			BYTE *tt=mf->tmap->BuildTrack();
						if (tt)
						{
							WriteBuf(tt,rev32(*(DWORD*)(tt+4))+8);
							ntrax1++;
							free(tt);
						}*/
			if (mf->tmap->BuildTrack(outbuf))
			{
				ntrax1++;
			}
		}
		if (!(_md&CLEAN_NOSYSEX) && mf->smap)
		{
			/*			BYTE *st=mf->smap->BuildTrack();
						if (st)
						{
							WriteBuf(st,rev32(*(DWORD*)(st+4))+8);
							ntrax1++;
							free(st);
						}*/
			if (mf->smap->BuildTrack(outbuf))
			{
				ntrax1++;
			}
		}



		for (n = 0;n < 16;n++) if (out_trax[n] && hasnotes[n] && out_trax[n]->pos)
			{
				TRACK *t = out_trax[n];
				t->pos += EncodeDelta(t->data + t->pos, ct - t->tm);
				t->data[t->pos++] = 0xFF;
				t->data[t->pos++] = 0x2F;
				t->data[t->pos++] = 0;
				WriteTrack(t);
			}
		{
			WORD t = rev16(ntrax1);
			outbuf.write_ptr(&t, 2, 10);
		}
		if (out_size) *out_size = outbuf.get_size();
		if (out_data) *out_data = outbuf.finish();
#if 0
		{
			HANDLE f = CreateFile("c:\\dump.mid", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			DWORD bw = 0;
			WriteFile(f, rv, bs, &bw, 0);
			CloseHandle(f);
		}
#endif

	}
	return 1;
fail:
	//	ErrorBox("WARNING: cleaner messed up");

	return 0;

	//TO DESTRUCTOR

}

INSTRUMENT_DESC* GetInstruments(MIDI_file* mf, BOOL do_lsb)
{
	CCleaner c;
	c.only_ins = 1;
	c.ins_no_lsb = !do_lsb;
	c.Run(mf, 0, 0, 0);
	return c.instr;
}
