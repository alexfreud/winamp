#ifndef _COMBOSKIN_H
#define _COMBOSKIN_H

#include <windows.h>

//class ScrollWnd;

class ComboSkin
{
public:
  ComboSkin(HWND hwnd);
  ~ComboSkin();

  HWND m_hwnd;
  WNDPROC m_old_wndproc;
};

#endif