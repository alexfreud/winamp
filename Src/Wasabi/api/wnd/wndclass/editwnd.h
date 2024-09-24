//NONPORTABLE
#ifndef _EDITWND_H
#define _EDITWND_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <tataki/color/skinclr.h>
#include <api/wnd/usermsg.h>
#include <bfc/common.h>

#define EDITWND_PARENT GuiObjectWnd
class EditWnd : public EDITWND_PARENT {
public:
  EditWnd(wchar_t *buffer=NULL, int buflen=0);
  virtual ~EditWnd();

  virtual int onInit();
  virtual int onPaint(Canvas *canvas);
  virtual int onResize();
#ifdef WIN32
  virtual LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

  // mig: Made these virtual to allow to be accessed by 
  // EditWndString object in editwndstring.h
  virtual void setBuffer(wchar_t *buffer, int len);
  virtual void getBuffer(wchar_t *outbuf, int len);

  virtual const wchar_t *getBufferPtr() { return outbuf; }
  virtual int getBufferLength() { return maxlen; }
  virtual void setBackgroundColor(ARGB32 c);
  virtual void setTextColor(ARGB32 c);

  void setModal(int modal);	//if modal, deletes self on enter
  void setAutoEnter(int a);	//fake an onEnter event when lose focus
  int getAutoEnter() { return autoenter; }
  void setAutoSelect(int a);	//true==grab the focus on init
  void setIdleTimerLen(int ms);	// how many ms keys are idle before send msg
  virtual void onSetVisible(int show);
  virtual int onGetFocus();
  virtual int wantFocus();
  virtual void setWantFocus(int w) { wantfocus = w; }
  virtual void selectAll();
  virtual void enter();
  virtual void setIdleEnabled(int i) { idleenabled = i; }
  virtual int getIdleEnabled() { return idleenabled; }

  void setBorder(int border);
  int getTextLength();
  
  HWND getEditWnd();
  virtual int handleRatio() { return 0; }
  virtual int getAutoSelect() { return autoselect; }

  void setMultiline(int ml);
  void setReadOnly(int ro);
  void setPassword(int pw);
  void setAutoHScroll(int hs);
  void setAutoVScroll(int vs);
  void setVScroll(int vs);
  int isEditorKey(int vk);
  virtual void invalidate();

  virtual int gotFocus();

  // the wndproc for the edit box
  virtual LRESULT editWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
  virtual void timerCallback(int id);

  // call down on these if you override them
  virtual void onEditUpdate();
  virtual void onIdleEditUpdate();
  virtual int onEnter();	// user hit enter.. return 1 to close window
  virtual int onAbort();	// user hit escape.. return 1 to close window
  virtual int onLoseFocus();	// different from onKillFocus() from BaseWnd!

  void setStyle(LONG style, int set);

#ifdef LINUX
  virtual int onLeftButtonDown( int x, int y );
  virtual int onLeftButtonUp( int x, int y );
  virtual int onMouseMove( int x, int y );
  virtual int onKeyDown(int key);
#endif

private:
#ifdef LINUX
  int textposFromCoord( int x, int y );
#endif

  HWND editWnd;
  WNDPROC prevWndProc;
  int maxlen;
  int retcode;
  int idletimelen;
  int modal;
  int bordered;
  int autoenter;
  int beforefirstresize;
  int autoselect;
  int multiline;
  int readonly;
  int password;
  int idleenabled;
  int autohscroll,autovscroll,vscroll;
  int nextenterfaked;
  SkinColor backgroundcolor, textcolor, selectioncolor;
#ifdef LINUX
  int selstart, selend;
  int cursorpos;
  int selectmode;
  int viewstart;
#endif
#ifdef WIN32
  HBRUSH oldbrush;
#endif

  // Basically, we're redoing the functionality of EditWndString 
  // (the bigger version), so we'll probably erase EditWndString 
  // completely as an object.
  MemBlock<wchar_t> buffer8;
  wchar_t *outbuf;
  int wantfocus;
#ifdef LINUX
  StringW inbuf;
#endif
};

#define EDITWND_RETURN_NOTHING	0	// user didn't do nothing
#define EDITWND_RETURN_OK	1	// user hit return
#define EDITWND_RETURN_CANCEL	2	// user hit escape or something

#endif
