#include <lib/std.mi>

Global Group frameGroup;
Global Slider slidercb;
Global Text fadertext;
Global Button CFIncrease, CFDecrease;
Global ToggleButton Crossfade;
Global Layer DisplayOverlay;

System.onScriptLoaded() {
	frameGroup = getScriptGroup();
	slidercb = frameGroup.findObject("sCrossfade");
	fadertext = frameGroup.findObject("CFDisplay");
	CFIncrease = frameGroup.findObject("CrossfadeIncrease");
	CFDecrease = frameGroup.findObject("CrossfadeDecrease");
	Crossfade = frameGroup.findObject("Crossfade");
	DisplayOverlay = frameGroup.findObject("crossfade.display.overlay");
	slidercb.onSetPosition(slidercb.getPosition());

	Crossfade.onToggle(Crossfade.getActivated());
}

slidercb.onSetPosition(int val) {
	String s = IntegerToString(val);
	fadertext.setText(s);
}

CFIncrease.onLeftClick() {
	slidercb.SetPosition(slidercb.getPosition()+1);
}

CFDecrease.onLeftClick() {
	slidercb.SetPosition(slidercb.getPosition()-1);
}

Crossfade.onToggle(boolean on) {
	if (!on)
	{
		fadertext.setAlpha(150);
		CFIncrease.setAlpha(150);
		CFDecrease.setXmlParam("ghost" , "1");
		CFDecrease.setAlpha(150);
		CFIncrease.setXmlParam("ghost" , "1");
		DisplayOverlay.show();
	}
	else
	{
		fadertext.setAlpha(255);
		CFIncrease.setAlpha(255);
		CFDecrease.setAlpha(255);
		CFIncrease.setXmlParam("ghost" , "0");
		CFDecrease.setXmlParam("ghost" , "0");
		DisplayOverlay.hide();
	}
}