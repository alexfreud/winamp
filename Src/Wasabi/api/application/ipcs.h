#ifndef _IPCS_H
#define _IPCS_H

// same value as winamp 2.x
#define WM_WA_IPC WM_USER

#ifdef LINUX
// Stolen from a webpage, I think Linux's is higher, but to be safe...
#define IPC_MSGMAX  4056

struct wa_msgbuf {
  long mtype;
  int paramlen;
  char param[IPC_MSGMAX - 4];
};
#endif

namespace IpcsCommand {
  enum {
#ifdef WASABI_API_MEDIACORE
    // wa2.x IPC messages
    IPC_PLAYFILE=100,
#endif

#ifdef WASABI_API_COMPONENT
    // wa3 IPC messages
    IPC_COMMANDLINE=1000,
#endif
  };
};

class IpcsPtr {
public:
#ifdef WIN32
  IpcsPtr(HWND hwnd);
#else
  IpcsPtr(int qid);
#endif

  void sendWasabiCommand(int command, void *param=0, int paramlen=0);
  void sendWasabiCommand(int command, const char *param);
  
  void moveToForeground();

private:
#ifdef WIN32
  HWND hwnd;
#else
  int qid;
#endif
};

class Ipcs {
public:
  static IpcsPtr *getOtherWasabiInstance();

  static int onIpcsMessage(int command, void *param, int paramlen);
};

#endif
