/*
** Copyright (C) 2003 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "../ml.h"
#include "resource.h"
#include "../listview.h"

#include "../childwnd.h"
#include "../../winamp/wa_dlg.h"

#include "../itemlist.h"


// configuration section in winamp.ini
#define CONFIG_SEC "ml_ex"


// columns in our big treeview
#define COL_ARTIST 0
#define COL_TITLE 1
#define COL_ALBUM 2
#define COL_LENGTH 3
#define COL_TRACK 4
#define COL_GENRE 5
#define COL_YEAR 6
#define COL_FILENAME 7

// makes a NULL char * an empty string
#define MAKESAFE(x) ((x)?(x):"")


// yes, we could easily use an itemRecord / itemRecordList here instead of 'Song's, but the point of this example
// is to show how to integrate with some other native structure
typedef struct 
{  
  char *artist;
  char *title;
  char *album;
  int songlen; // seconds?
  int track_nr;
  char *genre;
  int year;
  char *filename;
} Song;


// our leading crap reduction agent for use with sorting/etc
#define SKIP_THE_AND_WHITESPACE(x) { while (!isalnum(*x) && *x) x++; if (!_strnicmp(x,"the ",4)) x+=4; while (*x == ' ') x++; }


extern winampMediaLibraryPlugin plugin;
static int myParam; // param of our tree item
static C_ItemList m_songs, *m_songs_sorted;
static W_ListView m_list;
static HWND m_hwnd;
static HMENU m_context_menus;
static int m_skinlistview_handle;


void config(HWND parent);
void sortResults();

static void deleteSongPtr(Song *song)
{
  free(song->album);
  free(song->artist);
  free(song->title);
  free(song->genre);
  free(song->filename);
  free(song);
}

static void clearSongList()
{
  int i=m_songs.GetSize();
  while (i>0)
  {
    Song *song=(Song *)m_songs.Get(--i);
    deleteSongPtr(song);
    m_songs.Del(i);
  }
}

// this doesnt actually alloc the memory for all the strings, just references them (so it is only temporarily valid at best)
void SongsToItemList(itemRecordList *p, int all)
{
  if (!m_hwnd) all=1;

  p->Alloc=p->Size=0;
  p->Items=0;

  C_ItemList *list=(C_ItemList *)m_songs_sorted;
  if (!list) { list=&m_songs; all=1; }

  int x,l=list->GetSize();
  for (x = 0 ; x < l; x ++)
  {
    if (!all && !m_list.GetSelected(x)) continue;

    allocRecordList(p,p->Size+1,256);
    if (!p->Items) break;

    Song *s=(Song *)list->Get(x);

    memset(&p->Items[p->Size],0,sizeof(itemRecord));
    p->Items[p->Size].album=s->album;
    p->Items[p->Size].artist=s->artist;
    p->Items[p->Size].title=s->title;
    p->Items[p->Size].genre=s->genre;
    p->Items[p->Size].filename=s->filename;
    p->Items[p->Size].track=s->track_nr;
    p->Items[p->Size].year=s->year;
    p->Items[p->Size].length=s->songlen;
    p->Size++;
  }
}


static void playFiles(int enqueue, int all)
{
  if (!m_songs_sorted) return;
  if (!m_hwnd && !all) return;

  itemRecordList obj={0,};
  SongsToItemList(&obj,all);
  if (obj.Size)
  {
    mlSendToWinampStruct s={ML_TYPE_ITEMRECORDLIST,(void*)&obj,!!enqueue};
    SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&s,ML_IPC_SENDTOWINAMP);
  }
  free(obj.Items);
}


void addItemListToSongs(itemRecordList *p)
{
  if (p) for (int x = 0 ; x < p->Size; x ++)
  {
    Song *s=(Song *)calloc(1,sizeof(Song));
    if (p->Items[x].album) s->album=_strdup(p->Items[x].album);
    if (p->Items[x].artist) s->artist=_strdup(p->Items[x].artist);
    if (p->Items[x].title) s->title=_strdup(p->Items[x].title);
    if (p->Items[x].genre) s->genre=_strdup(p->Items[x].genre);
    if (p->Items[x].filename) s->filename=_strdup(p->Items[x].filename);
    s->track_nr=p->Items[x].track;
    s->year=p->Items[x].year;
    s->songlen=p->Items[x].length;
    m_songs.Add((void*)s);
  }
}

char *conf_file;
int init() {
  mlAddTreeItemStruct mla={
    0, // if you used 0, it would put it on top level, or ML_TREEVIEW_ID_DEVICES
    "Item Cache Example",
    1,
  };
  conf_file=(char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILE); // get winamp.ini name :)

  SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&mla,ML_IPC_ADDTREEITEM);
  myParam=mla.this_id;

  m_context_menus=LoadMenu(plugin.hDllInstance,MAKEINTRESOURCE(IDR_CONTEXTMENUS));
  return 0;
}

void quit() 
{
  clearSongList();
}

void loadSongList()
{
  clearSongList();

  // populate m_songs from whatever source we have
  Song *p=(Song *)calloc(sizeof(Song),1);
  p->filename = _strdup("http://www.firehose.net/~deadbeef/media/Misc/music/030223%20-%20pervert-in-a-satellite.mp3");
  p->album=_strdup("SEP");
  p->artist=_strdup("Nullsoft Band");
  p->genre=_strdup("Shit");
  p->songlen = 666;
  p->track_nr=1;
  p->title=_strdup("Pervert  In A Satellite");
  p->year=2003;

  m_songs.Add((void*)p);
}


// this is uberadvancedsearchtechnology[tm]
static void parsequicksearch(char *out, char *in) // parses a list into a list of terms that we are searching for
{
  int inquotes=0, neednull=0;
  while (*in)
  {
    char c=*in++;
    if (c != ' ' && c != '\t' && c != '\"')
    {
      neednull=1;
      *out++=c;
    }
    else if (c == '\"') 
    {
      inquotes=!inquotes;
      if (!inquotes) 
      {
        *out++=0;
        neednull=0;
      }
    }
    else
    {
      if (inquotes) *out++=c;
      else if (neednull)
      {
        *out++=0;
        neednull=0;
      }
    }
  }
  *out++=0;
  *out++=0;
}

static int in_string(char *string, char *substring)
{
  if (!string) return 0;
  if (!*substring) return 1;
  int l=strlen(substring);
  while (string[0]) if (!_strnicmp(string++,substring,l)) return 1; 
  return 0;
}


static void updateList()
{
  if(!m_hwnd) return;

  char filterstr[256],filteritems[300];
  GetDlgItemText(m_hwnd,IDC_QUICKSEARCH,filterstr,sizeof(filterstr)-1);
  parsequicksearch(filteritems,filterstr);

  delete m_songs_sorted;
  m_songs_sorted=new C_ItemList;
  unsigned int totallen=0,filterlen=0,filterval=0;
  for(int i=0;i<m_songs.GetSize();i++)
  {
    Song *s=(Song *)m_songs.Get(i);
    totallen+=s->songlen;
    char year[32]="";
    if (s->year < 5000 && s->year > 0) sprintf(year,"%d",s->year);
    char *p=filteritems;
    if (*p)
    {
      while (*p)
      {
        // search for 'p' in the song
        if (!in_string(s->album,p) && !in_string(s->artist,p) && !in_string(s->title,p) && !in_string(s->genre,p) && !in_string(year,p))
          break;

        p+=strlen(p)+1;
      }
      if (*p) continue;
    }
    filterval++;
    filterlen+=s->songlen;
    m_songs_sorted->Add((void *)s);
  }

  sortResults();

  char tmp[512];
  if (m_songs.GetSize() != m_songs_sorted->GetSize())
    wsprintf(tmp,"Found: %d items [%d:%02d:%02d]",
    m_songs_sorted->GetSize(),filterval,
    filterlen/3600,(filterlen/60)%60,filterlen%60);
  else
    wsprintf(tmp,"%d items [%d:%02d:%02d]",m_songs.GetSize(),totallen/3600,(totallen/60)%60,totallen%60);

  SetDlgItemText(m_hwnd,IDC_STATUS,tmp);
}


static ChildWndResizeItem resize_rlist[]={
  {IDC_QUICKSEARCH,0x0010},
  {IDC_LIST,0x0011},
  {IDC_BUTTON_CONFIG,0x0101},
  {IDC_STATUS,0x0111}
};



int g_sortcol, g_sortdir;
static int STRCMP_NULLOK(const char *pa, const char *pb)
{
  if (!pa) pa="";
  else SKIP_THE_AND_WHITESPACE(pa)

  if (!pb) pb="";
  else SKIP_THE_AND_WHITESPACE(pb)
  
  return _stricmp(pa,pb);
}

static int sortFunc(const void *elem1, const void *elem2)
{
  Song *a=(Song *)*(void **)elem1;
  Song *b=(Song *)*(void **)elem2;

  int use_by=g_sortcol;
  int use_dir=g_sortdir;

#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;

  // this might be too slow, but it'd be nice
  int x;
  for (x = 0; x < 4; x ++)
  {
    if (use_by == COL_YEAR) // year -> artist -> album -> track
    {
      int v1=a->year;
      int v2=b->year;
      if (v1<0)v1=0;
      if (v2<0)v2=0;
      RETIFNZ(v1-v2)
      use_by=COL_ARTIST;     
    }
    else if (use_by == COL_TITLE) // title -> artist -> album -> track
    {
      int v=STRCMP_NULLOK(a->title,b->title);
      RETIFNZ(v)
      use_by=COL_ARTIST;
    }
    else if (use_by == COL_ARTIST) // artist -> album -> track -> title
    {
      int v=STRCMP_NULLOK(a->artist,b->artist);
      RETIFNZ(v)
      use_by=COL_ALBUM;
    }
    else if (use_by == COL_ALBUM) // album -> track -> title -> artist
    {
      int v=STRCMP_NULLOK(a->album,b->album);
      RETIFNZ(v)
      use_dir=0;
      use_by=COL_TRACK;
    }
    else if (use_by == COL_GENRE) // genre -> artist -> album -> track
    {
      int v=STRCMP_NULLOK(a->genre,b->genre);
      RETIFNZ(v)
      use_by=COL_ARTIST;
    }
    else if (use_by == COL_TRACK) // track -> title -> artist -> album
    {
      int v1=a->track_nr;
      int v2=b->track_nr;
      if (v1<0)v1=0;
      if (v2<0)v2=0;
      RETIFNZ(v1-v2)
      use_by=COL_TITLE;     
    }
    else if (use_by == COL_LENGTH) // length -> artist -> album -> track
    {
      int v1=a->songlen;
      int v2=b->songlen;
      if (v1<0)v1=0;
      if (v2<0)v2=0;
      RETIFNZ(v1-v2)
      use_by=COL_ARTIST;
    }
    else break; // no sort order?
  } 
#undef RETIFNZ
  return 0;
}



void sortResults()
{
  if (!m_songs_sorted) return;
  qsort(m_songs_sorted->GetAll(),m_songs_sorted->GetSize(),sizeof(void*),sortFunc);

  ListView_SetItemCount(m_list.getwnd(),0);
  ListView_SetItemCount(m_list.getwnd(),m_songs_sorted->GetSize());
  ListView_RedrawItems(m_list.getwnd(),0,m_songs_sorted->GetSize()-1);
}

int (*wad_getColor)(int idx);
int (*wad_handleDialogMsgs)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
void (*wad_DrawChildWindowBorders)(HWND hwndDlg, int *tab, int tabsize);
void (*cr_init)(HWND hwndDlg, ChildWndResizeItem *list, int num);
void (*cr_resize)(HWND hwndDlg, ChildWndResizeItem *list, int num);


static BOOL CALLBACK dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  if (wad_handleDialogMsgs)
  {
    BOOL a=wad_handleDialogMsgs(hwndDlg,uMsg,wParam,lParam); if (a) return a;
  }
  switch (uMsg)
  {
    case WM_DISPLAYCHANGE:
      ListView_SetTextColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMFG):RGB(0xff,0xff,0xff));
      ListView_SetBkColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));
      ListView_SetTextBkColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));
      m_list.refreshFont();
    return 0;
    case WM_INITDIALOG:
      m_hwnd=hwndDlg;

      *(void **)&wad_getColor=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,1,ML_IPC_SKIN_WADLG_GETFUNC);
      *(void **)&wad_handleDialogMsgs=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,2,ML_IPC_SKIN_WADLG_GETFUNC);
      *(void **)&wad_DrawChildWindowBorders=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,3,ML_IPC_SKIN_WADLG_GETFUNC);
      
      *(void **)&cr_init=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,32,ML_IPC_SKIN_WADLG_GETFUNC);
// woof:      *(void **)&cr_resize=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,33,ML_IPC_SKIN_WADLG_GETFUNC);
      
      if (cr_init) cr_init(hwndDlg,resize_rlist,sizeof(resize_rlist)/sizeof(resize_rlist[0]));
      
      m_list.setLibraryParentWnd(plugin.hwndLibraryParent);
      m_list.setwnd(GetDlgItem(hwndDlg,IDC_LIST));
      m_list.AddCol("Artist",200);
      m_list.AddCol("Title",200);
      m_list.AddCol("Album",200);
      m_list.AddCol("Length",64);
      m_list.AddCol("Track #",64);
      m_list.AddCol("Genre",100);
      m_list.AddCol("Year",64);
      m_list.AddCol("Filename",80);
      ListView_SetTextColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMFG):RGB(0xff,0xff,0xff));
      ListView_SetBkColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));
      ListView_SetTextBkColor(m_list.getwnd(),wad_getColor?wad_getColor(WADLG_ITEMBG):RGB(0x00,0x00,0x00));

      g_sortdir=GetPrivateProfileInt(CONFIG_SEC,"sortdir",0,conf_file);
      g_sortcol=GetPrivateProfileInt(CONFIG_SEC,"sortcol",g_sortcol,conf_file);

      m_skinlistview_handle=SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(int)m_list.getwnd(),ML_IPC_SKIN_LISTVIEW);

      SetTimer(hwndDlg,32,50,NULL);

    return 0;
    case WM_NOTIFY:
    {
      LPNMHDR l=(LPNMHDR)lParam;
      if (l->idFrom==IDC_LIST)
      {
        if (l->code == NM_DBLCLK)
        {
          playFiles(!!(GetAsyncKeyState(VK_SHIFT)&0x8000),0);
        } 
        else if (l->code == LVN_BEGINDRAG)
        {
          SetCapture(hwndDlg);
        }
        else if (l->code == LVN_ODFINDITEM) // yay we find an item (for kb shortcuts)
        {
          NMLVFINDITEM *t = (NMLVFINDITEM *)lParam;
          int i=t->iStart;
          if (i >= m_songs_sorted->GetSize()) i=0;

          int cnt=m_songs_sorted->GetSize()-i;
          if (t->lvfi.flags & LVFI_WRAP) cnt+=i;

          while (cnt-->0)
          {
            Song *thissong = (Song *)m_songs_sorted->Get(i);
            char tmp[128];
            char *name=0;

            switch (g_sortcol)
            {
              case COL_ARTIST: name=thissong->artist; break;
              case COL_TITLE: name=thissong->title; break;
              case COL_ALBUM: name=thissong->album; break;
              case COL_LENGTH: 
                wsprintf(tmp,"%d:%02d",thissong->songlen/60,(thissong->songlen)%60); name=tmp; 
              break;
              case COL_TRACK: if (thissong->track_nr > 0 && thissong->track_nr < 1000) { wsprintf(tmp,"%d",thissong->track_nr); name=tmp; } break;
              case COL_GENRE: name=thissong->genre; break;
              case COL_YEAR:  if (thissong->year < 5000 && thissong->year > 0) { wsprintf(tmp,"%d",thissong->year); name=tmp; } break;
              case COL_FILENAME: name=thissong->filename; break;
            }


            if (!name) name="";
            else SKIP_THE_AND_WHITESPACE(name)

            if (t->lvfi.flags & (4|LVFI_PARTIAL))
            {
              if (!_strnicmp(name,t->lvfi.psz,strlen(t->lvfi.psz)))
              {
                SetWindowLong(hwndDlg,DWL_MSGRESULT,i);
                return 1;
              }
            }
            else if (t->lvfi.flags & LVFI_STRING)
            {
              if (!_stricmp(name,t->lvfi.psz))
              {
                SetWindowLong(hwndDlg,DWL_MSGRESULT,i);
                return 1;
              }
            }
            else 
            {
              SetWindowLong(hwndDlg,DWL_MSGRESULT,-1);
              return 1;
            }
            if (++i == m_songs_sorted->GetSize()) i=0;
          }
          SetWindowLong(hwndDlg,DWL_MSGRESULT,-1);
          return 1;
        }
        else if (l->code == LVN_GETDISPINFO)
        {
          NMLVDISPINFO *lpdi = (NMLVDISPINFO*) lParam;
          int item=lpdi->item.iItem;

          if (item < 0 || item >= m_songs_sorted->GetSize()) return 0;

          Song *thissong = (Song *)m_songs_sorted->Get(item);

          if (lpdi->item.mask & (LVIF_TEXT|/*LVIF_IMAGE*/0)) // we can always do images too :)
          {
            if (lpdi->item.mask & LVIF_TEXT)
            {
              char tmpbuf[128];
              char *nameptr=0;
              switch (lpdi->item.iSubItem)
              {
                case COL_ARTIST: nameptr=thissong->artist; break;
                case COL_TITLE: nameptr=thissong->title; break;
                case COL_ALBUM: nameptr=thissong->album; break;
                case COL_LENGTH: 
                  wsprintf(tmpbuf,"%d:%02d",thissong->songlen/60,(thissong->songlen)%60); nameptr=tmpbuf; 
                break;
                case COL_TRACK: if (thissong->track_nr > 0 && thissong->track_nr < 1000) { wsprintf(tmpbuf,"%d",thissong->track_nr); nameptr=tmpbuf; } break;
                case COL_GENRE: nameptr=thissong->genre; break;
                case COL_YEAR: if (thissong->year>0 && thissong->year<5000) { wsprintf(tmpbuf,"%d",thissong->year); nameptr=tmpbuf; } break;
                case COL_FILENAME: nameptr=thissong->filename; break;
              }
              if (nameptr) lstrcpyn(lpdi->item.pszText,nameptr,lpdi->item.cchTextMax);
              else lpdi->item.pszText[0]=0;
            }
           // if(lpdi->item.mask & LVIF_IMAGE)
          } // bother
          return 0;
        } // LVN_GETDISPINFO
        else if (l->code == LVN_COLUMNCLICK)
        {
          NMLISTVIEW *p=(NMLISTVIEW*)lParam;
          if (p->iSubItem == g_sortcol) g_sortdir=!g_sortdir;
          else g_sortcol=p->iSubItem;          

          char str[32];
          sprintf(str,"%d",g_sortdir);
          WritePrivateProfileString(CONFIG_SEC,"sortdir",str,conf_file);
          sprintf(str,"%d",g_sortcol);
          WritePrivateProfileString(CONFIG_SEC,"sortcol",str,conf_file);

          sortResults();
        }      
      }
    }
    break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) 
      {
        case IDC_BUTTON_CONFIG:
          config(hwndDlg);
        break;
        case IDC_QUICKSEARCH:
          if (HIWORD(wParam) == EN_CHANGE)
          {
            KillTimer(hwndDlg,500);
            SetTimer(hwndDlg,500,150,NULL);
          }
        break;
      }
    break;
    case WM_TIMER:
      if (wParam == 500)
      {
        KillTimer(hwndDlg,500);
        char buf[256];
        GetDlgItemText(hwndDlg,IDC_QUICKSEARCH,buf,sizeof(buf));
        buf[255]=0;
        WritePrivateProfileString(CONFIG_SEC,"lastfilter",buf,conf_file);
        updateList();
      }
      else if (wParam == 32)
      {
        KillTimer(hwndDlg,32);
        if (!m_songs.GetSize()) loadSongList();
        char buf[256];
        GetPrivateProfileString(CONFIG_SEC,"lastfilter","",buf,sizeof(buf),conf_file);
        SetDlgItemText(hwndDlg,IDC_QUICKSEARCH,buf); // automatically updates the list via EN_CHANGE
      }
    break;
  case WM_SIZE:
	  #if 0	// BP:
      if (wParam != SIZE_MINIMIZED) 
      {
        if (cr_resize) cr_resize(hwndDlg,resize_rlist,sizeof(resize_rlist)/sizeof(resize_rlist[0]));
      }
	  #endif
    break;
    case WM_PAINT:
      {
        if (wad_DrawChildWindowBorders)
        {  
          int tab[] = { IDC_QUICKSEARCH|DCW_SUNKENBORDER, IDC_LIST|DCW_SUNKENBORDER};
          wad_DrawChildWindowBorders(hwndDlg,tab,2);
        }
      }
    return 0;
    case WM_DESTROY:
      //clearSongList();
      m_hwnd=NULL;
      SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,m_skinlistview_handle,ML_IPC_UNSKIN_LISTVIEW);
    return 0;
    case WM_ML_CHILDIPC:
      if (lParam == ML_CHILDIPC_DROPITEM && wParam)
      {
        mlDropItemStruct *t=(mlDropItemStruct*)wParam;
        if (t->type == ML_TYPE_ITEMRECORDLIST) t->result=1;
        if (t->data)
        {
          if (t->type == ML_TYPE_ITEMRECORDLIST)  // we got a drag&drop to our window, hot!
          {
            addItemListToSongs((itemRecordList*)t->data);
            updateList();
          }
        }
      }
    return 0;
    case WM_LBUTTONUP:
      if (GetCapture() == hwndDlg)
      {
        ReleaseCapture();

        POINT p;
        p.x=GET_X_LPARAM(lParam);
        p.y=GET_Y_LPARAM(lParam);
        ClientToScreen(hwndDlg,&p);

        HWND h=WindowFromPoint(p);
        if (h != hwndDlg && !IsChild(hwndDlg,h))
        {        
          mlDropItemStruct m={ML_TYPE_ITEMRECORDLIST,NULL,0};
          m.p=p;
          m.flags=ML_HANDLEDRAG_FLAG_NOCURSOR;

          SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);

          if (m.result>0)
          {
            itemRecordList o={0,}; 
            SongsToItemList(&o,0);
            if (o.Size)
            {
              //fill in this itemCacheObject if you want to provide drag&drop out of the window
              m.flags=0;
              m.result=0;
              m.data=(void*)&o;
              SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDROP);
            }
            free(o.Items);
          }
        }           
      }
    break;
    case WM_MOUSEMOVE:
      if (GetCapture()==hwndDlg)
      {
        POINT p;
        p.x=GET_X_LPARAM(lParam);
        p.y=GET_Y_LPARAM(lParam);
        ClientToScreen(hwndDlg,&p);
        mlDropItemStruct m={ML_TYPE_ITEMRECORDLIST,NULL,0};
        m.p=p;
        HWND h=WindowFromPoint(p);
        if (!h || h == hwndDlg || IsChild(hwndDlg,h))
        {
          SetCursor(LoadCursor(NULL,IDC_NO));
        }
        else 
          SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);
      }
    break;

  }
  return 0;
}

static BOOL CALLBACK config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          // save combo box
        case IDCANCEL:
          EndDialog(hwndDlg,LOWORD(wParam) == IDOK);
        break;
      }
    return 0;
  }
  return 0;
}

static void config(HWND parent)
{
  DialogBox(plugin.hDllInstance,MAKEINTRESOURCE(IDD_CONFIG),parent,config_dlgproc);
}

int onTreeItemClick(int param, int action, HWND hwndParent) // if param is not yours, return 0
{
  if (action == ML_ACTION_RCLICK) 
  {
    POINT p;
    GetCursorPos(&p);
    int r=TrackPopupMenu(GetSubMenu(m_context_menus,0),TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,p.x,p.y,0,hwndParent,NULL);
    switch (r)
    {
      case ID_ABOUT:
        MessageBox(hwndParent,"ml_ex!!!","About ml_ex!!!",MB_OK);
      break;
      case ID_CONFIG:
        config(hwndParent);
      break;
    }
  }
  return 1;
}


int onTreeItemDropTarget(int param, int type, void *obj)
{
  if (type != ML_TYPE_ITEMRECORDLIST) return -1;

  if (!obj) return 1;

  // do somethihng with the itemCache object. do not free it however, since the caller owns it
  addItemListToSongs((itemRecordList*)obj);
  
  updateList();

  return 1;
}

int onTreeItemDrag(int param, POINT p, int *type)
{
  HWND h=WindowFromPoint(p);
  if (h && (h == m_hwnd || IsChild(m_hwnd,h))) return -1; // prevent from dropping into ourselves


  // if we wanted to be able to drag&drop our tree item to other people, we'd
  // return 1 and set type to ML_TYPE_ITEMRECORDLIST or ML_TYPE_FILENAMES etc.

  // *type = ML_TYPE_ITEMRECORDLIST;
  return -1;
}

int onTreeItemDrop(int param, POINT p) // you should send the appropriate ML_IPC_HANDLEDROP if you support it
{
  HWND h=WindowFromPoint(p);
  if (h && (h == m_hwnd || IsChild(m_hwnd,h))) return -1; // prevent from dropping into ourselves

  // if we wanted to be able to drag&drop our tree item to other people, we'd
  // create an itemCacheObject or a doublenull terminated list (depending on what we want),
  // and send it back to the media library so it can route it to the appropriate destination:
  //
  // itemCacheObject o={0,};
  // fillInMyObject(&o);
  // mlDropItemStruct m={0,};
  // m.type = ML_TYPE_ITEMRECORDLIST;
  // m.data = (void*)&o;
  // m.p=p;
  // SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDROP);
  // freeMyObject(&o);

  // or this:
  itemRecordList o={0,};
  SongsToItemList(&o,1);
  if (o.Size)
  {
    mlDropItemStruct m={0,};
    m.type = ML_TYPE_ITEMRECORDLIST;
    m.data = (void*)&o;
    m.p=p;
    SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDROP);
  }
  free(o.Items);

  return 1;
}

int PluginMessageProc(int message_type, int param1, int param2, int param3)
{
  // check for any global messages here

  if (message_type >= ML_MSG_TREE_BEGIN && message_type <= ML_MSG_TREE_END)
  {
    if (param1 != myParam) return 0;
    // local messages for a tree item

    switch (message_type)
    {
      case ML_MSG_TREE_ONCREATEVIEW:
        return (int)CreateDialog(plugin.hDllInstance,MAKEINTRESOURCE(IDD_VIEW_EX),(HWND)param2,dlgproc);
      case ML_MSG_TREE_ONCLICK:
        return onTreeItemClick(param1,param2,(HWND)param3);
      case ML_MSG_TREE_ONDROPTARGET:
        return onTreeItemDropTarget(param1,param2,(void*)param3);
      case ML_MSG_TREE_ONDRAG:
        return onTreeItemDrag(param1,*(POINT*)param2,(int*)param3);
      case ML_MSG_TREE_ONDROP:
        return onTreeItemDrop(param1,*(POINT*)param2);
    }
  }
  else if (message_type == ML_MSG_ONSENDTOBUILD)
  {
    if (param1 == ML_TYPE_ITEMRECORDLIST)
    {
      mlAddToSendToStruct s;
      s.context=param2;
      s.desc="ItemCacheEx";
      s.user32=(int)PluginMessageProc;
      SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&s,ML_IPC_ADDTOSENDTO);
    }
  }
  else if (message_type == ML_MSG_ONSENDTOSELECT)
  {
    if (param1 == ML_TYPE_ITEMRECORDLIST && param2 && param3 == (int)PluginMessageProc)
    {
      addItemListToSongs((itemRecordList*)param2);
      updateList();
      return 1;
    }
  }

  return 0;
}

winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"ml_ex v0.1",
	init,
	quit,
  PluginMessageProc,
};

extern "C" {

__declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin()
{
	return &plugin;
}

};