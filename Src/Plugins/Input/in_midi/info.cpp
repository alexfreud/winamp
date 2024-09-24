#include "main.h"
#include "resource.h"
#include <shlwapi.h>
#include <commdlg.h>
#include <strsafe.h>
#include "../nu/AutoWide.h"
#include "../nu/AutoCharFn.h"
#include "CompressionUtility.h"

extern In_Module mod;

extern "C"
{
#include "genres.h"
}
#define HAVE_BMPVIEW

#ifdef HAVE_BMPVIEW
static LRESULT CALLBACK BmpViewProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		RECT r;
		GetClientRect(wnd, &r);
		HDC wdc = GetDC(wnd);
		HDC dc = (HDC)GetWindowLongPtr(wnd, 4);
		DrawEdge(wdc, &r, EDGE_SUNKEN, BF_RECT);
		if (dc) BitBlt(wdc, 2, 2, r.right - 4, r.bottom - 4, dc, 0, 0, SRCCOPY);
		ReleaseDC(wnd, wdc);
		ValidateRect(wnd, 0);
		return 0;
	}
	break;
	case WM_USER:
	{
		HDC dc = CreateCompatibleDC(0);
		SelectObject(dc, (HBITMAP)lp);
		SetWindowLongPtr(wnd, 0, lp);
		SetWindowLongPtr(wnd, 4, (LONG_PTR)dc);
		//RedrawWindow(wnd,0,0,RDW_INVALIDATE);
	}
	return 0;
	case WM_DESTROY:
	{
		HBITMAP bmp = (HBITMAP)GetWindowLongPtr(wnd, 0);
		HDC dc = (HDC)GetWindowLongPtr(wnd, 4);
		if (dc) DeleteDC(dc);
		if (bmp) DeleteObject(bmp);
	}
	break;
	}
	return DefWindowProc(wnd, msg, wp, lp);
}

void bmpview_init()
{
	static bool got_class;
	if (!got_class)
	{
		got_class = 1;
		WNDCLASS wc =
		{
			0,
			BmpViewProc,
			0, 8,
			MIDI_callback::GetInstance(), 0, LoadCursor(0, IDC_ARROW), 0,
			0,
			L"BitmapView"
		};
		RegisterClassW(&wc);
	}
}
#endif

BOOL SaveFile(HWND w, MIDI_file* mf, BOOL info);
int SaveAsGZip(string filename, const void* buffer, size_t size);

UINT align(UINT x, UINT a)
{
	a--;
	return (x + a) & ~a;
}

typedef struct
{
	UINT ctrl_id;
	DWORD riff_id;
	//	char * name;
}
RMI_TAG;

static void _swap_ptrs(void** a, void* b)
{
	void* _a = *a;
	*a = b;
	if (_a) free(_a);
}

#define swap_ptrs(x,y) _swap_ptrs((void**)&x,(void*)(y))

static RMI_TAG rmi_tagz[] =
{
	{IDC_DISP, _rv('DISP')},
	{IDC_NAME, _rv('INAM')},
	{IDC_ARTIST, _rv('IART')},
	{IDC_ALBUM, _rv('IALB')},
	{IDC_TRACK, _rv('ITRK')},
	{IDC_GENRE, _rv('IGNR')},
	{IDC_COMPOSER, _rv('ICMP')},
	{IDC_COPYRIGHT, _rv('ICOP')},
	{IDC_COMMENT, _rv('ICMT')},
	{IDC_DATE, _rv('ICRD')},
	{IDC_SOFTWARE, _rv('ISFT')},
	{IDC_ENGINEER, _rv('IENG')},
	{IDC_SUBJECT, _rv('ISBJ')},
};

void SetDlgItemTextSiz(HWND wnd, UINT id, char* text, UINT siz)
{
	if (!text[siz - 1]) SetDlgItemTextA(wnd, id, text);
	else
	{
		char* foo = (char*)alloca(siz + 1);
		memcpy(foo, text, siz);
		foo[siz] = 0;
		SetDlgItemTextA(wnd, id, foo);
	}
}

#define N_RMI_TAGZ (sizeof(rmi_tagz)/sizeof(rmi_tagz[0]))


static void set_rmi_dlg_title(HWND wnd, MIDI_file* mf)
{
	char t[MAX_PATH + 100] = { 0 };
	StringCbPrintfA(t, sizeof(t), WASABI_API_LNGSTRING(STRING_RMI_INFO_FMT), (const char*)mf->path);
	SetWindowTextA(wnd, t);
}

static BOOL CALLBACK rmiproc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
#if defined(_WIN64)
		SetWindowLong(wnd, DWLP_USER, lp);
#else
		SetWindowLong(wnd, DWL_USER, lp);
#endif
		{
			MIDI_file* mf = (MIDI_file*)lp;
			set_rmi_dlg_title(wnd, mf);
			if (mf->title && *mf->title)
			{
				SetDlgItemTextA(wnd, IDC_DISP, mf->title);
			}
			if (mf->rmi_data)
			{
				BYTE* rmi = (BYTE*)mf->rmi_data;
				int p = 4;
				while (p < mf->rmi_size)
				{
					DWORD id = *(DWORD*)(rmi + p);
					UINT n;
					for (n = 0; n < N_RMI_TAGZ; n++)
					{
						if (id == rmi_tagz[n].riff_id)
						{
							SetDlgItemTextSiz(wnd, rmi_tagz[n].ctrl_id, (char*)(rmi + p + 8), *(DWORD*)(rmi + p + 4));
							break;
						}
					}
					p += 8 + align(*(DWORD*)(rmi + p + 4), 2);
				}
			}
#ifdef HAVE_BMPVIEW
			if (mf->bmp_data)
			{
				void* pixels;
				BITMAPINFOHEADER* foo = (BITMAPINFOHEADER*)mf->bmp_data;
				HBITMAP bmp = CreateDIBSection(0, (BITMAPINFO*)foo, DIB_RGB_COLORS, &pixels, 0, 0);
				if (bmp)
				{
					UINT clr_used = foo->biClrUsed;
					if (!clr_used) clr_used = 1 << foo->biBitCount;
					BYTE* ptr = (BYTE*)foo + foo->biSize + (clr_used << 2);
					int max = (BYTE*)mf->bmp_data + mf->bmp_size - ptr;
					BITMAP b;
					GetObject(bmp, sizeof(b), &b);
					int siz = b.bmWidthBytes * b.bmHeight;
					if (siz < 0) siz = -siz;
					if (siz > max) siz = max;
					memcpy(pixels, ptr, siz);
					SendDlgItemMessage(wnd, IDC_BMPVIEW, WM_USER, 0, (long)bmp);
				}
			}
#endif

		}
		genres_read(GetDlgItem(wnd, IDC_GENRE));
		return 1;
	case WM_COMMAND:
		switch (wp)
		{
		case IDOK:
		{
#if defined(_WIN64)
			MIDI_file* mf = (MIDI_file*)GetWindowLong(wnd, DWLP_USER);
#else
			MIDI_file* mf = (MIDI_file*)GetWindowLong(wnd, DWL_USER);
#endif
			char* title = 0;
			HWND w = GetDlgItem(wnd, IDC_DISP);
			UINT sz = GetWindowTextLength(w);
			if (sz)
			{
				sz++;
				title = (char*)malloc(sz);
				GetWindowTextA(w, title, sz);
			}
			swap_ptrs(mf->title, title);
			BYTE* rmi_info = 0;
			UINT rmi_siz = 0;
			UINT n;
			for (n = 0; n < N_RMI_TAGZ; n++)
			{
				UINT d = GetWindowTextLength(GetDlgItem(wnd, rmi_tagz[n].ctrl_id));
				if (d) rmi_siz += align(d + 1, 2) + 8;
			}
			if (rmi_siz)
			{
				rmi_siz += 4; //'INFO'
				rmi_info = (BYTE*)malloc(rmi_siz);
				UINT ptr = 4;
				*(DWORD*)rmi_info = _rv('INFO');
				for (n = 0; n < N_RMI_TAGZ; n++)
				{
					w = GetDlgItem(wnd, rmi_tagz[n].ctrl_id);
					if (GetWindowTextLength(w))
					{
						*(DWORD*)(rmi_info + ptr) = rmi_tagz[n].riff_id;
						ptr += 4;
						char* foo = (char*)(rmi_info + ptr + 4);
						GetWindowTextA(w, foo, rmi_siz - (ptr + 4));
						UINT s = strlen(foo) + 1;
						*(DWORD*)(rmi_info + ptr) = s;
						ptr += 4 + align(s, 2);
					}
				}
				rmi_siz = ptr;
			}
			mf->rmi_size = rmi_siz;
			swap_ptrs(mf->rmi_data, rmi_info);

			genres_write(GetDlgItem(wnd, IDC_GENRE));

			if (SaveFile(wnd, mf, 1)) EndDialog(wnd, 0);
		}
		break;
		case IDCANCEL:
			genres_write(GetDlgItem(wnd, IDC_GENRE));
			EndDialog(wnd, 1);
			break;
		}
		break;
	}
	return 0;
}

int show_rmi_info(HWND w, MIDI_file* mf)
{
#ifdef HAVE_BMPVIEW
	bmpview_init();
#endif
	return WASABI_API_DIALOGBOXPARAM(IDD_RMI_SHIZ, w, rmiproc, (long)mf);
}

static bool is_local(const char* url)
{
	if (!_strnicmp(url, "file://", 7) || !strnicmp(url, "partial://", 10)) return 1;
	if (url[1] == ':' && url[2] == '\\') return 1;
	return strstr(url, "://") ? 0 : 1;
}

static char* fmt_names[] = { "MIDI", "RIFF MIDI", "HMP", "HMI", "XMIDI", "MUS", "CMF", "GMD", "MIDS", "GMF", "MIDI(?)" };

const char* find_tag(MIDI_file* mf, DWORD id, UINT* siz);

char* getfmtstring(MIDI_file* f, char* s)
{
	const char* z = fmt_names[f->format];
	while (z && *z) *(s++) = *(z++);
	if (f->format <= 1)	//MID/RMI
	{
		char foo[32] = { 0 };
		StringCbPrintfA(foo, sizeof(foo), WASABI_API_LNGSTRING(STRING_INFO_FORMAT_FMT), f->data[4 + 4 + 1]);
		z = foo;
		while (z && *z) *(s++) = *(z++);
	}
	if (f->info.e_type)
	{
		*(s++) = ' ';
		*(s++) = '(';
		z = f->info.e_type;
		while (z && *z) *(s++) = *(z++);
		*(s++) = ')';
	}
	*s = 0;
	return s;
}

void file2title(const char* f, string& t);

static const char* find_tag(MIDI_file* mf, DWORD id, UINT* siz)
{
	if (!mf->rmi_data) return 0;
	char* rmi = (char*)mf->rmi_data;
	int ptr = 4;
	while (ptr < mf->rmi_size)
	{
		if (*(DWORD*)(rmi + ptr) == id)
		{
			UINT s = *(DWORD*)(rmi + ptr + 4);
			UINT s1 = 0;
			ptr += 8;
			while (rmi[ptr + s1] && s1 < s) s1++;
			if (siz) *siz = s1;
			return rmi + ptr;
		}
		ptr += align(*(DWORD*)(rmi + ptr + 4), 2) + 8;
	}
	return 0;
}

bool KeywordMatch(const char* mainString, const char* keyword)
{
	return !_stricmp(mainString, keyword);
}


static const wchar_t* pExtList[] = { L"MID",L"MIDI",L"RMI",L"KAR",L"HMP",L"HMI",L"XMI",L"MSS",L"MUS",L"CMF",L"GMD",L"MIDS",L"MIZ",L"HMZ" };
static const int pExtDescIdList[] = { 0, 0, 0, 1, 2, 2, 3, 4, 5, 6, 7, 0, 8, 9 };
static const int pExtDescList[] =
{
	IDS_FAMILY_STRING_MIDI,
	IDS_FAMILY_STRING_KARAOKE_MIDI,
	IDS_FAMILY_STRING_HMI_MIDI,
	IDS_FAMILY_STRING_EXTENDED_MIDI,
	IDS_FAMILY_STRING_MSS_MIDI,
	IDS_FAMILY_STRING_FINALE_MIDI,
	IDS_FAMILY_STRING_CREATIVE_MIDI,
	IDS_FAMILY_STRING_GENERAL_MIDI_DUMP,
	IDS_FAMILY_STRING_COMPRESSED_MIDI,
	IDS_FAMILY_STRING_COMPRESSED_HMI_MIDI
};

MIDI_file* wa2_open_file(const char* url);
extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t* fn, const char* data, wchar_t* dest, int destlen)
{
	MIDI_file* file = 0;

	if (KeywordMatch(data, "type"))
	{
		dest[0] = L'0';
		dest[1] = 0;
		return 1;
	}
	if (KeywordMatch(data, "BURNABLE"))
	{
		dest[0] = L'0';
		dest[1] = 0;
		return 1;
	}
	if (KeywordMatch(data, "noburnreason")) // note: this isn't supposed to be any kind of protection - just keeps the burner from misbehaving on protected tracks
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_MIDIS_ARE_NOT_BURNABLE, dest, destlen);
		return 1;
	}
	if (KeywordMatch(data, "family"))
	{
		INT index;
		LPCWSTR e;
		DWORD lcid;
		e = PathFindExtensionW(fn);
		if (L'.' != *e || 0x00 == *(++e)) return 0;

		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		for (index = sizeof(pExtList) / sizeof(wchar_t*) - 1; index >= 0 && CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, e, -1, pExtList[index], -1); index--);
		if (index >= 0 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pExtDescList[pExtDescIdList[index]]))) return 1;
		return 0;
	}

	file = wa2_open_file(AutoCharFn(fn));
	if (!file)
	{
		return 0;
	}
	const char* ret = 0;
	//else if (!_stricmp(tag,"FILEPATH") || !_stricmp(tag,"PATH")) ret=file->path;
	//else if (!_stricmp(tag,"DISPLAY")) ret=file->title;
	//else if (!_stricmp(tag,"FIRSTTRACK")) ret=file->info.traxnames[0];

	if (KeywordMatch(data, "LENGTH"))
	{
		_itow(file->GetLength(), dest, 10);
		file->Free();
		return 1;
	}
	else if (file->rmi_data)
	{
		if (KeywordMatch(data, "TITLE")) ret = find_tag(file, _rv('INAM'), 0);
		else if (KeywordMatch(data, "ARTIST")) ret = find_tag(file, _rv('IART'), 0);
		else if (KeywordMatch(data, "COMPOSER")) ret = find_tag(file, _rv('ICMP'), 0);
		else if (KeywordMatch(data, "GENRE")) ret = find_tag(file, _rv('IGNR'), 0);
		else if (KeywordMatch(data, "ALBUM")) ret = find_tag(file, _rv('IALB'), 0);
		else if (KeywordMatch(data, "COPYRIGHT")) ret = find_tag(file, _rv('ICOP'), 0);
		else if (KeywordMatch(data, "COMMENT")) ret = find_tag(file, _rv('ICMT'), 0);
		else if (KeywordMatch(data, "TRACK")) ret = find_tag(file, _rv('ITRK'), 0);
		else if (KeywordMatch(data, "DATE")) ret = find_tag(file, _rv('ICRD'), 0);
		else
		{
			file->Free();
			return 0;
		}
	}
	else
	{
		file->Free();
		return 0;
	}

	if (ret)
	{
		lstrcpynW(dest, AutoWide(ret), destlen);

	}
	file->Free();
	return !!ret;
}


void MIDI_file::GetTitle(char* buf, int maxlen)
{
	string file_title;
	file2title(path, file_title);
	lstrcpynA(buf, file_title, maxlen);
}

//int save_gzip(MIDI_file* mf, char* path);

static void do_ext(string& s, const char* ext)
{
	const char* p = strrchr(s, '.');
	if (p) s.truncate(p - (const char*)s);
	s += ext;
}

void* build_rmi(MIDI_file* mf, UINT* siz)
{
	UINT sz = 0x14 + align(mf->size, 2);
	UINT t_sz = 0;
	if (mf->title)
	{
		t_sz = strlen(mf->title);
		if (t_sz)
		{
			t_sz++; //add null;
			sz += 12 + align(t_sz, 2);
		}
	}
	if (mf->rmi_data)
	{
		sz += align(mf->rmi_size + 8, 2);
	}
	if (mf->bmp_data)
	{
		sz += align(mf->bmp_size + 12, 2);
	}

	if (mf->pDLSdata) sz += align(mf->DLSsize, 2);

	BYTE* block = (BYTE*)malloc(sz);
	BYTE* b_ptr = block;
	*(DWORD*)b_ptr = _rv('RIFF');
	b_ptr += 4;
	*(DWORD*)b_ptr = sz - 8;
	b_ptr += 4;
	*(DWORD*)b_ptr = _rv('RMID');
	b_ptr += 4;
	*(DWORD*)b_ptr = _rv('data');
	b_ptr += 4;
	*(DWORD*)b_ptr = mf->size;
	b_ptr += 4;
	memcpy(b_ptr, mf->data, mf->size);
	b_ptr += align(mf->size, 2);
	if (t_sz)
	{
		*(DWORD*)b_ptr = _rv('DISP');
		b_ptr += 4;
		*(DWORD*)b_ptr = t_sz + 4;
		b_ptr += 4;
		*(DWORD*)b_ptr = 1;
		b_ptr += 4;
		memcpy(b_ptr, mf->title, t_sz);
		b_ptr += align(t_sz, 2);
	}
	if (mf->rmi_data)
	{
		*(DWORD*)b_ptr = _rv('LIST');
		b_ptr += 4;
		*(DWORD*)b_ptr = mf->rmi_size;
		b_ptr += 4;
		memcpy(b_ptr, mf->rmi_data, mf->rmi_size);
		b_ptr += align(mf->rmi_size, 2);
	}
	if (mf->bmp_data)
	{
		*(DWORD*)b_ptr = _rv('DISP');
		b_ptr += 4;
		*(DWORD*)b_ptr = mf->bmp_size + 4;
		b_ptr += 4;
		*(DWORD*)b_ptr = 8;
		b_ptr += 4;
		memcpy(b_ptr, mf->bmp_data, mf->bmp_size);
		b_ptr += align(mf->bmp_size, 2);
	}
	if (mf->pDLSdata)
	{
		memcpy(b_ptr, mf->pDLSdata, mf->DLSsize);
		b_ptr += align(mf->DLSsize, 2);
	}
	*siz = sz;
	return block;
}

BOOL SaveFile(HWND w, MIDI_file* mf, BOOL info)
{
	BOOL rmi_only = info;
	string tmp;
	if (is_local(mf->path) && _strnicmp(mf->path, "partial://", 10))
	{
		tmp = mf->path;
		if (!info) do_ext(tmp, ".mid");
	}
	else
	{
		info = 0;
		file2title(mf->path, tmp);
		tmp += ".mid";
	}
	if (mf->format > 1) info = 0; //not MID/RMI

	UINT fmt = 0;
	BOOL do_gzip = 0;
	if (!info)
	{
		char filter[512] = { 0 };
		OPENFILENAMEA ofn = { sizeof(ofn),0 };
		ofn.hwndOwner = w;
		ofn.lpstrFile = tmp.buffer_get(MAX_PATH + 1);
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "";
		ofn.lpstrFilter = filter;
		char* pf = filter, * sf = 0;
		int len = 0;
#define APPEND(x) {memcpy(pf,x,len);pf+=len;}

		if (!rmi_only)
		{
			sf = BuildFilterString(IDS_MIDI_FILES, "MID", &len);
			APPEND(sf);

			sf = BuildFilterString(IDS_COMPRESSED_MIDI_FILES, "MIZ", &len);
			APPEND(sf);
		}
		sf = BuildFilterString(IDS_RMI_FILES, "RMI", &len);
		APPEND(sf);

		sf = BuildFilterString(IDS_COMPRESSED_RMI_FILES, "MIZ", &len);
		APPEND(sf);

#undef APPEND
		* pf = 0;

		if (!GetSaveFileNameA(&ofn)) return 0;

		tmp.buffer_done();

		fmt = ofn.nFilterIndex - 1;

		do_gzip = fmt & 1;
		fmt >>= 1;

		if (rmi_only) fmt = 1;

		if (do_gzip) do_ext(tmp, ".miz");
		else if (fmt == 1) do_ext(tmp, ".rmi");
		else do_ext(tmp, ".mid");


	}
	else
	{
		fmt = 1;
		const char* p = strrchr(tmp, '.');
		if (p && !_stricmp(p, ".miz")) do_gzip = 1;
	}

	{
		if (fmt > 1) fmt = 0;


		BOOL local = 0;
		const void* buf = 0;
		UINT buf_size = 0;
		if (fmt == 0)
		{
			buf = mf->data;
			buf_size = mf->size;
		}
		else //if (fmt==1)
		{
			local = 1;
			buf = build_rmi(mf, &buf_size);
		}
		int rv;

		if (do_gzip)
		{
			rv = SaveAsGZip(tmp, buf, buf_size);
		}
		else
		{
			HANDLE f = CreateFileA(tmp, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			if (f != INVALID_HANDLE_VALUE)
			{
				DWORD bw = 0;
				WriteFile(f, buf, buf_size, &bw, 0);
				CloseHandle(f);
				rv = 1;
			}
			else rv = 0;
		}
		if (local) free((void*)buf);
		if (!_stricmp(mf->path, tmp))
		{
			mf->format = fmt;
		}
		if (!rv)
		{
			char _m[320] = { 0 };
			StringCbPrintfA(_m, sizeof(_m), WASABI_API_LNGSTRING(STRING_WRITE_ERROR_FMT), (const char*)tmp);
			MessageBoxA(w, _m, ERROR, MB_ICONERROR);
		}
		return rv;
	}
}
/// <summary>
///		Compress given buffer with GZIP format and saves to given filename
/// </summary>
/// <param name="filename"></param>
/// <param name="buffer"></param>
/// <param name="size"></param>
/// <returns></returns>
int SaveAsGZip(string filename, const void* buffer, size_t size)
{
	void* data;
	size_t data_len = size;
	int ret = CompressionUtility::CompressAsGZip(buffer, size, &data, data_len);

	if (ret >= 0)
	{
		try
		{
			HANDLE f = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			if (f != INVALID_HANDLE_VALUE)
			{
				DWORD bw = 0;
				WriteFile(f, data, data_len, &bw, 0);
				CloseHandle(f);
				return 1;
			}
		}
		catch (...)
		{
			DWORD i = GetLastError();
			return 0;
		}
	}

	return 0;
}

#define _pr ((MIDI_file*)(lp))

static cfg_struct_t<RECT> cfg_infpos("infpos", -1);

static UINT inf_x_min = 0x80000000, inf_y_min, inf_c_x, inf_c_y;
static RECT r_trax, r_text;

static void SetWindowRect(HWND w, RECT* r)
{
	SetWindowPos(w, 0, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER);
}

void cGetWindowRect(HWND w, RECT* r)
{
	RECT tr, tr1;
	GetWindowRect(w, &tr);
	SetWindowPos(w, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	GetWindowRect(w, &tr1);
	r->left = tr.left - tr1.left;
	r->right = tr.right - tr1.left;
	r->top = tr.top - tr1.top;
	r->bottom = tr.bottom - tr1.top;
	SetWindowRect(w, r);
}


#define RB_NUM 3

static struct
{
	UINT id, dx, dy;
}
rb_dat[RB_NUM] =
{
	{IDC_SAVE, 0, 0},
	{IDOK, 0, 0},
	{IDC_RMI_CRAP, 0, 0},
};

#define BOTTOM_NUM 8

static struct
{
	UINT id;
	UINT x, dy;
}
b_dat[BOTTOM_NUM] =
{
	{IDC_STATIC1, 0, 0},
	{IDC_STATIC2, 0, 0},
	{IDC_STATIC3, 0, 0},
	{IDC_TIX, 0, 0},
	{IDC_MS, 0, 0},
	{IDC_FSIZE, 0, 0},
	{IDC_DLS, 0, 0},
	{IDC_LOOP, 0, 0}
};

static void OnSize(HWND wnd)
{
	RECT cl, t;
	GetClientRect(wnd, &cl);
	t.left = r_text.left;
	t.right = r_text.right;
	t.top = r_text.top;
	t.bottom = cl.bottom - (inf_c_y - r_text.bottom);
	SetWindowRect(GetDlgItem(wnd, IDC_COPYRIGHT), &t);
	t.left = r_trax.left;
	t.right = cl.right - (inf_c_x - r_trax.right);
	t.top = r_trax.top;
	t.bottom = cl.bottom - (inf_c_y - r_trax.bottom);
	SetWindowRect(GetDlgItem(wnd, IDC_TRAX), &t);
	UINT n;
	for (n = 0; n < BOTTOM_NUM; n++)
	{
		SetWindowPos(GetDlgItem(wnd, b_dat[n].id), 0, b_dat[n].x, cl.bottom - b_dat[n].dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
	for (n = 0; n < RB_NUM; n++)
	{
		SetWindowPos(GetDlgItem(wnd, rb_dat[n].id), 0, cl.right - rb_dat[n].dx, cl.bottom - rb_dat[n].dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}

BOOL WINAPI InfoProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		if (inf_x_min == 0x80000000)
		{
			RECT r;
			GetWindowRect(wnd, &r);
			inf_x_min = r.right - r.left;
			inf_y_min = r.bottom - r.top;
			GetClientRect(wnd, &r);
			inf_c_x = r.right;
			inf_c_y = r.bottom;
			cGetWindowRect(GetDlgItem(wnd, IDC_COPYRIGHT), &r_text);
			cGetWindowRect(GetDlgItem(wnd, IDC_TRAX), &r_trax);
			UINT n;
			for (n = 0; n < BOTTOM_NUM; n++)
			{
				cGetWindowRect(GetDlgItem(wnd, b_dat[n].id), &r);
				b_dat[n].x = r.left;
				b_dat[n].dy = inf_c_y - r.top;
			}
			for (n = 0; n < RB_NUM; n++)
			{
				cGetWindowRect(GetDlgItem(wnd, rb_dat[n].id), &r);
				rb_dat[n].dx = inf_c_x - r.left;
				rb_dat[n].dy = inf_c_y - r.top;
			}
		}
		if (cfg_infpos.get_val().left != -1)
		{
			int sx = GetSystemMetrics(SM_CXSCREEN), sy = GetSystemMetrics(SM_CYSCREEN);
			if (cfg_infpos.get_val().right > sx)
			{
				cfg_infpos.get_val().left -= cfg_infpos.get_val().right - sx;
				cfg_infpos.get_val().right = sx;
			}
			if (cfg_infpos.get_val().bottom > sy)
			{
				cfg_infpos.get_val().top -= cfg_infpos.get_val().bottom - sy;
				cfg_infpos.get_val().bottom = sy;
			}
			if (cfg_infpos.get_val().left < 0)
			{
				cfg_infpos.get_val().right -= cfg_infpos.get_val().left;
				cfg_infpos.get_val().left = 0;
			}
			if (cfg_infpos.get_val().top < 0)
			{
				cfg_infpos.get_val().bottom -= cfg_infpos.get_val().top;
				cfg_infpos.get_val().top = 0;
			}
			SetWindowRect(wnd, &cfg_infpos.get_val());
			OnSize(wnd);
		}
#if defined(_WIN64)
		SetWindowLong(wnd, DWLP_USER, lp);
#else
		SetWindowLong(wnd, DWL_USER, lp);
#endif
		SetDlgItemTextA(wnd, IDC_COPYRIGHT, _pr->info.copyright);
		SetDlgItemInt(wnd, IDC_MS, _pr->len, 0);
		SetDlgItemInt(wnd, IDC_TIX, _pr->info.tix, 0);
		{
			char tmp[128] = { 0 };
			getfmtstring(_pr, tmp);
			SetDlgItemTextA(wnd, IDC_FORMAT, tmp);

			HANDLE f = CreateFileA(_pr->path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
			if (f != INVALID_HANDLE_VALUE)
			{
				StringCbPrintfA(tmp, sizeof(tmp), WASABI_API_LNGSTRING(STRING_BYTES_FMT), GetFileSize(f, 0));
				CloseHandle(f);
				SetDlgItemTextA(wnd, IDC_FSIZE, tmp);
			}
		}
		{
			char tmp[1024] = { 0 };
			if (_pr->info.traxnames)
			{
				HWND lb = GetDlgItem(wnd, IDC_TRAX);
				UINT n;
				for (n = 0; n < _pr->info.ntrax; n++)
				{
					StringCbPrintfA(tmp, sizeof(tmp), "(%u) %s", n, (const char*)(_pr->info.traxnames[n]));
					SendMessageA(lb, LB_ADDSTRING, 0, (LPARAM)tmp);
				}
			}

			StringCbPrintfA(tmp, sizeof(tmp), WASABI_API_LNGSTRING(STRING_TRACKS_FMT), _pr->info.ntrax);
			SetDlgItemTextA(wnd, IDC_NTRAX, tmp);
			if (_pr->title)
			{
				StringCbPrintfA(tmp, sizeof(tmp), WASABI_API_LNGSTRING(STRING_MIDI_INFO_FMT1), (const char*)_pr->title, (const char*)_pr->path);
			}
			else
			{
				StringCbPrintfA(tmp, sizeof(tmp), WASABI_API_LNGSTRING(STRING_MIDI_INFO_FMT2), (const char*)_pr->path);
			}
			if (_pr->flags & FLAG_INCOMPLETE) StringCbCatA(tmp, sizeof(tmp), WASABI_API_LNGSTRING(STRING_INCOMPLETE));
			SetWindowTextA(wnd, tmp);
		}
		if (_pr->pDLSdata) EnableWindow(GetDlgItem(wnd, IDC_DLS), 1);
		if (_pr->loopstart) EnableWindow(GetDlgItem(wnd, IDC_LOOP), 1);
		//if (_pr->rmi_data) EnableWindow(GetDlgItem(wnd,IDC_RMI_CRAP),1);
		return 1;
	case WM_SIZE:
		OnSize(wnd);
		break;
	case WM_SIZING:
		if (lp)
		{
			RECT* r = (RECT*)lp;
			if ((UINT)(r->right - r->left) < inf_x_min)
			{
				if (wp != WMSZ_LEFT && wp != WMSZ_TOPLEFT && wp != WMSZ_BOTTOMLEFT)
					r->right = r->left + inf_x_min;
				else r->left = r->right - inf_x_min;
			}
			if ((UINT)(r->bottom - r->top) < inf_y_min)
			{
				if (wp != WMSZ_TOP && wp != WMSZ_TOPLEFT && wp != WMSZ_TOPRIGHT)
					r->bottom = r->top + inf_y_min;
				else r->top = r->bottom - inf_y_min;
			}
		}
		break;
	case WM_COMMAND:
		if (wp == IDOK || wp == IDCANCEL)
		{
			EndDialog(wnd,  /*changed ? 0 : 1*/0);
		}
		else if (wp == IDC_SAVE)
		{
#if defined(_WIN64)
			SaveFile(wnd, (MIDI_file*)GetWindowLong(wnd, DWLP_USER), 0);
#else
			SaveFile(wnd, (MIDI_file*)GetWindowLong(wnd, DWL_USER), 0);
#endif
		}
		else if (wp == IDC_RMI_CRAP)
		{
#if defined(_WIN64)
			MIDI_file* mf = (MIDI_file*)GetWindowLong(wnd, DWLP_USER);
#else
			MIDI_file* mf = (MIDI_file*)GetWindowLong(wnd, DWL_USER);
#endif
			show_rmi_info(wnd, mf);
		}
		break;
	case WM_CLOSE:
		EndDialog(wnd,  /*changed ? 0 : 1*/0);
		break;
	case WM_DESTROY:
		GetWindowRect(wnd, &cfg_infpos.get_val());
		break;
	}
	return 0;
}
#undef _pr
