#ifndef __SYSTRAY_H
#define __SYSTRAY_H

#include <bfc/string/StringW.h>

class Systray 
{
public: 
  Systray(HWND wnd, int uid, int msg, HICON smallicon);
  ~Systray();
  void setTip(const wchar_t *tip);
private:
  bool addIcon();
  bool deleteIcon();
  bool setTip();
  int id, message;
  HWND hwnd;
  HICON icon;
  StringW tip;
};


#endif
