#include "video.h"
#include <multimon.h>
#include "subtitles.h"

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

DDrawVideoOutput::DDrawVideoOutput() {
  lpDD=NULL;
  lpddsOverlay=NULL;
  lastresizerect.bottom=0;
  lastresizerect.top=0;
  lastresizerect.left=0;
  lastresizerect.right=0;

  lpddsPrimary=NULL;
  lpddsClipper=NULL;
  lpddsSTTemp=NULL;
  is_fullscreen=0;
  m_parent=NULL;
  initing=false;
  needchange=0;
  m_palette=NULL;
  m_lastsubtitle=NULL;
  sttmp_w=sttmp_h=0;
  subFont=NULL;
  m_sub_needremeasure=0;
  m_fontsize=0;
  memset(&winRect,0,sizeof(winRect));
}

DDrawVideoOutput::~DDrawVideoOutput() {
//  LPDIRECTDRAWSURFACE o=lpddsOverlay;
  lpddsOverlay=NULL;
//  if(o) o->Release();
 // if (lpddsSTTemp) lpddsSTTemp->Release();
  //if(lpddsPrimary) lpddsPrimary->Release();
//  if(lpddsClipper) lpddsClipper->Release();
  if (lpDD) lpDD->Release();	// BU added NULL check in response to talkback
  if(subFont) DeleteObject(subFont);
  if (is_fullscreen) removeFullScreen();
}

void DDrawVideoOutput::drawSubtitle(SubsItem *item)
{
  m_lastsubtitle=item;
  m_sub_needremeasure=1;
}

int DDrawVideoOutput::create(VideoOutput *parent, int w, int h, unsigned int ptype, int flipit, double aspectratio) {
  m_lastsubtitle=NULL;
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

  HRESULT v=-1;
  DDSURFACEDESC DDsd={sizeof(DDsd),};
  lpddsPrimary->GetSurfaceDesc(&ddsd);
  DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT; //create the surface at screen depth
	DDsd.dwWidth=w;
	DDsd.dwHeight=h;
	DDsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	if (parent->vid_ddraw) v=lpDD->CreateSurface(&DDsd, &lpddsOverlay, NULL);
  if(!parent->vid_ddraw || FAILED(v)) {
    // fall back to system memory if video mem doesn't work
    DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
  	v=lpDD->CreateSurface(&DDsd, &lpddsOverlay, NULL);
  }
  if(FAILED(v)) {
    // this video card sucks then :)
    lpddsOverlay=NULL;
    initing=false;
    return 0;
  }

  // get the depth
  m_depth=8;
  INIT_DIRECTDRAW_STRUCT(m_ddpf);
  if(lpddsOverlay->GetPixelFormat(&m_ddpf)>=0) {
    m_depth=m_ddpf.dwRGBBitCount;
    if (m_depth==16 && m_ddpf.dwGBitMask==0x03e0) m_depth=15;
  }

  lpDD->CreateClipper(0,&lpddsClipper,NULL);
  lpddsClipper->SetHWnd(0,hwnd);
  lpddsPrimary->SetClipper(lpddsClipper);
  initing=false;
  return 1;
}

int DDrawVideoOutput::onPaint(HWND hwnd, HDC hdc) {
  return 0;
}

void DDrawVideoOutput::displayFrame(const char *buf, int size, int time) {
  DDSURFACEDESC dd={sizeof(dd),};
  if (m_parent->vid_vsync) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
  HRESULT result;
  if ((result=lpddsOverlay->Lock(NULL,&dd,DDLOCK_WAIT,NULL)) != DD_OK) {
    needchange=1;
    return;
  }
  if(type==NSV_MAKETYPE('Y','V','1','2')) {
    const YV12_PLANES *planes=(YV12_PLANES *)buf;
    // convert yv12 to rgb
    int bytes = m_depth >> 3; 
    if(m_depth==15) bytes=2;
    int i, j, y00, y01, y10, y11, u, v;
	  unsigned char *pY = (unsigned char *)planes->y.baseAddr;
    unsigned char *pU = (unsigned char *)planes->u.baseAddr;
	  unsigned char *pV = (unsigned char *)planes->v.baseAddr;
	  unsigned char *pOut = (unsigned char*)dd.lpSurface;
   	const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int addOut=dd.lPitch*2-width*bytes;
    int yrb=planes->y.rowBytes;
    int addL=dd.lPitch;

    /* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
    #define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

    if(flip) {
      pOut+=(dd.lPitch)*(height-1);
      addOut=-dd.lPitch*2 - width*bytes;
      addL=-addL;
    }

    for (j = 0; j <= height - 2; j += 2) {
      for (i = 0; i <= width - 2; i += 2) {
			  y00 = *pY;
			  y01 = *(pY + 1);
			  y10 = *(pY + yrb);
			  y11 = *(pY + yrb + 1);
			  u = (*pU++) - 128;
			  v = (*pV++) - 128;

        {
      	  int r, g, b;

	        g = guScale * v + gvScale * u;
    		  r = buScale * v;
    		  b = rvScale * u;

	        y00 *= yScale; y01 *= yScale;
	        y10 *= yScale; y11 *= yScale;

          switch(m_depth) {
          case 15:
            {
              unsigned short *rgb=(unsigned short *)pOut;
              rgb[0]=((LIMIT(r+y00)>>3)<<10)|((LIMIT(g+y00)>>3)<<5)|(LIMIT(b+y00)>>3);
              rgb[1]=((LIMIT(r+y01)>>3)<<10)|((LIMIT(g+y01)>>3)<<5)|(LIMIT(b+y01)>>3);
              rgb+=addL/2;
              rgb[0]=((LIMIT(r+y10)>>3)<<10)|((LIMIT(g+y10)>>3)<<5)|(LIMIT(b+y10)>>3);
              rgb[1]=((LIMIT(r+y11)>>3)<<10)|((LIMIT(g+y11)>>3)<<5)|(LIMIT(b+y11)>>3);
            }
            break;
          case 16:
            {
              unsigned short *rgb=(unsigned short *)pOut;
              rgb[0]=((LIMIT(r+y00)>>3)<<11)|((LIMIT(g+y00)>>2)<<5)|(LIMIT(b+y00)>>3);
              rgb[1]=((LIMIT(r+y01)>>3)<<11)|((LIMIT(g+y01)>>2)<<5)|(LIMIT(b+y01)>>3);
              rgb+=addL/2;
              rgb[0]=((LIMIT(r+y10)>>3)<<11)|((LIMIT(g+y10)>>2)<<5)|(LIMIT(b+y10)>>3);
              rgb[1]=((LIMIT(r+y11)>>3)<<11)|((LIMIT(g+y11)>>2)<<5)|(LIMIT(b+y11)>>3);
            }
            break;
          case 24:
            {
              unsigned char *rgb=pOut;
              /* Write out top two pixels */
		          rgb[0] = LIMIT(b+y00); rgb[1] = LIMIT(g+y00); rgb[2] = LIMIT(r+y00);
		          rgb[3] = LIMIT(b+y01); rgb[4] = LIMIT(g+y01); rgb[5] = LIMIT(r+y01);

		          /* Skip down to next line to write out bottom two pixels */
		          rgb += addL;
		          rgb[0] = LIMIT(b+y10); rgb[1] = LIMIT(g+y10); rgb[2] = LIMIT(r+y10);
              rgb[3] = LIMIT(b+y11); rgb[4] = LIMIT(g+y11); rgb[5] = LIMIT(r+y11);
            }
            break;
          case 32:
            {
              unsigned char *rgb=pOut;
              /* Write out top two pixels */
		          rgb[0] = LIMIT(b+y00); rgb[1] = LIMIT(g+y00); rgb[2] = LIMIT(r+y00);
		          rgb[4] = LIMIT(b+y01); rgb[5] = LIMIT(g+y01); rgb[6] = LIMIT(r+y01);

		          /* Skip down to next line to write out bottom two pixels */
		          rgb += addL;
		          rgb[0] = LIMIT(b+y10); rgb[1] = LIMIT(g+y10); rgb[2] = LIMIT(r+y10);
              rgb[4] = LIMIT(b+y11); rgb[5] = LIMIT(g+y11); rgb[6] = LIMIT(r+y11);
            }
            break;
          }
        }
	
			  pY += 2;
			  pOut += 2 * bytes;
      }
		  pY += yrb+yrb-width;
		  pU += planes->u.rowBytes-width/2;
		  pV += planes->v.rowBytes-width/2;
		  pOut += addOut;
	  }
  } else if(type==NSV_MAKETYPE('R','G','3','2')) {
    //FUCKO: do we need to support 8bits depth?
    switch(m_depth) {
    case 15: 
      { // convert RGB32 -> RGB16 (555)
        const char *a=buf;
        char *b=(char *)dd.lpSurface;
        int l=width*4,l2=dd.lPitch;
        int ladj=l;
        if (flip) { a+=l*(height-1); ladj=-ladj; }
        for(int i=0;i<height;i++) {
          short *dest=(short *)b;
          int *src=(int *)a;
          for(int j=0;j<width;j++) {
            int c=*(src++);
            int r=c>>16;
            int g=(c>>8) & 0xff;
            int b=(c) & 0xff;
            *(dest++)=((r>>3)<<10)|((g>>3)<<5)|(b>>3);
          }
          a+=ladj; b+=l2;
        }
      }
      break;
    case 16:
      { // convert RGB32 -> RGB16
        //FUCKO: this assumes 565
        const char *a=buf;
        char *b=(char *)dd.lpSurface;
        int l=width*4,l2=dd.lPitch;
        int ladj=l;
        if (flip) { a+=l*(height-1); ladj=-ladj; }
        for(int i=0;i<height;i++) {
          short *dest=(short *)b;
          int *src=(int *)a;
          for(int j=0;j<width;j++) {
            //FUCKO: optimize here
            int c=*(src++);
            int r=c>>16;
            int g=(c>>8) & 0xff;
            int b=(c) & 0xff;
            *(dest++)=((r>>3)<<11)|((g>>2)<<5)|(b>>3);
          }
          a+=ladj; b+=l2;
        }
      }
      break;
    case 24:
      { // convert RGB32 -> RGB24
        const char *a=buf;
        char *b=(char *)dd.lpSurface;
        int l=width*4,l2=dd.lPitch;
        int ladj=l;
        if (flip) { a+=l*(height-1); ladj=-ladj; }
        for(int i=0;i<height;i++) {
          char *dest=(char *)b;
          int *src=(int *)a;
          for(int j=0;j<width;j++) {
            //FUCKO: optimize here
            int c=*(src++);
            int r=c>>16;
            int g=(c>>8) & 0xff;
            int b=(c) & 0xff;
            *dest++=b;
            *dest++=g;
            *dest++=r;
          }
          a+=ladj; b+=l2;
        }
      }
      break;
    case 32:
      { // straight RGB32 copy
        const char *a=buf;
        char *b=(char *)dd.lpSurface;
        int l=width*4,l2=dd.lPitch;
        int ladj=l;
        if (flip) { a+=l*(height-1); ladj=-ladj; }
        for(int i=0;i<height;i++) {
          memcpy(b,a,l);
          a+=ladj; b+=l2;
        }
      }
      break;
    }
  } else if(type==NSV_MAKETYPE('Y','U','Y','2') || type==NSV_MAKETYPE('U','Y','V','Y')) {
    const char *a=buf;
    char *b=(char *)dd.lpSurface;
    int l=width*2,l2=dd.lPitch;
    if(flip) {
      b+=(height-1)*l2;
      l2=-l2;
    }
    switch(m_depth) {
    case 15:
      {
        // yuy2->rgb16 (555) conversion
        unsigned char *src=(unsigned char *)buf;
        unsigned short *dst=(unsigned short *)dd.lpSurface;
  	    int line, col;//, linewidth;
        int y, yy;
        int u, v;
        int vr, ug, vg, ub;
        unsigned char *py, *pu, *pv;

        //linewidth = width - (width >> 1);
        py = src;
        pu = src + 1;
        pv = src + 3;

        int pitchadd=dd.lPitch/2-width;

        for (line = 0; line < height; line++) {
 		      for (col = 0; col < width; col++) {
            #undef LIMIT
            #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

 		        y = *py;
	          yy = y << 8;
	          u = *pu - 128;
	          ug =   88 * u;
	          ub =  454 * u;
	          v = *pv - 128;
	          vg =  183 * v;
	          vr =  359 * v;
                
            unsigned char b=LIMIT(yy + ub     );
            unsigned char g=LIMIT(yy - ug - vg);
            unsigned char r=LIMIT(yy +      vr);
            *(dst++)=((r>>3)<<10)|((g>>3)<<5)|(b>>3);
    
            py += 2;
			      if ( (col & 1) == 1) {
    			    pu += 4; // skip yvy every second y
			        pv += 4; // skip yuy every second y
			      }
          } // ..for col 
          dst+=pitchadd;
        } /* ..for line */          
      }
      break;
    case 16:
      {
        // yuy2->rgb16 conversion
        //FUCKO: only supports 565
        unsigned char *src=(unsigned char *)buf;
        unsigned short *dst=(unsigned short *)dd.lpSurface;
  	    int line, col;//, linewidth;
        int y, yy;
	      int u, v;
        int vr, ug, vg, ub;
        unsigned char *py, *pu, *pv;

        //linewidth = width - (width >> 1);
        py = src;
        pu = src + 1;
        pv = src + 3;

        int pitchadd=dd.lPitch/2-width;

        for (line = 0; line < height; line++) {
 		      for (col = 0; col < width; col++) {
            #undef LIMIT
            #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

		        y = *py;
	          yy = y << 8;
	          u = *pu - 128;
	          ug =   88 * u;
	          ub =  454 * u;
	          v = *pv - 128;
	          vg =  183 * v;
	          vr =  359 * v;
                
            unsigned char b=LIMIT(yy + ub     );
            unsigned char g=LIMIT(yy - ug - vg);
            unsigned char r=LIMIT(yy +      vr);
            *(dst++)=((r>>3)<<11)|((g>>2)<<5)|(b>>3);
    
            py += 2;
			      if ( (col & 1) ) {
    			    pu += 4; // skip yvy every second y
			        pv += 4; // skip yuy every second y
			      }
		      } // ..for col 
          dst+=pitchadd;
	      } /* ..for line */          }
      break;
    case 24:
      {
        // yuy2->rgb24 conversion
        unsigned char *src=(unsigned char *)buf;
        unsigned char *dst=(unsigned char *)dd.lpSurface;
  	    int line, col;//, linewidth;
        int y, yy;
        int u, v;
        int vr, ug, vg, ub;
        unsigned char *py, *pu, *pv;

        //linewidth = width - (width >> 1);
        py = src;
        pu = src + 1;
        pv = src + 3;

        int pitchadd=dd.lPitch-(width*3);

        for (line = 0; line < height; line++) {
 		      for (col = 0; col < width; col++) {
            #undef LIMIT
            #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

		        y = *py;
	          yy = y << 8;
	          u = *pu - 128;
	          ug =   88 * u;
	          ub =  454 * u;
	          v = *pv - 128;
	          vg =  183 * v;
	          vr =  359 * v;
                
            *(dst++)=LIMIT(yy + ub     );
            *(dst++)=LIMIT(yy - ug - vg);
            *(dst++)=LIMIT(yy +      vr);
    
	          py += 2;
            if ( (col & 1) == 1) {
    			    pu += 4; // skip yvy every second y
			        pv += 4; // skip yuy every second y
			      }
		      } // ..for col 
          dst+=pitchadd;
	      } /* ..for line */          }
      break;
    case 32:
      {
        // yuy2->rgb32 conversion
        unsigned char *src=(unsigned char *)buf;
        unsigned char *dst=(unsigned char *)dd.lpSurface;
   	    int line, col;//, linewidth;
        int y, yy;
        int u, v;
        int vr, ug, vg, ub;
        unsigned char *py, *pu, *pv;

        //linewidth = width - (width >> 1);
        py = src;
        pu = src + 1;
        pv = src + 3;

        int pitchadd=dd.lPitch-(width*4);

        for (line = 0; line < height; line++) {
 		      for (col = 0; col < width; col++) {
            #undef LIMIT
            #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

 		        y = *py;
	          yy = y << 8;
	          u = *pu - 128;
	          ug =   88 * u;
	          ub =  454 * u;
	          v = *pv - 128;
	          vg =  183 * v;
	          vr =  359 * v;
                
		        *dst++ = LIMIT(yy + ub     ); // b
	          *dst++ = LIMIT(yy - ug - vg); // g
	          *dst++ = LIMIT(yy +      vr); // r
            dst++;
 
            py += 2;
			      if ( (col & 1) == 1) {
    			    pu += 4; // skip yvy every second y
			        pv += 4; // skip yuy every second y
			      }
		      } // ..for col 
          dst+=pitchadd;
	      } /* ..for line */
      }
      break;
    }
  } else if(type==NSV_MAKETYPE('R','G','2','4')) {
    //FUCKO: only ->RGB32 conversion supported
    switch(m_depth) {
    case 32:
    {
      const char *a=buf;
      char *b=(char *)dd.lpSurface;
      int l=width,l2=dd.lPitch;
      int ladj=l*3;
      if (flip) { a+=(l*3)*(height-1); ladj=-(ladj+l*3); }
      l2-=l*4;
      for(int i=0;i<height;i++) {
        //memcpy(b,a,l);
        for(int j=0;j<l;j++) {
          b[0]=a[0];
          b[1]=a[1];
          b[2]=a[2];
          b+=4; a+=3;
        }
        a+=ladj; b+=l2;
      }
    }
    break;
    }
  } else if(type==NSV_MAKETYPE('R','G','B','8') && m_palette) {
    unsigned char *d=(unsigned char *)dd.lpSurface;
    int pitch=dd.lPitch;
    unsigned char *src=(unsigned char *)buf;
    int newwidth=(width+3)&0xfffc;
    src+=newwidth*height-1;
    for(int j=0;j<height;j++) {
      switch(m_depth) {
      case 15:
      case 16:
        {
          unsigned short *dest=(unsigned short *)d;
          for(int i=0;i<newwidth;i++) {
            unsigned char c=src[-newwidth+1+i];
            RGBQUAD *rgb=&m_palette[c];
            switch(m_depth) {
              case 15: *(dest++)=((rgb->rgbRed>>3)<<10)|((rgb->rgbGreen>>3)<<5)|(rgb->rgbBlue>>3); break;
              case 16: *(dest++)=((rgb->rgbRed>>3)<<11)|((rgb->rgbGreen>>2)<<5)|(rgb->rgbBlue>>3); break;
            }
          }
        }
        break;
      case 24:
      case 32:
        {
          unsigned char *dest=d;
          for(int i=0;i<newwidth;i++) {
            unsigned char c=src[-newwidth+1+i];
            RGBQUAD *rgb=&m_palette[c];
            *dest++=rgb->rgbBlue;
            *dest++=rgb->rgbGreen;
            *dest++=rgb->rgbRed;
            if(m_depth==32) dest++;
          }
        }
        break;
      }
      d+=pitch;
      src-=newwidth;
    }
  }

  lpddsOverlay->Unlock(&dd);


  RECT r;
  HWND hwnd=m_parent->getHwnd();
  if (!IsWindow(hwnd)) return;

  if(GetParent(hwnd)) hwnd=GetParent(hwnd);

  GetClientRect(hwnd,&r);
  RECT fullr=r;
  m_parent->adjustAspect(r);
  if (r.left != lastresizerect.left || r.right != lastresizerect.right || r.top != lastresizerect.top || 
        r.bottom != lastresizerect.bottom)
  {
    if (r.left != 0)
    {
      RECT tmp={0,0,r.left,fullr.bottom};
      InvalidateRect(hwnd,&tmp,TRUE); 
    }
    
    if (r.right != fullr.right) 
    {
      RECT tmp={r.right,0,fullr.right,fullr.bottom};
      InvalidateRect(hwnd,&tmp,TRUE); 
    }
    if (r.top != 0) 
    {
      RECT tmp={r.left,0,r.right,r.top};
      InvalidateRect(hwnd,&tmp,TRUE); 
    }
    if (r.bottom != fullr.bottom) 
    {
      RECT tmp={r.left,r.bottom,r.right,fullr.bottom};
      InvalidateRect(hwnd,&tmp,TRUE); 
    }

    lastresizerect=r;
  }

  ClientToScreen(hwnd,(LPPOINT)&r);
  ClientToScreen(hwnd,((LPPOINT)&r) + 1);

  // transform coords from windows desktop coords (where 0,0==upper-left corner of box encompassing all monitors)
  // to the coords for the monitor we're displaying on:
  r.left-=m_mon_x;
  r.right-=m_mon_x;
  r.top-=m_mon_y;
  r.bottom-=m_mon_y;

  HDC hdc = NULL;
  HDC inhdc = NULL;

  RECT srcrect;
  RECT *pSrcRect = NULL;

  if (m_parent->osdShowing() && m_parent->osdReady())
  {
    // squish image upward to make room for the OSD.
    int vert_margin = ((fullr.bottom-fullr.top) - (r.bottom-r.top)) / 2;
    int pixels_to_clip = max(0, m_parent->getOSDbarHeight() - vert_margin);

    // adjust source rectangle:
    int src_y0 = (int)(height*pixels_to_clip/(float)(r.bottom-r.top) + 0.5f);
    int src_y1 = height - src_y0;
    SetRect(&srcrect, 0, SHOW_STREAM_TITLE_AT_TOP ? src_y0 : 0, width, src_y1);
    pSrcRect = &srcrect;

    // adjust destination rectangle:
    r.bottom -= pixels_to_clip;
#if (SHOW_STREAM_TITLE_AT_TOP)
    r.top += pixels_to_clip;
#endif
  }

  int needst=0;


  SubsItem *mlst=m_lastsubtitle;
  if (mlst)
  { 
    int curw=r.right-r.left, curh=r.bottom-r.top;
    if (!lpddsSTTemp || sttmp_w != curw || sttmp_h != curh)
    {
      if (lpddsSTTemp) lpddsSTTemp->Release();
      lpddsSTTemp=0;

      HRESULT v=-1;
      DDSURFACEDESC DDsd={sizeof(DDsd),};
      DDSURFACEDESC   ddsd;
      INIT_DIRECTDRAW_STRUCT(ddsd);
      ddsd.dwFlags = DDSD_CAPS;
      ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
      lpddsPrimary->GetSurfaceDesc(&ddsd);
      DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT; //create the surface at screen depth
	    DDsd.dwWidth=sttmp_w=curw;
	    DDsd.dwHeight=sttmp_h=curh;
	    DDsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	    if (m_parent->vid_ddraw) v=lpDD->CreateSurface(&DDsd, &lpddsSTTemp, NULL);
      if (!m_parent->vid_ddraw || FAILED(v)) {
        // fall back to system memory if video mem doesn't work
        DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
  	    v=lpDD->CreateSurface(&DDsd, &lpddsSTTemp, NULL);
      }
      m_sub_needremeasure=1;
    }
    if (lpddsSTTemp) needst=1;
  }

  if (needst)
  {
    HDC tmpdc=NULL;
    if (!m_parent->vid_ddraw || lpddsSTTemp->Blt(NULL,lpddsOverlay,NULL,DDBLT_WAIT,0) != DD_OK) {
      // as a last resort, BitBlt().
      HDC tmpdc2;
      if (lpddsOverlay->GetDC(&tmpdc2)==DD_OK) {
        if (lpddsSTTemp->GetDC(&tmpdc)==DD_OK) {
          BitBlt(tmpdc,0,0,sttmp_w,sttmp_h,tmpdc2,0,0,SRCCOPY);
        }
      }
    }

    if (tmpdc||lpddsSTTemp->GetDC(&tmpdc)==DD_OK)
    {
      int m_lastsubxp=mlst->xPos;
      int m_lastsubyp=mlst->yPos;

      RECT oldwinRect=winRect;
      GetClientRect(hwnd,&winRect);
      if(!subFont || ((winRect.bottom-winRect.top)!=(oldwinRect.bottom-oldwinRect.top)) || m_fontsize!=mlst->fontSize) {
        if(subFont) DeleteObject(subFont);
        m_fontsize=mlst->fontSize;
        subFont=CreateFont(14+m_fontsize+18*(winRect.bottom-winRect.top)/768,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial");
      }

      HWND hwnd=m_parent->getHwnd();
      HGDIOBJ oldobj=SelectObject(tmpdc,subFont);

      int centerflags=0;
      if (m_lastsubxp < 127) centerflags |= DT_LEFT;
      else if (m_lastsubxp > 127) centerflags |= DT_RIGHT;
      else centerflags |= DT_CENTER;

      if (m_lastsubyp < 127) centerflags |= DT_TOP;
      else if (m_lastsubyp > 127) centerflags |= DT_BOTTOM;

      if (m_sub_needremeasure)
      {
        subRect=r;
        subRect.bottom-=subRect.top;
        subRect.right -=subRect.left;
        subRect.top=subRect.left=0;

        SIZE s;
        GetTextExtentPoint32(tmpdc,mlst->text,strlen(mlst->text),&s);

        // calcul for multiline text
          const char *p=mlst->text;
          int n=0;
          while(*p!=0) if(*p++=='\n') n++;
          if(n) s.cy*=(n+1);
 
        if (m_lastsubxp > 127) // towards the right
        {
          subRect.right -= ((subRect.right-subRect.left) * (255-m_lastsubxp)) / 256;
        }
        else if (m_lastsubxp < 127)
        {
          subRect.left += ((subRect.right-subRect.left) * m_lastsubxp) / 256;
        }

        subRect.top += ((subRect.bottom-s.cy-subRect.top) * m_lastsubyp)/255;

        subRect.bottom=subRect.top + s.cy;  
      }
  
      SetBkMode(tmpdc,TRANSPARENT);
  
      // draw outline
      SetTextColor(tmpdc,RGB(0,0,0));
      int y=1;
      int x=1;
      RECT r2={subRect.left+x,subRect.top+y,subRect.right+x,subRect.bottom+y};
      DrawText(tmpdc,mlst->text,-1,&r2,centerflags|DT_NOCLIP|DT_NOPREFIX);
      // draw text
      SetTextColor(tmpdc,RGB(mlst->colorRed,mlst->colorGreen,mlst->colorBlue));
      DrawText(tmpdc,mlst->text,-1,&subRect,centerflags|DT_NOCLIP|DT_NOPREFIX);
      SelectObject(tmpdc,oldobj);
      lpddsSTTemp->ReleaseDC(tmpdc);
    }
    if (!m_parent->vid_ddraw || lpddsPrimary->Blt(&r,lpddsSTTemp,pSrcRect,DDBLT_WAIT,0) != DD_OK) {
      // as a last resort, BitBlt().
      if (lpddsSTTemp->GetDC(&inhdc)==DD_OK) {
        if (lpddsPrimary->GetDC(&hdc)==DD_OK) {
          int src_w = width;
          int src_h = pSrcRect ? (pSrcRect->bottom - pSrcRect->top) : height;
          if (r.right-r.left == src_w && r.bottom-r.top == src_h) 
            BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,inhdc,0,0,SRCCOPY);
          else 
            StretchBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,inhdc,0,0,src_w,src_h,SRCCOPY);
        }
      }
    }
  }
  else
  {
    if (!m_parent->vid_ddraw || lpddsPrimary->Blt(&r,lpddsOverlay,pSrcRect,DDBLT_WAIT,0) != DD_OK) {
      // as a last resort, BitBlt().
      if (lpddsOverlay->GetDC(&inhdc)==DD_OK) {
        if (lpddsPrimary->GetDC(&hdc)==DD_OK) {
          int src_w = width;
          int src_h = pSrcRect ? (pSrcRect->bottom - pSrcRect->top) : height;
          if (r.right-r.left == src_w && r.bottom-r.top == src_h) 
            BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,inhdc,0,0,SRCCOPY);
          else 
            StretchBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,inhdc,0,0,src_w,src_h,SRCCOPY);
        }
      }
    }
  }

#if 0 //faster style
  if (m_parent->osdShowing())
  {
    if (hdc || lpddsPrimary->GetDC(&hdc)==DD_OK)
      m_parent->drawOSD(hdc, &r);
  }
#endif

  if (hdc) { lpddsPrimary->ReleaseDC(hdc); hdc = NULL; }
  if (inhdc) { lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL; }

#if 1 // safer style
  if (m_parent->osdShowing())
  {
    HWND h=m_parent->getHwnd();
    hdc=GetDC(h);
    m_parent->drawOSD(hdc, &r);
    ReleaseDC(h,hdc);
  }
#endif
}

void DDrawVideoOutput::goFullScreen() {
}

void DDrawVideoOutput::removeFullScreen() {
}

void DDrawVideoOutput::timerCallback() {
}

int DDrawVideoOutput::showOSD() { 
  return 1;
}

void DDrawVideoOutput::hideOSD() { 
  // repaint the client area, to black, where there is no video
  // (otherwise the OSD might be left painted there)

  RECT r;
  HWND hwnd=m_parent->getHwnd();
  if(GetParent(hwnd)) hwnd=GetParent(hwnd);
  GetClientRect(hwnd,&r);

  HDC hdc = GetDC(hwnd);
  if (hdc) {
    HGDIOBJ oldobj1=SelectObject(hdc,CreateSolidBrush(RGB(0,0,0)));
    HGDIOBJ oldobj2=SelectObject(hdc,CreatePen(PS_SOLID,0,RGB(0,0,0)));
    int margin = ((r.bottom - r.top) - (lastresizerect.bottom - lastresizerect.top) + 1) / 2;
    Rectangle(hdc,r.left,r.top,r.right,r.top + margin);
    Rectangle(hdc,r.left,r.bottom - margin,r.right,r.bottom);
    margin = ((r.right - r.left) - (lastresizerect.right - lastresizerect.left) + 1) / 2;
    Rectangle(hdc,r.left,r.top,r.left + margin,r.bottom);
    Rectangle(hdc,r.right - margin,r.top,r.right,r.bottom);
    DeleteObject(SelectObject(hdc,oldobj2));
    DeleteObject(SelectObject(hdc,oldobj1));
    
    ReleaseDC(hwnd, hdc);
  }
}

void DDrawVideoOutput::resetSubtitle()
{
  m_lastsubtitle=0;
}