#include <lib/std.mi>
#include "attribs.m"

Global Layer TimeBG,TimeDisplay;
Global Text PLTime;
Global Group frameGroup;
Global GuiObject SongTicker;
Global Timer callback;

function setSongtickerScrolling();

System.onScriptLoaded() {
	initAttribs();
	frameGroup = getScriptGroup();
	TimeBG = frameGroup.findobject("player.pl.time.left");
	TimeDisplay = frameGroup.findobject("player.pl.time.display.left");
	PLTime = frameGroup.findobject("PLTime");
	callback = new Timer; callback.setDelay(1); callback.start();
}

callback.onTimer() {
	Container c = getContainer("pledit");
	if (c) {
		Layout l = c.getLayout("shade");
		if (l) {
			SongTicker = l.findObject("PESongticker");
		}
	}
	if (SongTicker) callback.stop();
	setSongtickerScrolling();
}

frameGroup.onResize(int x, int y, int w, int h) {
	if (w>394) {
		TimeBG.show();
		TimeDisplay.show();
		PLTime.setXMLParam("x","-215");
		PLTime.setXMLParam("w","90");
	} else {
		TimeBG.hide();
		TimeDisplay.hide();
		PLTime.setXMLParam("x","-180");
		PLTime.setXMLParam("w","55");
	}
}

songticker_scrolling_attrib.onDataChanged() {
	setSongtickerScrolling();
}

setSongtickerScrolling() {
	if (!Songticker)
		return;
	
	if (songticker_scrolling_modern_attrib.getData()=="1") {
		SongTicker.setXMLParam("ticker","bounce");
	} else if (songticker_scrolling_classic_attrib.getData()=="1") {
		SongTicker.setXMLParam("ticker","scroll");
	} else {
		SongTicker.setXMLParam("ticker","off");
	}
}
