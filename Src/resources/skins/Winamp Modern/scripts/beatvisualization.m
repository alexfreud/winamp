#include <lib/std.mi>
#include "attribs.m"

Function setObjects();

Global Group frameGroup,beatdisplay;
Global Layer beatOverlay,DisplayRight,DisplayRightOverlay,DisplaySongtickerBG,VisOverlay;
Global Timer refreshEQ;
Global AnimatedLayer beatbarLeft,beatbarRight;
Global int lastBeatLeft,lastBeatRight;
Global Button Toggler,Toggler2;
Global Int dobeat2;

System.onScriptLoaded() {
	initAttribs();
  frameGroup = getScriptGroup();
  beatdisplay = frameGroup.findObject("player.normal.display.beatvisualization");
  beatOverlay = frameGroup.findObject("beatdisplayoverlay");
  beatbarLeft = frameGroup.findObject("beatleft");
  beatbarRight = frameGroup.findObject("beatright");

  Toggler = frameGroup.findObject("beatvisualization");
  Toggler2 = frameGroup.findObject("beatvisualization2");

  DisplayRight = frameGroup.findObject("display.right");
  DisplayRightOverlay = frameGroup.findObject("display.right.overlay2");
  DisplaySongtickerBG = frameGroup.findObject("display.st.right");
  VisOverlay = frameGroup.findObject("visualization.overlay");

  lastBeatLeft = 0;
  lastBeatRight = 0;

  refreshEQ = new Timer;
  refreshEQ.setDelay(10);
}

System.onscriptunloading() {
  delete refreshEQ;
}

setObjects() {
  int group_width = frameGroup.getWidth();

  if ( group_width % 2 !=0 ) {
		DisplayRight.setXmlParam("image","player.display.right");
  	DisplayRightOverlay.setXmlParam("image","player.display.right");
  	DisplaySongtickerBG.setXmlParam("image","player.display.songticker.bg.right");
  	VisOverlay.setXmlParam("image","player.visualization.overlay");
	} else {
		DisplayRight.setXmlParam("image","player.display.right2");
  	DisplayRightOverlay.setXmlParam("image","player.display.right2");
  	DisplaySongtickerBG.setXmlParam("image","player.display.songticker.bg.right2");
  	VisOverlay.setXmlParam("image","player.visualization.overlay2");
	}

  if ( group_width > 480 ) {
    int newXpos = (group_width-60)/2;
    beatdisplay.setXmlParam("x", IntegerToString(newXpos));
    beatdisplay.show();

    if ( beatvisualization_attrib.getData()=="1" ) {
      refreshEQ.stop();
      refreshEQ.start();
    } else {
      refreshEQ.stop();
      beatbarLeft.gotoframe(0);
      beatbarRight.gotoframe(0);
    }
  } else {
    beatdisplay.hide();
    refreshEQ.stop();
  }
}

frameGroup.onResize(int x, int y, int w, int h) {
  setObjects();
}

refreshEQ.onTimer() {
  int beatLeft= System.getLeftVuMeter();
  int beatRight= System.getRightVuMeter();

  int frameLeft=beatLeft/16;
  int frameRight=beatRight/16;

  if (frameLeft>14) frameLeft=14;
  if (frameRight>14) frameRight=14;

  if (frameLeft<lastBeatLeft) {
    frameLeft=lastBeatLeft-1;
    if (frameLeft<0) frameLeft=0;
  }

  if (frameRight<lastBeatRight) {
    frameRight=lastBeatRight-1;
    if (frameRight<0) frameRight=0;
  }

  lastBeatLeft=frameLeft;
  lastBeatRight=frameRight;

  beatbarLeft.gotoframe(frameLeft);
  beatbarRight.gotoframe(frameRight);
}

beatvisualization_attrib.onDataChanged() {
  setObjects();
}

System.onKeyDown(String key) {
  if (key == "shift+ctrl+alt") {
    dobeat2 = 1;
    complete;
  } else dobeat2 = 0;
}

Toggler.onLeftClick() {
	if ( beatvisualization_attrib.getData()=="1" ) {
		beatvisualization_attrib.setData("0");
	} else {
		beatvisualization_attrib.setData("1");
	}
}

Toggler2.onActivate(boolean on) {
	if (!dobeat2) { Toggler.leftClick(); return; }
	refreshEQ.stop();

	if (on) {
		beatbarLeft.setXMLParam("image","player.display.beat.left2");
		beatbarRight.setXMLParam("image","player.display.beat.right2");
		beatOverlay.hide();
	} else {
		beatbarLeft.setXMLParam("image","player.display.beat.left");
		beatbarRight.setXMLParam("image","player.display.beat.right");
		beatOverlay.show();
	}
	setObjects();
}
