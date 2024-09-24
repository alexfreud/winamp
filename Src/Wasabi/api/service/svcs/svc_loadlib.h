#ifndef _LOADLIB_H
#define _LOADLIB_H

//UNDER CONSTRUCTION

class NOVTABLE svc_loadLib 
{
public:
  int isMine(const char *filename);

  int load(const char *filename);
  void unload();
  void *getProcAddress(const char *name);
};

#endif
