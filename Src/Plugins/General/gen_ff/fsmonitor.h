#ifndef _FS_MONITOR_H
#define _FS_MONITOR_H

#include <bfc/ptrlist.h>
#include <api/timer/timerclient.h>

class FSCallback {
public:
  virtual void onGoFullscreen()=0;
  virtual void onCancelFullscreen()=0;
};

class FullScreenMonitor : public TimerClientDI {
public:
  FullScreenMonitor();
  virtual ~FullScreenMonitor();

  void registerCallback(FSCallback *cb);
  void unregisterCallback(FSCallback *cb);

  int isFullScreen() { return m_fs; }

  int wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void timerclient_timerCallback(int id);

private:
  void onGoFullscreen();
  void onCancelFullscreen();
  void sendGoFSCallbacks();
  void sendCancelFSCallbacks();
  PtrList<FSCallback> m_callbacks;
  HWND hWnd;
  int m_fs;
  int m_go_fs_timer_set;
  int m_cancel_fs_timer_set;
};

#endif
