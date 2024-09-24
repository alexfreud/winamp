#include "main.h"
#include "api__in_vorbis.h"
#include "genres.h"
#include <commctrl.h>
#include "../Agave/Language/api_language.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "resource.h"
#include <strsafe.h>

extern In_Module mod;
/* old PP info box. still used for streams */


extern CfgFont cfg_font;

extern CfgInt cfg_modeless,cfg_remember_infosize;

static CfgInt
	cfg_hide_special_fields("hide_special_fields",1),
	cfg_adv_info("adv_info",0),
	cfg_infobox_sx("infobox_sx",0),
	cfg_infobox_sy("infobox_sy",0),
	cfg_infobox_x("infobox_x",0),
	cfg_infobox_y("infobox_y",0);

void SetDlgItemTextWrap(HWND w,UINT id,wchar_t * tx)
{
	SetDlgItemTextW(w,id,tx);
}

typedef struct
{
	UINT id;
	wchar_t * name;	
} STD_TAG_ITEM;

#define N_STD_TAGZ 7

extern BOOL cfg_adv_warn;

static const STD_TAG_ITEM std_tagz[N_STD_TAGZ]=
{
	{IDC_TITLE,L"TITLE"},
	{IDC_ARTIST,L"ARTIST"},
	{IDC_ALBUM,L"ALBUM"},
	{IDC_GENRE,L"GENRE"},
	{IDC_DATE,L"DATE"},
	{IDC_COMMENT,L"COMMENT"},
	{IDC_TRACK,L"TRACKNUMBER"},
};

const wchar_t * special_fields[]={L"RG_PEAK",L"RG_RADIO",L"RG_AUDIOPHILE",L"LWING_GAIN",L"REPLAYGAIN_ALBUM_GAIN",L"REPLAYGAIN_ALBUM_PEAK",L"REPLAYGAIN_TRACK_GAIN",L"REPLAYGAIN_TRACK_PEAK"};
#define N_SPECIAL_FIELDS (sizeof(special_fields)/sizeof(special_fields[0]))


typedef struct tagTAG
{
	tagTAG * next;
	wchar_t * name;
	wchar_t * value;
} TAG;

typedef struct
{
	wchar_t *name,*value;
} TAGDESC;

class OggTagData
{
public:
	TAG * tags;
	TAG ** last;
	TAG * newtag()
	{
		TAG * t=new TAG;
		*last=t;
		last=&t->next;
		t->next=0;
		return t;
	}

	OggTagData()
	{
		tags=0;
		last=&tags;
	}

	String vendor;

	OggTagData(vorbis_comment * vc) : vendor(vc->vendor)
	{
		tags=0;
		last=&tags;
		int n;
		for(n=0;n<vc->comments;n++)
		{
			TAG * t=newtag();
			char * c=vc->user_comments[n];
			char * p=strchr(c,'=');
			if (p)
			{
				int size = MultiByteToWideChar(CP_UTF8, 0, c, (int)(p-c), 0,0);
				t->name=(wchar_t*)malloc((size+1)*sizeof(wchar_t));
				MultiByteToWideChar(CP_UTF8, 0, c, (int)(p-c), t->name, size);
				t->name[size]=0;
				p++;
			}
			else
			{
				t->name=_wcsdup(L"COMMENT");
				p=c;
			}

				int size = MultiByteToWideChar(CP_UTF8, 0, p, -1, 0,0);
				t->value=(wchar_t*)malloc((size)*sizeof(wchar_t));
				MultiByteToWideChar(CP_UTF8, 0, p, -1, t->value, size);
		}
	}
	void Clear()
	{
		TAG * t=tags;
		while(t)
		{
			TAG * t1=t->next;
			free(t->name);
			free(t->value);
			delete t;
			t=t1;
		}
		tags=0;
		last=&tags;
	}
	void AddTag(const wchar_t * name,const wchar_t * value)
	{
		TAG * t=newtag();
		t->name=_wcsdup(name);
		t->value=_wcsdup(value);
	}
	~OggTagData()
	{
		Clear();
	}
};

static void SetWindowRect(HWND w,RECT * r)
{
	SetWindowPos(w,0,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOACTIVATE);
}

class DlgBase
{
protected:
	bool DieOnDestroyWindow,is_modeless,is_modal_ex,modal_ex_quit;
	int modal_ex_quit_val;

	void endDialog(int x)
	{
		if (is_modeless) DestroyWindow(wnd);
		else if (is_modal_ex)
		{
			modal_ex_quit=1;
			modal_ex_quit_val=x;
			DestroyWindow(wnd);
		}
		else EndDialog(wnd,x);
	}
	
	void _do_size_x(RECT * r,UINT id,UINT wx,UINT min_x)
	{
		RECT r1={r->left,r->top,(LONG)(wx-min_x)+r->right,r->bottom};
		SetWindowRect(GetDlgItem(wnd,id),&r1);
	}

	void _do_size_xy(RECT * r,UINT id,UINT wx,UINT wy,UINT min_x,UINT min_y)
	{
		RECT r1={r->left,r->top,(LONG)(wx-min_x)+r->right,(LONG)(wy-min_y)+r->bottom};
		SetWindowRect(GetDlgItem(wnd,id),&r1);
	}

	void _do_align_x_size_y(RECT * r,UINT id,UINT wx,UINT wy,UINT min_x,UINT min_y)
	{
		RECT r1={ (LONG)(wx-min_x)+r->left,r->top,(LONG)(wx-min_x)+r->right,(LONG)(wy-min_y)+r->bottom};
		SetWindowRect(GetDlgItem(wnd,id),&r1);
	}

	void _do_align_x(RECT * r,UINT id,UINT wx,UINT min_x)
	{
		RECT r1={ (LONG)(wx-min_x+r)->left,(LONG)r->top,(LONG)(wx-min_x+r)->right,(LONG)r->bottom};
		SetWindowRect(GetDlgItem(wnd,id),&r1);
	}

	void _do_align_xy(RECT * r,UINT id,UINT wx,UINT wy,UINT min_x,UINT min_y)
	{
		RECT r1={(LONG)(wx-min_x+r)->left,(LONG)(wy-min_y+r)->top,(LONG)(wx- min_x+r)->right,(LONG)(wy-min_y+r)->bottom};
		SetWindowRect(GetDlgItem(wnd,id),&r1);
	}

#define do_size_x(id,r) _do_size_x(r,id,sx,min_size_x)
#define do_size_xy(id,r) _do_size_xy(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_x_size_y(id,r) _do_align_x_size_y(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_xy(id,r) _do_align_xy(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_x(id,r) _do_align_x(r,id,sx,min_size_x)

	HWND wnd;
	UINT min_size_x,min_size_y;
	UINT min_size_x_w,min_size_y_w;

	void do_sizing(UINT wp,RECT * r)
	{
		UINT dx,dy;
		dx=r->right-r->left;
		dy=r->bottom-r->top;
		if (dx<min_size_x_w)
		{
			switch(wp)
			{
			case WMSZ_BOTTOMLEFT:
			case WMSZ_LEFT:
			case WMSZ_TOPLEFT:
				r->left=r->right-min_size_x_w;
				break;
			case WMSZ_BOTTOMRIGHT:
			case WMSZ_RIGHT:
			case WMSZ_TOPRIGHT:
				r->right=r->left+min_size_x_w;
				break;
			}
		}
		if (dy<min_size_y_w)
		{
			switch(wp)
			{
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMRIGHT:
				r->bottom=r->top+min_size_y_w;
				break;
			case WMSZ_TOPLEFT:
			case WMSZ_TOP:
			case WMSZ_TOPRIGHT:
				r->top=r->bottom-min_size_y_w;
				break;
			}
		}
	}
	void MakeComboEdit(UINT id,DWORD s)
	{
		HWND w=GetDlgItem(wnd,id);
		RECT r;
		GetChildRect(id,r);
		DestroyWindow(w);
		CreateWindowEx( WS_EX_CLIENTEDGE, L"EDIT", 0, WS_CHILD | s, r.left - 1, r.top - 1, r.right - r.left, r.bottom - r.top, wnd, (HMENU)id, 0, 0 );
	}
	void GetChildRect(UINT id,RECT& child)
	{
		RECT r_parent,r_child;
		GetWindowRect(wnd,&r_parent);
		GetWindowRect(GetDlgItem(wnd,id),&r_child);
		int dx=r_parent.left;
		int dy=r_parent.top;
		if (!(GetWindowLong(wnd,GWL_STYLE)&WS_CHILD))
		{
			dy+=GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYDLGFRAME);
			dx+=GetSystemMetrics(SM_CXDLGFRAME);			
		}
		child.left=r_child.left-dx;
		child.right=r_child.right-dx;
		child.top=r_child.top-dy;
		child.bottom=r_child.bottom-dy;
	}

	virtual BOOL DlgProc(UINT msg,WPARAM wp,LPARAM lp) {return 0;};
	static BOOL CALLBACK TheDialogProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		DlgBase * p;
		if (msg==WM_INITDIALOG)
		{
			p=(DlgBase*)lp;
#ifdef WIN64
			SetWindowLong(wnd, DWLP_USER, (LONG)lp);
#else
			SetWindowLong(wnd, DWL_USER, lp);
#endif
			p->wnd=wnd;
			RECT r;
			GetClientRect(wnd,&r);
			p->min_size_x=r.right;
			p->min_size_y=r.bottom;
			GetWindowRect(wnd,&r);
			p->min_size_x_w=r.right-r.left;
			p->min_size_y_w=r.bottom-r.top;
		}
#ifdef WIN64
		else p = (DlgBase*)GetWindowLong(wnd, DWLP_USER);
#else
		else p = (DlgBase*)GetWindowLong(wnd, DWL_USER);
#endif
		BOOL rv=0;
		if (p)
		{
			rv=p->DlgProc(msg,wp,lp);
			if (msg==WM_DESTROY)
			{
				p->wnd=0;
				if (p->DieOnDestroyWindow)
				{
					delete p;
#ifdef WIN64
					SetWindowLong(wnd, DWLP_USER, 0);
#else
					SetWindowLong(wnd, DWL_USER, 0);
#endif
				}
			}
		}
		return rv;
	}
	HWND myCreateDialog(UINT id,HWND parent)
	{
		DieOnDestroyWindow=1;
		is_modeless=1;
		is_modal_ex=0;
		return WASABI_API_CREATEDIALOGPARAMW(id,parent,TheDialogProc,(LPARAM)this);
	}
	virtual void myProcessMessage(MSG * msg)
	{
		if (!IsDialogMessage(wnd,msg))
		{
			TranslateMessage(msg);
			DispatchMessage(msg);
		}
	}

	int myDialogBoxEx(UINT id,HWND parent)
	{
		DieOnDestroyWindow=0;
		is_modeless=0;
		is_modal_ex=1;
		modal_ex_quit=0;
		modal_ex_quit_val=-1;
		WASABI_API_CREATEDIALOGPARAMW(id,parent,TheDialogProc,(LPARAM)this);
		if (wnd)
		{
			BOOL b=IsWindowEnabled(parent);

			if (b) EnableWindow(parent,0);

			MSG msg;
			while(!modal_ex_quit && GetMessage(&msg,0,0,0))
			{
				myProcessMessage(&msg);
			}

			if (wnd)
			{
				DestroyWindow(wnd);
				wnd=0;
			}
			
			if (b) EnableWindow(parent,1);
			SetActiveWindow(parent);
		}
		return modal_ex_quit_val;
	}

	int myDialogBox(UINT id,HWND parent)
	{
		DieOnDestroyWindow=0;
		is_modeless=0;
		is_modal_ex=0;
		return (int)WASABI_API_DIALOGBOXPARAMW(id,parent,TheDialogProc,(LPARAM)this);
	}
	DlgBase()
	{
		wnd=0;
		DieOnDestroyWindow=0;
		is_modeless=0;
		is_modal_ex=0;
		modal_ex_quit=0;
		modal_ex_quit_val=0;
		min_size_x=min_size_y=min_size_x_w=min_size_y_w=0;
	}
	virtual ~DlgBase() {DieOnDestroyWindow=0;if (wnd) DestroyWindow(wnd);}
public:
	BOOL isDialogMessage(MSG * m) {return wnd ? IsDialogMessage(wnd,m) : 0;}
};

static char tags_file[]="tags.txt";
static wchar_t genres_file[] = L"genres.txt";

class /*_declspec(novtable) */InfoDlgPanel : public DlgBase
{
protected:
	HFONT font;
	HWND list;
	RECT info_list;
	OggTagData hidden;

	InfoDlgPanel(UINT id,HWND parent,HFONT foo)
	{
		font=foo;
		myCreateDialog(id,parent);
		//SetWindowLong(wnd,GWL_STYLE,GetWindowLong(wnd,GWL_STYLE)|WS_TABSTOP);
		list=GetDlgItem(wnd,IDC_LIST);
		SendMessage(list,WM_SETFONT,(WPARAM)font,0);

		GetChildRect(IDC_LIST,info_list);
	}

	int lb_addstring(TAGDESC * tag,int idx=-1)
	{
		StringW tmp;
		tmp+=tag->name;
		tmp+=L"=";
		tmp+=tag->value;

		const WCHAR * p=(const WCHAR*)tmp;
		const WCHAR * foo=wcsstr(p,L"\x0d\x0a");
		if (foo)
		{
			tmp.Truncate((UINT)(foo-p));
			tmp.AddString(L" (...)");
		}
		int rv=
			(int)SendMessageW(list,
			idx<0 ? LB_ADDSTRING : LB_INSERTSTRING,
			idx<0 ? 0 : idx,
			((LPARAM)(const WCHAR*)tmp)
			);
		if (rv>=0) SendMessage(list,LB_SETITEMDATA,rv,(LPARAM)tag);
		else
		{
			free(tag->name);
			free(tag->value);
			delete tag;
		}
		return rv;

	}

	virtual void OnUpdateRect(UINT sx,UINT sy)
	{//WM_SIZE-ish
		do_size_xy(IDC_LIST,&info_list);
	}
	
	virtual BOOL DlgProc(UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_DESTROY:
			lb_clear();
			list=0;
			break;
		case WM_COMMAND:
			switch(wp)
			{
			case IDOK:
			case IDCANCEL:
				SendMessage(GetParent(wnd),msg,wp,lp);
				break;
			}
			break;
//		case WM_INITDIALOG:
//			return 1;
		}
		return 0;
	}

	void lb_clear()
	{
		if (!list) return;
		int num=(int)SendMessage(list,LB_GETCOUNT,0,0);
		while(num>0)
		{
			TAGDESC * l=(TAGDESC*)SendMessage(list,LB_GETITEMDATA,0,0);
			if (l)
			{
				free(l->name);
				free(l->value);
				delete l;
			}
			SendMessage(list,LB_DELETESTRING,0,0);
			num--;
		}
	}

	virtual void SetTag(wchar_t * name,wchar_t * value)
	{
		TAGDESC * l=new TAGDESC;
		l->name=_wcsdup(name);
		l->value=_wcsdup(value);
		lb_addstring(l);
	}
	
public:
	virtual void Clear()
	{
		hidden.Clear();
		lb_clear();
	}
	void UpdateRect(RECT &r)
	{
		SetWindowPos(wnd,0,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		RECT z;
		GetClientRect(wnd,&z);
		OnUpdateRect(z.right-z.left,z.bottom-z.top);
	}
	
	virtual void GetTags(OggTagData & tags)
	{
		if (!list) return;
		int num=(int)SendMessage(list,LB_GETCOUNT,0,0);
		int n;
		for(n=0;n<num;n++)
		{
			TAGDESC * l=(TAGDESC*)SendMessage(list,LB_GETITEMDATA,n,0);
			tags.AddTag(l->name,l->value);			
		}
		TAG * t=hidden.tags;
		while(t)
		{
			tags.AddTag(t->name,t->value);
			t=t->next;
		}
	}

	void SetTags(OggTagData & tags,BOOL hidespec)
	{
		TAG * t=tags.tags;
		while(t)
		{
			bool hide=0;
			if (hidespec)
			{
				int n;
				for(n=0;n<N_SPECIAL_FIELDS;n++)
				{
					if (!_wcsicmp(t->name,special_fields[n]))
					{
						hidden.AddTag(t->name,t->value);
						hide=1;break;
					}
				}
			}
			if (!hide) SetTag(t->name,t->value);
			t=t->next;
		}
	}
};

class InfoDlgPanel_adv : public InfoDlgPanel
{
private:
	virtual void OnUpdateRect(UINT sx,UINT sy)
	{
		InfoDlgPanel::OnUpdateRect(sx,sy);
	}
public:
	InfoDlgPanel_adv(HWND parent,HFONT foo) : InfoDlgPanel(IDD_INFO_PANEL_ADVANCED,parent,foo)
	{
	}
};

class InfoDlgPanel_simple : public InfoDlgPanel
{
private:
	RECT info_static_std,info_title,info_artist,info_album,info_track,info_genre,info_comment,info_static_track,info_static_tags;
	wchar_t *tag_bk[N_STD_TAGZ];
	BOOL STFU;
protected:
	virtual void OnUpdateRect(UINT sx,UINT sy)
	{
		do_size_x(IDC_STATIC_STD,&info_static_std);
		do_size_x(IDC_TITLE,&info_title);
		do_size_x(IDC_ARTIST,&info_artist);
		do_size_x(IDC_ALBUM,&info_album);
		do_align_x(IDC_TRACK,&info_track);
		do_size_x(IDC_GENRE,&info_genre);
		do_size_x(IDC_COMMENT,&info_comment);
		do_align_x(IDC_STATIC_TRACK,&info_static_track);
		do_size_xy(IDC_STATIC_TAGS,&info_static_tags);
		InfoDlgPanel::OnUpdateRect(sx,sy);
	}
	virtual BOOL DlgProc(UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_COMMAND:
			if (!STFU && (HIWORD(wp)==EN_CHANGE || HIWORD(wp)==CBN_EDITCHANGE || HIWORD(wp)==CBN_SELCHANGE))
			{
				UINT n;
				for(n=0;n<N_STD_TAGZ;n++)
				{
					if (LOWORD(wp)==std_tagz[n].id)
					{
						if (tag_bk[n]) {free(tag_bk[n]);tag_bk[n]=0;}
						break;
					}
				}
			}
			break;
		}
		return InfoDlgPanel::DlgProc(msg,wp,lp);
	}
public:
	virtual void Clear()
	{
		int n;
		for(n=0;n<N_STD_TAGZ;n++)
		{
			if (tag_bk[n])
			{
				free(tag_bk[n]);
				tag_bk[n]=0;
			}
			SetDlgItemText(wnd,std_tagz[n].id,L"");
		}
		InfoDlgPanel::Clear();

	}
	~InfoDlgPanel_simple()
	{
		UINT n;
		for(n=0;n<N_STD_TAGZ;n++)
		{
			if (tag_bk[n]) free(tag_bk[n]);
		}
	}
	InfoDlgPanel_simple(HWND parent,HFONT foo) : InfoDlgPanel(IDD_INFO_PANEL_SIMPLE,parent,foo)
	{
		STFU=0;
		memset(tag_bk,0,sizeof(tag_bk));
		UINT n;
		MakeComboEdit(IDC_GENRE,ES_READONLY|ES_AUTOHSCROLL|WS_VISIBLE);
		for(n=0;n<N_STD_TAGZ;n++)
		{
			HWND w=GetDlgItem(wnd,std_tagz[n].id);
			SendMessage(w,WM_SETFONT,(WPARAM)font,0);
			SendMessage(w,EM_SETREADONLY,1,0);
		}

		GetChildRect(IDC_STATIC_STD,info_static_std);
		GetChildRect(IDC_TITLE,info_title);
		GetChildRect(IDC_ARTIST,info_artist);
		GetChildRect(IDC_ALBUM,info_album);
		GetChildRect(IDC_TRACK,info_track);
		GetChildRect(IDC_GENRE,info_genre);
		GetChildRect(IDC_COMMENT,info_comment);
		GetChildRect(IDC_STATIC_TRACK,info_static_track);
		GetChildRect(IDC_STATIC_TAGS,info_static_tags);
		
		genres_read(GetDlgItem(wnd,IDC_GENRE), genres_file);
	}

	virtual void GetTags(OggTagData & tags)
	{
		genres_write(GetDlgItem(wnd,IDC_GENRE),genres_file);
		UINT n;
		for(n=0;n<N_STD_TAGZ;n++)
		{
			if (tag_bk[n])
			{
				tags.AddTag(std_tagz[n].name,tag_bk[n]);
			}
			else
			{
				StringW t;
				t.s_GetWindowText(GetDlgItem(wnd,std_tagz[n].id));
				if (t.Length()>0)
				{
					tags.AddTag(std_tagz[n].name,t);
				}
			}
		}
		InfoDlgPanel::GetTags(tags);
	}
	virtual void SetTag(wchar_t * name,wchar_t * value)
	{
		STFU=1;
		UINT n;
		for(n=0;n<N_STD_TAGZ;n++)
		{
			if (tag_bk[n]) continue;
			if (!_wcsicmp(name,std_tagz[n].name))
			{
				tag_bk[n]=_wcsdup(value);
				SetDlgItemTextWrap(wnd,std_tagz[n].id,value);
				STFU=0;
				return;
			}
		}
		STFU=0;
		InfoDlgPanel::SetTag(name,value);
	}
};

char * rstrcpy(char* s1,char* s2)
{
	while(s2 && *s2) *(s1++)=*(s2++);
	return s1;
}

static void _inline print_misc(VorbisFile * _vf,int link,char * out,int len)
{
	OggVorbis_File * vf=&_vf->vf;
	char* p=out, kbps_str[16] = {0};
	double t=ov_time_total(vf,link);
	vorbis_info * vi=ov_info(vf,link);
	vorbis_comment * vc=ov_comment(vf,link);
	if (!vi || !vc) {WASABI_API_LNGSTRING_BUF(IDS_FILE_ERROR,out,512);return;}

	StringCchPrintfA(kbps_str, 16, " %s\r\n", WASABI_API_LNGSTRING(IDS_KBPS));

	if (t>0)
	{
		p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_LENGTH));
		int h=(int)(t/3600.0);
		if (h>0)
		{
			uitoa(h,p);
			while(p && *p) p++;
			*(p++)=':';
		}
		int m=(int)(t/60.0)%60;
//		if (m>0 || h>0)
		{
			sprintf(p,h>0 ? "%02u" : "%u",m);
			while(p && *p) p++;
			*(p++)=':';
		}
		int s=(int)t%60;
		//sprintf(p,(m>0 || h>0) ? "%02u" : "%u seconds",s);
		sprintf(p,"%02d",s);
		while(p && *p) p++;
		p=rstrcpy(p,"\r\n");

//		uitoa((int)(t*1000.0),p);
//		while(p && *p) p++;
//		p=rstrcpy(p," ms");
		UINT fs=_vf->FileSize();
		if (fs>0)
		{
			if (vf->links==1)
			{
				p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_AVERAGE_BITRATE));
				uitoa((int)(((double)fs)/(t*125.0)),p);
				while(p && *p) p++;
				p=rstrcpy(p,kbps_str);
			}
			if (fs>0)
			{
				p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_FILE_SIZE));

				UINT fs1=fs/1000000;
				UINT fs2=(fs/1000)%1000;
				UINT fs3=fs%1000;
				if (fs1) sprintf(p,"%u%03u%03u %s\r\n",fs1,fs2,fs3,WASABI_API_LNGSTRING(IDS_BYTES));
				else if (fs2) sprintf(p,"%u%03u %s\r\n",fs2,fs3,WASABI_API_LNGSTRING(IDS_BYTES));
				else sprintf(p,"%u %s\r\n",fs3,WASABI_API_LNGSTRING(IDS_BYTES));
				while(p && *p) p++;
			}
		}
	}
	if (vi->bitrate_nominal>0)
	{
		p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_NOMINAL_BITRATE));
		uitoa(vi->bitrate_nominal/1000,p);
		while(p && *p) p++;
		p=rstrcpy(p,kbps_str);
	}
	if (vi->bitrate_lower>0)
	{
		p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_MIN_BITRATE));
		uitoa(vi->bitrate_lower/1000,p);
		while(p && *p) p++;
		p=rstrcpy(p,kbps_str);
	}
	if (vi->bitrate_upper>0)
	{
		p=rstrcpy(p,WASABI_API_LNGSTRING(IDS_MAX_BITRATE));
		uitoa(vi->bitrate_upper/1000,p);
		while(p && *p) p++;
		p=rstrcpy(p,kbps_str);
	}

	char tmp[32] = {0}, tmp2[32] = {0}, tmp3[32] = {0}, tmp4[32] = {0}, tmp5[32] = {0}, tmp6[8] = {0};
	StringCchPrintfA(p,len, "%s : %u\r\n"
							"%s : %u %s\r\n"
							"%s : %u\r\n"
							"%s : %u\r\n"
							"%s : \r\n%s",
					 WASABI_API_LNGSTRING_BUF(IDS_CHANNELS,tmp,32),vi->channels,
					 WASABI_API_LNGSTRING_BUF(IDS_SAMPLING_RATE,tmp2,32),vi->rate,WASABI_API_LNGSTRING_BUF(IDS_HZ,tmp6,8),
					 WASABI_API_LNGSTRING_BUF(IDS_SERIAL_NUMBER,tmp3,32),ov_serialnumber(vf,link),
					 WASABI_API_LNGSTRING_BUF(IDS_VERSION,tmp4,32),vi->version,
					 WASABI_API_LNGSTRING_BUF(IDS_VENDOR,tmp5,32),vc->vendor);
}

class InfoDlg : public DlgBase
{
private:
	InfoDlg * next;
	static InfoDlg* Instances;
	OggTagData **tags;
	char ** infos;
	int n_streams,cur_stream;
	StringW url;
	HFONT ui_font;
	BOOL is_adv;
	InfoDlgPanel* panel;
	RECT info_static_misc,info_misc,info_url,info_cancel,info_mode,info_static_cs,info_cs_next,info_cs_prev,info_hidespec;
	
	void calc_panel_rect(RECT &r)
	{
		RECT cr,cr1,cr2;
		GetClientRect(wnd,&cr);
		GetChildRect(IDC_STATIC_MISC,cr1);
		GetChildRect(IDC_URL,cr2);
		r.left=0;
		r.top=cr2.bottom+1;
		r.right=cr1.left-1;
		r.bottom=cr.bottom;
	}
	void do_size()
	{
		if (panel)
		{
			RECT r;
			calc_panel_rect(r);
			panel->UpdateRect(r);
		}
	}
	void do_panel(BOOL mode,int d_stream)
	{
		//if (panel && is_adv==mode && !d_stream) return;
		if (panel)
		{
			if (mode!=is_adv)
			{
				delete panel;
				panel=0;
			}
			else
			{
				panel->Clear();
			}
		}
		cur_stream+=d_stream;
		if (cur_stream<0) cur_stream=0;
		else if (cur_stream>=n_streams) cur_stream=n_streams-1;
		is_adv=mode;
		if (!panel)
		{
			panel = mode ?  (InfoDlgPanel*) new InfoDlgPanel_adv(wnd,ui_font) : (InfoDlgPanel*) new InfoDlgPanel_simple(wnd,ui_font);
			do_size();
		}
		if (panel)
		{
			panel->SetTags(*tags[cur_stream],(BOOL)SendDlgItemMessage(wnd,IDC_HIDE_SPEC,BM_GETCHECK,0,0));
		}
		SetDlgItemText(wnd,IDC_MODE_TOGGLE,WASABI_API_LNGSTRINGW((is_adv ? IDS_TO_SIMPLE_MODE : IDS_TO_ADVANCED_MODE)));
		EnableWindow(GetDlgItem(wnd,IDC_PREV_STREAM),cur_stream>0);
		EnableWindow(GetDlgItem(wnd,IDC_NEXT_STREAM),cur_stream<n_streams-1);
		// TODO
		SetDlgItemTextA(wnd,IDC_MISC,infos[cur_stream]);
	}
protected:
	virtual BOOL DlgProc(UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			if (n_streams<=1)
			{
				ShowWindow(GetDlgItem(wnd,IDC_STATIC_CS),SW_HIDE);
				ShowWindow(GetDlgItem(wnd,IDC_PREV_STREAM),SW_HIDE);
				ShowWindow(GetDlgItem(wnd,IDC_NEXT_STREAM),SW_HIDE);
			}

			SendDlgItemMessage(wnd,IDC_HIDE_SPEC,BM_SETCHECK,cfg_hide_special_fields,0);

			do_panel(cfg_adv_info,0);
			url.s_SetDlgItemText(wnd,IDC_URL);
			GetChildRect(IDC_URL,info_url);
			GetChildRect(IDC_STATIC_MISC,info_static_misc);
			GetChildRect(IDC_MISC,info_misc);
			GetChildRect(IDCANCEL,info_cancel);
			GetChildRect(IDC_MODE_TOGGLE,info_mode);
			GetChildRect(IDC_STATIC_CS,info_static_cs);
			GetChildRect(IDC_NEXT_STREAM,info_cs_next);
			GetChildRect(IDC_PREV_STREAM,info_cs_prev);
			GetChildRect(IDC_HIDE_SPEC,info_hidespec);

			if (cfg_remember_infosize && cfg_infobox_sx>0 && cfg_infobox_sy>0)
			{
				int max_x=GetSystemMetrics(SM_CXSCREEN),max_y=GetSystemMetrics(SM_CYSCREEN);
				if (cfg_infobox_x<0) cfg_infobox_x=0;
				else if (cfg_infobox_x+cfg_infobox_sx>max_x) cfg_infobox_x=max_x-cfg_infobox_sx;
				if (cfg_infobox_y<0) cfg_infobox_y=0;
				else if (cfg_infobox_y+cfg_infobox_sy>max_y) cfg_infobox_y=max_y-cfg_infobox_sy;

				SetWindowPos(wnd,0,cfg_infobox_x,cfg_infobox_y,cfg_infobox_sx,cfg_infobox_sy,SWP_NOZORDER|SWP_NOACTIVATE);
			}

			StringPrintfW(WASABI_API_LNGSTRINGW(IDS_OGG_VORBIS_INFO),(const WCHAR*)StringF2T_W((const WCHAR*)url)).s_SetWindowText(wnd);
			return 1;
		case WM_SIZE:
			{
				UINT sx=LOWORD(lp),sy=HIWORD(lp);
				do_size_x(IDC_URL,&info_url);
				do_align_x(IDC_STATIC_MISC,&info_static_misc);
				do_align_x(IDC_MISC,&info_misc);
				do_align_xy(IDCANCEL,&info_cancel);
				do_align_xy(IDC_MODE_TOGGLE,&info_mode);
				do_align_xy(IDC_STATIC_CS,&info_static_cs);
				do_align_xy(IDC_PREV_STREAM,&info_cs_prev);
				do_align_xy(IDC_NEXT_STREAM,&info_cs_next);
				do_align_xy(IDC_HIDE_SPEC,&info_hidespec);
			}
			//RedrawWindow(wnd,0,0,RDW_INVALIDATE);
			do_size();
			break;
		case WM_SIZING:
			do_sizing((UINT)wp,(RECT*)lp);
			break;
		case WM_COMMAND:
			switch(wp)
			{
			case IDCANCEL:
				endDialog(0);
				break;
			case IDC_MODE_TOGGLE:
				do_panel(!is_adv,0);
				break;
			case IDC_PREV_STREAM:
				do_panel(is_adv,-1);
				break;
			case IDC_NEXT_STREAM:
				do_panel(is_adv,1);
				break;
			case IDC_HIDE_SPEC:
				cfg_hide_special_fields=(int)SendMessage((HWND)lp,BM_GETCHECK,0,0);
				do_panel(is_adv,0);
				break;
			}
			break;
		case WM_CLOSE:
			endDialog(0);
			break;
		case WM_DESTROY:
			if (!is_modeless)//fucko close
			{
				modal_ex_quit=1;
			}

			{
				RECT r;
				GetWindowRect(wnd,&r);
				cfg_infobox_sx=r.right-r.left;
				cfg_infobox_sy=r.bottom-r.top;
				cfg_infobox_x=r.left;
				cfg_infobox_y=r.top;
			}
			break;
		}
		return 0;
	}
	virtual void myProcessMessage(MSG * msg)
	{
		if (!panel || !panel->isDialogMessage(msg))
		{
			DlgBase::myProcessMessage(msg);
		}
	}

public:
	InfoDlg(VorbisFile * _vf,const wchar_t * _url)
		: url(_url), is_adv(FALSE)
	{
		OggVorbis_File * vf=&_vf->vf;
		n_streams=vf->links;
		cur_stream=vf->current_link;
		tags=(OggTagData**)malloc(n_streams*sizeof(void*));
		int n;
		for(n=0;n<n_streams;n++) tags[n]=new OggTagData(ov_comment(vf,n));
		infos=(char**)malloc(sizeof(void*)*n_streams);
		for(n=0;n<n_streams;n++)
		{
			int l = 512+(int)strlen(vf->vc->vendor);
			infos[n]=(char*)malloc(l);
			print_misc(_vf,n,infos[n],l);
		}

		ui_font=CreateFontIndirect(&cfg_font.data);
		panel=0;
		next=0;
	}
	~InfoDlg()
	{
		if (ui_font) DeleteObject((HGDIOBJ)ui_font);
		cfg_adv_info=is_adv;
		if (tags)
		{
			int n;
			for(n=0;n<n_streams;n++)
				delete tags[n];
			free(tags);
		}
		if (infos)
		{
			int n;
			for(n=0;n<n_streams;n++) free(infos[n]);
			free(infos);
		}
		
		InfoDlg ** pp=&Instances,*p=Instances;
		while(p)
		{
			if (p==this)
			{
				*pp=next;
				break;
			}
			else {pp=&p->next;p=*pp;}
		}
	}
	void Run(HWND parent,bool modeless)
	{
		next=Instances;
		Instances=this;//HACK - prevent crash on shutdown (used to be only for modeless)

		if (modeless)
		{
			myCreateDialog(IDD_INFO_DLG_NEW,parent);
		}
		else myDialogBoxEx(IDD_INFO_DLG_NEW,parent);
	}

friend int RunInfoDlg(const in_char * url,HWND parent);
};

int RunInfoDlg(const in_char * url,HWND parent)
{
	static bool in_modal;
	if (in_modal) return 1;
	else  in_modal=1;
	VorbisFile * vf;
	bool vf_global=0;
	int ret=0;
	StringW _url;
	_url.AddString(url);

	{
		InfoDlg * p=InfoDlg::Instances;
		while(p)
		{
			if (!_wcsicmp(p->url,_url))
			{
				ShowWindow(p->wnd,SW_SHOW);
				SetActiveWindow(p->wnd);
				return 0;
			}
			p=p->next;
		}
	}

	EnterCriticalSection(&sync);
	if ((url==cur_file || !_wcsicmp(url,cur_file)) && theFile) {vf=theFile;vf_global=1;}
	else
	{
		LeaveCriticalSection(&sync);
		vf=VorbisFile::Create(url,1);
		if (!vf) 
		{
		in_modal=0;
		return 0;
		}
	}
	{
		InfoDlg d(vf,_url);
		if (vf_global) LeaveCriticalSection(&sync);
		else delete vf;
		d.Run(parent,0);
		ret = !d.modal_ex_quit_val;
	}
	in_modal=0;
	return ret;
}

InfoDlg* InfoDlg::Instances=0;
/* end crappy PP dialog */

bool VorbisTagToWinampTag(wchar_t * tag, int len) 
{
#define TAG_ALIAS(b,a) if(!_wcsicmp(L ## a, tag)) { lstrcpynW(tag, L ## b, len); return true; }
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUMARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("bpm", "BPM");
	return false;
#undef TAG_ALIAS
}

bool WinampTagToVorbisTag(wchar_t * tag, int len) 
{
#define TAG_ALIAS(a,b) if(!_wcsicmp(L ## a, tag)) { lstrcpynW(tag, L ## b, len); return true; }
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUMARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("bpm", "BPM");
	return false;
#undef TAG_ALIAS
}

static INT_PTR CALLBACK ChildProc_Advanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int sel=-1;
	static int ismychange=0;
	switch(msg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;
	case WM_INITDIALOG:
		{
			#define ListView_InsertColumnW(hwnd, iCol, pcol) \
					(int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
			sel=-1;
			HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
			ListView_SetExtendedListViewStyle(hwndlist, LVS_EX_FULLROWSELECT);
			LVCOLUMNW lvc = {0, };
			lvc.mask = LVCF_TEXT|LVCF_WIDTH;
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_NAME);
			lvc.cx = 82;
			ListView_InsertColumnW(hwndlist, 0, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_VALUE);
			lvc.cx = 160;
			ListView_InsertColumnW(hwndlist, 1, &lvc);

			Info *info = (Info *)lParam;
			int n = info->GetNumMetadataItems();
			for(int i=0; i<n; i++) {
				wchar_t key[512] = {0};
				wchar_t value[2048] = {0};
				info->EnumMetadata(i,key,512,value,2048);
				if(value[0] && key[0]) {
					LVITEMW lvi={LVIF_TEXT,i,0};
					lvi.pszText = key;
					SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
					lvi.iSubItem=1;
					lvi.pszText = (wchar_t*)value;
					SendMessage(hwndlist,LVM_SETITEMW,0,(LPARAM)&lvi);
				}
			}
			ListView_SetColumnWidth(hwndlist,0,(n?LVSCW_AUTOSIZE:LVSCW_AUTOSIZE_USEHEADER));
			ListView_SetColumnWidth(hwndlist,1,(n?LVSCW_AUTOSIZE:LVSCW_AUTOSIZE_USEHEADER));

			SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
			SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
			EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
		}
		break;
	case WM_DESTROY:
		{
			HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
			ListView_DeleteAllItems(hwndlist);
			while(ListView_DeleteColumn(hwndlist,0));
			Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			delete info;
		}
		break;
	case WM_USER:
		if(wParam && lParam && !ismychange)
		{
			wchar_t * value = (wchar_t*)lParam;
			wchar_t tag[100] = {0};
			lstrcpynW(tag,(wchar_t*)wParam,100);
			WinampTagToVorbisTag(tag,100);
			Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if(!*value) 
			{
				info->RemoveMetadata(tag);
				if(!_wcsicmp(L"ALBUMARTIST",tag)) 
				{
					// need to remove these two, also, or else it's gonna look like delete doesn't work
					// if the file was tagged using these alternate fields
					info->RemoveMetadata(L"ALBUM ARTIST");
					info->RemoveMetadata(L"ENSEMBLE");
				}
				if(!_wcsicmp(L"PUBLISHER",tag)) 
				{
					// need to remove this also, or else it's gonna look like delete doesn't work
					// if the file was tagged using this alternate field
					info->RemoveMetadata(L"ORGANIZATION");
				}
				if(!_wcsicmp(L"DATE",tag)) 
				{
					// need to remove this also, or else it's gonna look like delete doesn't work
					// if the file was tagged using this alternate field
					info->RemoveMetadata(L"YEAR");
				}
				if(!_wcsicmp(L"TRACKNUMBER",tag)) 
				{
					// need to remove this also, or else it's gonna look like delete doesn't work
					// if the file was tagged using this alternate field
					info->RemoveMetadata(L"TRACK");
				}
			}
			else 
			{
				info->SetMetadata(tag,value);
			}

			HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
			int n = ListView_GetItemCount(hlist);
			for(int i=0; i<n; i++)
			{
				wchar_t key[100]=L"";
				LVITEMW lvi={LVIF_TEXT,i,0};
				lvi.pszText=key;
				lvi.cchTextMax=100;
				SendMessage(hlist,LVM_GETITEMW,0,(LPARAM)&lvi);
				if(!_wcsicmp(key,tag))
				{
					lvi.iSubItem = 1;
					lvi.pszText = value;
					SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
					if(!*value)
						ListView_DeleteItem(hlist,i);
					else if(ListView_GetItemState(hlist,i,LVIS_SELECTED))
						SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
					return 0;
				}
			}
			// bew hew, not found
			LVITEMW lvi={0,0x7FFFFFF0,0};
			n = (int)SendMessage(hlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
			lvi.mask = LVIF_TEXT;
			lvi.iItem = n;
			lvi.iSubItem = 0;
			lvi.pszText = tag;
			SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
			lvi.iSubItem = 1;
			lvi.pszText = value;
			SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
		}
		break;
	case WM_NOTIFY:
    {
			LPNMHDR l=(LPNMHDR)lParam;
			if(l->idFrom==IDC_LIST && l->code == LVN_KEYDOWN) {
				if((((LPNMLVKEYDOWN)l)->wVKey) == VK_DELETE){
				int selitem = ListView_GetNextItem(l->hwndFrom,-1,LVNI_SELECTED|LVNI_FOCUSED);
					if(selitem != -1)
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_BUTTON_DEL,BN_CLICKED),(LPARAM)GetDlgItem(hwndDlg,IDC_BUTTON_DEL));
				}
			}
			else if(l->idFrom==IDC_LIST && l->code == LVN_ITEMCHANGED) {
				LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
				if(lv->uNewState & LVIS_SELECTED) {
					int n = lv->iItem;
					LVITEMW lvi={LVIF_TEXT,lv->iItem,0};
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					lvi.pszText=key;
					lvi.cchTextMax=100;
					SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
					lvi.pszText=value;
					lvi.cchTextMax=1024;
					lvi.iSubItem=1;
					SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
					SetDlgItemTextW(hwndDlg,IDC_NAME,key);
					SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
					sel = n;
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),TRUE);
				}
				if(lv->uOldState & LVIS_SELECTED) {
					sel = -1;
					SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
					SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
				}
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
			case IDOK:
				{
					Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					if (!info->Save())
					{
						MessageBox(hwndDlg,
							L"Cannot save metadata: Error writing file or file is read-only.",
							L"Error saving metadata.",
							MB_OK);
					}
				}
				break;
			case IDC_NAME:
			case IDC_VALUE:
				if(HIWORD(wParam) == EN_CHANGE && sel>=0) {
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					LVITEMW lvi={LVIF_TEXT,sel,0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,key,100);
					GetDlgItemTextW(hwndDlg,IDC_VALUE,value,1024);
					lvi.pszText=key;
					lvi.cchTextMax=100;
					SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
					lvi.pszText=value;
					lvi.cchTextMax=1024;
					lvi.iSubItem=1;
					SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
					VorbisTagToWinampTag(key,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
					ismychange=0;
				}
				else if(HIWORD(wParam) == EN_KILLFOCUS && sel>=0) {
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,key,100);
					GetDlgItemTextW(hwndDlg,IDC_VALUE,value,1024);
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					wchar_t oldkey[100]=L"";
					bool newitem=true;
					if(sel < info->GetNumMetadataItems()) {
						info->EnumMetadata(sel,oldkey,100,0,0);
						newitem=false;
					}

					if(!newitem && wcscmp(oldkey,key)) { // key changed
						info->SetTag(sel,key);
					} else {
						info->SetMetadata(key,value);
					}
					VorbisTagToWinampTag(key,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
					ismychange=0;
				}
				break;
			case IDC_BUTTON_DEL:
				if(sel >= 0) {
					wchar_t tag[100] = {0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,tag,100);
					SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
					SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if(sel < info->GetNumMetadataItems())
						info->RemoveMetadata(sel);
					ListView_DeleteItem(GetDlgItem(hwndDlg,IDC_LIST),sel);
					sel=-1;
					VorbisTagToWinampTag(tag,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)tag,(WPARAM)L"");
					ismychange=0;
				}
				break;
			case IDC_BUTTON_DELALL:
				ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_LIST));
				SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
				SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
				EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
				sel=-1;
				{
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int n = info->GetNumMetadataItems();
					while(n>0) {
						--n;
						wchar_t tag[100] = {0};
						info->EnumMetadata(n,tag,100,0,0);
						VorbisTagToWinampTag(tag,100);
						ismychange=1;
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)tag,(WPARAM)L"");
						ismychange=0;
						info->RemoveMetadata(n);
					}
				}
				break;
			case IDC_BUTTON_ADD:
				{
					HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
					LVITEMW lvi={0,0x7FFFFFF0,0};
					int n = (int)SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
					ListView_SetItemState(hwndlist,n,LVIS_SELECTED,LVIS_SELECTED);
				}
				break;
		}
		break;
	}
	return 0;
}

extern "C"
{
	// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
	// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
	__declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
	{
		if (PathIsURLW(fn))
			return 0;
		return 1;
	}

	// should return a child window of 513x271 pixels (341x164 in msvc dlg units), or return NULL for no tab.
	// Fill in name (a buffer of namelen characters), this is the title of the tab (defaults to "Advanced").
	// filename will be valid for the life of your window. n is the tab number. This function will first be 
	// called with n == 0, then n == 1 and so on until you return NULL (so you can add as many tabs as you like).
	// The window you return will recieve WM_COMMAND, IDOK/IDCANCEL messages when the user clicks OK or Cancel.
	// when the user edits a field which is duplicated in another pane, do a SendMessage(GetParent(hwnd),WM_USER,(WPARAM)L"fieldname",(LPARAM)L"newvalue");
	// this will be broadcast to all panes (including yours) as a WM_USER.
	__declspec(dllexport) HWND winampAddUnifiedFileInfoPane(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen)
	{
		if(n == 0) { // add first pane
			SetPropW(parent,L"INBUILT_NOWRITEINFO", (HANDLE)1);
			Info *info = new Info(filename);
			if(info->Error())
			{
				delete info;
				return NULL;
			}
			return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO,parent,ChildProc_Advanced,(LPARAM)info);
		}
		return NULL;
	}
};