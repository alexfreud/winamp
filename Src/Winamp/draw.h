#ifndef NULLSOFT_DRAWH
#define NULLSOFT_DRAWH

#include <windows.h>
#include "Main.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_DRAW
#define setSrcBM(x) _setSrcBM(x,#x)
#define unsetSrcBM() _setSrcBM(0, 0)
#else
#define setSrcBM(x) _setSrcBM(x)
#define unsetSrcBM() _setSrcBM(0)
#endif

#define OSD_TEXT_SIZE 28

#ifdef DEBUG_DRAW
void _setSrcBM(HBITMAP hbm, char *a);
#else
void _setSrcBM(HBITMAP hbm);
#endif

#define update_rect(r) update_area((r).left,(r).top,(r).right-(r).left,(r).bottom-(r).top)
void update_area(int x1, int y1, int w, int h);

HBITMAP draw_LBitmap(LPCTSTR bmname, const wchar_t *filename);
HDC draw_GetWindowDC(HWND hwnd);
int draw_ReleaseDC(HWND hwnd, HDC hdc);
void getXYfromChar(wchar_t ic, int *x, int *y);
void do_palmode(HDC hdc);

extern COLORREF mfont_bgcolor, mfont_fgcolor;
extern unsigned char *specData;
extern int sa_safe;
extern int disable_skin_borders;
extern int mfont_height;
extern int g_has_deleted_current;
extern volatile int draw_initted;

extern HFONT font, mfont, shadefont, osdFontText;
extern HBRUSH selbrush, normbrush, mfont_bgbrush;
extern HBITMAP fontBM, embedBM, panBM, shufflerepeatBM, tbBM,
               cbuttonsBM, volBM,mainBM2, oldmainBM2, numbersBM,
			   numbersBM_ex, playpauseBM, posbarBM, monostereoBM;
extern HDC bmDC, mainDC, specDC, mainDC2;
extern CRITICAL_SECTION g_mainwndcs, g_srcdccs;
extern int titlebar_font_offsets[26];
extern int titlebar_font_widths[26];
extern int titlebar_font_num_offsets[12];
extern int titlebar_font_num_widths[12];
extern int titlebar_font_unknown_width;
extern int updateen;
#ifdef __cplusplus
}
#endif

#endif