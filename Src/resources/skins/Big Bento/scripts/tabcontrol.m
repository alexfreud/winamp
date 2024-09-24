/*---------------------------------------------------
-----------------------------------------------------
Filename:	tabcontrol.m
Version:	1.0

Type:		maki
Date:		30. Okt. 2007 - 17:40  
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>
#include attribs/init_appearance.m

Function updateTabPos();
Function setAutoWidth(guiobject tab);

Class Text WatchText;

Global Boolean HAVE_ML = TRUE;

Global GuiObject tabMl, tabVideo, tabVis, tabPl;
Global WatchText txtMl, txtVideo, txtVis, txtPL;
Global int startX, curX;
Global Button btnMl, btnPl;

System.onScriptLoaded ()
{
	initAttribs_Appearance();
	HAVE_ML = stringToInteger(getParam());

	group sg = getScriptGroup();

	tabML = sg.findObject("switch.ml");
	btnMl = sg.findObject("switch.ml");
	txtMl = tabMl.findObject("bento.tabbutton.normal.text");
	tabPL = sg.findObject("switch.pl");
	btnPl = sg.findObject("switch.pl");
	txtPl = tabPl.findObject("bento.tabbutton.normal.text");
	tabVis = sg.findObject("switch.vis");
	txtVis = tabMl.findObject("bento.tabbutton.normal.text");
	tabVideo = sg.findObject("switch.video");
	txtVideo = tabMl.findObject("bento.tabbutton.normal.text");

	startX = tabMl.getGuiX();

	updateTabPos();
}

updateTabPos ()
{	
	curX = startX;
	if (!HAVE_ML)
	{
		tabMl.hide();
	}
	else
	{
		curX += setAutoWidth(tabMl) + 1;
	}

	if (pl_tab_attrib.getData() == "1")
	{
		tabPL.setXmlParam("x", integerToString(curX));
		curX += setAutoWidth(tabPL) + 1;
		tabPL.show();
	}
	else
	{
		tabPL.hide();
	}

	if (System.hasVideoSupport())
	{
		tabVideo.setXmlParam("x", integerToString(curX));
		curX += setAutoWidth(tabVideo) + 1;
	}
	else
	{
		tabVideo.hide();
	}

    // commented out because System.isSafeMode() is not recognized by mc.exe
	// if (!System.isSafeMode())
	// {
		tabVis.setXmlParam("x", integerToString(curX));
		curX += setAutoWidth(tabVis) + 1;
	// }
	// else
	// {
	// 	tabVis.hide();
	// }
}

int setAutoWidth (guiObject tab)
{
	text source = tab.findObject("bento.tabbutton.normal.text");
	int x = stringToInteger(source.getXmlparam("x"));
	int w = source.getAutoWidth();

	tab.setXmlParam("w", integerToString(2*x+w));

	return 2*x + w;
}

pl_tab_attrib.onDataChanged ()
{
	if (pl_tab_attrib.getData() == "0")
	{
		setPrivateString(getSkinName(), "Pledit_pos", "top");
		if (btnPl.getActivated())
		{	
			btnMl.leftClick();
			btnMl.getParentLayout().sendAction("load_comp", "pledit", 0,0,0,0);	
		}
	}
	
	updateTabPos();
}