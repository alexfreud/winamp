#include <lib/std.mi>
#include "attribs.m"

function setObjects(int w);
function setSongtickerScrolling();

Global Group frameGroup, gX1,gX2,gX3,display,display2,displaytimer,displayticker,displayvis;
Global Button btnExpand,btnReduce,shadeeq;
Global Layout main;
Global Layout main_normal;
Global Layer volumebar;
Global Timer SongTickerTimer;
Global GuiObject SongTicker;
Global text InfoTicker;
Global Button colorthemes;

System.onScriptLoaded() {
	initAttribs();

	frameGroup = getScriptGroup();
	main = frameGroup.getParentLayout();
	main_normal = frameGroup.getParentLayout().getContainer().getLayout("normal");
	gX1 = frameGroup.findObject("shadeX1");
	gX2 = frameGroup.findObject("shadeX2");
	gX3 = frameGroup.findObject("shadeX3");
	shadeeq = frameGroup.findObject("shadeeq");
	display = frameGroup.findObject("shadedisplay");
	display2 = frameGroup.findObject("shadedisplay2");
	displaytimer = frameGroup.findObject("shadetimer");
	displayticker = frameGroup.findObject("shadeticker");
	displayvis = frameGroup.findObject("shadevis");
	btnExpand = frameGroup.findObject("shade.expand");
	btnReduce = frameGroup.findObject("shade.reduce");
	colorthemes = frameGroup.findObject("shadect");

	SongTicker = frameGroup.findObject("SongtickerShade");
	InfoTicker = frameGroup.findObject("infotickerShade");
	SongTickerTimer = new Timer;
	SongTickerTimer.setDelay(1000);

	volumebar = frameGroup.findObject("volumebarShade");
	volumebar.setXmlParam("w",integertostring( (system.getVolume()/255) *40 + 5));

	setSongtickerScrolling();
}

System.onScriptUnloading() {
	delete SongTickerTimer;
}

SongTickerTimer.onTimer() {
	SongTicker.show();
	InfoTicker.hide();
	SongTickerTimer.stop();
}

frameGroup.onResize(int x, int y, int w, int h) {
  setObjects(w);
}

shadeeq.onLeftClick() {
  eq_visible_attrib.setData("1");
}

setObjects(int w) {

	int ShowButtons=getPrivateInt("winamp5", "ShowShadeButtons", 0);

	if (ShowButtons) {
		btnExpand.show();
		btnReduce.hide();
	} else {
		btnExpand.hide();
		btnReduce.show();
	}

	gX1.hide();
	gX2.hide();
	gX3.hide();
	display.hide();
	display2.hide();
	displaytimer.hide();
	displayticker.hide();
	displayvis.hide();

	if (w>413) {
		int w_display=w-356;
		display.setXMLParam("w", integertostring(w_display));
		display.show();
		displaytimer.show();

		if (w>433) {
			displayticker.show();
			displayticker.setXMLParam("w", integertostring(w_display-13));

			if (w>573) {
				display.hide();
				display2.setXMLParam("w", integertostring(w_display-45));
				display2.show();
				displayticker.setXMLParam("w", integertostring(w_display-56-13));

				if (ShowButtons) {
					gX1.setXMLParam("x", "-100");
					gX1.show();

					if (w>596) {
						display2.setXMLParam("w", integertostring(w_display-68));
						displayticker.setXMLParam("w", integertostring(w_display-79-13));

						gX1.setXMLParam("x", "-123");

						gX2.setXMLParam("x", "-80");
						gX2.show();

						if (w>655) {
							display2.setXMLParam("w", integertostring(w_display-127));
							displayticker.setXMLParam("w", integertostring(w_display-138-13));

							gX1.setXMLParam("x", "-182");

							gX2.setXMLParam("x", "-139");

							gX3.setXMLParam("x", "-116");
							gX3.show();

							if (w>673) {
								displayvis.setXMLParam("x", integertostring(-257));
								displayvis.show();
								displayticker.setXMLParam("w", integertostring(w_display-194-13));
							}
						}
					}
				} else {
					display2.setXMLParam("w", integertostring(w_display));
					displayvis.setXMLParam("x", integertostring(-130));
					displayvis.show();
					displayticker.setXMLParam("w", integertostring(w_display-66-13));
				}
			}
		}
	}
}

System.onKeyDown(String key) {
  	if (StrLeft(key,4) == "ctrl" && StrSearch(key, "+w") != -1 && (main.isActive() || main_normal.isActive())) {
		if (main.isVisible())
			main.getContainer().switchToLayout("normal");
		else
			main.getContainer().switchToLayout("shade");
		complete;
	}
}

btnExpand.onleftClick() {
	setPrivateInt("winamp5", "ShowShadeButtons", 0);
	setObjects(main.getGUIw());
}

btnReduce.onleftClick() {
	setPrivateInt("winamp5", "ShowShadeButtons", 1);
	setObjects(main.getGUIw());
}

System.onvolumechanged(int newvol)
{
	volumebar.setXmlParam("w",integertostring( (newvol/255) *40 + 5));
	SongTickerTimer.start();
	SongTicker.hide();
	InfoTicker.show();
	InfoTicker.setText(translate("Volume") + ": " + integerToString(newvol/2.55) + "%");
}

songticker_scrolling_attrib.onDataChanged() {
	setSongtickerScrolling();
}

setSongtickerScrolling() {
	if (songticker_scrolling_modern_attrib.getData()=="1") {
		SongTicker.setXMLParam("ticker","bounce");
	} else if (songticker_scrolling_classic_attrib.getData()=="1") {
		SongTicker.setXMLParam("ticker","scroll");
	} else {
		SongTicker.setXMLParam("ticker","off");
	}
}

colorthemes.onRightButtonDown(int x, int y) {
	triggerAction(colorthemes, "ThemesSlotsMenu", "");
	complete;
}