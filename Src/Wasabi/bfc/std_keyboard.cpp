#include "precomp_wasabi_bfc.h"
#include "std_keyboard.h"

int Std::keyDown(int code) 
{
#ifdef WIN32
  return !!(GetKeyState(code) & 0x8000);
#elif defined(LINUX)
  if ( code == MK_RBUTTON || code == MK_LBUTTON ) {
    Window t1, t2;
    int rx, ry, wx, wy;
    unsigned int buttons;
    
    XQueryPointer( Linux::getDisplay(), Linux::RootWin(), &t1, &t2,
                   &rx, &ry, &wx, &wy, &buttons );
    
    if ( code == MK_RBUTTON )
      return buttons & Button3Mask;
    else
      return buttons & Button1Mask;
  }
  
  int code1 = XKeysymToKeycode( Linux::getDisplay(), code & 0xFFFF );
  int code2 = XKeysymToKeycode( Linux::getDisplay(), (code>>16) & 0xFFFF );
  
  char keys_return[32] = {0};
  XQueryKeymap( Linux::getDisplay(), keys_return );
  
  if ( code1 && code2 )
    return (keys_return[ (code1 >> 3) & 31 ] & (1 << (code1 & 7))) ||
      (keys_return[ (code2 >> 3) & 31 ] & (1 << (code2 & 7)));
  
  return (keys_return[ (code1 >> 3) & 31 ] & (1 << (code1 & 7)));
#else
  return 0;
#warning port me!
#endif
}

// TODO: add async flag to be able to choose between GetKeyState/GetAsyncKeyState (win32) GetCurrentKeyModifiers/GetCurrentEventkeyModifiers (mac)
bool Std::keyModifier(int code)
{
#ifdef WIN32
  return !!(GetKeyState(code) & 0x8000);
#elif defined(__APPLE__)
  return GetCurrentKeyModifiers() & code;
#elif defined(LINUX)
#error port me 
#endif
}

