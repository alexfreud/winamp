#ifndef _FILETYPES_H
#define _FILETYPES_H

#include <api/config/items/cfgitemi.h>

#define FILETYPES_PARENT CfgItemI
class Filetypes : public FILETYPES_PARENT {
public:
  Filetypes();
  void registerAttributes();
  
  static int isRegistered(const char *ext);
  static void registerExtension(const char *ext, int reg);
  static int isCdPlayer();
  static void registerCdPlayer(int reg);

  void updateKeepers();

private:

  static void createWinampTypes();
  static void regmimetype(const char *mtype, const char *programname, const char *ext, int nsonly);

  // workaround for (damn) c++ type checking
  static LONG RegQueryValueEx(HKEY hKey, LPTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, char *lpData, LPDWORD lpcbData) {
#ifdef WIN32
    return ::RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, (unsigned char *)lpData, lpcbData);
#endif
  }
  static LONG RegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, char *lpData, DWORD cbData) {
#ifdef WIN32
    return ::RegSetValueEx(hKey, lpValueName, Reserved, dwType, (const unsigned char *)lpData, cbData);
#endif
  }
  static LONG RegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, const char *lpData, DWORD cbData) {
#ifdef WIN32
    return ::RegSetValueEx(hKey, lpValueName, Reserved, dwType, (const unsigned char *)lpData, cbData);
#endif
  }
public:
  static LONG myRegDeleteKeyEx(HKEY thiskey, LPCTSTR lpSubKey);

  static int whichicon, whichicon2, addtolist;
};

#endif
