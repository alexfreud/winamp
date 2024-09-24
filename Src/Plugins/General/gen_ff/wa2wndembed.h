#ifndef __WA2WNDEMBED_H
#define __WA2WNDEMBED_H

#include "../winamp/wa_ipc.h"
#include <api/service/svcs/svc_wndcreate.h>
#include <bfc/depview.h>
#include <bfc/reentryfilter.h>
#include <api/wndmgr/appcmds.h>
#include <api/skin/widgets/xuioswndhost.h>
#include <api/script/objects/c_script/h_layout.h>
#include <api/syscb/callbacks/wndcb.h>
#include "wa2pldirobj.h"

class BucketItem;
class XuiOSWndHost;
class Wa2BucketItem;

#define EMBED_STATE_EXTRA_LINK 0
#define EMBED_STATE_EXTRA_ATTACHED 1
#define EMBED_STATE_EXTRA_HOSTCOUNT 61
#define EMBED_STATE_EXTRA_REPARENTING 62
#define EMBED_STATE_EXTRA_FFROOTWND 63

//-----------------------------------------------------------------------------------------------
// {E6323F86-1724-4cd3-9D87-70591FC16E5E}
static const GUID playerWndGuid = 
{ 0xe6323f86, 0x1724, 0x4cd3, { 0x9d, 0x87, 0x70, 0x59, 0x1f, 0xc1, 0x6e, 0x5e } };
// benski> don't use this in a windowholder, this is for <container primarycontent="guid:main"/>


//-----------------------------------------------------------------------------------------------
// {45F3F7C1-A6F3-4ee6-A15E-125E92FC3F8D}
static const GUID pleditWndGuid = 
{ 0x45f3f7c1, 0xa6f3, 0x4ee6, { 0xa1, 0x5e, 0x12, 0x5e, 0x92, 0xfc, 0x3f, 0x8d } };

//-----------------------------------------------------------------------------------------------
// {F0816D7B-FFFC-4343-80F2-E8199AA15CC3}
static const GUID videoWndGuid = 
{ 0xf0816d7b, 0xfffc, 0x4343, { 0x80, 0xf2, 0xe8, 0x19, 0x9a, 0xa1, 0x5c, 0xc3 } };

#ifdef MINIBROWSER_SUPPORT

//-----------------------------------------------------------------------------------------------
// {CF477C3E-FDC8-44a2-9066-58D0184D47A8}
static const GUID minibrowserWndGuid = 
{ 0xcf477c3e, 0xfdc8, 0x44a2, { 0x90, 0x66, 0x58, 0xd0, 0x18, 0x4d, 0x47, 0xa8 } };

#endif
// scan 
static const GUID embedWndGuids = 
{ 0x00000000, 0xf000, 0x44a2, { 0x90, 0x66, 0x58, 0xd0, 0x18, 0x4d, 0x47, 0xa8 } };

// {A8533CEC-D05D-45b8-A617-E2B7F2C2CF82}
static const GUID embeddedWndStateGuid = 
{ 0xa8533cec, 0xd05d, 0x45b8, { 0xa6, 0x17, 0xe2, 0xb7, 0xf2, 0xc2, 0xcf, 0x82 } };

// {6B0EDF80-C9A5-11d3-9F26-00C04F39FFC6}
static const GUID library_guid = 
{ 0x6b0edf80, 0xc9a5, 0x11d3, { 0x9f, 0x26, 0x0, 0xc0, 0x4f, 0x39, 0xff, 0xc6 } };

// {B397A4CE-455A-4d62-8BF6-D0F91ACB70E6} 
static const GUID preferences_guid = 
{ 0xb397a4ce, 0x455a, 0x4d62, { 0x8b, 0xf6, 0xd0, 0xf9, 0x1a, 0xcb, 0x70, 0xe6 } };

// {0000000A-000C-0010-FF7B-01014263450C}
static const GUID avs_guid = 
{ 10, 12, 16, { 255, 123, 1, 1, 66, 99, 69, 12 } };

// {8DDA9D48-B915-4320-A888-831A1D837516}
static const GUID about_guid = 
{ 0x8dda9d48, 0xb915, 0x4320, { 0xa8, 0x88, 0x83, 0x1a, 0x1d, 0x83, 0x75, 0x16 } };

// {D6201408-476A-4308-BF1B-7BACA1124B12}
static const GUID lightning_bolt_guid = 
{ 0xd6201408, 0x476a, 0x4308, { 0xbf, 0x1b, 0x7b, 0xac, 0xa1, 0x12, 0x4b, 0x12 } };

// {53DE6284-7E88-4c62-9F93-22ED68E6A024}
static const GUID colorthemes_guid = 
{ 0x53de6284, 0x7e88, 0x4c62, { 0x9f, 0x93, 0x22, 0xed, 0x68, 0xe6, 0xa0, 0x24 } };


//-----------------------------------------------------------------------------------------------
class WaOsWndHost : public XuiOSWndHost
{
  public:
    WaOsWndHost() : transparencysafe(1), transfer(0) {}
    virtual void onBeforeReparent(int i);
    virtual void onAfterReparent(int i);
    virtual int wantHideOnUnhost() { return 1; }
    virtual int wantFocus();
    virtual int onGetFocus();
    virtual int handleTransparency() { return transparencysafe; }
    virtual void setNoTransparency() { transparencysafe = 0; }
    void setTransfering(int i) { transfer = i; }
    int isTransfering() { return transfer; }
    virtual int onMouseWheelUp(int click, int lines);
    virtual int onMouseWheelDown(int click, int lines);
  private:
    int transparencysafe;
    int transfer;
};

//-----------------------------------------------------------------------------------------------
class VideoLayoutMonitor : public H_Layout
{
  public:
    VideoLayoutMonitor(ScriptObject *o) : H_Layout(o) { }
    VideoLayoutMonitor() {}
    virtual void hook_onResize(int x, int y, int w, int h);
    virtual void hook_onMove();
};

//-----------------------------------------------------------------------------------------------
class EmbedEntry 
{
  public:
    WaOsWndHost *host;
    ifc_dependent *dep;
    intptr_t whichwnd;
    AppCmds *cmds;
    VideoLayoutMonitor *monitor;
    GUID g;
};

class WndStatus 
{
  public:
  int wndcode; // or -1
  HWND wnd;
  int visible;
  RECT position;
};

//-----------------------------------------------------------------------------------------------
class Wa2WndEmbed : public svc_windowCreateI, DependentViewerTPtr<ifc_window>, public WndCallbackI
{
  public:
    Wa2WndEmbed();
    virtual ~Wa2WndEmbed();

    static const char *getServiceName() { return "Playlist Editor window creator"; }

    virtual int testGuid(GUID g);
    virtual ifc_window *createWindowByGuid(GUID g, ifc_window *parent);
    virtual int testType(const wchar_t *windowtype);
    virtual ifc_window *createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n);
    virtual int destroyWindow(ifc_window *w);
    virtual int viewer_onEvent(ifc_window *item, int event, intptr_t param, void *ptr, size_t ptrlen);

    static void rememberVisibleWindows();
    static void restoreVisibleWindows();
    static int hadRememberedWndVisible(HWND wnd);
    static int embedRememberProc(embedWindowState *p, embedEnumStruct *parms);

    virtual int onShowWindow(Container *c, GUID guid, const wchar_t *groupid);
    virtual int onHideWindow(Container *c, GUID guid, const wchar_t *groupid);

    PtrList<Wa2BucketItem> bucketitems;
    PtrList<EmbedEntry> wndhosts;
    PtrList<PlDirObject> pldirs;
    static PtrList<WndStatus> wa2wndstatus;
    static int switching_holder;

};

extern ReentryFilterObject wndMsgFilter;

//-----------------------------------------------------------------------------------------------
class PlaylistAppCmds : public AppCmdsI 
{
  public:
    PlaylistAppCmds();
    virtual ~PlaylistAppCmds() {}
    virtual void appcmds_onCommand(int id, const RECT *buttonRect, int which_button);

  enum {
    PL_ADD=0,
    PL_REM,
    PL_SEL,
    PL_MISC,
    PL_LIST,
  };

protected:
	CmdRec addCmd, remCmd, selCmd, miscCmd, listCmd;
};

#ifdef MINIBROWSER_SUPPORT

//-----------------------------------------------------------------------------------------------
class MinibrowserAppCmds : public AppCmdsI {
  public:
    MinibrowserAppCmds();
    virtual ~MinibrowserAppCmds() {}
    virtual void appcmds_onCommand(int id, const RECT *buttonRect, int which_button);

  enum {
    MB_BACK=0,
    MB_FORWARD,
    MB_STOP,
    MB_RELOAD,
    MB_MISC,
  };
};
#endif

//-----------------------------------------------------------------------------------------------
class VideoAppCmds : public AppCmdsI {
  public:
    VideoAppCmds();
    virtual ~VideoAppCmds() {}
    virtual void appcmds_onCommand(int id, const RECT *buttonRect, int which_button);

  enum {
    VID_FULLSCREEN=0,
    VID_1X,
    VID_2X,
    VID_LIB,
    VID_MISC,
  };
};

#endif