#ifndef _SVC_PLAYER_H
#define _SVC_PLAYER_H

#include <bfc/dispatch.h>

#include <api/service/services.h>

class api_window;

class svc_player {
public:
  static const char *getServiceName() { return "Standard player service"; }
  static int getServiceType() { return WaSvc::UNIQUE; }
  static GUID getServiceGuid() {
    // {9DE79C4A-B9E6-46f4-9AE2-40E0F09CCA51}
    const GUID guid = 
    { 0x9de79c4a, 0xb9e6, 0x46f4, { 0x9a, 0xe2, 0x40, 0xe0, 0xf0, 0x9c, 0xca, 0x51 } };
    return guid;
  }
  virtual void openFile(const char *filename, FOURCC droptarg=MK3CC('d','e','f'))=0;
  virtual void openFiles(api_window *parent=NULL, const char *type=NULL, FOURCC droptarg=MK3CC('d','e','f'))=0;
};

class svc_playerI : public svc_player {
};

#include <api/service/servicei.h>

template <class T>
class PlayerCreator : public waServiceFactoryTSingle<svc_player, T> {
public:
  PlayerCreator() : waServiceFactoryTSingle<svc_player, T>(svc_player::getServiceGuid()) {}
};

#endif
