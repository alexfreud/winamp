#include <windows.h>

#include "main.h"
#include "video.h"
#include "resource.h"

typedef struct
{
  VideoOutput *vidOut;
  NSVDecoder *decode;
  int quit;
} parms;

HINSTANCE g_hInstance;
int g_bitmap_id=IDB_BITMAP1;

//#define NO_ABOUT_EGG

extern long glCounterNSVf, glSyncFrameCount, glNonSyncFrameCount;

#define WNDMENU_CAPTION "NSVPlay/0.994"
#include "about.h"
#include "wndmenu.h"

DWORD WINAPI pooThread(LPVOID p)
{
  parms *parm=(parms*)p;
  unsigned int last_title_check=0;

  while (!parm->quit)
  {
    int r=parm->decode->run(&parm->quit);
    if (r < 0)
    {
      if (parm->decode->get_error()) MessageBox(parm->vidOut->getHwnd(),parm->decode->get_error(),"NSV Player Error",MB_OK|MB_ICONSTOP);
      break;
    }
    else if (!r) 
    {
      if ((GetTickCount()-last_title_check) > 1000)
      {
        char *v=parm->decode->getTitle();
        char *buf=(char*)malloc((v?strlen(v):0)+128);
        int posms=parm->decode->getpos();
        int lenms=parm->decode->getlen();
        char *s=parm->decode->getStatus();
        if (lenms != ~0)
          wsprintf(buf,"%s [%d:%02d/%d:%02d] @ %dkbps %s%s%s- NSV Player",v?v:"",
            posms/60000,(posms/1000)%60,
            lenms/60000,(lenms/1000)%60,
            parm->decode->getBitrate()/1000,
            s?"[":"",s?s:"",s?"] ":""
            );
        else
          wsprintf(buf,"%s [%d:%02d] @ %dkbps %s%s%s- NSV Player",v?v:"",
            posms/60000,(posms/1000)%60,
            parm->decode->getBitrate()/1000,
            s?"[":"",s?s:"",s?"] ":""
            );

        char *p=buf;
        while (*p)
        {
          if (*p == '_') *p=' ';
          p++;
        }
        
        SetWindowText(parm->vidOut->getHwnd(),buf);
        free(buf);

        last_title_check=GetTickCount();
      }
      Sleep(1);
    }
  }
  parm->quit++;
  return 0;
}


int g_quit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmdParam, int nCmdShow) 
{
  g_hInstance=hInstance;

  BOOL	bDisplayInfoAtEnd = FALSE;
  char *subtitlefile=NULL;
  char buf[11];
  lstrcpyn(buf,lpszCmdParam,11);

  if(strstr(lpszCmdParam, "/info")) {
	  bDisplayInfoAtEnd = TRUE;
  }

  if (!lstrcmpi(buf,"/subtitle="))
  {
    lpszCmdParam += 10;
    char scanc=' ';
    if (*lpszCmdParam == '\"')
    {
      scanc=*lpszCmdParam++;
    }
    subtitlefile=lpszCmdParam;
    while (*lpszCmdParam && *lpszCmdParam != scanc) lpszCmdParam++;
    *lpszCmdParam++=0;
    while (*lpszCmdParam == ' ') lpszCmdParam++;    
  }

  if (*lpszCmdParam == '\"') 
  {
    lpszCmdParam++;
    char *p=strstr(lpszCmdParam,"\"");
    if (p) *p=0;
  }

  if (!*lpszCmdParam) 
  {
    MessageBox(NULL,"Usage: nsvplay.exe [/subtitle=\"file|url\"] filename.nsv|http://url.nsv","NSV Player",MB_OK|MB_ICONINFORMATION);
    return 0;
  }

  int xpos=CW_USEDEFAULT;
  int ypos=CW_USEDEFAULT;

  _ReadConfigItemInt("xpos",&xpos);
  _ReadConfigItemInt("ypos",&ypos);

  VideoOutput *vidOut = new VideoOutput(NULL,xpos,ypos);
  Decoders_Init();

  
  NSVDecoder *decode=new NSVDecoder(lpszCmdParam,vidOut,subtitlefile);
  vidOut->setNSVDecoder(decode);

  int m_pause=0;

  
  vidOut->vid_vsync=false;


  _ReadConfigItemInt("vsync",&vidOut->vid_vsync);
  _ReadConfigItemInt("overlays",&vidOut->vid_overlays);
  _ReadConfigItemInt("ddraw",&vidOut->vid_ddraw);
  _ReadConfigItemInt("aspectadj",&vidOut->vid_aspectadj);
  _ReadConfigItemInt("use_mixer",&g_audio_use_mixer);
  {
    int temp=1;
    _ReadConfigItemInt("subtitles",&temp);
    decode->enableSubs(temp);
    temp=0;
    _ReadConfigItemInt("subtitles_size",&temp);
    decode->setSubsFontSize(temp);
  }

  DWORD id;
  parms parm={0,};
  parm.decode=decode;
  parm.vidOut=vidOut;

  vidOut->setcallback(my_wndcallback,&parm);
  
  HANDLE hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)pooThread,(LPVOID)&parm,0,&id);

  for (;;)
  {
    MSG msg;
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
    {
      if (msg.hwnd == vidOut->getHwnd() && msg.message == WM_KEYDOWN)
      {
        if (msg.wParam == VK_SPACE) decode->pause(m_pause=!m_pause);
        if (msg.wParam == VK_LEFT) decode->seek(decode->getpos()-30000);
        if (msg.wParam == VK_RIGHT) decode->seek(decode->getpos()+30000);
        if (msg.wParam == VK_UP)
        {
          int vol=decode->getvolume()+16;
          if (vol > 255) vol=255;
          decode->setvolume(vol);
        }
        if (msg.wParam == VK_DOWN)
        {
          int vol=decode->getvolume()-16;
          if (vol < 0) vol=0;
          decode->setvolume(vol);
        }
        if (msg.wParam == VK_RETURN)
        {
          if (vidOut->is_fullscreen()) vidOut->remove_fullscreen();
          else vidOut->fullscreen();
        }
        if (msg.wParam == 'v' || msg.wParam == 'V')
        {
          vidOut->vid_vsync=!vidOut->vid_vsync;
        }
        if (msg.wParam == 'a' || msg.wParam == 'A')
        {
          vidOut->vid_aspectadj=!vidOut->vid_aspectadj;
          PostMessage(vidOut->getHwnd(),WM_TIMER,1,0);
        }
      }
      DispatchMessage(&msg);
    }

    if (!IsWindow(vidOut->getHwnd()) || parm.quit) break;

    Sleep(50);

  }
  parm.quit++;
  WaitForSingleObject(hThread,INFINITE);
  CloseHandle(hThread);
  delete decode;
  delete vidOut;

  Decoders_Quit();

#if _DEBUG
  // if "/info" on the command line, display some statistics about the movie
  if(bDisplayInfoAtEnd) {
	char	szMessage[MAX_PATH];

	wsprintf(szMessage, "File Header Count=%ld\nSync Frames=%ld\nNon-sync Frames=%ld", glCounterNSVf, glSyncFrameCount, glNonSyncFrameCount);
	MessageBox(NULL, szMessage, WNDMENU_CAPTION, MB_OK);
  }
#endif

  return 0;
}

