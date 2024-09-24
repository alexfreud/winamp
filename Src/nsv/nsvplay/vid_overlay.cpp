#include "video.h"
#include <multimon.h>
#include "subtitles.h"

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#define OV_COL_R 16
#define OV_COL_G 0
#define OV_COL_B 16

OverlayVideoOutput::OverlayVideoOutput() {
  lpDD=NULL;
  lpddsOverlay=NULL;
  lpddsPrimary=NULL;
  is_fullscreen=0;
  yuy2_output=uyvy_output=0;
  m_parent=NULL;
  initing=false;
  needchange=0;
  memset(&m_oldrd,0,sizeof(m_oldrd));
  memset(&winRect,0,sizeof(winRect));
  subFont=NULL;
  m_fontsize=0;
  resetSubtitle();
}

OverlayVideoOutput::~OverlayVideoOutput() {
  if(is_fullscreen) removeFullScreen();
  LPDIRECTDRAWSURFACE o=lpddsOverlay;
  lpddsOverlay=NULL;
  if(o) o->Release();
  if(lpddsPrimary) lpddsPrimary->Release();
  if (lpDD) lpDD->Release();	// BU added NULL check in response to talkback
  if(subFont) DeleteObject(subFont);
}

static DWORD DD_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
  COLORREF rgbT;
  HDC hdc;
  DWORD dw = CLR_INVALID;
  DDSURFACEDESC ddsd;
  HRESULT hres;
  
  //
  //  use GDI SetPixel to color match for us
  //
  if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
  {
	  rgbT = GetPixel(hdc, 0, 0);     // save current pixel value
	  SetPixel(hdc, 0, 0, rgb);       // set our value
	  pdds->ReleaseDC(hdc);
  }
  
  //
  // now lock the surface so we can read back the converted color
  //
  ddsd.dwSize = sizeof(ddsd);
  while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == 
	  DDERR_WASSTILLDRAWING)
	  ;
  
  if (hres == DD_OK)
  {
	  dw  = *(DWORD *)ddsd.lpSurface;    // get DWORD
    if(ddsd.ddpfPixelFormat.dwRGBBitCount<32)
	    dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;                        // mask it to bpp
	  pdds->Unlock(NULL);
  }
  
  //
  //  now put the color that was there back.
  //
  if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
  {
	  SetPixel(hdc, 0, 0, rgbT);
	  pdds->ReleaseDC(hdc);
  }
  
  return dw;
}

int OverlayVideoOutput::create(VideoOutput *parent, int w, int h, unsigned int ptype, int flipit, double aspectratio) {
  type=ptype;
  width=w;
  height=h;
  flip=flipit;
  m_parent=parent;

  initing=true;
  HWND hwnd=parent->getHwnd();

  if (lpDD) lpDD->Release();
  lpDD=NULL;

  update_monitor_coords(parent);

  if(!m_found_devguid) DirectDrawCreate(NULL,&lpDD,NULL);
  else DirectDrawCreate(&m_devguid,&lpDD,NULL);

  if(!lpDD) {
    initing=false;
    return 0;
  }

  lpDD->SetCooperativeLevel(hwnd,DDSCL_NOWINDOWCHANGES|DDSCL_NORMAL);

  DDSURFACEDESC   ddsd;
  INIT_DIRECTDRAW_STRUCT(ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  HRESULT ddrval = lpDD->CreateSurface(&ddsd, &lpddsPrimary, NULL );

  // init overlay
  DDSURFACEDESC   ddsdOverlay;
  INIT_DIRECTDRAW_STRUCT(ddsdOverlay);
  ddsdOverlay.ddsCaps.dwCaps=DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
  ddsdOverlay.dwFlags= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_PITCH;
  ddsdOverlay.dwWidth=w;
  ddsdOverlay.dwHeight=h;
  ddsdOverlay.lPitch=w*4;
  ddsdOverlay.dwBackBufferCount=0;
  DDPIXELFORMAT pf[]=
  {
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},
    {sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0}, // UYVY
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC,MAKEFOURCC('Y','V','1','2'),0,0,0,0,0},
  };
  int tab[5];
  if(type==NSV_MAKETYPE('Y','U','Y','2')) {
    tab[0]=0; // default is YUY2
    tab[1]=1;
    tab[2]=-1;
  } else if(type==NSV_MAKETYPE('U','Y','V','Y')) {
      tab[0]=1; // make UYVY default
      tab[1]=0;
      tab[2]=-1;
  } else if(type==NSV_MAKETYPE('Y','V','1','2')) {
      /*tab[0]=2;
      tab[1]=0;
      tab[2]=1;
      tab[3]=-1;*/
    //CT> Make YUY2 default too, cause YV12 is borked on some ATI cards/drivers :(
    tab[0]=0;
    tab[1]=1;
    tab[2]=-1;
  } else {
      tab[0]=-1; // default is RGB
  }

  int x=4096;
  HRESULT v=-1;
  for (x = 0; x < sizeof(tab)/sizeof(tab[0]) && tab[x]>=0; x ++) {
    ddsdOverlay.ddpfPixelFormat=pf[tab[x]];
    v=lpDD->CreateSurface(&ddsdOverlay, &lpddsOverlay, NULL);
    if (!FAILED(v)) break;
  }
  if(FAILED(v)||x>=sizeof(tab)/sizeof(tab[0])||tab[x]<0) {
    initing=false;
    return 0;
  }

  yuy2_output = (tab[x] == 0);
  uyvy_output = (tab[x] == 1);

  INIT_DIRECTDRAW_STRUCT(capsDrv);
  ddrval = lpDD->GetCaps(&capsDrv, NULL);

  uDestSizeAlign = capsDrv.dwAlignSizeDest;
  uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;

  dwUpdateFlags = DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE;

  DEVMODE d;
  d.dmSize=sizeof(d);
  d.dmDriverExtra=0;
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &d);

  int rv=OV_COL_R, gv=OV_COL_G, bv=OV_COL_B;

  INIT_DIRECTDRAW_STRUCT(ovfx);
  ovfx.dwDDFX=0;
  switch(d.dmBitsPerPel) {
    case 16:
      ovfx.dckDestColorkey.dwColorSpaceLowValue=((rv>>3) << 11) | ((gv>>2) << 5) | (bv>>3);
      break;
    case 15:
      ovfx.dckDestColorkey.dwColorSpaceLowValue=((rv>>3) << 10) | ((gv>>3) << 5) | (bv>>3);
      break;
    case 24: case 32:
      ovfx.dckDestColorkey.dwColorSpaceLowValue=(rv << 16) | (gv << 8) | bv;
      break;
  }

  //try to get the correct bit depth thru directdraw (for fucked up 16 bits displays for ie.)
  {
    DDSURFACEDESC DDsd={sizeof(DDsd),};
    lpddsPrimary->GetSurfaceDesc(&ddsd);
    DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT; //create the surface at screen depth
    DDsd.dwWidth=8;
    DDsd.dwHeight=8;
    DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
    LPDIRECTDRAWSURFACE tempsurf;
    if(lpDD->CreateSurface(&DDsd, &tempsurf, NULL)==DD_OK)
    {
      int res=DD_ColorMatch(tempsurf, RGB(rv,gv,bv));
      if(res!=CLR_INVALID) ovfx.dckDestColorkey.dwColorSpaceLowValue=res;
      tempsurf->Release();
    }
  }

  ovfx.dckDestColorkey.dwColorSpaceHighValue=ovfx.dckDestColorkey.dwColorSpaceLowValue;

  getRects(&rs,&rd);
  if(FAILED(lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx))) {
    initing=false;
    return 0;
  }
  initing=false;

  DDSURFACEDESC dd={sizeof(dd),};
  if (lpddsOverlay->Lock(NULL,&dd,DDLOCK_WAIT,NULL) != DD_OK) return 0;
  unsigned char *o=(unsigned char*)dd.lpSurface;
  if (uyvy_output||yuy2_output) 
  {
    int x=dd.lPitch*height/2;
    while (x--)
    {
      if (uyvy_output)
      {
        *o++=128;
        *o++=0;
      }
      else
      {
        *o++=0;
        *o++=-128;
      }
    }
  }
  else
  {
    memset(o,0,dd.lPitch*height); o+=dd.lPitch*height;
    memset(o,128,dd.lPitch*height/2);
  }
  lpddsOverlay->Unlock(&dd);

  InvalidateRect(hwnd,NULL,TRUE);
  return 1;
}

void OverlayVideoOutput::getRects(RECT *drs, RECT *drd) {
  HWND hwnd=m_parent->getHwnd();
  if(GetParent(hwnd)) hwnd=GetParent(hwnd);

  RECT rd,rs;
  GetClientRect(hwnd,&rd);
  ClientToScreen(hwnd,(LPPOINT)&rd);
  ClientToScreen(hwnd,((LPPOINT)&rd) + 1);

  m_parent->adjustAspect(rd);
  rd.left-=m_mon_x;
  rd.right-=m_mon_x;
  rd.top-=m_mon_y;
  rd.bottom-=m_mon_y;

  memset(&rs,0,sizeof(rs));
  rs.right=width;
  rs.bottom=height;

  //resize overlay for off-screen
  RECT rfull;
  //m_parent->getViewport(&rfull,NULL,1); //FUCKO: assume monitor 0
  m_parent->getViewport(&rfull,hwnd,1);   //FUCKO: okay to use this hwnd? (fixes multimon! -RG)
  if(rd.right>rfull.right) {
    int diff=rd.right-rfull.right;
    float sc=(float)(width)/(float)(rd.right-rd.left);
    rd.right=rfull.right;
    rs.right=width-(int)(diff*sc);
  }
  if(rd.left<rfull.left) {
    int diff=rfull.left-rd.left;
    float sc=(float)(width)/(float)(rd.right-rd.left);
    rd.left=rfull.left;
    rs.left=(int)(diff*sc);
  }
  if(rd.bottom>rfull.bottom) {
    int diff=rd.bottom-rfull.bottom;
    float sc=(float)(height)/(float)(rd.bottom-rd.top);
    rd.bottom=rfull.bottom;
    rs.bottom=height-(int)(diff*sc);
  }
  if(rd.top<rfull.top) {
    int diff=rfull.top-rd.top;
    float sc=(float)(height)/(float)(rd.bottom-rd.top);
    rd.top=rfull.top;
    rs.top=(int)(diff*sc);
  }

  if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uDestSizeAlign) {
    rs.left = (int)((rs.left+uDestSizeAlign-1)/uDestSizeAlign)*uDestSizeAlign;
    rs.right = (int)((rs.right+uDestSizeAlign-1)/uDestSizeAlign)*uDestSizeAlign;
  }
  if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign) {
    rd.left = (int)((rd.left+uDestSizeAlign-1)/uDestSizeAlign)*uDestSizeAlign;
    rd.right = (int)((rd.right+uDestSizeAlign-1)/uDestSizeAlign)*uDestSizeAlign;
  }

  *drd=rd;
  *drs=rs;
}

void OverlayVideoOutput::timerCallback() {
  if(!m_parent) return;

  RECT rd,rs;
  getRects(&rs,&rd);

  if(memcmp(&m_oldrd,&rd,sizeof(RECT))) {
    m_oldrd=rd;
    if(!initing && lpddsOverlay) 
      if(FAILED(lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx))) {
        needchange=1;
      }
  }
}

int OverlayVideoOutput::onPaint(HWND hwnd, HDC hdc) {
  if(!m_parent) return 0;

  PAINTSTRUCT p;
  BeginPaint(hwnd,&p);

  RECT r;
  GetClientRect(hwnd,&r);
  LOGBRUSH lb={BS_SOLID,RGB(OV_COL_R,OV_COL_G,OV_COL_B),};
  HBRUSH br=CreateBrushIndirect(&lb);
  FillRect(p.hdc,&r,br);
  DeleteObject(br);

  if (curSubtitle)
  {
    int m_lastsubxp=curSubtitle->xPos;
    int m_lastsubyp=curSubtitle->yPos;

    HDC out=p.hdc;
  
    HGDIOBJ oldobj=SelectObject(out,subFont);
  
    SetBkMode(out,TRANSPARENT);
    int centerflags=0;
    if (m_lastsubxp < 127) centerflags |= DT_LEFT;
    else if (m_lastsubxp > 127) centerflags |= DT_RIGHT;
    else centerflags |= DT_CENTER;

    if (m_lastsubyp < 127) centerflags |= DT_TOP;
    else if (m_lastsubyp > 127) centerflags |= DT_BOTTOM;
  
    // draw outline
    SetTextColor(out,RGB(0,0,0));
    for (int y = -1; y < 2; y++)
      for (int x = -1; x < 2; x++)
      {
        if(!y && !x) continue;
        RECT r2={subRect.left+x,subRect.top+y,subRect.right+x,subRect.bottom+y};
        DrawText(out,curSubtitle->text,-1,&r2,centerflags|DT_NOCLIP|DT_NOPREFIX);
      }
    // draw text
    SetTextColor(out,RGB(curSubtitle->colorRed,curSubtitle->colorGreen,curSubtitle->colorBlue));
    DrawText(out,curSubtitle->text,-1,&subRect,centerflags|DT_NOCLIP|DT_NOPREFIX);
    SelectObject(out,oldobj);
  }

  EndPaint(hwnd,&p);

  return 1;
}

void OverlayVideoOutput::displayFrame(const char *buf, int size, int time) {
  if(!m_parent) return;
  
  DDSURFACEDESC dd={sizeof(dd),};
  if (m_parent->vid_vsync) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
  HRESULT result;
  if ((result=lpddsOverlay->Lock(NULL,&dd,DDLOCK_WAIT,NULL)) != DD_OK) {
    //CT>FUCKO:reenable me (ctrl+alt+del on win2k)
    //if(result==DDERR_SURFACELOST) width=-1; //will try to recreate the surface in the next processData() call
    return;
  }
  if(type==NSV_MAKETYPE('Y','V','1','2')) {
    const YV12_PLANES *planes=(YV12_PLANES *)buf;
    if (uyvy_output||yuy2_output) { // YV12planar->UYVY or YUY2
      unsigned char *o=(unsigned char*)dd.lpSurface;
      const unsigned char *yi=planes->y.baseAddr;
      const unsigned char *ui=planes->u.baseAddr;
      const unsigned char *vi=planes->v.baseAddr;
      int y=height;
      if (flip) o+=dd.lPitch*(height-1);
      while (y>0) {
        int x=width;
        unsigned char *oo=o;
      
        if (uyvy_output) while (x>0) {
          o[0]=*ui++; o[1]=*yi++; o[2]=*vi++; o[3]=*yi++;
          o+=4; x-=2;
        }
        else while (x>0) {
          o[0]=*yi++; o[1]=*ui++; o[2]=*yi++; o[3]=*vi++;
          o+=4; x-=2;
        }
        ui-=width/2;
        vi-=width/2;
        yi+=planes->y.rowBytes-width;
        x=width;
        if (flip) o=oo-dd.lPitch;
        else o+=dd.lPitch-width*2;
        oo=o;
        if (uyvy_output) while (x>0) {
          o[0]=*ui++; o[1]=*yi++; o[2]=*vi++; o[3]=*yi++;
          o+=4; x-=2;
        } else while (x>0) {
          o[0]=*yi++; o[1]=*ui++; o[2]=*yi++; o[3]=*vi++;
          o+=4; x-=2;
        }
        if (flip) o=oo-dd.lPitch;
        else o+=dd.lPitch-width*2;
        ui+=planes->u.rowBytes-(width/2);
        vi+=planes->v.rowBytes-(width/2);
        yi+=planes->y.rowBytes-width;
        y-=2;
      }
    } else { // woo native YV12 copy
      int f=!!flip;
      char *o=(char*)dd.lpSurface+(f*height*dd.lPitch);
      const char *i=(const char*)planes->y.baseAddr;
      int d_o=dd.lPitch;
      if (f) d_o=-d_o;
      else o-=d_o;
  
      int h2=height;
      while (h2--) {
        o+=d_o; memcpy(o,i,width); i+=planes->y.rowBytes;
      }

      d_o/=2;

      int w2=width/2;
      h2=height/2;
      i=(const char*)planes->v.baseAddr;
      o=(char*)dd.lpSurface+(height*dd.lPitch*(f+4))/4;

      if (!f) o-=d_o;
      while (h2--) {
        o+=d_o; memcpy(o,i,w2); i+=planes->v.rowBytes;
      }
      o=(char*)dd.lpSurface+(height*dd.lPitch*(f+5))/4;
      i=(const char*)planes->u.baseAddr;
      h2=height/2;

      if (!f) o-=d_o;
      while (h2--) {
        o+=d_o; memcpy(o,i,w2);i+=planes->u.rowBytes;
      }
    }
  } else if(type==NSV_MAKETYPE('Y','U','Y','2') || type==NSV_MAKETYPE('U','Y','V','Y')) {
    const char *a=buf;
    char *b=(char *)dd.lpSurface;
    int l=width*2,l2=dd.lPitch;
    if(flip) {
      b+=(height-1)*l2;
      l2=-l2;
    }
    int is_uyvy=type==NSV_MAKETYPE('U','Y','V','Y');
    if (uyvy_output && !is_uyvy || (yuy2_output && is_uyvy)) // convert to uyvy
    {
      for(int i=0;i<height;i++) {
        int x=width/2;
        while (x-->0) {
          b[0]=a[1];
          b[1]=a[0];
          b[2]=a[3];
          b[3]=a[2];
          a+=4;
          b+=4;          
        }
        memcpy(b,a,l);
        b+=l2;
        a+=l;
      }
    } else {
      //wee straight YUY2 copy
      for(int i=0;i<height;i++) {
        memcpy(b,a,l);
        b+=l2;
        a+=l;
      }
    }
  }

  lpddsOverlay->Unlock(&dd);

  if (m_parent->osdShowing())
  {
    RECT rs, rd;
    getRects(&rs,&rd);

    HDC hdc;
#if 1 // set both these 1s to 0s to put it back on ryan's mode
    HWND h=m_parent->getHwnd();
    hdc=GetDC(h);
#else
    if (lpddsPrimary->GetDC(&hdc)==DD_OK)
    {
#endif
      m_parent->drawOSD(hdc, &rd);
#if 1
      ReleaseDC(h,hdc);
#else
      lpddsPrimary->ReleaseDC(hdc);
    }
#endif
  }
}

void OverlayVideoOutput::goFullScreen() {
/*  fullscreen_controls = new GuiObjectWnd;
  fullscreen_controls->setContent("video.fullscreen_controls");
  fullscreen_controls->init(m_parent);

  RECT r;
  Std::getViewport(&r,m_parent->gethWnd(),1);

  RECT nr = r;
  nr.top = (int)(r.bottom - (r.bottom - r.top) * 0.15);
  nr.bottom = (int)(r.bottom - (r.bottom - r.top) * 0.05);
  fullscreen_controls->resizeToRect(&nr);
  */
  is_fullscreen=1;
}

void OverlayVideoOutput::removeFullScreen() {
/*  delete fullscreen_controls;
  fullscreen_controls = NULL;*/
  is_fullscreen=0;
}

int OverlayVideoOutput::showOSD() {
//  if (fullscreen_controls != NULL) fullscreen_controls->setVisible(TRUE);

  // enabling the following code will cause the top & bottom OSD bars 
  // to squish the image (instead of crop it):
  /*if(lpddsOverlay) {
    RECT rd,rs;
    getRects(&rs,&rd);
    
    HWND hwnd=m_parent->getHwnd();
    if(GetParent(hwnd)) hwnd=GetParent(hwnd);
    
    RECT temp;
    GetClientRect(hwnd,&temp);
    int bottom_margin = ((temp.bottom-temp.top) - (rd.bottom-rd.top)) / 2;
    int pixels_to_clip = max(0, m_parent->getOSDbarHeight() - bottom_margin);
    rd.bottom -= pixels_to_clip;

    lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
  }*/


  return 1;
}

void OverlayVideoOutput::hideOSD() { 
  //if (fullscreen_controls != NULL) fullscreen_controls->setVisible(FALSE);

  // 1) repaint the OSD area with the overlay color here
  HWND hwnd = m_parent->getHwnd();
  if(GetParent(hwnd)) hwnd=GetParent(hwnd);

  HDC hdc = GetDC(hwnd);
  if (hdc) {
    RECT r;
    GetClientRect(hwnd,&r);
    LOGBRUSH lb={BS_SOLID,RGB(OV_COL_R,OV_COL_G,OV_COL_B),};
    HBRUSH br=CreateBrushIndirect(&lb);
    FillRect(hdc,&r,br);
    DeleteObject(br);

    ReleaseDC(hwnd, hdc);
  }

  // 2) readjust the overlay destination rectangle
  /*if(lpddsOverlay) {
    RECT rd,rs;
    getRects(&rs,&rd);
    lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
  }*/

}

void OverlayVideoOutput::drawSubtitle(SubsItem *item) {
  curSubtitle=item;

  HWND hwnd=m_parent->getHwnd();

  RECT oldrect=subRect;
  GetClientRect(hwnd,&subRect);
 
  if(item) {

    RECT oldwinRect=winRect;
    GetClientRect(hwnd,&winRect);
    if(!subFont || ((winRect.bottom-winRect.top)!=(oldwinRect.bottom-oldwinRect.top)) || m_fontsize!=item->fontSize) {
      if(subFont) DeleteObject(subFont);
      m_fontsize=item->fontSize;
      subFont=CreateFont(14+item->fontSize+18*(winRect.bottom-winRect.top)/768,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial");
    }

    HDC out=GetDC(hwnd);
    SelectObject(out,subFont);
    SIZE s;
    GetTextExtentPoint32(out,item->text,strlen(item->text),&s);
    {
      // calcul for multiline text
      const char *p=item->text;
      int n=0;
      while(*p!=0) if(*p++=='\n') n++;
      if(n) s.cy*=(n+1);
    }

    if (item->xPos > 127) // towards the right
    {
      subRect.right -= ((subRect.right-subRect.left) * (255-item->xPos)) / 256;
    }
    else if (item->xPos < 127)
    {
      subRect.left += ((subRect.right-subRect.left) * item->xPos) / 256;
    }

    subRect.top += ((subRect.bottom-s.cy-subRect.top) * item->yPos)/255;

    subRect.bottom=subRect.top + s.cy;  

    ReleaseDC(hwnd,out);
  }

  //just redraw the correct portion
  InvalidateRect(hwnd,&oldrect,TRUE); 
  InvalidateRect(hwnd,&subRect,TRUE); 
}

void OverlayVideoOutput::resetSubtitle()
{
  curSubtitle=NULL;
  subRect.top=65536;
}