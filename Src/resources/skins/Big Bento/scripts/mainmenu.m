/*---------------------------------------------------
-----------------------------------------------------
Filename:	songinfo.m
Version:	1.0

Type:		maki
Date:		20. Nov. 2006 - 22:47 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>
#include attribs/init_appearance.m

Class Layer LinkedLayer;

Function initLL(linkedLayer l);
Function fadeLL(linkedLayer l, boolean in);
Function setLL(linkedLayer l, boolean in);

Global Group MenuBar;
Global GuiObject mousetrap;
Global Int texth;

Global LinkedLayer _play, _options, _file, _view, _help;
Global int xpos;

Global guiObject titlebargrid;


System.onScriptLoaded()
{
	initAttribs_Appearance();
	MenuBar = getscriptgroup().findobject("player.mainmenu");
	xpos = 0;

	titlebargrid = getScriptGroup().findObject("titlebar.grid.right");

	_file = MenuBar.getObject("menu.text.file");
	initLL(_file);

	_play = MenuBar.getObject("menu.text.play");
	initLL(_play);

	_options = MenuBar.getObject("menu.text.options");
	initLL(_options);

	_view = MenuBar.getObject("menu.text.view");
	initLL(_view);

	_help = MenuBar.getObject("menu.text.help");
	initLL(_help);

	mousetrap = MenuBar.findObjecT("menu.hidden.mousetrap");

	texth = _file.getGuiH();

	if (menubar_main_attrib.getData() == "1")
	{
		_options.setXmlParam("h", integerToString(texth));
		_file.setXmlParam("h", integerToString(texth));
		_help.setXmlParam("h", integerToString(texth));
		_view.setXmlParam("h", integerToString(texth));
		_play.setXmlParam("h", integerToString(texth));
		mousetrap.hide();
	}
	else
	{
		_options.setXmlParam("h", "0");
		_file.setXmlParam("h", "0");
		_help.setXmlParam("h", "0");
		_view.setXmlParam("h", "0");
		_play.setXmlParam("h", "0");
		mousetrap.show();
	}
}

menubar_main_attrib.onDataChanged() {
	if (getData() == "1")
	{
		mousetrap.hide();
		fadeLL(_play, 1);
		fadeLL(_view, 1);
		fadeLL(_help, 1);
		fadeLL(_file, 1);
		fadeLL(_options, 1);
	}
	else
	{
		mousetrap.show();
		fadeLL(_play, 0);
		fadeLL(_view, 0);
		fadeLL(_help, 0);
		fadeLL(_file, 0);
		fadeLL(_options, 0);
	}
}

System.onAccelerator(String action, String section, String key) {
	if (menubar_main_attrib.getData() == "0") return;

	Layout l = getScriptGroup().getParentLayout();
	if (!l.isActive()) return;
	if (action == "MENUHOTKEY_FILE")
	{
		MenuBar.findObject("file.menu").sendAction("open", "", 0, 0, 0, 0);
		complete;
	}
	if (action == "MENUHOTKEY_PLAY")
	{
		MenuBar.findObject("play.menu").sendAction("open", "", 0, 0, 0, 0);
		complete;
	}
	if (action == "MENUHOTKEY_OPTIONS")
	{
		MenuBar.findObject("options.menu").sendAction("open", "", 0, 0, 0, 0);
		complete;
	}
	if (action == "MENUHOTKEY_VIEW")
	{
		MenuBar.findObject("view.menu").sendAction("open", "", 0, 0, 0, 0);
		complete;
	}
	if (action == "MENUHOTKEY_HELP")
	{
		MenuBar.findObject("help.menu").sendAction("open", "", 0, 0, 0, 0);
		complete;
	}
}

initLL (LinkedLayer l)
{
	int w = l.getAutoWidth();
	String id = getToken(l.getId(), ".", 2);
	GuiObject o = MenuBar.findObject("menu.layer." + id + ".normal");
	if (o) o.setXmlParam("w", integerToString(w));
	if (o) o.setXmlParam("x", integerToString(xpos));

	o = MenuBar.findObject("menu.layer." + id + ".hover");
	if (o) o.setXmlParam("w", integerToString(w));
	if (o) o.setXmlParam("x", integerToString(xpos));

	o = MenuBar.findObject("menu.layer." + id + ".down");
	if (o) o.setXmlParam("w", integerToString(w));
	if (o) o.setXmlParam("x", integerToString(xpos));

	Menu m = MenuBar.findObject(id + ".menu");
	if (m) m.setXmlParam("w", integerToString(w));
	if (m) m.setXmlParam("x", integerToString(xpos));

	l.setXmlParam("x", integerToString(xpos));
	xpos += w;
}

fadeLL (linkedLayer l, boolean in)
{
	l.cancelTarget();
	l.setTargetH(texth*in);
	l.setTargetSpeed(0.5);
	l.gotoTarget();
}