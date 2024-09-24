#include <lib/std.mi>

Function updateVolume(int v);

Global Group frameGroup;
Global Togglebutton MuteBtn,MuteBtnShade;
Global Timer SongTickerTimer;
Global Text InfoTicker;
Global GuiObject SongTicker;
Global Float VolumeLevel;
Global Boolean Muted,BtnPressed;
Global Layer volumebar;
Global Timer callback;

System.onScriptLoaded() { 
	Muted = getPrivateInt("winamp5", "muted", 0);
	VolumeLevel = getPrivateInt("winamp5", "old_volume", 0);
	frameGroup = getScriptGroup();

	MuteBtn = frameGroup.findObject("mute");
	MuteBtn.setActivated(Muted);

	callback = new Timer; callback.setDelay(5); callback.start();
	SongTicker = frameGroup.findObject("songticker");
	InfoTicker = frameGroup.findObject("infoticker");

	volumebar = frameGroup.findObject("volumebar");
	volumebar.setXmlParam("w",integertostring( (system.getVolume()/255) *70 + 5));

	SongTickerTimer = new Timer;
	SongTickerTimer.setDelay(1000);
	if (Muted) {
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText("Mute ON");
	}
	BtnPressed = 0;
}

System.onScriptUnloading() {
	setPrivateInt("winamp5", "muted", Muted);
	setPrivateInt("winamp5", "old_volume", VolumeLevel);
	delete callback;
}

callback.onTimer() {
	MuteBtnShade = getcontainer("main").getlayout("shade").findObject("shademute");
	if (MuteBtnShade != NULL) {
		MuteBtnShade.setActivated(Muted);
		stop();
	}
}

SongTickerTimer.onTimer() {
	SongTicker.show();
	InfoTicker.hide();
	SongTickerTimer.stop();
}

MuteBtn.onLeftClick() {
	BtnPressed = 1;
	if (!Muted) {
		VolumeLevel = System.getVolume();
		System.setVolume(0);
		Muted = 1;
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText("Mute ON");
		MuteBtnShade.setActivated(1);
	} else {
		System.setVolume(VolumeLevel);
		Muted = 0;
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText("Mute OFF");
		MuteBtnShade.setActivated(0);
	}
}

MuteBtnShade.onLeftClick() {
	BtnPressed = 1;
	if (!Muted) {
		VolumeLevel = System.getVolume();
		System.setVolume(0);
		Muted = 1;
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText("Mute ON");
		MuteBtn.setActivated(1);
	} else {
		System.setVolume(VolumeLevel);
		Muted = 0;
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText("Mute OFF");
		MuteBtn.setActivated(0);
	}
}

System.onScriptUnloading() {
	delete SongTickerTimer;
}

System.onvolumechanged(int newvol)
{
	volumebar.setXmlParam("w",integertostring( (newvol/255) *70 + 5));
	if (!BtnPressed) {
		SongTickerTimer.start();
		SongTicker.hide();
		InfoTicker.show();
		InfoTicker.setText(translate("Volume") + ": " + integerToString(newvol/2.55) + "%");

		if (Muted) {
			MuteBtn.setActivated(0);
			MuteBtnShade.setActivated(0);
			Muted = 0;
		}
	}
	BtnPressed = 0;
}