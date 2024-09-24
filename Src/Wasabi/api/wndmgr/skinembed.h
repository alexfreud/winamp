#ifndef __SKIN_EMBEDDER_H
#define __SKIN_EMBEDDER_H

#include <bfc/nsguid.h>
#include <bfc/string/bfcstring.h>
#include <bfc/string/StringW.h>
#include <bfc/ptrlist.h>
#include <bfc/depend.h>
#include <api/timer/timerclient.h>

#define CB_DESTROYCONTAINER 0x887

class ifc_window;
class WindowHolder;
class Container;
class Layout;

class SkinEmbedEntry {
  public:
  SkinEmbedEntry(api_dependent *d, ifc_window *w, GUID g, const wchar_t *gid, const wchar_t *prefered_container, int container_flag, Container *c, WindowHolder *wh) : groupid(gid), guid(g), dep(d), wnd(w), required(container_flag), layout(prefered_container), container(c), wndholder(wh) { }
  virtual ~SkinEmbedEntry() { }

  StringW groupid;
  GUID guid;
  api_dependent *dep;
  ifc_window *wnd;
  int required;
  StringW layout;
  Container *container;
  WindowHolder *wndholder;
};

class SkinEmbedder : public DependentViewerI, public TimerClientDI {
  public:

    SkinEmbedder();
    virtual ~SkinEmbedder();

    int toggle(GUID g, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *r=NULL, int transcient=0);
    int toggle(const wchar_t *groupid, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *r=NULL, int transcient=0);
    ifc_window *create(GUID g, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *r=NULL, int transcient=0, int starthidden=0, int *isnew=NULL);
    ifc_window *create(const wchar_t *groupid, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *r=NULL, int transcient=0, int starthidden=0, int *isnew=NULL);
    void destroy(ifc_window *w, RECT *r=NULL);
    int getNumItems(GUID g);
    int getNumItems(const wchar_t *groupid);
    ifc_window *enumItem(GUID g, int n);
    ifc_window *enumItem(const wchar_t *groupid, int n);
    WindowHolder *getSuitableWindowHolder(GUID g, const wchar_t *group_id, Container *cont, Layout *lay, int visible, int dynamic, int empty, int has_self, int autoavail);
    void registerWindowHolder(WindowHolder *w);
    void unregisterWindowHolder(WindowHolder *w);
    void destroyContainer(Container *o);
    virtual int timerclient_onDeferredCallback(intptr_t param1, intptr_t param2);
    virtual void timerclient_timerCallback(int id);
#ifdef WASABI_COMPILE_CONFIG
    void restoreSavedState();
    void saveState();
#endif
    void attachToSkin(ifc_window *w, int side, int size);

    virtual int viewer_onItemDeleted(api_dependent *item);
    static void cancelDestroyContainer(Container *c);

  private:
    ifc_window *create(GUID g, const wchar_t *groupid, const wchar_t *prefered_container=NULL, int container_flag=0, RECT *r=NULL, int transcient=0, int starthidden=0, int *isnew=NULL);

    PtrList<WindowHolder> wndholders;
    PtrList<SkinEmbedEntry> inserted;
    PtrList<SkinEmbedEntry> allofthem;
    static PtrList<SkinEmbedEntry> in_deferred_callback;
    static PtrList<Container> cancel_deferred_destroy;
    static PtrList<Container> deferred_destroy;
};

extern SkinEmbedder *skinEmbedder;

#endif
