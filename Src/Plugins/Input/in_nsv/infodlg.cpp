#include <windows.h>
#include "api.h"
#include "resource.h"

#include "../Winamp/in2.h"

extern In_Module mod;			// the output module (filled in near the bottom of this file)

extern "C" { extern wchar_t lastextfn[1024]; };

#define MAX_EDITABLE_METASTRING 8192

#include "../nsv/nsvlib.h"
      static int isplaying;
      static int timems;
extern char lastfn[1024];
extern int g_play_needseek;

  static void restartPlayback()
  {
    if (isplaying)
    {
      SendMessage(mod.hMainWindow,WM_USER,0,3007); // disable stats updating
      SendMessage(mod.hMainWindow,WM_COMMAND,40045,0);
      if (timems)
      {
        g_play_needseek=timems;
//        SendMessage(mod.hMainWindow,WM_USER,timems,106);
      }
      if (isplaying & 2)
      {
        SendMessage(mod.hMainWindow,WM_COMMAND,40046,0);
      }
      SendMessage(mod.hMainWindow,WM_USER,1,3007); // enable stats updating
    }
  }

  static void stopPlayback(const char *fn)
  {
    isplaying=0;
    timems=0;
    if (!_stricmp(lastfn,fn))
    {
      isplaying= (int)SendMessage(mod.hMainWindow,WM_USER,0,104);
      if (isplaying)
      {
        timems= (int)SendMessage(mod.hMainWindow,WM_USER,0,105);
        SendMessage(mod.hMainWindow,WM_COMMAND,40047,0);
      }
    }
  }


extern int config_padtag;
int fillBs(HANDLE hFile, nsv_InBS &bs, int lenbytes);
  static nsv_fileHeader m_set_lhdr;
  static char m_set_lfile[1024];

  static void closeset()
  {
    free(m_set_lhdr.toc);
    free(m_set_lhdr.metadata);
    memset(&m_set_lhdr,0,sizeof(m_set_lhdr));
    m_set_lfile[0]=0;
  }

extern int config_bufms, config_prebufms, config_underunbuf;

extern "C" 
{

	__declspec( dllexport ) int winampSetExtendedFileInfo(const char *fn, const char *data, char *val)
	{
    if(!fn || !fn[0]) return 0;

    //muahaha, <3 hacks
    if(!_stricmp(data,"setHttpConfigValues"))
    {
      config_bufms=atoi(val);
      char *p=strstr(val,",");
      if(!p) return 0;
      config_prebufms=atoi(p+1);
      p=strstr(p+1,",");
      if(!p) return 0;
      config_underunbuf=atoi(p+1);
      return 1;
    }
    
    if(strcmpi(fn,m_set_lfile)) 
    {
      closeset();

      lstrcpynA(m_set_lfile,fn,sizeof(m_set_lfile));

      HANDLE hFile = CreateFileA(fn,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
      if (hFile != INVALID_HANDLE_VALUE) 
      {
        nsv_InBS bs;
        for (;;)
        {
          int ret=nsv_readheader(bs,&m_set_lhdr);
          if (ret <= 0 || fillBs(hFile,bs,ret)) break;
        }
        CloseHandle(hFile);
      }
    }

    char *p=(char*)m_set_lhdr.metadata;
    unsigned int pos=0;

    int omdl=m_set_lhdr.metadata_len;
    if (p) while (pos < m_set_lhdr.metadata_len)
    {
      // scan for =
      while (pos < m_set_lhdr.metadata_len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) { pos++; p++; }
      if (pos >= m_set_lhdr.metadata_len) break;

      char *lp=p;
      while (pos < m_set_lhdr.metadata_len && *p != '=') { pos++; p++; }
      if (pos >= m_set_lhdr.metadata_len) break;

      // skip =
      p++; if (++pos >= m_set_lhdr.metadata_len) break;

      // get delim char
      char divc=*p++; if (++pos >= m_set_lhdr.metadata_len) break;

      // scan for new delim char
      while (pos < m_set_lhdr.metadata_len && *p != divc) { pos++; p++; }

      p++; // advance over our delim char
      if (++pos > m_set_lhdr.metadata_len) break;
    

      if (!strncmp(lp,data,strlen(data)) && lp[strlen(data)]=='=') 
      {
        if (pos >= m_set_lhdr.metadata_len)
        {
          m_set_lhdr.metadata_len = (unsigned int)(lp - (char*)m_set_lhdr.metadata);
        }
        else
        {
          memcpy(lp,p,m_set_lhdr.metadata_len - (p-(char*)m_set_lhdr.metadata));
          m_set_lhdr.metadata_len -= (unsigned int)(p-lp);
        }
        break;
      }
    }

    if (val && *val)
    {
      unsigned char divc; //fucko
      int x;
      for (x = 1; x < 256 && strchr(val,x); x ++);
      if (x == 256) return 1;
      divc=(unsigned char)x;

      int nmdl= (int)(m_set_lhdr.metadata_len + 5 + strlen(data) + strlen(val));

      if (!m_set_lhdr.metadata || omdl<nmdl)
        m_set_lhdr.metadata=realloc(m_set_lhdr.metadata,nmdl);
      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len,data,strlen(data));
      m_set_lhdr.metadata_len+= (int)strlen(data);
      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len,"=",1);
      m_set_lhdr.metadata_len++;
      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len,&divc,1);
      m_set_lhdr.metadata_len++;

      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len,val,strlen(val));
      m_set_lhdr.metadata_len+= (int)strlen(val);

      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len,&divc,1);
      m_set_lhdr.metadata_len++;

      memcpy((char*)m_set_lhdr.metadata + m_set_lhdr.metadata_len," ",1); //space to be ghey
      m_set_lhdr.metadata_len++;
    }

    return 1;
  }    


	__declspec( dllexport ) int winampWriteExtendedFileInfo()
	{
    lastextfn[0]=0; // flush cache

    if (m_set_lfile[0])
    {



      HANDLE hFile = CreateFileA(m_set_lfile,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
      if (hFile == INVALID_HANDLE_VALUE)
      {
//        MessageBox(mod.hMainWindow,"Error opening NSV file for update","NSV Tag Error",MB_OK);
        return 0;
      }

      nsv_OutBS bs;
      int osize=m_set_lhdr.header_size;
      nsv_writeheader(bs,&m_set_lhdr,osize);

      int hdrlen;
      char *hdr=(char*)bs.get(&hdrlen);

      if (hdr && hdrlen == (int)osize) // fast update of header
      {
        DWORD dw = 0;
        SetFilePointer(hFile,0,NULL,SEEK_SET);
        WriteFile(hFile,hdr,hdrlen,&dw,NULL);
        CloseHandle(hFile);
      }
      else
      {
        if (hdr && config_padtag>0) // enlarge header by config_padtag bytes =)
        {
          bs.clear();
          nsv_writeheader(bs,&m_set_lhdr,config_padtag+m_set_lhdr.header_size);
          hdr=(char*)bs.get(&hdrlen); // update
        }

        char tmpfn[1024+8];
        char tmpfn2[1024+8];
        wsprintfA(tmpfn,"%s.new",m_set_lfile);
        wsprintfA(tmpfn2,"%s.old",m_set_lfile);

        HANDLE hTempFile=CreateFileA(tmpfn,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
        {
  //        MessageBox(mod.hMainWindow,"Can't create tempfile - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
          CloseHandle(hFile);
          return 0;
        }
        SetFilePointer(hFile,osize,NULL,SEEK_SET);
        if (hdrlen)
        {
          DWORD dw = 0;
          if (!WriteFile(hTempFile,hdr,hdrlen,&dw,NULL) || (int)dw != hdrlen)
          {
            CloseHandle(hTempFile);
            CloseHandle(hFile);
            DeleteFileA(tmpfn);
          //  MessageBox(mod.hMainWindow,"Error copying source - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
            return 0;
          }
        }

        for (;;)
        {
		  char buf[8192] = {0};
          DWORD dw = 0;
          BOOL r1=ReadFile(hFile,buf,sizeof(buf),&dw,NULL);
          if (r1 && !dw) break;
          DWORD dwout = 0;
          if (!r1 || !WriteFile(hTempFile,buf,dw,&dwout,NULL) || dwout < dw)
          {
            CloseHandle(hTempFile);
            DeleteFileA(tmpfn);
        //    MessageBox(mod.hMainWindow,"Error copying source - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
            return 0;
          }
        }
        if (GetFileSize(hFile,NULL)-osize != GetFileSize(hTempFile,NULL)-hdrlen)
        {
          CloseHandle(hTempFile);
          CloseHandle(hFile);
          DeleteFileA(tmpfn);
        //  MessageBox(mod.hMainWindow,"Error size mismatch - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
          return 0;
        }
        CloseHandle(hFile);
        CloseHandle(hTempFile);

        stopPlayback(m_set_lfile);

        if (!MoveFileA(m_set_lfile,tmpfn2))
        {
          DeleteFileA(tmpfn);
          restartPlayback();
        //  MessageBox(mod.hMainWindow,"Error renaming source - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
          return 0;
        }
        if (!MoveFileA(tmpfn,m_set_lfile))
        {
          MoveFileA(tmpfn2,m_set_lfile);
          DeleteFileA(tmpfn);
          restartPlayback();
        //  MessageBox(mod.hMainWindow,"Error renaming new file - can't update tag!","NSV Tag Editor Error",MB_ICONSTOP|MB_OK);
          return 0;
        }
        DeleteFileA(tmpfn2);
        restartPlayback();
      }
    }
    closeset();
    lastextfn[0]=0;
    return 1;
  }
}



const char *g_lastfile;
HANDLE g_hFile=INVALID_HANDLE_VALUE;
nsv_fileHeader g_filehdr={0,~0,~0,};
unsigned int g_oldtag_size;
unsigned int *g_toc_save, *g_toc_save_ex, g_toc_savesize;

void enableControls(HWND hwndDlg, int en)
{
  EnableWindow(GetDlgItem(hwndDlg,IDC_SETTOCSIZE),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_TOC),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_ANALYZE),en);
  EnableWindow(GetDlgItem(hwndDlg,IDOK),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_REMTAG),en);  
  EnableWindow(GetDlgItem(hwndDlg,IDC_ADD),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_REM),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT),en);
  EnableWindow(GetDlgItem(hwndDlg,IDC_FASTUPD),en);  
}

void closeNsv(HWND hwndDlg)
{
  if (g_hFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(g_hFile);
    g_hFile=INVALID_HANDLE_VALUE;
  }
  free(g_filehdr.metadata);
  g_filehdr.metadata=0;
  g_filehdr.metadata_len=0;
  free(g_filehdr.toc);
  g_filehdr.toc=0;
  g_filehdr.toc_ex=0;
  free(g_toc_save);
  g_toc_save=0;
  g_toc_save_ex=0;

  g_toc_savesize=0;
  g_filehdr.toc_size=0;
  g_filehdr.toc_alloc=0;
  g_filehdr.file_lenbytes=~0;
  g_filehdr.file_lenms=~0;
  g_filehdr.header_size=0;
  SetDlgItemTextA(hwndDlg,IDC_LENBYTES,"");
  SetDlgItemTextA(hwndDlg,IDC_LENMS,"");
  SetDlgItemTextA(hwndDlg,IDC_AVGBITRATE,"");
  CheckDlgButton(hwndDlg,IDC_TOC,0);
  SetDlgItemTextA(hwndDlg,IDC_TOCSIZE,"");
  SetDlgItemTextA(hwndDlg,IDC_SETTOCSIZE,"");
  SetDlgItemTextA(hwndDlg,IDC_FN,"");
  int x;
  int cnt= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETCOUNT,0,0);
  for ( x= 0; x < cnt; x ++)
  {
    void *v=(void *)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,x,0);
    if (v) free(v);
    SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_SETITEMDATA,x,0);
  }
  SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_RESETCONTENT,0,0);
  SetDlgItemTextA(hwndDlg,IDC_TAG_LEN,WASABI_API_LNGSTRING(IDS_TOTAL_TAG_SIZE));
  SetDlgItemTextA(hwndDlg,IDC_METADATA_LEN,WASABI_API_LNGSTRING(IDS_NO_METADATA));

  enableControls(hwndDlg,0);
}

int fillBs(HANDLE hFile, nsv_InBS &bs, int lenbytes)
{
  while (lenbytes > 0)
  {
    DWORD r=0;
    char buf[8192] = {0};
    BOOL ret=ReadFile(hFile,buf,sizeof(buf),&r,NULL);
    lenbytes-=r;
    bs.add(buf,r);
    if (!ret || !r) return 1;
  }
  return lenbytes > 0;
}

void makeNewHeaderInfo(HWND hwndDlg)
{
  nsv_OutBS bs;
  g_filehdr.toc_alloc=0;
  nsv_writeheader(bs,&g_filehdr,0);
  EnableWindow(GetDlgItem(hwndDlg,IDC_FASTUPD),g_filehdr.header_size <= g_oldtag_size);
  char buf[128] = {0};
  if (g_filehdr.header_size) wsprintfA(buf,WASABI_API_LNGSTRING(IDS_TOTAL_TAG_SIZE_X_BYTES),g_filehdr.header_size);
  else WASABI_API_LNGSTRING_BUF(IDS_NO_TAG,buf,128);
  SetDlgItemTextA(hwndDlg,IDC_TAG_LEN,buf);
}

void populateInfo(HWND hwndDlg)
{
  if (g_filehdr.file_lenbytes != ~0) 
  {
	wchar_t buf[128] = {0};
    unsigned int low=g_filehdr.file_lenbytes;
    SetDlgItemTextW(hwndDlg,IDC_LENBYTES,WASABI_API_LNG->FormattedSizeString(buf, 128, low));
  }
  else SetDlgItemTextA(hwndDlg,IDC_LENBYTES,"?");

  if (g_filehdr.file_lenms != ~0) 
  {
    char buf[128] = {0};
    unsigned int timems=g_filehdr.file_lenms;
    if (timems < 1000) wsprintfA(buf,"%ums",timems);
    else if (timems < 1000*60) wsprintfA(buf,"%02u.%03us",timems/1000,timems%1000);
    else if (timems < 1000*60*60) wsprintfA(buf,"%02u:%02u.%03us",timems/60000,(timems/1000)%60,timems%1000);
    else wsprintfA(buf,"%u:%02u:%02u.%03us",timems/3600000,(timems/60000)%60,(timems/1000)%60,timems%1000);

    SetDlgItemTextA(hwndDlg,IDC_LENMS,buf);
  }
  else SetDlgItemTextA(hwndDlg,IDC_LENMS,"?");
  CheckDlgButton(hwndDlg,IDC_TOC,g_filehdr.toc_size?BST_CHECKED:BST_UNCHECKED);
  if (g_filehdr.toc_size) 
  {
    char buf[128] = {0};
    wsprintfA(buf,"%d%s",g_filehdr.toc_size,g_filehdr.toc_ex ? " (TOC 2.0)":"");
    SetDlgItemTextA(hwndDlg,IDC_TOCSIZE,buf);
    SetDlgItemInt(hwndDlg,IDC_SETTOCSIZE,g_filehdr.toc_size,FALSE);
  }
  else 
  {
    SetDlgItemTextA(hwndDlg,IDC_TOCSIZE,"");
    SetDlgItemTextA(hwndDlg,IDC_SETTOCSIZE,"4096");
  }

  if (g_filehdr.file_lenms != ~0 && g_filehdr.file_lenbytes != ~0)
  {
    unsigned int bitrate = g_filehdr.file_lenms ? MulDiv(g_filehdr.file_lenbytes,8000,g_filehdr.file_lenms) : 0;
    char buf[1024] = {0};
    wsprintfA(buf,"%u %s",bitrate/1000,WASABI_API_LNGSTRING(IDS_KBPS));
    SetDlgItemTextA(hwndDlg,IDC_AVGBITRATE,buf);
  }
  else SetDlgItemTextA(hwndDlg,IDC_AVGBITRATE,"?");

  char buf[128] = {0};
  if (g_filehdr.header_size) wsprintfA(buf,WASABI_API_LNGSTRING(IDS_TOTAL_TAG_SIZE_X_BYTES),g_filehdr.header_size);
  else WASABI_API_LNGSTRING_BUF(IDS_NO_TAG,buf,128);
  SetDlgItemTextA(hwndDlg,IDC_TAG_LEN,buf);
}

void updateMetaData(HWND hwndDlg)
{
  free(g_filehdr.metadata);
  g_filehdr.metadata=0;
  g_filehdr.metadata_len=0;
  int n= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETCOUNT,0,0);
  int x;
  int total_size=0;

  for (x = 0; x < n; x ++)
  {
    int l;
    char *bigstr = (char *)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,x,0);
    if (bigstr) l = (int)strlen((char *)bigstr)+2;
    else l= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETTEXTLEN,x,0);
    total_size+=l+2; // text+ two chars
    if (x) total_size++; // space
  }

  g_filehdr.metadata=malloc(total_size+1);
  char *metaout=(char*)g_filehdr.metadata;
  for (x = 0; x < n; x ++)
  {
    if (x) *metaout++=' ';

    const char *bigstr = (const char *)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,x,0);

    char *this_text;
    if (bigstr) this_text=_strdup((const char *)bigstr);
    else
    {
      int l= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETTEXTLEN,x,0);
      this_text=(char*)malloc(l+1);
      SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETTEXT,x,(LPARAM)this_text);
    }
    char *p=this_text;
    while (p && *p != '=' && *p) *metaout++=*p++;
    if (p && *p)
    {
      *metaout++=*p++;
      int x;
      for (x = 1; x < 256 && strchr(p,x); x ++);

      if (x == 256)
      {
		char title[64] = {0};
        MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_WRITING_STRING_TO_TAG),
				   WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				   MB_OK|MB_ICONSTOP);
        x=1;
      }

      *metaout++=x;
      while (p && *p)
      {
        int a=*p++;
        if (a == x) a = '!';
        *metaout++=a;
      }
      *metaout++=x;
    }
    free(this_text);
  }
  g_filehdr.metadata_len= unsigned int(metaout - (char*)g_filehdr.metadata);
  *metaout=0;
}

void populateMetaData(HWND hwndDlg)
{
  wchar_t buf[128] = {0};
  if (g_filehdr.metadata_len) WASABI_API_LNG->FormattedSizeString(buf,128,g_filehdr.metadata_len);
  else WASABI_API_LNGSTRINGW_BUF(IDS_NO_METADATA, buf, 128);
  SetDlgItemTextW(hwndDlg,IDC_METADATA_LEN,buf);

  int x;
  int cnt= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETCOUNT,0,0);
  for ( x= 0; x < cnt; x ++)
  {
    void *v=(void *)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,x,0);
    if (v) free(v);
    SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_SETITEMDATA,x,0);
  }
  SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_RESETCONTENT,0,0);

  if (g_filehdr.metadata)
  {
    char *p=(char*)g_filehdr.metadata;
    for (;;)
    {
      while (p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
      if (!p || !*p) break;

      char *this_name=p;

      // advance to next item
      while (p && *p && *p != '=') p++;
      char *end_name=p;

      if (!*p++) break;
      if (!*p) break;
     
      char c=*p++;
      char *begin_value=p;
      while (p && *p && *p != c) p++;
      char *end_value=p;
      if (*p) p++;

      char *name=(char *)malloc(end_name-this_name + 1 + end_value-begin_value + 1);
      char *tmp=name;
      memcpy(tmp,this_name,end_name-this_name); tmp+=end_name-this_name;
      *tmp++='=';
      memcpy(tmp,begin_value,end_value-begin_value); tmp+=end_value-begin_value;
      *tmp=0;

      if (strlen(name) < MAX_EDITABLE_METASTRING)
      {
        LRESULT a=SendDlgItemMessageA(hwndDlg,IDC_METADATA,LB_ADDSTRING,0,(LPARAM)name);
        SendDlgItemMessageA(hwndDlg,IDC_METADATA,LB_SETITEMDATA,(WPARAM)a,(LPARAM)0);
        free(name);
      }
      else
      {
        char buf[512] = {0};
		WASABI_API_LNGSTRING_BUF(IDS_LARGE_DATA,buf,512);
        lstrcpynA(buf+strlen(buf),name,128);
        strcpy(buf+strlen(buf),"...");
        LRESULT a=SendDlgItemMessageA(hwndDlg,IDC_METADATA,LB_ADDSTRING,0,(LPARAM)buf);
        SendDlgItemMessageA(hwndDlg,IDC_METADATA,LB_SETITEMDATA,(WPARAM)a,(LPARAM)name);
      }
    }
  }
}

void openNsv(HWND hwndDlg)
{
  closeNsv(hwndDlg);
  g_hFile = CreateFileA(g_lastfile,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  SetDlgItemTextA(hwndDlg,IDC_FN,g_lastfile);

  if (g_hFile == INVALID_HANDLE_VALUE) return;
  enableControls(hwndDlg,1);
  CheckDlgButton(hwndDlg,IDC_FASTUPD,BST_CHECKED);
  // try to read existing tag.
  nsv_InBS bs;
  for (;;)
  {
    int ret=nsv_readheader(bs,&g_filehdr);
    if (ret <= 0 || fillBs(g_hFile,bs,ret)) break;
  }
  nsv_Unpacketer unpacket;
  char infobuf[256] = {0};
  WASABI_API_LNGSTRING_BUF(IDS_NO_VALID_NSV_BITSTREAM_FOUND,infobuf,256);
  for (;;)
  {
    int ret=unpacket.unpacket(bs);
    if (ret < 0) break;
    if (ret > 0 && fillBs(g_hFile,bs,ret)) break;
    if (!ret)
    {
      char vfmt[32] = {0};
      char afmt[5] = {0};

      int fr=(int)(unpacket.getFrameRate()*100.0);

      if (unpacket.getVidFmt()!=NSV_MAKETYPE('V','L','B',' '))  nsv_type_to_string(unpacket.getVidFmt(),vfmt);
      else strcpy(vfmt,"Dolby AAC");

      nsv_type_to_string(unpacket.getAudFmt(),afmt);

      wsprintfA(infobuf,WASABI_API_LNGSTRING(IDS_VIDEO_X_AUDIO_X),
			   vfmt,unpacket.getWidth(),
			   unpacket.getHeight(),
			   fr/100,fr%100,afmt);
      break;
    }
  }
  SetDlgItemTextA(hwndDlg,IDC_BSINFO,infobuf);

  g_oldtag_size=g_filehdr.header_size;
  populateInfo(hwndDlg);
  populateMetaData(hwndDlg);
}

static int g_metaitem_edit;

INT_PTR CALLBACK EditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
  if (uMsg == WM_INITDIALOG)
  {
    if (g_metaitem_edit>=0)
    {
      HWND ctl=GetDlgItem(GetParent(hwndDlg),IDC_METADATA);
      int x= (int)SendMessage(ctl,LB_GETTEXTLEN,g_metaitem_edit,0);
      if (x != LB_ERR)
      {
        char *t=(char*)malloc(x+1);
        if (SendMessage(ctl,LB_GETTEXT,g_metaitem_edit,(LPARAM)t) != LB_ERR)
        {
          SetDlgItemTextA(hwndDlg,IDC_EDIT1,t);
        }
        free(t);
      }
    }
  }
  if (uMsg == WM_CLOSE) EndDialog(hwndDlg,0);
  if (uMsg == WM_COMMAND)
  {
    if (LOWORD(wParam) == IDCANCEL) EndDialog(hwndDlg,0);
    if (LOWORD(wParam) == IDOK)
    {
      int x= (int)SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_GETTEXTLENGTH,0,0);
      char *t=(char*)malloc(x+3);
      GetDlgItemTextA(hwndDlg,IDC_EDIT1,t,x+2);
      if (!strstr(t,"="))
      {
		char title[32] = {0};
        free(t);
        MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_METADATA_STRING_MUST_CONTAIN_ONE_EQUAL_SIGN),
				   WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR,title,32),MB_OK|MB_ICONINFORMATION);
      }
      else
      {
        HWND ctl=GetDlgItem(GetParent(hwndDlg),IDC_METADATA);
        LRESULT added;
        if (g_metaitem_edit>=0)
        {
          SendMessage(ctl,LB_DELETESTRING,g_metaitem_edit,0);
          added=SendMessage(ctl,LB_INSERTSTRING,g_metaitem_edit,(LPARAM)t);
        }
        else added=SendMessage(ctl,LB_ADDSTRING,0,(LPARAM)t);
        SendMessage(ctl,LB_SETITEMDATA,(WPARAM)added,0);
        free(t);
        EndDialog(hwndDlg,1);
      }
    }
  }
  return 0;

}

int doTagWrite(HWND hwndDlg, int writeheader)
{
  nsv_OutBS bs;
  if (writeheader) 
  {
    g_filehdr.toc_alloc=0;
    nsv_writeheader(bs,&g_filehdr,IsDlgButtonChecked(hwndDlg,IDC_FASTUPD)?g_oldtag_size:0);
  }

  int hdrlen;
  char *hdr=(char*)bs.get(&hdrlen);

  if (hdr && writeheader && hdrlen == (int)g_oldtag_size) // fast update of header
  {
    DWORD dw = 0;
    SetFilePointer(g_hFile,0,NULL,SEEK_SET);
    WriteFile(g_hFile,hdr,hdrlen,&dw,NULL);
  }
  else if (writeheader || g_oldtag_size)
  {
    if (hdr && writeheader && config_padtag>0) // enlarge header by config_padtag bytes =)
    {
      bs.clear();
      g_filehdr.toc_alloc=0;
      nsv_writeheader(bs,&g_filehdr,config_padtag+g_filehdr.header_size);
      hdr=(char*)bs.get(&hdrlen); // update
    }

    char tmpfn[1024+8] = {0};
    char tmpfn2[1024+8] = {0};
    wsprintfA(tmpfn,"%s.new",g_lastfile);
    wsprintfA(tmpfn2,"%s.old",g_lastfile);

    HANDLE hTempFile=CreateFileA(tmpfn,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
    if (hTempFile == INVALID_HANDLE_VALUE)
    {
	  char title[64] = {0};
      MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_CANNOT_CREATE_TEMPFILE_CANNOT_UPDATE_TAG),
				 WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				 MB_ICONSTOP|MB_OK);
      return 1;
    }
    SetFilePointer(g_hFile,g_oldtag_size,NULL,SEEK_SET);
    if (hdrlen)
    {
      DWORD dw = 0;
      if (!WriteFile(hTempFile,hdr,hdrlen,&dw,NULL) || (int)dw != hdrlen)
      {
		char title[64] = {0};
        CloseHandle(hTempFile);
        DeleteFileA(tmpfn);
        MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_COPYING_SOURCE),
				   WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				   MB_ICONSTOP|MB_OK);
        return 1;
      }
    }

    unsigned int bytes=0;
    for (;;)
    {
      DWORD dw = 0;
      char buf[8192] = {0};
      BOOL r1=ReadFile(g_hFile,buf,sizeof(buf),&dw,NULL);
      if (r1 && !dw) break;
      DWORD dwout = 0;
      if (!r1 || !WriteFile(hTempFile,buf,dw,&dwout,NULL) || dwout < dw)
      {
		char title[64] = {0};
        CloseHandle(hTempFile);
        DeleteFileA(tmpfn);
        MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_COPYING_SOURCE),
				   WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				   MB_ICONSTOP|MB_OK);
        return 1;
      }
      bytes+=dwout;
      wsprintfA(buf,WASABI_API_LNGSTRING(IDS_COPYING_X_BYTES),bytes);
      SetDlgItemTextA(hwndDlg,IDC_TAG_LEN,buf);
    }
    if (GetFileSize(g_hFile,NULL)-g_oldtag_size != GetFileSize(hTempFile,NULL)-hdrlen)
    {
	  char title[64] = {0};
      CloseHandle(hTempFile);
      DeleteFileA(tmpfn);
      MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_SIZE_MISMATCH),
				 WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				 MB_ICONSTOP|MB_OK);
      return 1;
    }
    CloseHandle(g_hFile);
    CloseHandle(hTempFile);
    stopPlayback(g_lastfile);
    if (!MoveFileA(g_lastfile,tmpfn2))
    {
	  char title[64] = {0};
      DeleteFileA(tmpfn);
      restartPlayback();
      MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_RENAMING_SOURCE),
				 WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				 MB_ICONSTOP|MB_OK);
      return 1;
    }
    if (!MoveFileA(tmpfn,g_lastfile))
    {
	  char title[64] = {0};
      MoveFileA(tmpfn2,g_lastfile);
      DeleteFileA(tmpfn);
      restartPlayback();
      MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_RENAMING_NEW_FILE),
				 WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64),
				 MB_ICONSTOP|MB_OK);
      return 1;
    }
    g_hFile = CreateFileA(g_lastfile,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

    DeleteFileA(tmpfn2);
    restartPlayback();
  }
  return 0;
}

void analyzeFile(HWND hwndDlg)
{
	unsigned int nframes=0;
	nsv_Unpacketer unpack;
	nsv_InBS bs;
    GrowBuf framePos;
	unsigned int lastPos=0;
	SetFilePointer(g_hFile,g_oldtag_size,NULL,SEEK_SET);
	for (;;)
	{
		int ret=unpack.unpacket(bs);
		if (ret)
		{
			if (ret<0) break;
			if (fillBs(g_hFile,bs,ret>0?ret:8)) unpack.setEof();
		}
		else
		{
			if (unpack.isSynchFrame())
			{
				framePos.add(&lastPos,4);
				framePos.add(&nframes,4);
			}
			lastPos=SetFilePointer(g_hFile,0,NULL,SEEK_CUR) - (unsigned int)(bs.avail()+7)/8 - g_oldtag_size;
			bs.compact();
			nframes++;
			char buf[128] = {0};
			wsprintfA(buf,WASABI_API_LNGSTRING(IDS_READING_X_FRAMES),nframes);
			SetDlgItemTextA(hwndDlg,IDC_TAG_LEN,buf);
		}
	}
	if (unpack.isValid() && nframes)
	{
	    g_filehdr.file_lenbytes=lastPos;
	    g_filehdr.file_lenms = (int) (nframes * 1000.0 / unpack.getFrameRate());
		if (IsDlgButtonChecked(hwndDlg,IDC_TOC))
		{
			BOOL t;
			DWORD d=GetDlgItemInt(hwndDlg,IDC_SETTOCSIZE,&t,FALSE);
			if (d && t)
			{
				g_filehdr.toc_size=d;
				g_filehdr.toc_alloc=0;
				free(g_filehdr.toc);

				unsigned int x;
				unsigned int *in=(unsigned int *)framePos.get();
				unsigned int tf=(unsigned int)framePos.getlen()/8;
				g_filehdr.toc=(unsigned int *)malloc(g_filehdr.toc_size * sizeof(unsigned int) * 2);
				g_filehdr.toc_ex=g_filehdr.toc + g_filehdr.toc_size;

				if (tf < g_filehdr.toc_size) // we can store all keyframes without dropping any
				{
					g_filehdr.toc_size=tf;
					for (x = 0; x < tf; x ++)
					{
						g_filehdr.toc[x]=in[x*2];
						g_filehdr.toc_ex[x]=in[x*2+1];
					}
				}
				else // drop keyframes to fit
				{
					double pos=0.0;
					double dpos = (double) tf / (double) g_filehdr.toc_size;
					for (x = 0; x < g_filehdr.toc_size; x ++)
					{
						unsigned int ipos=(unsigned int)pos;
						if (ipos >= tf) 
						{
							g_filehdr.toc_size=x;
							break;
						}
						g_filehdr.toc[x]=in[ipos*2];
						g_filehdr.toc_ex[x]=in[ipos*2+1];
						pos+=dpos;
					}
				}
			}
		}
	}
	else
	{
		char title[64] = {0};
		MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_NSV_ANALYSIS_FAILED),
				   WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR_ERROR,title,64), MB_OK);
	}
}

INT_PTR CALLBACK StreamProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	extern int g_streaminfobuf_used;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextA(hwndDlg,IDC_FN,g_lastfile);
			g_streaminfobuf_used=1;
			SetTimer(hwndDlg,1,1000,NULL);
			SendMessage(hwndDlg,WM_TIMER,1,0);
			return TRUE;

		case WM_CLOSE:
			g_streaminfobuf_used=0;
			KillTimer(hwndDlg,1);
			EndDialog(hwndDlg,1);
			return 0;

		case WM_TIMER:
			if (wParam == 1)
			{
				extern char lastfn[];
				extern char g_streaminfobuf[];
				extern CRITICAL_SECTION g_decoder_cs;
				if (!lstrcmpiA(g_lastfile,lastfn) && g_streaminfobuf[0])
				{
					int start = -1, end = 0;
					EnterCriticalSection(&g_decoder_cs);
					SendDlgItemMessage(hwndDlg, IDC_INFO, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
					SetDlgItemTextA(hwndDlg,IDC_INFO,g_streaminfobuf);
					SendDlgItemMessage(hwndDlg, IDC_INFO, EM_SETSEL, start, end);
					LeaveCriticalSection(&g_decoder_cs);
					EnableWindow(GetDlgItem(hwndDlg,IDC_INFO),1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_INFOBORDER),1);
				}
				else 
				{
					SetDlgItemTextA(hwndDlg,IDC_INFO,"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_INFO),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_INFOBORDER),0);
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					g_streaminfobuf_used=0;
					KillTimer(hwndDlg,1);
					EndDialog(hwndDlg,1);
					return 0;
			}
			return 0;
	}
	return 0;
}

INT_PTR CALLBACK MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			openNsv(hwndDlg);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					closeNsv(hwndDlg);
					EndDialog(hwndDlg,1);
					return 0;

				case IDOK:
				case IDC_REMTAG:
					if (!doTagWrite(hwndDlg,LOWORD(wParam) == IDOK))
					{
						closeNsv(hwndDlg);
						lastextfn[0]=0;
						EndDialog(hwndDlg,0);
					}
					else lastextfn[0]=0;
					return 0;

				case IDC_ANALYZE:
					analyzeFile(hwndDlg);
					populateInfo(hwndDlg);
					makeNewHeaderInfo(hwndDlg);
					return 0;

				case IDC_ADD:
					g_metaitem_edit=-1;
					if (WASABI_API_DIALOGBOXW(IDD_DIALOG2,hwndDlg,EditProc))
					{
						updateMetaData(hwndDlg);
						populateMetaData(hwndDlg);
						makeNewHeaderInfo(hwndDlg);
					}
					return 0;

				case IDC_METADATA:
					if (HIWORD(wParam) != LBN_DBLCLK) return 0;

				case IDC_EDIT:
				{
					DWORD res=(DWORD)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETCURSEL,0,0);
					if (res != LB_ERR)
					{
						int ptr= (int)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,res,0);
						if (ptr)
						{
							char title[32] = {0};
							MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_METADATA_ITEM_CONTAINS_LARGE_AMOUNT_DATA),
							WASABI_API_LNGSTRING_BUF(IDS_NSV_TAG_EDITOR,title,32), MB_OK);
						}
						else
						{
							g_metaitem_edit=res;
							if (WASABI_API_DIALOGBOXW(IDD_DIALOG2,hwndDlg,EditProc))
							{
								updateMetaData(hwndDlg);
								populateMetaData(hwndDlg);
								makeNewHeaderInfo(hwndDlg);
								SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_SETCURSEL,(WPARAM)res,0);
							}
						}
					}
				}
					return 0;

				case IDC_REM:
				{
					DWORD res=(DWORD)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETCURSEL,0,0);
					if (res != LB_ERR)
					{
						void *ptr=(void *)SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_GETITEMDATA,res,0);
						if (ptr) free(ptr);
						SendDlgItemMessage(hwndDlg,IDC_METADATA,LB_DELETESTRING,res,0);
						updateMetaData(hwndDlg);
						populateMetaData(hwndDlg);
						makeNewHeaderInfo(hwndDlg);
					}
				}
					return 0;

				case IDC_TOC:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						int ch=!!IsDlgButtonChecked(hwndDlg,IDC_TOC);
						if (ch)
						{
							if (g_toc_save)
							{
								g_filehdr.toc=g_toc_save;
								g_filehdr.toc_ex=g_toc_save_ex;
								g_filehdr.toc_size=g_toc_savesize;
								g_toc_save=0;
								g_toc_save_ex=0;
								g_toc_savesize=0;
								makeNewHeaderInfo(hwndDlg);
							}
						}
						else
						{
							if (!g_toc_save)
							{
								g_toc_save=g_filehdr.toc;
								g_toc_save_ex=g_filehdr.toc_ex;
								g_toc_savesize=g_filehdr.toc_size;
								g_filehdr.toc=0;
								g_filehdr.toc_size=0;
								g_filehdr.toc_ex=0;
								makeNewHeaderInfo(hwndDlg);
							}
						}
					}
					return 0;
			}
			return 0;

		case WM_CLOSE:
			closeNsv(hwndDlg);
			EndDialog(hwndDlg,1);
			return 0;
	}
	return 0;
}

int infoDlg(const char *fn, HWND hwnd)
{
	g_lastfile=fn;
	if (strstr(fn,"://")) return (int)WASABI_API_DIALOGBOXW(IDD_DIALOG4,hwnd,StreamProc);
	else return (int)WASABI_API_DIALOGBOXW(IDD_DIALOG3,hwnd,MainProc);
}