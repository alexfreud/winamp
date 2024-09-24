#ifndef _DD_H_
#define _DD_H_

class C_DD
{
public:
  C_DD();
  char *open(int w, int h, HWND hwnd); // returns error string on error, or NULL on success
  void close(void);
  int lock(unsigned char **input, unsigned char **output);
  void unlock(void);
  void setpalette(unsigned char palette[768], unsigned int time_ms);
  int palette_fadeleft(void); // returns how long til palette fade will be done
private:
  LPDIRECTDRAWPALETTE g_ddp;
  LPDIRECTDRAW g_lpDD;
  LPDIRECTDRAWSURFACE g_lpDDSPrim,g_lpPrimSurfBack;
  LPDIRECTDRAWSURFACE g_RenderSurface[2];
  void *g_RenderSurfaceLocked[2];
  unsigned char g_palette_seek[768];
  unsigned char g_palette_orig[768];
  unsigned char g_palette_last[768];
  int g_pal_lastv;
  int cursurface;
  unsigned int g_palette_starttime, g_palette_endtime;
};

#endif // _DD_H_