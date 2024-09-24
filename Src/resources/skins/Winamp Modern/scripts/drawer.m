// -----------------------------------------------------------------------
// Generic Video/Vis Application Drawer, by Nullsoft.
//
// Please #include this script, and override the appropriate events 
// (see end of file), rather than modifying this script into your own 
// version. 
//
// *You should not have to edit this file*, it's just a bad idea, period. 
// If you need something that is not supported in this version, we 
// recommend that you contact Nullsoft to suggest the feature.
//
// Satisfying user experience depends on *fully working* scripts, if you 
// insist on taking this file and modifying it for yourself, be sure to 
// *thoroughly* test its behavior once you are done.
//
// If you do add a feature, please contact us so that your extention can 
// be made available to others without each skin developper making its 
// own (potentially broken) implementation.
//
// Note: this script requires mc 1.1.2+ to compile, and wa5.8+ to run.
// -----------------------------------------------------------------------

#ifndef included
#error This script can only be compiled as a #include
#endif

#include <lib/std.mi>
#include <lib/config.mi>
#include <lib/winampconfig.mi>

// call these -- the first two are mandatory
Function initDrawer(Layout lay, String id); // call in your onScriptLoaded();
Function shutdownDrawer();                  // call in your onScriptUnloading();
Function openDrawer();                      // opens the drawer to the last page unless video plays, in which case it opens to it. does animations according to attribs
Function openDrawerForVideo();              // opens the drawer to the video page, does animations according to attribs
Function openDrawerForVis();                // opens the drawer to the vis page, does animations according to attribs
Function openDrawerForNothing();            // opens the drawer without putting anything in it, does animations according to attribs
Function closeDrawer();                     // closes the drawer, does animations according to attribs
Function detachVis();
Function attachVis();
Function detachVideo();
Function attachVideo();
Function switchToVis();
Function switchToVideo();
Function Int getDrawerState();              // returns OPEN or CLOSED
Function Int getDrawerContent();            // returns CONTENT_VIDEO, CONTENT_VIS or CONTENT_NOTHING
Function maximizeWindow();
Function restoreWindow();
Function Int isDrawerToTop();               // returns 1 if the drawer will open to the top or will close from the top, rather than the normal to/from bottom
// whenever the main window is resized while its drawer is closed, you should compute a new layout 
// height for the next opening of the drawer, this will avoid opening to a gigantic height after 
// closing a big video and resizing the player horizontally. return -1 if you do not want this feature
Function Int getDrawerOpenAutoHeight(int layoutwidth); 

// implement these -- mandatory
Function WindowHolder getVideoWindowHolder();
Function WindowHolder getVisWindowHolder();
Function Int getDrawerClosedHeight();
Function Int getDefaultDrawerOpenHeight();

// override these -- optional
Function onBeforeOpeningDrawer();
Function onBeforeClosingDrawer();
Function onDoneOpeningDrawer();
Function onDoneClosingDrawer();
Function onShowVis();
Function onHideVis();
Function onShowVideo();
Function onHideVideo();
Function onAttachVideo();
Function onDetachVideo();
Function onAttachVis();
Function onDetachVis();
Function onBeforeMaximize();
Function onAfterMaximize();
Function onBeforeRestore();
Function onAfterRestore();
Function onCancelMaximize();

// bind these -- mandatory (they don't have to be exposed in the menus)
Global ConfigAttribute __drawer_directiontop_attrib;
Global ConfigAttribute __scrolldrawerattrib;
Global ConfigAttribute __drawer_directionbypass_attrib;
Global ConfigAttribute __vis_detach_attrib;
Global ConfigAttribute __video_detach_attrib;

// -----------------------------------------------------------------------

#define VIDEO_GUID "{F0816D7B-FFFC-4343-80F2-E8199AA15CC3}"
#define VIS_GUID "{0000000A-000C-0010-FF7B-01014263450C}"

// this is used to temporarilly disable playback stop on video window close, in case it's set
#define SKINTWEAKS_CFGPAGE "{0542AFA4-48D9-4c9f-8900-5739D52C114F}"

// this is used to handle video auto fullscreen on play when the video window is attached to the drawer
#define VIDEO_CONFIG_GROUP "{2135E318-6919-4bcf-99D2-62BE3FCA8FA6}"

#define DEBUG

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0
#ifdef TRUE
#undef TRUE
#endif
#define TRUE -1

#define CLOSED 0
#define OPEN   1

#define DIRECTION_NONE    0
#define DIRECTION_OPENING 1
#define DIRECTION_CLOSING 2

#define CONTENT_NOTHING 0
#define CONTENT_VIDEO   1
#define CONTENT_VIS     2

#define DETACHED_VIS   1
#define DETACHED_VIDEO 2

// avoid calling these functions directly. if you do so, be sure to know what
// you're doing, and to test your script thoroughly.

Function drawer_expandWindow(int withdrawer);
Function drawer_reduceWindow(int withdrawer);
Function drawer_showWindowContent();
Function drawer_hideWindowContent();
Function drawer_hideVis();
Function drawer_showVis();
Function drawer_hideVideo();
Function drawer_showVideo();
Function drawer_dc_showVis();
Function drawer_dc_showVideo();
Function drawer_dc_hideVis();
Function drawer_dc_hideVideo();
Function drawer_dc_linkup_showVis();
Function drawer_dc_linkup_showVideo();
Function drawer_doDetachVis();
Function drawer_doAttachVis();
Function drawer_doDetachVideo();
Function drawer_doAttachVideo();
Function drawer_disablePSOVC();
Function drawer_enablePSOVC();
Function drawer_linkup_showVis();
Function drawer_linkup_showVideo();
Function drawer_doMaximizeWindow(int notif);

Global Int __drawer_direction;

Global Timer __callbackTimer;
Global Int __callback_vis_show, __callback_video_show, __callback_vis_hide, __callback_video_hide;
Global Timer __callbackTimer2;
Global Int __callback2_what;
Global Timer __PSOVCTimer;
Global Int __bypasscancel;
Global Int __isinited;
Global Int __play_auto_fs_video;

Global Int __hiding_video, __hiding_vis, __showing_vis, __showing_video;
Global Int __last_forcedbottom, __last_forcedtop;
Global Timer __tempDisable;

Global Layout __main;
Global Container __maincontainer;

Global String __myname;
Global Int __windowshade_openvid;
Global Int __windowshade_openvis;

Global int __maximized;
Global int __oldx,__oldy,__oldw,__oldh;

// -----------------------------------------------------------------------
initDrawer(Layout lay, String name) {
    // todo: test all attribs assigned

    __isinited = 0;
    __play_auto_fs_video = 0;

    __main = lay;
    __maincontainer = __main.getContainer();

    if (name == "") __myname = "Drawer";
    else __myname = name;
    __drawer_direction = DIRECTION_NONE;

    drawer_hideVis();
    drawer_hideVideo();

    __callbackTimer = new Timer;
    __callbackTimer.setDelay(1);
    __callbackTimer2 = new Timer;
    __callbackTimer2.setDelay(1);
    __PSOVCTimer = new Timer;
    __PSOVCTimer.setDelay(1000);
    __tempDisable = new Timer;
    __tempDisable.setDelay(50);

    __maximized = getPrivateInt("winamp5", __myname+"Maximized", 0);
    if (__maximized) {
      onBeforeMaximize();
      onAfterMaximize();
    }

    __oldx=getPrivateInt("winamp5", __myname+"ox", 0);
    __oldy=getPrivateInt("winamp5", __myname+"oy", 0);
    __oldw=getPrivateInt("winamp5", __myname+"ow", 0);
    __oldh=getPrivateInt("winamp5", __myname+"oh", 0);
    __last_forcedtop = getPrivateInt("winamp5", __myname+"ForcedTop", 0);
    __last_forcedbottom = getPrivateInt("winamp5", __myname+"ForcedBottom", 0);
}

// -----------------------------------------------------------------------
shutdownDrawer() {
    delete __callbackTimer;
    delete __callbackTimer2;
    delete __PSOVCTimer;
    delete __tempDisable;
}

// -----------------------------------------------------------------------
Int isDrawerToTop() {
  int fromtop = 0;
  if (__drawer_directiontop_attrib.getData() =="1") fromtop = 1;
  if (__drawer_directionbypass_attrib.getData() == "0") return fromtop;

  int curstate = getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
  if (curstate != CLOSED) return __last_forcedtop;

  int h=getPrivateInt("winamp5", __myname+"Height", getDefaultDrawerOpenHeight());
  if (h == getDrawerClosedHeight()) h = getDefaultDrawerOpenHeight();
  if (__maximized) h = getViewportHeight()+__main.getSnapAdjustBottom();

  __last_forcedbottom = 0;
  __last_forcedtop = 0;

  // clienttoscreen auto adjusts for render ratio
  if (fromtop) {
    int y = __main.getGuiY();
    int curh = __main.clientToScreenH(__main.getGuiH());
    if (y + curh < __main.clientToScreenH(h) + getViewportTop()) {
      int offset = __main.getSnapAdjustBottom();
      if (!(y + __main.clientToScreenH(h-offset) > getViewPortTop()+getViewPortHeight())) {
        __last_forcedbottom = 1;
        return 0;
      }
    }
  } else {
    int offset = __main.getSnapAdjustBottom();
    int y = __main.getGuiY();
    if (y + __main.clientToScreenH(h-offset) > getViewPortTop()+getViewPortHeight()) {
      int curh = __main.clientToScreenH(__main.getGuiH());
      if (!(y + curh < __main.clientToScreenH(h) + getViewportTop())) {
        __last_forcedtop = 1;
        return 1;
      }
    }
  }
  return fromtop;
}

// -----------------------------------------------------------------------
__main.onTargetReached() {
    unlockUI();
    if (__drawer_directiontop_attrib.getData() =="1") __main.reverseTarget(0);
    if (__drawer_direction == DIRECTION_OPENING) {
        setPrivateInt("winamp5", __myname+"OpenState", OPEN);
        drawer_showWindowContent();
        onDoneOpeningDrawer();
    } else if (__drawer_direction == DIRECTION_CLOSING) {
        setPrivateInt("winamp5", __myname+"OpenState", CLOSED);
        onDoneClosingDrawer();
    }
    __drawer_direction = DIRECTION_NONE;
}

// -----------------------------------------------------------------------
drawer_expandWindow(int withdrawer) {
    int curstate = getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
#ifdef DEBUG
    debugstring("expand - curstate = " + integertostring(curstate), 0);
#endif
    if (curstate == OPEN) {
      drawer_showWindowContent();
      onBeforeOpeningDrawer();
      onDoneOpeningDrawer();
      return;
    }
    int fromtop = isDrawerToTop();
    setPrivateInt("winamp5", __myname+"OpenState", OPEN);

    int h=getPrivateInt("winamp5", __myname+"Height", getDefaultDrawerOpenHeight());
    if (h == getDrawerClosedHeight()) h = getDefaultDrawerOpenHeight();
    if (__maximized) h = getViewportHeight()+__main.getSnapAdjustBottom();
    int w = __main.getGuiW();

    if (h == __main.getHeight()) withdrawer = 0;

    onBeforeOpeningDrawer();

    int delay = 0;
    if (!__main.isLayoutAnimationSafe()) withdrawer = 0;
    if (withdrawer && StringToInteger(__scrolldrawerattrib.getData())) delay = 1;

    __drawer_direction = DIRECTION_OPENING;
    __main.setTargetX(__main.getGuiX());
    __main.setTargetY(__main.getGuiY());
    __main.setTargetW(w);
    __main.setTargetH(h);
    __main.reverseTarget(fromtop);
    __main.setTargetSpeed(delay);
    __main.gotoTarget();
    lockUI();

    if (!__maximized)
      setPrivateInt("winamp5", __myname+"Height", h);
    setPrivateInt("winamp5", __myname+"ForcedBottom", __last_forcedBottom);
    setPrivateInt("winamp5", __myname+"ForcedTop", __last_forcedtop);
}

// -----------------------------------------------------------------------
drawer_reduceWindow(int withdrawer) {
#ifdef DEBUG
    debugstring("reduce", 0);
#endif
    drawer_hideVis();
    drawer_hideVideo();
    setPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    if (__drawer_direction == DIRECTION_NONE && !__maximized) { // avoid saving new size if we're currenly opening
        int h=__main.getHeight();
        setPrivateInt("winamp5", __myname+"Height", h);
    }

    drawer_hideWindowContent();
    onBeforeClosingDrawer();

    int fromtop=0;
    if (__drawer_directiontop_attrib.getData() =="1") fromtop = 1;

    int delay = 0;
    if (!__main.isLayoutAnimationSafe()) withdrawer = 0;
    if (withdrawer && StringToInteger(__scrolldrawerattrib.getData())) delay = 1;

    if (__drawer_directionbypass_attrib.getData() == "1") {
        if (__last_forcedtop) fromtop = 1; 
        if (__last_forcedbottom) fromtop = 0;
    }

    __drawer_direction = DIRECTION_CLOSING;
    __main.setTargetX(__main.getGuiX());
    __main.setTargetY(__main.getGuiY());
    __main.setTargetW(__main.getGuiW());
    __main.setTargetH(getDrawerClosedHeight());
    __main.reverseTarget(fromtop);
    __main.setTargetSpeed(delay);
    __main.gotoTarget();
    lockUI();
    __last_forcedtop = 0;
    __last_forcedbottom = 0;
    setPrivateInt("winamp5", __myname+"ForcedBottom", 0);
    setPrivateInt("winamp5", __myname+"ForcedTop", 0);
}

// -----------------------------------------------------------------------
openDrawer() {
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    int s = getStatus();
    if (s == STATUS_PLAYING || s == STATUS_PAUSED) {
        if (!isVideo()) {
            if (__vis_detach_attrib.getData() == "0") {
                openDrawerForVis();
            } else if (__video_detach_attrib.getData() == "0") {
                openDrawerForVideo();
            } else {
                openDrawerForNothing();
            }
        } else {
            if (__video_detach_attrib.getData() == "0") {
                openDrawerForVideo();
            } else if (__vis_detach_attrib.getData() == "0") {
                openDrawerForVis();
            } else {
                openDrawerForNothing();
            }
        }
    } else {
        int Window_Content=getPrivateInt("winamp5", __myname+"State", CONTENT_VIS);
        if (window_content == CONTENT_VIS && __vis_detach_attrib.getData() == "0") {
            openDrawerForVis();
        } else if (window_content == CONTENT_VIDEO && __video_detach_attrib.getData() == "0") {
            openDrawerForVideo();
        } else if (__vis_detach_attrib.getData() == "0") {
            openDrawerForVis();
        } else if (__video_detach_attrib.getData() == "0") {
            openDrawerForVideo();
        } else {
            openDrawerForNothing();
        }
    }
}

// -----------------------------------------------------------------------
closeDrawer() {
    drawer_reduceWindow(1);
}

// -----------------------------------------------------------------------
System.onPlay() {
    // needed to handle video auto fullscreen on play in drawer_showVideo()
    WinampConfigGroup vidwcg = WinampConfig.getGroup(VIDEO_CONFIG_GROUP);
    boolean auto_fs = vidwcg.getBool("auto_fs");
    if (auto_fs && __video_detach_attrib.getData() == "0") __play_auto_fs_video = 1;
    else __play_auto_fs_video = 0;
}

System.onTitleChange(String newtitle) {
    // needed to handle video auto fullscreen on play in drawer_showVideo()
    WinampConfigGroup vidwcg = WinampConfig.getGroup(VIDEO_CONFIG_GROUP);
    boolean auto_fs = vidwcg.getBool("auto_fs");
    if (auto_fs && __video_detach_attrib.getData() == "0") __play_auto_fs_video = 1;
    else __play_auto_fs_video = 0;
}

// -----------------------------------------------------------------------
System.onPause() {
    __play_auto_fs_video = 0;
}

// -----------------------------------------------------------------------
System.onResume() {
    __play_auto_fs_video = 0;
}

// -----------------------------------------------------------------------
System.onStop() {
    __play_auto_fs_video = 0;
}

// -----------------------------------------------------------------------
// The heart of the machine, here we detect when a window wants to open
// or close, and we decide what to do about it. When we return FALSE, the
// window performs what it notified us about. When we return TRUE, the
// showing/hiding of the window is cancelled, and it is now up to us to
// show or hide the window once we're done with our animations.
// To show the window ourselves, we later show a windowholder with the
// autoopen="1" param, and to hide the window, we simply hide the
// windowholder, and its autoclose="1" param will do the rest
// -----------------------------------------------------------------------
System.onGetCancelComponent(String guid, boolean goingvisible) {
#ifdef DEBUG
    DebugString("+", 0);
#endif

    // fix for when the UI sometimes is locked after switching video file in fullscreen
    unlockUI();

    // isVideo() hasn't been set yet in System.onPlay and System.onTitleChange, so we check it here instead.
    if (__play_auto_fs_video && !isVideo()) __play_auto_fs_video = 0;

    if (__bypasscancel) return FALSE;
    if (guid == VIDEO_GUID && !goingvisible && __hiding_video) return FALSE;
    if (guid == VIS_GUID && !goingvisible && __hiding_vis) return FALSE;
    if (guid == VIDEO_GUID && goingvisible && __showing_video) return FALSE;
    if (guid == VIS_GUID && goingvisible && __showing_vis) return FALSE;

#ifdef DEBUG
    DebugString("--------------- onGetCancelComponent ----------------", 0);
    DebugString(" GUID : " + guid, 0);
    if (goingvisible) DebugString(" Going Visible", 0); else DebugString(" Going Invisible", 0);
    DebugString(" Last Content : " + IntegerToString(getPrivateInt("winamp5", __myname+"State", CONTENT_VIS)), 0);
    DebugString(" Drawer State : " + IntegerToString(getPrivateInt("winamp5", __myname+"OpenState", CLOSED)), 0);
    DebugString("-----------------------------------------------------", 0);
#endif

    if (!__main.isVisible()) return FALSE;

    int Window_Content=getPrivateInt("winamp5", __myname+"State", CONTENT_VIS);
    int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);

    if (window_status == CLOSED) {
        if (guid == VIDEO_GUID) {
            if (__video_detach_attrib.getData() == "0") {
                if (goingvisible) {
                    openDrawerForVideo();
                    return TRUE;
                }
            }
        }
        if (guid == VIS_GUID) {
            if (__vis_detach_attrib.getData() == "0") {
                if (goingvisible) {
                    openDrawerForVis();
                    return TRUE;
                }
            }
        }
    } else if (window_status == OPEN) {
        if (goingvisible) {
            if (guid == VIDEO_GUID && window_content == CONTENT_VIS) {
                if (__video_detach_attrib.getData() == "0") {
                    window_content = CONTENT_VIDEO;
                    drawer_hideVis();
                    drawer_dc_showVideo();
                    return TRUE;
                }
            } else if (guid == VIS_GUID && window_content == CONTENT_VIDEO) {
                if (__vis_detach_attrib.getData() == "0") {
                    window_content = CONTENT_VIS;
                    drawer_disablePSOVC();
                    drawer_hideVideo();
                    drawer_dc_showVis();
                    return TRUE;
                }
            }
        }
    }
    if (!goingvisible && window_status == OPEN) {
#ifdef DEBUG
        DebugString("closing " + guid, 0);
#endif
        if (guid == VIDEO_GUID && window_content == CONTENT_VIDEO) {
            drawer_hideVideo();
            drawer_reduceWindow(1);
            return FALSE;
        }
        if (guid == VIS_GUID && window_content == CONTENT_VIS) {
            drawer_hideVis();
            if ((getStatus() == STATUS_PLAYING ||
                getStatus() == STATUS_PAUSED) &&
                isVideo() &&
                __video_detach_attrib.getData() == "0") {
                drawer_dc_showVideo();
            } else {
	            drawer_reduceWindow(1);
            }
            return FALSE;
        }
    }
#ifdef DEBUG
    DebugString("Went thru", 0);
#endif
    return FALSE;
}

// -----------------------------------------------------------------------
drawer_showVis() {
#ifdef DEBUG
    DebugString("drawer_showVis",0 );
#endif
    __showing_vis = 1;
    setPrivateInt("winamp5", __myname+"OpenState", OPEN);
    setPrivateInt("winamp5", __myname+"State", CONTENT_VIS);
    GuiObject o = getVisWindowHolder();
    if (o != NULL) { __bypasscancel = 1; o.show(); __bypasscancel = 0; }
#ifdef DEBUG
    else DebugString("vis object not provided (show)", 0);
#endif
    onShowVis();
    __showing_vis = 0;
}

// -----------------------------------------------------------------------
drawer_hideVis() {
    __callback_vis_show = 0;
#ifdef DEBUG
    DebugString("drawer_hideVis",0 );
#endif
    __hiding_vis = 1;
    GuiObject o = getVisWindowHolder();
    if (o != NULL) { __bypasscancel = 1; o.hide(); __bypasscancel = 0; }
#ifdef DEBUG
    else DebugString("video object not found (hide)", 0);
#endif
    onHideVis();
    __hiding_vis = 0;
}

// -----------------------------------------------------------------------
drawer_showVideo() {
#ifdef DEBUG
    DebugString("drawer_showVideo",0 );
#endif
    __showing_video = 1;
    setPrivateInt("winamp5", __myname+"OpenState", OPEN);
    setPrivateInt("winamp5", __myname+"State", CONTENT_VIDEO);
    GuiObject o = getVideoWindowHolder();
    if (o != NULL) {
        __bypasscancel = 1;

        // hack to fix bug for auto fullscreen on play
        if (__play_auto_fs_video) setVideoFullscreen(FALSE);
            
        o.show();
        
        // hack to fix bug for auto fullscreen on play
        if (__play_auto_fs_video) setVideoFullscreen(TRUE);

        __bypasscancel = 0;
    }
#ifdef DEBUG
    else DebugString("vis object not found (show)", 0);
#endif
    onShowVideo();
    __play_auto_fs_video = 0;
    __showing_video = 0;
}

// -----------------------------------------------------------------------
drawer_hideVideo() {
    __callback_video_show = 0;
#ifdef DEBUG
    DebugString("drawer_hideVideo",0 );
#endif
    __hiding_video = 1;
    GuiObject o = getVideoWindowHolder();
    if (o != NULL) { __bypasscancel = 1; o.hide(); __bypasscancel = 0; }
#ifdef DEBUG
    else DebugString("video object not found (hide)", 0);
#endif
    onHideVideo();
    __hiding_video = 0;
}

// -----------------------------------------------------------------------
__callbackTimer.onTimer() {
    stop();
    int cvds = __callback_video_show;
    int cvss = __callback_vis_show;
    int cvdh = __callback_video_hide;
    int cvsh = __callback_vis_hide;
    __callback_video_show = 0;
    __callback_vis_show = 0;
    __callback_video_hide = 0;
    __callback_vis_hide = 0;
    if (cvds == 1) drawer_showVideo();
    if (cvss == 1) drawer_showVis();
    if (cvsh == 1) drawer_hideVis();
    if (cvdh == 1) drawer_hideVideo();
}

// -----------------------------------------------------------------------
drawer_dc_showVideo() {
    __callback_video_show = 1;
    __callback_video_hide = 0;
    __callbackTimer.start();
}

// -----------------------------------------------------------------------
drawer_dc_showVis() {
    __callback_vis_show = 1;
    __callback_vis_hide = 0;
    __callbackTimer.start();
}

// -----------------------------------------------------------------------
drawer_dc_hideVideo() {
    __callback_video_show = 0;
    __callback_video_hide = 1;
    __callbackTimer.start();
}

// -----------------------------------------------------------------------
drawer_dc_hideVis() {
    __callback_vis_show = 0;
    __callback_vis_hide = 1;
    __callbackTimer.start();
}

// -----------------------------------------------------------------------
drawer_showWindowContent() {
    int lastWindowContent=getPrivateInt("winamp5", __myname+"State", 2);
#ifdef DEBUG
    DebugString("drawer_showWindowContent = " + IntegerToString(lastWindowContent), 0);
#endif
    if (lastWindowContent==CONTENT_VIDEO) drawer_dc_showVideo();
    if (lastWindowContent==CONTENT_VIS) drawer_dc_showVis();
}

// -----------------------------------------------------------------------
drawer_hideWindowContent() {
    int lastWindowContent=getPrivateInt("winamp5", __myname+"State", 2);
#ifdef DEBUG
    DebugString("drawer_hideWindowContent = " + IntegerToString(lastWindowContent), 0);
#endif
    /*if (lastWindowContent==CONTENT_VIDEO)*/ drawer_hideVideo();
    /*if (lastWindowContent==CONTENT_VIS)*/ drawer_hideVis();
}

// -----------------------------------------------------------------------
OpenDrawerForVideo() {
    setPrivateInt("winamp5", __myname+"State", CONTENT_VIDEO);
    drawer_expandWindow(1);
}

// -----------------------------------------------------------------------
OpenDrawerForVis() {
    setPrivateInt("winamp5", __myname+"State", CONTENT_VIS);
    drawer_expandWindow(1);
}

// -----------------------------------------------------------------------
OpenDrawerForNothing() {
    setPrivateInt("winamp5", __myname+"State", CONTENT_NOTHING);
    drawer_expandWindow(1);
}

// -----------------------------------------------------------------------
__main.onResize(int x, int y, int w, int h) {
    if (!isGoingToTarget() && !__isinited) {
        __isinited = 1;
        if (h > getDrawerClosedHeight()) { setPrivateInt("winamp5", __myname+"OpenState", OPEN); drawer_expandWindow(0); }
        else setPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    }
}

// -----------------------------------------------------------------------
__main.onUserResize(int x, int y, int w, int h) {
    int window_status=getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    if (window_status == OPEN) {
        int h = getHeight();
        if (h != getDrawerClosedHeight()) {
#ifdef DEBUG
            DebugString("h = "+integerTostring(h), 0);
#endif
            if (!__maximized)
              setPrivateInt("winamp5", __myname+"Height", h);
        }
    } else if (window_status == CLOSED) {
      int h = getDrawerOpenAutoHeight(w);
      if (h != -1) {
        setPrivateInt("winamp5", __myname+"Height", h);
      }
    }
    if (__maximized) {
      __maximized = 0;
      setPrivateInt("winamp5", __myname+"Maximized", 0);
      onCancelMaximize();
    }
}

// -----------------------------------------------------------------------
switchToVideo() {
    if (__callbackTimer.isRunning()) return;
    if (__callbackTimer2.isRunning()) return;
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    drawer_hideVis();
    drawer_showVideo();
}

// -----------------------------------------------------------------------
switchToVis() {
    if (__callbackTimer.isRunning()) return;
    if (__callbackTimer2.isRunning()) return;
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    drawer_disablePSOVC();
    drawer_hideVideo();
    drawer_showVis();
}

// -----------------------------------------------------------------------
__tempDisable.onTimer() {
    stop();
}

// -----------------------------------------------------------------------
detachVis() {
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    __vis_detach_attrib.setData("1");
}

// -----------------------------------------------------------------------
detachVideo() {
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    __video_detach_attrib.setData("1");
}

// -----------------------------------------------------------------------
attachVis() {
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    __vis_detach_attrib.setData("0");
}

// -----------------------------------------------------------------------
attachVideo() {
    if (__tempDisable.isRunning()) return;
    __tempDisable.start();
    __video_detach_attrib.setData("0");
}

// -----------------------------------------------------------------------
__video_detach_attrib.onDataChanged() {
#ifdef DEBUG
    DebugString("detach video changed", 0);
#endif
    if (getData() == "1") {
        drawer_doDetachVideo();
        onDetachVideo();
    } else {
        if (getData() == "0") {
            drawer_doAttachVideo();
            onAttachVideo();
        }
    }
}

// -----------------------------------------------------------------------
__vis_detach_attrib.onDataChanged() {
#ifdef DEBUG
    DebugString("detach vis changed", 0);
#endif
    if (getData() == "1") {
        drawer_doDetachVis();
        onDetachVis();
    } else {
        if (getData() == "0") {
            drawer_doAttachVis();
            onAttachVis();
        }
    }
}

// -----------------------------------------------------------------------
drawer_doDetachVideo() {
    int wasvisible = isNamedWindowVisible(VIDEO_GUID);
    int lastWindowContent=getPrivateInt("winamp5", __myname+"State", 2);
    int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    if (!wasvisible) return;
    if (lastWindowContent != CONTENT_VIDEO) return;
    if (window_status == OPEN) {
        drawer_disablePSOVC();
        drawer_reduceWindow(1);
    }
    drawer_dc_linkup_showVideo();
}

// -----------------------------------------------------------------------
drawer_doDetachVis() {
    int lastWindowContent=getPrivateInt("winamp5", __myname+"State", 2);
    int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    if (lastWindowContent != CONTENT_VIS) return;
    int wasvisible = isNamedWindowVisible(VIS_GUID);
    if (!wasvisible) return;
    if (window_status == OPEN) {
        drawer_hideVis();
        if ((getStatus() == STATUS_PLAYING ||
             getStatus() == STATUS_PAUSED) && isVideo() &&
             __video_detach_attrib.getData() == "0") {
             setPrivateInt("winamp5", __myname+"State", CONTENT_VIDEO);
            drawer_dc_showVideo();
        } else {
            drawer_reduceWindow(1);
        }
    }
    drawer_dc_linkup_showVis();
}

// -----------------------------------------------------------------------
drawer_doAttachVideo() {
    drawer_disablePSOVC();
    int wasvisible = isNamedWindowVisible(VIDEO_GUID);
    if (wasvisible) {
        hideNamedWindow(VIDEO_GUID);
        int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
        int window_content=getPrivateInt("winamp5", __myname+"State", 2);
        if (window_content == CONTENT_VIS) drawer_hideVis();
        if (window_status == CLOSED) openDrawerForVideo();
        else drawer_dc_showVideo();
    }
}

// -----------------------------------------------------------------------
drawer_doAttachVis() {
    drawer_disablePSOVC();
    int wasvisible = isNamedWindowVisible(VIS_GUID);
    if (wasvisible) {
        hideNamedWindow(VIS_GUID);
        int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
        int window_content=getPrivateInt("winamp5", __myname+"State", 2);
        if (window_content == CONTENT_VIDEO) drawer_hideVideo();
        if (window_status == CLOSED) openDrawerForVis();
        else drawer_dc_showVis();
    }
}

// -----------------------------------------------------------------------
__callbackTimer2.onTimer() {
    stop();
    if (__callback2_what == DETACHED_VIDEO) drawer_linkup_showVideo();
    if (__callback2_what == DETACHED_VIS) drawer_linkup_showVis();
}

// -----------------------------------------------------------------------
drawer_dc_linkup_showVis() {
    __callback2_what = DETACHED_VIS;
    __callbackTimer2.start();
}

// -----------------------------------------------------------------------
drawer_dc_linkup_showVideo() {
    __callback2_what = DETACHED_VIDEO;
    __callbackTimer2.start();
}

// -----------------------------------------------------------------------
drawer_linkup_showVis() {
#ifdef DEBUG
    DebugString("show detached vis",0 );
#endif
    showWindow(VIS_GUID, "", 0);
}

// -----------------------------------------------------------------------
drawer_linkup_showVideo() {
#ifdef DEBUG
    DebugString("show detached video",0 );
#endif
    showWindow(VIDEO_GUID, "", 0);
    drawer_enablePSOVC();
}

// -----------------------------------------------------------------------
drawer_disablePSOVC() {
#ifdef DEBUG
    DebugString("disabling stop on video close",0 );
#endif
    ConfigItem item = Config.getItem(SKINTWEAKS_CFGPAGE);
    if (item) {
        ConfigAttribute attr = item.getAttribute("Prevent video playback Stop on video window Close");
        if (attr) attr.setData("1");
    }
    __PSOVCTimer.start();
}

// -----------------------------------------------------------------------
drawer_enablePSOVC() {
#ifdef DEBUG
    DebugString("enabling stop on video close",0 );
#endif
    __PSOVCTimer.stop();
    ConfigItem item = Config.getItem(SKINTWEAKS_CFGPAGE);
    if (item) {
        ConfigAttribute attr = item.getAttribute("Prevent video playback Stop on video window Close");
        if (attr) attr.setData("0");
    }
}

// -----------------------------------------------------------------------
__PSOVCTimer.onTimer() {
    drawer_enablePSOVC();
}

// -----------------------------------------------------------------------
__maincontainer.onBeforeSwitchToLayout(Layout oldl, Layout newl) {
    int window_status =getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
    int window_content=getPrivateInt("winamp5", __myname+"State", 2);
    if (oldl == __main && window_status == OPEN && window_content == CONTENT_VIDEO && getStatus() == STATUS_PLAYING && isVideo()) {
        drawer_disablePSOVC();
        __windowshade_openvid = 1;
    }
    if (oldl == __main && window_status == OPEN && window_content == CONTENT_VIS) {
        __windowshade_openvis = 1;
    }
}

// -----------------------------------------------------------------------
__maincontainer.onSwitchToLayout(Layout newl) {
    // these do not call drawer_doDetachVis or drawer_doDetachVideo but showDetachVis and showDetachVideo so that the change is temporary
    if (__windowshade_openvid) {
        __windowshade_openvid = 0;
        drawer_linkup_showVideo(); 
    }
    if (__windowshade_openvis) {
        __windowshade_openvis = 0;
        drawer_linkup_showVis(); 
    }
}

// -----------------------------------------------------------------------
Int getDrawerState() {
    return getPrivateInt("winamp5", __myname+"OpenState", CLOSED);
}

// -----------------------------------------------------------------------
Int getDrawerContent() {
    return getPrivateInt("winamp5", __myname+"State", CONTENT_VIS);
}

// -----------------------------------------------------------------------
maximizeWindow() {
  __oldx=__main.getGuiX();
  __oldy=__main.getGuiY();
  __oldw=__main.getGuiW();
  __oldh=__main.getGuiH();

  setPrivateInt("winamp5", __myname+"ox", __oldx);
  setPrivateInt("winamp5", __myname+"oy", __oldy);
  setPrivateInt("winamp5", __myname+"ow", __oldw);
  setPrivateInt("winamp5", __myname+"oh", __oldh);

  drawer_doMaximizeWindow(1);
}

// -----------------------------------------------------------------------
drawer_doMaximizeWindow(int notif) {
  int vx=getViewportLeft();
  int vy=getViewportTop();
  int vw=getViewportWidth();
  int vh=getViewportHeight();

  if (notif) onBeforeMaximize();
  __maximized = 1;
  setPrivateInt("winamp5", __myname+"Maximized", 1);
  __main.resize(vx, vy, vw, vh+__main.getSnapAdjustBottom());
  if (notif) onAfterMaximize();
}

// -----------------------------------------------------------------------
__main.onSnapAdjustChanged() {
  if (__maximized)
    drawer_doMaximizeWindow(0);
}

// -----------------------------------------------------------------------
restoreWindow() {
    onBeforeRestore();
    __maximized = 0;
    setPrivateInt("winamp5", __myname+"Maximized", 0);
    __main.resize(__oldx, __oldy, __oldw, __oldh);
    onAfterRestore();
}

// -----------------------------------------------------------------------
// default events implementations - override them in your script
// -----------------------------------------------------------------------
onBeforeOpeningDrawer() {}
onBeforeClosingDrawer() {}
onDoneOpeningDrawer() {}
onDoneClosingDrawer() {}
onShowVis() {}
onHideVis() {}
onShowVideo() {}
onHideVideo() {}
onAttachVideo() {}
onDetachVideo() {}
onAttachVis() {}
onDetachVis() {}
onBeforeMaximize() {}
onBeforeRestore() {}
onAfterMaximize() {}
onAfterRestore() {}
onCancelMaximize() {}
