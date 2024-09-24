#ifndef NULLSOFT_WASABI_STD_KEYBOARD_H
#define NULLSOFT_WASABI_STD_KEYBOARD_H

#ifdef _WIN32
#include <windows.h>
enum
{
  STDKEY_SHIFT = VK_SHIFT,
  STDKEY_ALT = VK_MENU,
  STDKEY_CONTROL = VK_CONTROL,
  
    STDKEY_UP = VK_UP,
  STDKEY_DOWN = VK_DOWN,
  STDKEY_LEFT = VK_LEFT,
  STDKEY_RIGHT = VK_RIGHT,
  
  STDKEY_HOME = VK_HOME,
  STDKEY_END = VK_END,
};

#elif defined(__APPLE__)
#include <Carbon/Carbon.h>
enum
{
  STDKEY_SHIFT = shiftKey,
  STDKEY_ALT = cmdKey, // yes, I know the option call has "alt" written on it, but Mac programs use Apple key like windows Alt key
  STDKEY_CONTROL = controlKey,
  
    STDKEY_UP = kUpArrowCharCode,
  STDKEY_DOWN = kDownArrowCharCode,
  STDKEY_LEFT = kLeftArrowCharCode,
  STDKEY_RIGHT = kRightArrowCharCode,
  
    STDKEY_HOME = kHomeCharCode,
  STDKEY_END = kEndCharCode,
};

#else
#error port me
#endif
namespace Std
{
  int keyDown(int code);
  bool keyModifier(int code);
}
#endif