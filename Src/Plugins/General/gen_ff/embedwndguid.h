#ifndef _EMBEDWNDGUID_H
#define _EMBEDWNDGUID_H

#include "../winamp/wa_ipc.h"

class EmbedWndGuid 
{
  public:
    EmbedWndGuid(EmbedWndGuid *wg);
    EmbedWndGuid(embedWindowState *ws);
    GUID getGuid() { return guid; }
    embedWindowState *getEmbedWindowState() { return ws; }
    void setGuid(GUID g) { guid = g; }
    HWND getHWND() { return hwnd; }
    void setHWND(HWND w) { hwnd = w; }


  private:
    GUID guid;
    embedWindowState *ws;
    HWND hwnd;
};

class EmbedWndGuidMgr
{ 
  public:
    GUID getGuid(EmbedWndGuid *wg);
    GUID getGuid(embedWindowState *ws);
    embedWindowState *getEmbedWindowState(GUID g);
    int testGuid(GUID g);
    void retireEmbedWindowState(embedWindowState *ws);
    int getNumWindowStates();
    GUID enumWindowState(int n, embedWindowState **ws=NULL);

  private:
    PtrList<EmbedWndGuid> table;

};

extern EmbedWndGuidMgr embedWndGuidMgr;

#endif // _EMBEDWNDGUID_H