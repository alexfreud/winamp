#ifndef _ABOUT_H_
#define _ABOUT_H_

#include <math.h>


#pragma warning(disable : 4731)

static HWND about_hwnd;

#ifndef NO_ABOUT_EGG

#define BITMAP_W 100
#define BITMAP_H 64
static HDC m_hdc;
static HBITMAP m_hbm;
static char *m_dib;
static char ge_fbuf[(BITMAP_H+1)*BITMAP_W+1];
static int ge_tmp;
#define M_NUM_FX 9
static int m_effect;
#define BLOBS_NPOINTS 8
static int BLOBPOINTS[4*BLOBS_NPOINTS];
static int fire_textcnt,fire_textpos;
static char *fire_texts[]={
  "nsvplay",
  "Nullsoft 2003-8",
  "",
  "greets to",
  "winamp forums",
  "#nullsoft",
  "britney spears",
  "p.s.",
  "DrO was here",
  " <3",
  "",
};
static HDC scrolldc;
static HBITMAP scrollbitmap;
static char *scrolldib;
static const char scrolltext[]="nullsoft presents you nSVpLAY hidden part!  cracked by rOn   +5 trainer by deadbeef ";
static int scrolloffs;
static char *rototmp;

#endif //NO_ABOUT_EGG

static INT_PTR CALLBACK aboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch (uMsg)
	{
 		case WM_INITDIALOG:
#ifndef NO_ABOUT_EGG
		{
			HBITMAP about_bmp = NULL;
			m_effect=M_NUM_FX-1;

			// try to use the localised image and then revert to the normal dll image
			about_bmp = (HBITMAP)LoadImage((HINSTANCE)lParam,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
			if(about_bmp == NULL){
				about_bmp = (HBITMAP)LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
			}

			// set on control with id of -1 (0xFFFFFFFF) or 0xFFFF (not sure how/why this happened)
			SendDlgItemMessage(hwndDlg,0xFFFF,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)about_bmp);
			SendDlgItemMessage(hwndDlg,-1,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)about_bmp);
		}
#endif
			SetDlgItemText(hwndDlg,IDC_VERSION,WNDMENU_CAPTION);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				case IDOK:
#ifndef MODAL_ABOUT
					DestroyWindow(hwndDlg);
#else
					EndDialog(hwndDlg,1);
#endif
				return FALSE;
			}
			break;

		case WM_DESTROY:
			about_hwnd=NULL;
#ifndef NO_ABOUT_EGG
			if (m_hbm) DeleteObject(m_hbm);
			if (m_hdc) DeleteDC(m_hdc);
			if (scrollbitmap) DeleteObject(scrollbitmap);
			if (scrolldc) DeleteDC(scrolldc);
			m_hbm=0;
			m_hdc=0;
			scrollbitmap=0;
			scrolldc=0;
			if (rototmp) free(rototmp);
			rototmp=NULL;
#endif
			break;
#ifndef NO_ABOUT_EGG
		case WM_LBUTTONDBLCLK :
			//easter eggs :)
			if (++m_effect >= M_NUM_FX) m_effect=1;

			KillTimer(hwndDlg,0x1234);

			if (m_hbm) DeleteObject(m_hbm);
			if (m_hdc) DeleteDC(m_hdc);
			if (scrollbitmap) DeleteObject(scrollbitmap);
			if (scrolldc) DeleteDC(scrolldc);
			scrollbitmap=0;
			scrolldc=0;
			if (rototmp) free(rototmp);
			rototmp=NULL;

			{
				struct
				{
					BITMAPINFO bmi;
					RGBQUAD more_bmiColors[256];
					LPVOID data;
				} m_bitmap;
				m_hdc = CreateCompatibleDC(NULL);
				m_bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				m_bitmap.bmi.bmiHeader.biPlanes = 1;
				m_bitmap.bmi.bmiHeader.biBitCount = 8;
				m_bitmap.bmi.bmiHeader.biCompression = BI_RGB;
				m_bitmap.bmi.bmiHeader.biSizeImage = 0;
				m_bitmap.bmi.bmiHeader.biClrUsed = 256;
				m_bitmap.bmi.bmiHeader.biClrImportant = 256;
				m_bitmap.bmi.bmiHeader.biWidth = BITMAP_W;
				m_bitmap.bmi.bmiHeader.biHeight = -BITMAP_H;
				m_bitmap.bmi.bmiHeader.biSizeImage = BITMAP_W*BITMAP_H;

				memset(ge_fbuf,0,BITMAP_W*(BITMAP_H+1));
				fire_textcnt=0;

				if (m_effect < 3)
				{
					unsigned char *t=(unsigned char *)m_bitmap.bmi.bmiColors;
					int x=255;
					int adj=!!m_effect;
					t[0]=t[1]=t[2]=0;
					t+=4;
					while (x)
					{
						if (m_effect == 2)
						{
							if (x > 128)
							{
								t[0]=0;
								t[1]=((256-x)*2)/3;
								t[2]=(256-x)*2;
							}
							else
							{
								t[0]=256-x*2;
								t[1]=255/3 + ((256-x)*2)/3;
								t[2]=255;
							}
						}
						else
						{
							int a=x*2;
							if (a>255) a=255;
							t[0]=a;
							t[2-adj]=a;
							a+=a;
							if (a > 255) a=255;
							t[1+adj]=a;
						}

						t+=4; 
						x--;
					}
				}

				if(m_effect==0)
				{
					//sine scroll
					struct
					{
						BITMAPINFO bmi;
						RGBQUAD more_bmiColors[1];
						LPVOID data;
					} m_bitmap;
					scrolldc = CreateCompatibleDC(NULL);
					m_bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					m_bitmap.bmi.bmiHeader.biPlanes = 1;
  					m_bitmap.bmi.bmiHeader.biBitCount = 8;
					m_bitmap.bmi.bmiHeader.biCompression = BI_RGB;
					m_bitmap.bmi.bmiHeader.biSizeImage = 0;
					m_bitmap.bmi.bmiHeader.biClrUsed = 1;
					m_bitmap.bmi.bmiHeader.biClrImportant = 1;
					m_bitmap.bmi.bmiHeader.biWidth = 128;
					m_bitmap.bmi.bmiHeader.biHeight = -32;
					m_bitmap.bmi.bmiHeader.biSizeImage = 128*32;
					scrollbitmap = CreateDIBSection(scrolldc,&m_bitmap.bmi,DIB_RGB_COLORS, &m_bitmap.data, NULL, 0);
					SelectObject(scrolldc,scrollbitmap);
					SetBkMode(scrolldc,TRANSPARENT);
					scrolldib = (char *)m_bitmap.data;
					scrolloffs = 0;
				}

				if (m_effect == 1)
				{
					int *t=BLOBPOINTS;
					int a=BLOBS_NPOINTS;
					while (a)
					{
						t[0]=(rand()&127) - 64;
						t[2]=(rand()&127) - 64;
						t[1]=t[3]=a;
						t+=4;
						a--;
					}
				}

				if((m_effect >= 3 && m_effect <= 8)) //rotozooms
				{
					rototmp=(char *)malloc(65536+256);
					//generate noise texture
					memset(rototmp,0xd0,65536+256);
					__asm
					{
						mov cx,0ffffh
						xor ax,ax
						xor ebx,ebx
						xor dx,dx
						mov edi,[rototmp]
					TEXGEN:
						mov	bx,cx
						add	ax,cx
						rol	ax,cl
						mov	dh,al
						sar	dh,5
						adc	dl,dh
						adc	dl,[edi+ebx+255]
						shr	dl,1
						mov	[edi+ebx],dl
						not	bh
						mov	[edi+ebx],dl
						loop TEXGEN
					}

					if ((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ (m_effect==5)) // secondary easter egg, hah!
						for (int x = 0; x < 256*256; x ++) rototmp[x] = 0x40 + ((x^(x>>8)) & 0x1F);

					rototmp[0]=rototmp[1];
					rototmp[0xff00]=rototmp[0xff01];

					//generate palette
					unsigned char *t=((unsigned char *)(&m_bitmap.bmi.bmiColors[0x40]));
					for(int i=0;i<0x20;i++) 
					{
						int r,g,b;
						switch(m_effect)
						{
							case 3: r=i*4; g=i*5; b=i*8; break;
							case 4: r=i*4; g=i*7; b=i*8; break;
							case 5: r=i*8; g=i*2; b=i*2; break;
							case 6: r=i*6; g=i*8; b=i*6; break;
							case 7: r=i*8; g=i*6; b=i*8; break;
							case 8: r=i*6; g=i*6; b=i*8; break;
						}
						t[0]=b;
						t[1]=g;
						t[2]=r;
						t+=4;
					}
				}

				m_hbm = CreateDIBSection(m_hdc,&m_bitmap.bmi,DIB_RGB_COLORS, &m_bitmap.data, NULL, 0);
				SelectObject(m_hdc,m_hbm);
				m_dib = (char *)m_bitmap.data;

				SetTimer(hwndDlg,0x1234,35,NULL);
			}
			break;

		case WM_TIMER:
		{
			int nomemcpy=0;
			static float inc=0;
			inc++;
			if (m_effect == 0) // oldsk00l sine scroll
			{
				double blah=3.14/BITMAP_W;
				double val1=1;
				double val2=(BITMAP_H-16)/2;
				static double vinc=0;
				__asm
				{
					mov edi, offset ge_fbuf
					mov ecx, BITMAP_W*BITMAP_H
					xor eax, eax
					rep stosb
				}

				for(int j=0;j<8;j++) {
					for(int k=0;k<16;k++) {
						int col=255-k*(256/16);
						if(col<128) col=128+(128-col);
						memset(ge_fbuf+(k+(int)((BITMAP_H-16)*(cos(vinc+(j*8)*3.14/50)+1)/2))*BITMAP_W,col,BITMAP_W);
					}
				}

				__asm
				{
					mov edi, offset ge_fbuf
					mov ecx, 0
					mov esi, dword ptr [scrolldib]

				SINELOOP:
					mov [ge_tmp], ecx
					fild dword ptr [ge_tmp]
					fmul qword ptr [blah]
					fadd qword ptr [vinc]
					fcos
					fadd qword ptr [val1]
					fmul qword ptr [val2]
					fistp dword ptr [ge_tmp]
					mov eax, [ge_tmp]
					mov ebx, eax

					mov edx, BITMAP_W
					mul edx

					mov dh, bl
					push ecx
					mov ebx, 0
					mov ecx, 16

				SINELOOP2:
					cmp byte ptr [esi+ebx],0
					je SINECONT

					mov dl,0ffh
					sub dl,dh
					sub dl,cl

					mov [edi+eax], dl
				SINECONT:
					add eax, BITMAP_W
					add ebx, 128
					loop SINELOOP2

					pop ecx

		            inc esi
				    inc edi

					inc ecx
					cmp ecx, BITMAP_W
					jl SINELOOP

					mov edi, [scrolldib]
					mov esi, edi
					inc esi
					mov ebx, 32
				SINESCROLL:
					mov ecx, 127
					rep movsb
					inc edi
					inc esi
					dec ebx
					jnz SINESCROLL
				}
				vinc+=0.2;
				scrolloffs++;
				if((scrolloffs&7)==7)
				{
					int o=scrolloffs/8;
					if(!scrolltext[o]) scrolloffs=0;
					else TextOutA(scrolldc,100,0,&scrolltext[o],1);
				}
			} 
			else if (m_effect == 1) // blobs
			{
				int *blobptr=BLOBPOINTS;
				int i=BLOBS_NPOINTS*2;
				while (i--)
				{
					if (blobptr[0] > 0) blobptr[1]--;
					else blobptr[1]++;

					int a=blobptr[1];
					if (rand()&1) a++;
					else a--;
					blobptr[0]+=a/8;

					blobptr+=2;
				}

				int y=BITMAP_H;
				unsigned char *p=(unsigned char *)ge_fbuf;
				while (y--)
				{
					int x=BITMAP_W;
					while (x--)
					{
						blobptr=BLOBPOINTS;
						i=BLOBS_NPOINTS;
						double sum=0.0;
						while (i--)
						{
							double as=(x-(BITMAP_W/2)) - blobptr[0];
							double bs=(y-(BITMAP_H/2)) - blobptr[2];
							sum+=sqrt(as*as + bs*bs);
							blobptr+=4;
						}
						sum *= 6.0/BLOBS_NPOINTS;
						int a=(int)sum;
						if (a > 0xff) a= 0xff;
						*p++=a;
					}
				}
			}
			else if(m_effect==2) //gayfire
			{
				unsigned char *p=(unsigned char *)ge_fbuf;
				int x;
				unsigned char *t=p + BITMAP_W*BITMAP_H;
				for (x = 0; x < BITMAP_W; x ++)
				{
					int a=*t - 10;
					if ((rand()&0x7) == 7) a+=100;
					if (a < 0) a=0;
					else if (a > 192) a=192;
					*t++=a;//rand()&0xf0;
				}
				int y;
				for (y = 0; y < BITMAP_H; y ++)
				{
					*p++=p[0]/4 + p[BITMAP_W]/2 + p[BITMAP_W+1]/4;

					for (x = 1; x < BITMAP_W-1; x ++)
						*p++=p[0]/4 + p[BITMAP_W]/4 + p[BITMAP_W-1]/4 + p[BITMAP_W+1]/4;

					*p++=p[0]/4 + p[BITMAP_W]/2 + p[BITMAP_W-1]/4;
				}
				if (fire_textcnt-- <= 0)
				{
					memcpy(m_dib,ge_fbuf,BITMAP_W*BITMAP_H);
					SetBkMode(m_hdc,TRANSPARENT);
					SetTextColor(m_hdc,RGB(255,255,255));
					RECT r={0,0,BITMAP_W,BITMAP_H};
					DrawTextA(m_hdc,fire_texts[fire_textpos%(sizeof(fire_texts)/sizeof(fire_texts[0]))],-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
					if (fire_textcnt < -30)
					{
						memcpy(ge_fbuf,m_dib,BITMAP_W*BITMAP_H);
						fire_textpos++;
						fire_textcnt=30;
					}
					else nomemcpy=1;
				}
			}
			else if (m_effect == 3) //rotozoom
			{
				char *p=ge_fbuf;
				static float angle=0;
				for(int j=-32;j<32;j++)
					for(int i=-50;i<50;i++)
					{
						//rotozoom
						double x=(i*cosf(angle)-j*sinf(angle))*(2+cosf(angle*1.4f));
						double y=(i*sinf(angle)+j*cosf(angle))*(2+cosf(angle*1.4f));
						//slime
						x+=cos(angle*x)*4;
						y+=sin(angle*y)*4;
						int x2=(int)x & 0xff;
						int y2=(int)y & 0xff;
						*p++=rototmp[256*y2+x2];
					}
            
				angle+=0.01f;
			} 
			else if (m_effect == 4) //rotozoom 2
			{
				char *p=ge_fbuf;
				double angle=cos(inc*0.01f)*2;
				for(int j=-32;j<32;j++)
					for(int i=-50;i<50;i++)
					{
						//position
						double x=i-cos(inc*0.013f)*64;
						double y=j+sin(inc*0.013f)*64;

						//slime
						x+=cos((angle+i)*50)*cos(angle)*4;
						y+=sin((angle+j)*50)*cos(angle)*4;

						//rotozoom
						double x3=(x*cos(angle)-y*sin(angle))*(2+cos(angle*1.4f));
						double y3=(x*sin(angle)+y*cos(angle))*(2+cos(angle*1.4f));

						int x2=(int)x3 & 0xff;
						int y2=(int)y3 & 0xff;
						*p++=rototmp[256*y2+x2];
					}
				} 
				else if (m_effect == 5) //3d rotozoom
				{
					char *p=ge_fbuf;
					static float angle=0;
					const double b=50;
					for(int j=-32;j<32;j++)
						for(int i=-50;i<50;i++)
						{
							//rotozoom
							double x=(i*cos(angle)+j*sin(angle));//*(2+cos(angle*1.4f));
							double y=(i*sin(angle)-j*cos(angle));//*(2+cos(angle*1.4f));
							//gay z-projection
							x*=b/(((double)j+32));
							y*=b/(((double)j+32));
							//position
							x-=cos(inc*0.013f)*64;
							y+=sin(inc*0.013f)*64;
              
							int x2=(int)x & 0xff;
							int y2=(int)y & 0xff;

							char c=rototmp[256*y2+x2];
							*p++=0x40+((c-0x40)*(j+32)/56);
						}
            
					angle+=0.01f;
					//b++;
				} 
				else if (m_effect == 6) //tunnel
				{
					const double TINYNUM=1.0E-6;
					const double CONE_RADIUS=128;
					const double FOV=120.0;
					#define sqr(a) ((a)*(a))
	    
					char *p=ge_fbuf;
					for(int y=-32;y<32;y++)
						for(int x=-50;x<50;x++)
						{
							double originx=cos(inc*0.025f)*20;
							double originy=sin(inc*0.04f)*20;
							double originz=inc*4;
							double dirx=x/FOV;
							double diry=y/FOV;
							double dirz=1;

							//normalize dir vector
							{
  								double l=1.0f/sqrt(sqr(dirx)+sqr(diry)+sqr(dirz));
								dirx*=l;
								diry*=l;
								dirz*=l;
							}

							//y-axis rotation
							{
								double rot=inc*0.015f;
								double dirx2=dirx*cos(rot)+dirz*sin(rot);
								dirz=dirx*sin(rot)-dirz*cos(rot);
								dirx=dirx2;
							}

							//tunnel algo shit
							double a=sqr(dirx)+sqr(diry);
							double b=2*(originx*dirx + originy*diry);
							double c=sqr(originx)+sqr(originy)-sqr(CONE_RADIUS);
							double delta=sqrt(sqr(b)-(4*a*c));

							double t1=(-b+delta)/(2*a+TINYNUM);
							double t2=(-b-delta)/(2*a+TINYNUM);

							double t=t1>0?t1:t2;

							double intx=originx+dirx*t;
							double inty=originy+diry*t;
							double intz=originz+dirz*t;

							//tex. coords
							int u=(int)(fabs(intz)*0.6);
							int v=(int)(fabs(atan2(inty,intx)*256/3.14159265));

							//depth
							t=20000.0/t;
							int z=(int)(t>63?63:t);

							u&=0xff;
							v&=0xff;
							z&=0xff;

							{
								char c=rototmp[256*u+v];
								*p++=0x40+((c-0x40)*z/64);
							}
					}
				} 
				else if(m_effect==7) //washing machine
				{
					char *p=ge_fbuf;
					for(int j=-32;j<32;j++)
						for(int i=-50;i<50;i++)
						{
							double dist=sqrt(double(sqr(i)+sqr(j))); // pythagoras rules :)
							double angle=cos(dist*0.05f)*(cos(inc*0.1f)) + inc*0.07f;
							//rotozoom
							double x=(i*cos(angle)-j*sin(angle));
							double y=(i*sin(angle)+j*cos(angle));
							int x2=(int)x & 0xff;
							int y2=(int)y & 0xff;
							*p++=rototmp[256*y2+x2];
						}
				}
				else if(m_effect==8) //reflection-like(?) effect
				{
					char *p=ge_fbuf;
					for(int j=-32;j<32;j++)
						for(int i=-50;i<50;i++)
						{
							double dist=sqrt(double(sqr(i)+sqr(j)));
							double zoom=cos(dist*0.05f)*(cos(inc*0.02f)*8)+1;
							//rotozoom
							double x=i*zoom+inc;
							double y=j*zoom+inc;
							int x2=(int)x & 0xff;
							int y2=(int)y & 0xff;
							*p++=rototmp[256*x2+y2];
						}
				}

				if (!nomemcpy) memcpy(m_dib,ge_fbuf,BITMAP_W*BITMAP_H);
				if (hwndDlg != NULL)
				{
					HDC h = GetDC(hwndDlg);
					BitBlt(h, 11, 11, BITMAP_W, BITMAP_H, m_hdc, 0, 0, SRCCOPY);
					ReleaseDC(hwndDlg, h);
				}
			}
			break;
#endif
	}
	return 0;
}

static void do_about(HWND hwnd, HINSTANCE hinst) {
#ifndef MODAL_ABOUT
	if(about_hwnd) {
		SetForegroundWindow(about_hwnd);
		return;
	}
	about_hwnd=CreateDialogParam((!hinst?g_hInstance:hinst),MAKEINTRESOURCE(IDD_ABOUT),hwnd,aboutProc,(LPARAM)hinst);
	ShowWindow(about_hwnd,SW_SHOW);
#else
#ifdef LOC_MODAL_ABOUT
	WASABI_API_DIALOGBOXPARAMW(IDD_ABOUT,hwnd,aboutProc,(LPARAM)hinst);
#else
	DialogBoxParam((!hinst?g_hInstance:hinst),MAKEINTRESOURCE(IDD_ABOUT),hwnd,aboutProc,(LPARAM)hinst);
#endif
#endif
}

#endif//_ABOUT_H_