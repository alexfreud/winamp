#ifndef _WNDMENU_H_
#define _WNDMENU_H_

//need to define WNDMENU_CAPTION to the caption
// you can define these to remove menu functionality
//WNDMENU_NOABOUT // no about header
//WNDMENU_NOVOLUME // no volume submenu
//WNDMENU_NOINFO // no info submenu
//WNDMENU_NOZOOM // no zoom 50/100/200/400
//WNDMENU_NOZOOMFS // no zoom fullscreen
//WNDMENU_NOOPTIONS // no options submenu
//WNDMENU_NOPOSITIONSAVE // disables saving of position
//WNDMENU_NOSUBTITLES // no subtitles submenu

#ifndef ABS 
#define ABS(x) ((x)<0?-(x):(x))
#endif


static void _ReadConfigItemInt(char *name, int *value)
{
  HKEY hKey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Nullsoft\\NSVPlay",0,KEY_READ,&hKey) == ERROR_SUCCESS)
  {
    DWORD t,l=4;
    int a;
    if (RegQueryValueEx(hKey,name,NULL,&t,(unsigned char *)&a,&l ) == ERROR_SUCCESS && t == REG_DWORD)
      *value=a;
    RegCloseKey(hKey);
  }
}

static void _WriteConfigItemInt(char *name, int value)
{
  HKEY hKey;
  if (RegCreateKey(HKEY_LOCAL_MACHINE,"Software\\Nullsoft\\NSVPlay",&hKey) == ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,name,0,REG_DWORD,(unsigned char*)&value,4);
    RegCloseKey(hKey);
  }
}

static void _InsertString(HMENU hMenu, int pos, char *string, int id, int checked=0, int disabled=0)
{
  MENUITEMINFO inf={sizeof(inf),MIIM_ID|MIIM_STATE|MIIM_TYPE,MFT_STRING,
                    (checked?MFS_CHECKED:0) | (disabled?MFS_DISABLED:MFS_ENABLED), id,
                    };
  inf.dwTypeData = string;

  InsertMenuItem(hMenu,pos,TRUE,&inf);
}

static void _InsertSep(HMENU hMenu, int pos)
{
  MENUITEMINFO inf={sizeof(inf),MIIM_TYPE,MFT_SEPARATOR };
  InsertMenuItem(hMenu,pos,TRUE,&inf);
}

static HMENU _InsertSub(HMENU hMenu, int pos, char *string)
{
  MENUITEMINFO inf={sizeof(inf),MIIM_TYPE|MIIM_SUBMENU,MFT_STRING };
  inf.dwTypeData = string;
  inf.hSubMenu=CreatePopupMenu();
  InsertMenuItem(hMenu,pos,TRUE,&inf);
  return inf.hSubMenu;
}


#define MENUITEM_ABOUT 1
#define MENUITEM_INFOBASE 10
#define MENUITEM_VOLUMEBASE 100
#define MENUITEM_VSYNC 120
#define MENUITEM_ASPECTADJ 121
#define MENUITEM_VOVERLAYS 122
#define MENUITEM_VDDRAW 123
#define MENUITEM_SOFTVOLMIX 124
#define MENUITEM_ENABLESUBTITLES 125
#define MENUITEM_SUBSIZEBASE 126 // 5 sizes

#define MENUITEM_ZOOM50 112
#define MENUITEM_ZOOM100 113
#define MENUITEM_ZOOM200 114
#define MENUITEM_ZOOM400 115
#define MENUITEM_ZOOMFS 116

#define MENUITEM_CLOSE 200

#define MENUITEM_SUBSLANGBASE 300

extern int g_audio_use_mixer;

LRESULT my_wndcallback(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (token)
  {
    parms *parm=(parms*)token;
#ifndef WNDMENU_NOPOSITIONSAVE
    if (uMsg == WM_MOVE)
    {
      if (!parm->vidOut->is_fullscreen())
      {
        RECT r;
        GetWindowRect(hwnd,&r);
        _WriteConfigItemInt("xpos",r.left);
        _WriteConfigItemInt("ypos",r.top);
      }
    }
#endif
    if(uMsg==WM_SETCURSOR && parm->vidOut->is_fullscreen()) {
      SetCursor(NULL);
      return TRUE;    
    }

    if (uMsg == WM_RBUTTONUP && !parm->vidOut->is_fullscreen())
    {
      int pos=0;
      HMENU hMenu=CreatePopupMenu();

#ifndef WNDMENU_NOABOUT
      _InsertString(hMenu,pos++,WNDMENU_CAPTION,MENUITEM_ABOUT);
      _InsertSep(hMenu,pos++);
#endif
#ifndef WNDMENU_NOINFO
      if (parm->decode)
      {
        char buf[1024];
        int subpos=0;
        int tmpid=MENUITEM_INFOBASE;
        HMENU sub=_InsertSub(hMenu,pos++,"Info");

        { // title
          char *v=parm->decode->getTitle();
          if (v && *v)
          {
            char *tmp=(char*)malloc(strlen(v)+32);
            wsprintf(tmp,"Title: %s",v);
            char *p=tmp;
            while (*p)
            {
              if (*p == '_') *p=' ';
              p++;
            }
            _InsertString(sub,subpos++,tmp,tmpid++);
            free(tmp);
          }
        }

        
        { // length
          int lenms=parm->decode->getlen();
          if (lenms != ~0)
          {
            wsprintf(buf,"Length: %d:%02d",lenms/60000,(lenms/1000)%60);
            _InsertString(sub,subpos++,buf,tmpid++);
          }
        }


        { // codecs
          int a=(parm->decode->getBitrate()+500)/1000;
          unsigned int a2=parm->decode->getFileSize();
          if (a)
          {
            wsprintf(buf,"Bitrate: %dkbps",a);
            if (a2 && a2 != ~0)
              wsprintf(buf+strlen(buf)," (Total %u bytes)",a2);

            _InsertString(sub,subpos++,buf,tmpid++);
          }
          else if (a2 && a2 != ~0)
          {
            wsprintf(buf,"Size: %u bytes",a2);
            _InsertString(sub,subpos++,buf,tmpid++);
          }

          lstrcpy(buf,"Video: ");
          char *p=buf+strlen(buf);
          parm->decode->getVideoDesc(p);
          if (*p) _InsertString(sub,subpos++,buf,tmpid++);

          lstrcpy(buf,"Audio: ");
          p=buf+strlen(buf);
          parm->decode->getAudioDesc(p);
          if (*p) _InsertString(sub,subpos++,buf,tmpid++);
        }

        {
          const char *p=parm->decode->getServerHeader("Server");
          if (p && *p)
          {
            lstrcpy(buf,"Server: ");
            lstrcpyn(buf+strlen(buf),p,sizeof(buf)-strlen(buf)-2);
            _InsertString(sub,subpos++,buf,tmpid++);
          }
        }
      }
#endif //WNDMENU_NOINFO
#ifndef WNDMENU_NOVOLUME
      if (parm->decode)
      {
        HMENU sub=_InsertSub(hMenu,pos++,"Volume");
        int cv=(parm->decode->getvolume()*100)/255;
        int x;
        for (x=0;x<=10;x++)
        {
          char buf[64];
          int a=100-x*10;
          wsprintf(buf,"%d%%",a);
          _InsertString(sub,x,buf,MENUITEM_VOLUMEBASE+x,cv >= a-5 && cv < a+5);
        }
      }
#endif//WNDMENU_NOVOLUME
#ifndef WNDMENU_NOOPTIONS
      if (parm->vidOut)
      {
        HMENU sub=_InsertSub(hMenu,pos++,"Options");
        int subpos=0;
        _InsertString(sub,subpos++,"Synchronize video to refresh",MENUITEM_VSYNC,!!parm->vidOut->vid_vsync);
        _InsertString(sub,subpos++,"Maintain video aspect ratio",MENUITEM_ASPECTADJ,!!parm->vidOut->vid_aspectadj);
        _InsertString(sub,subpos++,"Allow video overlay",MENUITEM_VOVERLAYS,!!parm->vidOut->vid_overlays);
        //_InsertString(sub,subpos++,"Allow DirectDraw acceleration",MENUITEM_VDDRAW,!!parm->vidOut->vid_ddraw);
        _InsertString(sub,subpos++,"Use software volume mixing",MENUITEM_SOFTVOLMIX,!g_audio_use_mixer);
      }
#endif//WNDMENU_NOOPTIONS
#if !defined(WNDMENU_NOZOOM) || !defined(WNDMENU_NOZOOMFS)
      if (parm->decode && parm->vidOut)
      {
        HMENU sub=_InsertSub(hMenu,pos++,"Zoom");
        int subpos=0;
#ifndef WNDMENU_NOZOOM
        int ow,oh;
        int w=parm->decode->getWidth();
        int h=parm->decode->getHeight();
        parm->vidOut->getOutputSize(&ow,&oh);
        _InsertString(sub,subpos++,"50%",MENUITEM_ZOOM50,ABS(w/2-ow)<4);
        _InsertString(sub,subpos++,"100%",MENUITEM_ZOOM100,ABS(w-ow)<4);
        _InsertString(sub,subpos++,"200%",MENUITEM_ZOOM200,ABS(w*2-ow)<4);
        _InsertString(sub,subpos++,"400%",MENUITEM_ZOOM400,ABS(w/4-ow)<4);
#ifndef WNDMENU_NOZOOMFS
        _InsertSep(sub,subpos++);
#endif
#endif

#ifndef WNDMENU_NOZOOMFS
        _InsertString(sub,subpos++,"Fullscreen",MENUITEM_ZOOMFS,!!parm->vidOut->is_fullscreen());
#endif//WNDMENU_NOZOOMFS
      }
#endif//WNDMENU_NOZOOM||WNDMENU_NOZOOMFS

#ifndef WNDMENU_NOSUBTITLES
      if(parm->decode)
      {
        HMENU sub=_InsertSub(hMenu,pos++,"Subtitles");
        int subpos=0;
        _InsertString(sub,subpos++,"Enabled",MENUITEM_ENABLESUBTITLES,parm->decode->subsEnabled());
        _InsertSep(sub,subpos++);
        HMENU sizesub=_InsertSub(sub,subpos++,"Font size");
        int sizesubpos=0;
        int fontsize=parm->decode->getSubsFontSize();
        _InsertString(sizesub,sizesubpos++,"Largest",MENUITEM_SUBSIZEBASE,fontsize==20);
        _InsertString(sizesub,sizesubpos++,"Larger",MENUITEM_SUBSIZEBASE+1,fontsize==10);
        _InsertString(sizesub,sizesubpos++,"Medium",MENUITEM_SUBSIZEBASE+2,fontsize==0);
        _InsertString(sizesub,sizesubpos++,"Smaller",MENUITEM_SUBSIZEBASE+3,fontsize==-4);
        _InsertString(sizesub,sizesubpos++,"Smallest",MENUITEM_SUBSIZEBASE+4,fontsize==-8);
        HMENU langsub=_InsertSub(sub,subpos++,"Language");
        int langsubpos=0;
        for(int i=0;;i++) {
          const char *lang=parm->decode->getSubLanguage(i);
          if(!lang) break;
          _InsertString(langsub,langsubpos++,(char *)lang,MENUITEM_SUBSLANGBASE+i,i==parm->decode->getCurSubLanguage());
        }
      }
#endif//WNDMENU_NOSUBTITLES

#ifndef WNDMENU_NOCLOSE
      _InsertSep(hMenu,pos++);
      _InsertString(hMenu,pos++,"E&xit\tAlt+F4",MENUITEM_CLOSE);
#endif//WNDMENU_NOCLOSE

      POINT p;
      GetCursorPos(&p);
      int x=TrackPopupMenu(hMenu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,p.x,p.y,0,hwnd,NULL);
      DestroyMenu(hMenu);
      switch (x)
      {
#ifndef WNDMENU_NOABOUT
        case MENUITEM_ABOUT:
#ifdef _ABOUT_H_ // dont display the about box if do_about isn't there anyway
          do_about(hwnd);
#endif
        break;
#endif//WNDMENU_NOABOUT
#ifndef WNDMENU_NOOPTIONS
        case MENUITEM_VOVERLAYS:
          _WriteConfigItemInt("overlays",parm->vidOut->vid_overlays=!parm->vidOut->vid_overlays);
          PostMessage(hwnd,WM_USER+0x1,0,0); //renegotiate surface
        break;
        case MENUITEM_VDDRAW:
          _WriteConfigItemInt("ddraw",parm->vidOut->vid_ddraw=!parm->vidOut->vid_ddraw);
          PostMessage(hwnd,WM_USER+0x1,0,0); //renegotiate surface
        break;
        case MENUITEM_ASPECTADJ:
          _WriteConfigItemInt("aspectadj",parm->vidOut->vid_aspectadj=!parm->vidOut->vid_aspectadj);
          PostMessage(hwnd,WM_TIMER,1,0);
        break;
        case MENUITEM_VSYNC:
          _WriteConfigItemInt("vsync",parm->vidOut->vid_vsync=!parm->vidOut->vid_vsync);
        break;
        case MENUITEM_SOFTVOLMIX:
          _WriteConfigItemInt("use_mixer",g_audio_use_mixer=!g_audio_use_mixer);
          parm->decode->setvolume(parm->decode->getvolume());
        break;
#endif//WNDMENU_NOOPTIONS

#ifndef WNDMENU_NOZOOM
        case MENUITEM_ZOOM50:
        case MENUITEM_ZOOM100:
        case MENUITEM_ZOOM200:
        case MENUITEM_ZOOM400:
          if (parm->vidOut->is_fullscreen()) parm->vidOut->remove_fullscreen();
          {
            int w=parm->decode->getWidth();
            int h=parm->decode->getHeight();
            if (x == MENUITEM_ZOOM50) { w/=2; h/=2; }
            else if (x == MENUITEM_ZOOM200) { w*=2; h*=2; }
            else if (x == MENUITEM_ZOOM400) { w*=4; h*=4; }
            parm->vidOut->setOutputSize(w,h);

          }
        break;
#endif//WNDMENU_NOZOOM
#ifndef WNDMENU_NOZOOMFS
        case MENUITEM_ZOOMFS:
          if (parm->vidOut->is_fullscreen()) parm->vidOut->remove_fullscreen();
          else parm->vidOut->fullscreen();
        break;
#endif//WNDMENU_NOZOOMFS
#ifndef WNDMENU_NOCLOSE
        case MENUITEM_CLOSE:
          SendMessage(hwnd,WM_CLOSE,0,0);
        break;
#endif//WNDMENU_NOCLOSE
#ifndef WNDMENU_NOSUBTITLES
        case MENUITEM_ENABLESUBTITLES:
          if(parm->decode) parm->decode->enableSubs(!parm->decode->subsEnabled());
          _WriteConfigItemInt("subtitles",parm->decode->subsEnabled());
        break;
        case MENUITEM_SUBSIZEBASE:
        case MENUITEM_SUBSIZEBASE+1:
        case MENUITEM_SUBSIZEBASE+2:
        case MENUITEM_SUBSIZEBASE+3:
        case MENUITEM_SUBSIZEBASE+4:
          if(parm->decode) {
            int sizes[]={20,10,0,-4,-8};
            int s=x-MENUITEM_SUBSIZEBASE;
            parm->decode->setSubsFontSize(sizes[s]);
            _WriteConfigItemInt("subtitles_size",parm->decode->getSubsFontSize());
          }
        break;
#endif//WNDMENU_NOSUBTITLES
        default:
#ifndef WNDMENU_NOVOLUME
          if (x >= MENUITEM_VOLUMEBASE && x <= MENUITEM_VOLUMEBASE+10)
          {
            int v=10-(x-MENUITEM_VOLUMEBASE);
            parm->decode->setvolume((v*255)/10);
            _WriteConfigItemInt("volume",parm->decode->getvolume());
          }
#endif
          break;
#ifndef WNDMENU_NOCLOSE
    case WM_CLOSE:
      DestroyWindow(hwnd);      
    return 0;
#endif
      }
#ifndef WNDMENU_NOSUBTITLES
      if(x>=MENUITEM_SUBSLANGBASE && x<=MENUITEM_SUBSLANGBASE+64) {
        parm->decode->setSubLanguage(x-MENUITEM_SUBSLANGBASE);
      }
#endif
    }
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


#endif//_WNDMENU_H_