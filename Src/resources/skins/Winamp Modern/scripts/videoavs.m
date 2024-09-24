#include <lib/std.mi>
#include <lib/config.mi>

#define MAIN_ATTRIBS_MGR

Function updateVisCmd();
Function updateOpenCloseDirection();

// this implements the main app drawer
#include "drawer.m"

// this loads the skin config attributes definitions
#include "attribs.m"

Global Layout main, shade;
Global Container maincontainer;
Global Group frameGroup, ButtonsVideo,ButtonsVis,ButtonsVideoDetach,ButtonsVisDetach,ButtonsVideoSwitchto,ButtonsVisSwitchto;
Global Button btnOpen,btnClose,btnMaximize,btnRestore, btnVisDetach, btnVideoDetach, btnVisSwitchto, btnVideoSwitchto;
Global Layer resizer,resizerDrawer,OpenCloseHider;
Global GuiObject VideoVisGroup;
Global Int evershown;
Global Int ismaximized;
Global Int lasttotop;

//------------------------------------------------------------------------
// script startup
//------------------------------------------------------------------------
System.onScriptLoaded() {
    // init skin attributes for this script

    initAttribs();

    // bind objects

    frameGroup = getScriptGroup();
    btnOpen = frameGroup.findObject("videoavs.open");
    btnClose = frameGroup.findObject("videoavs.close");
    btnMaximize = frameGroup.findObject("button.vid.max");
    btnRestore = frameGroup.findObject("button.vid.restore");
    resizer = frameGroup.findObject("player.main.resizer");
    resizerDrawer = frameGroup.findObject("drawer.resizer");

    ButtonsVideo = frameGroup.findObject("buttons.video");
    ButtonsVis = frameGroup.findObject("buttons.vis");

    ButtonsVideoDetach = frameGroup.findObject("buttons.video.detach");
    ButtonsVisDetach = frameGroup.findObject("buttons.vis.detach");
    BtnVisDetach = ButtonsVisDetach.findObject("button.vis.detach");
    BtnVideoDetach = ButtonsVideoDetach.findObject("button.vid.detach");

    ButtonsVideoSwitchto = frameGroup.findObject("buttons.video.switchto");
    ButtonsVisSwitchto = frameGroup.findObject("buttons.vis.switchto");

    BtnVisSwitchto = ButtonsVisSwitchto.findObject("button.vis.Switchto");
    BtnVideoSwitchto = ButtonsVideoSwitchto.findObject("button.vid.Switchto");

    OpenCloseHider=frameGroup.findObject("openclosehider");

    main = frameGroup.getParentLayout();
    maincontainer = main.getContainer();
    VideoVisGroup = frameGroup.findObject("AVSGroup");

    VideoVisGroup.hide();

    // bind drawer script attribs to our attribs 

    __drawer_directiontop_attrib = drawer_directiontop_attrib;
    __scrolldrawerattrib = scrolldrawerattrib;
    __drawer_directionbypass_attrib = drawer_directionbypass_attrib;
    __vis_detach_attrib = vis_detach_attrib;
    __video_detach_attrib = video_detach_attrib;

    // startup drawer script

    initDrawer(main, "VideoAVS");

    // more init

    if (vis_detach_attrib.getData() == "1" && video_detach_attrib.getData() == "1") OpenCloseHider.show();
    else OpenCloseHider.hide();

    lasttotop = 0;
}

//------------------------------------------------------------------------
// grab a handle to the windowshade layout, can't get it in onScriptLoaded since it doesn't exists yet
// also, first time we are shown, we update the viscmd button's action
//------------------------------------------------------------------------
main.onSetVisible(int show) {
    if (!evershown) {
        evershown = 1;
        if (!shade) {
            shade = maincontainer.getLayout("shade");
        }
        updateVisCmd();
        updateOpenCloseDirection();
    }
}

//------------------------------------------------------------------------
// script shutdown
//------------------------------------------------------------------------
System.onScriptUnloading() {
	
    // shutdown the drawer script

    shutdownDrawer();
}

//------------------------------------------------------------------------
// drawer script backend
//------------------------------------------------------------------------

Int getDrawerClosedHeight() {
    return 280; // this is the size of the layout when the drawer is closed
}

Int getDefaultDrawerOpenHeight() {
    return 510; // this is the default size of the layout, used when it opens the first time
}

// whenever the main window is resized while its drawer is closed, we change the height of the drawer the 
// next time it opens so that video will fit 4:3
Int getDrawerOpenAutoHeight(int layoutwidth) {
    return (layoutwidth-6)*(3/4)+55+270; 
}

WindowHolder getVisWindowHolder() {
    WindowHolder wh = getScriptGroup().findObject("myviswnd");
    return wh; // we return our vis windowholder object
}

WindowHolder getVideoWindowHolder() {
    WindowHolder wh = getScriptGroup().findObject("myvideownd");
    return wh; // we return our video windowholder object
}

//------------------------------------------------------------------------
// optional drawer events
//------------------------------------------------------------------------

onDoneOpeningDrawer() {
    // nothing to do
}

onDoneClosingDrawer() {
    VideoVisGroup.hide();
}

onBeforeOpeningDrawer() {
    resizer.setXmlParam("resize", "bottomright");
    resizerDrawer.setXmlParam("resize", "bottomright");
    btnOpen.hide();
    main.setXmlParam("minimum_h", "380");

    VideoVisGroup.show();
}

onBeforeClosingDrawer() {
    resizer.setXmlParam("resize", "right");
    resizerDrawer.setXmlParam("resize", "right");
    main.setXmlParam("minimum_h", "280");
    btnOpen.show();
}

onShowVis() {
    ButtonsVideo.hide();
    ButtonsVideoDetach.hide();
    ButtonsVideoSwitchto.hide();
    ButtonsVis.show();
    ButtonsVisDetach.show();
    if (video_detach_attrib.getData() == "0") ButtonsVisSwitchto.show();
}

onHideVis() {
    ButtonsVis.hide();
    ButtonsVisDetach.hide();
    ButtonsVisSwitchto.hide();
}

onShowVideo() {
    ButtonsVis.hide();
    ButtonsVisDetach.hide();
    ButtonsVisSwitchto.hide();
    ButtonsVideo.show();
    ButtonsVideoDetach.show();
    if (vis_detach_attrib.getData() == "0") ButtonsVideoSwitchto.show();
}

onHideVideo() {
    ButtonsVideo.hide();
    ButtonsVideoDetach.hide();
    ButtonsVideoSwitchto.hide();
}

onDetachVideo() {
    ButtonsVisSwitchto.hide();
    if (vis_detach_attrib.getData() == "1") OpenCloseHider.show();
}

onAttachVideo() {
    ButtonsVisSwitchto.show();
    OpenCloseHider.hide();
}

onDetachVis() {
    ButtonsVideoSwitchto.hide();
    if (video_detach_attrib.getData() == "1") OpenCloseHider.show();
}

onAttachVis() {
    ButtonsVideoSwitchto.show();
    OpenCloseHider.hide();
}

onBeforeRestore() {
    btnMaximize.show();
    btnRestore.hide();
}

onBeforeMaximize() {
    btnRestore.show();
    btnMaximize.hide();
}

onCancelMaximize() {
    btnMaximize.show();
    btnRestore.hide();
}

// -----------------------------------------------------------------------
// drawer control
// -----------------------------------------------------------------------

btnOpen.onLeftClick() {
    openDrawer();
}

btnClose.onLeftClick() {
    closeDrawer();
}

btnMaximize.onLeftClick() {
    maximizeWindow();
}

btnRestore.onLeftClick() {
    restoreWindow();
}

BtnVisDetach.onLeftClick() {
    detachVis();
}

BtnVideoDetach.onLeftClick() {
    detachVideo();
}

BtnVisSwitchto.onLeftClick() {
    switchToVideo();
}

BtnVideoSwitchto.onLeftClick() {
    switchToVis();
}

// -----------------------------------------------------------------------
// when the player window is moved on the screen, we should update the up/down arrow for the open/close buttons since the
// drawer may have changed the direction it will open depending on user settings
// -----------------------------------------------------------------------
main.onMove() {
  updateOpenCloseDirection();
}

// -----------------------------------------------------------------------
// just invert the images when the direction is to top and restore them otherwise
// -----------------------------------------------------------------------
updateOpenCloseDirection() {
  if (isDrawerToTop()) {
    if (!lasttotop) {
      btnClose.setXmlParam("image", "player.button.videoavs");
      btnClose.setXmlParam("downimage", "player.button.videoavs.pressed");
      btnClose.setXmlParam("hoverImage", "player.button.videoavs.hover");
      btnOpen.setXmlParam("image", "player.button.videoavs.up");
      btnOpen.setXmlParam("downimage", "player.button.videoavs.up.pressed");
      btnOpen.setXmlParam("hoverImage", "player.button.videoavs.up.hover");
      lasttotop = 1;
     }
  } else {
    if (lasttotop) {
      btnOpen.setXmlParam("image", "player.button.videoavs");
      btnOpen.setXmlParam("downimage", "player.button.videoavs.pressed");
      btnOpen.setXmlParam("hoverImage", "player.button.videoavs.hover");
      btnClose.setXmlParam("image", "player.button.videoavs.up");
      btnClose.setXmlParam("downimage", "player.button.videoavs.up.pressed");
      btnClose.setXmlParam("hoverImage", "player.button.videoavs.up.hover");
      lasttotop = 0;
    }
  }
}

// -----------------------------------------------------------------------
updateVisCmd() {
    Button btn = getScriptGroup().findObject("button.vis.misc");
    if (btn) {
        if (viscmd_menu_attrib.getData() == "1") {
            btn.setXmlParam("action", "Vis_Menu");
        } else {
            btn.setXmlParam("action", "Vis_Cfg");
        }
    }
}

