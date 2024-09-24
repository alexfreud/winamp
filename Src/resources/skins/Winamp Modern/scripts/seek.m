#include <lib/std.mi>

Global Group frameGroup;
Global Slider Seeker;
Global Int Seeking;
Global Timer SongTickerTimer;
Global Text InfoTicker;
Global GuiObject SongTicker;
Global layer SeekPos;

System.onScriptLoaded() {

	frameGroup = getScriptGroup();
	Seeker = frameGroup.findObject("SeekerGhost");
	SongTicker = frameGroup.findObject("songticker");
	SeekPos = frameGroup.findObject("player.seekbar.pos");
	InfoTicker = frameGroup.findObject("infoticker");

	SongTickerTimer = new Timer;
	SongTickerTimer.setDelay(1000);
}

SongTickerTimer.onTimer() {
	SongTicker.show();
	InfoTicker.hide();
	SongTickerTimer.stop();
}

System.onScriptUnloading() {
	delete SongTickerTimer;
}


Seeker.onSetPosition(int p) {
	if (seeking) {
		Float f;
		f = p;
		f = f / 255 * 100;
		Float len = getPlayItemLength();
		if (len != 0) {
			int np = len * f / 100;
			SongTickerTimer.start();
			SongTicker.hide();
			InfoTicker.show();
			InfoTicker.setText(translate("Seek") + ":" + integerToTime(np) + "/" + integerToTime(len) + " (" + integerToString(f) + "%) ");
		}
	}
}

Seeker.onLeftButtonDown(int x, int y) {
	seeking = 1;
}

Seeker.onLeftButtonUp(int x, int y) {
	seeking = 0;
	SongTickerTimer.start();
	SongTicker.show();
	InfoTicker.hide();
}

Seeker.onSetFinalPosition(int p) {
	SongTickerTimer.start();
	SongTicker.show();
	InfoTicker.hide();
}