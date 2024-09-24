/*---------------------------------------------------
-----------------------------------------------------
Filename:	shadesize.m
Version:	2.1
Type:		maki
Date:		23. Jul. 2007 - 17:24 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>
#include attribs/init_appearance.m

Function updateObjectPosition(int w);

Global Group scriptGroup;
//Global Group g_cbuttons, g_sysbuttons, g_display, g_volume, g_seek, g_vis, g_links;
Global Group g_cbuttons, g_sysbuttons, g_display, g_volume, g_seek, g_vis;

//Global Int minumim_w, x_cbuttons, w_cbuttons, w_sysbuttons, w_volume, w_display, w_seek, w_vis, x_seek, w_links, w_links_, x_links, x_links_;
Global Int minumim_w, x_cbuttons, w_cbuttons, w_sysbuttons, w_volume, w_display, w_seek, w_vis, x_seek;

System.onScriptLoaded ()
{
	initAttribs_Appearance();

	scriptGroup = getScriptGroup();

	g_cbuttons = scriptGroup.findObject("shade.cbuttons");
	x_cbuttons = stringToInteger(g_cbuttons.getXmlParam("x"));
	w_cbuttons = stringToInteger(g_cbuttons.getXmlParam("w"));

	g_seek = scriptGroup.findObject("shade.seek");
	w_seek = stringToInteger(g_seek.getXmlParam("w"));
	x_seek = stringToInteger(g_seek.getXmlParam("x"));

	g_vis = scriptGroup.findObject("shade.vis");
	w_vis = stringToInteger(g_vis.getXmlParam("w"));

/*
	g_links = scriptGroup.findObject("shade.links");
	w_links_ = stringToInteger(g_links.getXmlParam("w"));
	x_links_ = stringToInteger(g_links.getXmlParam("x"));
*/
	w_sysbuttons = stringToInteger(getParam());

	g_display = scriptGroup.findObject("shade.display");
	w_display = stringToInteger(g_display.getXmlParam("w"));
	g_volume = scriptGroup.findObject("shade.volume");
	w_volume = stringToInteger(g_volume.getXmlParam("w"));
}

scriptGroup.onResize (int x, int y, int w, int h)
{
	updateObjectPosition(w);
}

scriptGroup.onSetVisible (Boolean onoff)
{
	if (onoff)
	{
		updateObjectPosition(scriptGroup.getWidth());
	}
}
/*
artist_info_buttons_attrib.onDataChanged ()
{
	updateObjectPosition(scriptGroup.getWidth());
}
*/
updateObjectPosition (int w)
{
/*	if (artist_info_buttons_attrib.getData() == "1")
	{
		w_links = w_links_;
		x_links = x_links_;
	}
	else
	{
		w_links = x_links = 0;
	}
*/	
	
//	if (w >= x_cbuttons + w_cbuttons + 2 * w_display + w_volume + w_sysbuttons + 3.5 * w_seek + w_vis + w_links)
	if (w >= x_cbuttons + w_cbuttons + 2 * w_display + w_volume + w_sysbuttons + 3.5 * w_seek + w_vis)
	{
//		if(g_links != null) g_links.setXmlParam("x", integerToString(x_seek - 2*w_seek - w_links));
		g_seek.setXmlParam("x", integerToString(x_seek - 2*w_seek));
		g_seek.setXmlParam("w", integerToString(3 * w_seek));
		g_cbuttons.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis));
		g_volume.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis + w_cbuttons));
		g_display.setXmlParam("x", integerToString(2 + x_cbuttons + w_cbuttons + w_volume + w_vis));
//		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - 3 * w_seek - w_vis - w_links));
		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - 3 * w_seek - w_vis));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.show();
		g_vis.show();
/*		if(w_links != 0) g_links.show();
		else g_links.hide();
*/
	}
//	else if (w >= x_cbuttons + w_cbuttons + 2 * w_display + w_volume + w_sysbuttons + 2 * w_seek + w_vis + w_links)
	else if (w >= x_cbuttons + w_cbuttons + 2 * w_display + w_volume + w_sysbuttons + 2 * w_seek + w_vis)
	{
//		if(g_links != null) g_links.setXmlParam("x", integerToString(x_seek - w_seek - w_links));
		g_seek.setXmlParam("x", integerToString(x_seek - w_seek));
		g_seek.setXmlParam("w", integerToString(2 * w_seek));
		g_cbuttons.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis));
		g_volume.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis + w_cbuttons));
		g_display.setXmlParam("x", integerToString(2 + x_cbuttons + w_cbuttons + w_volume + w_vis));
//		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - 2 * w_seek - w_vis - w_links));
		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - 2 * w_seek - w_vis));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.show();
		g_vis.show();
/*		if(w_links != 0) g_links.show();
		else g_links.hide();
*/
	}
//	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons + w_seek + w_vis + w_links)
	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons + w_seek + w_vis)
	{
//		if(g_links != null) g_links.setXmlParam("x", integerToString(x_seek - w_links));
		g_seek.setXmlParam("x", integerToString(x_seek));
		g_seek.setXmlParam("w", integerToString(w_seek));
		g_cbuttons.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis));
		g_volume.setXmlParam("x", integerToString(2 + x_cbuttons + w_vis + w_cbuttons));
		g_display.setXmlParam("x", integerToString(2 + x_cbuttons + w_cbuttons + w_volume + w_vis));
//		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - w_seek - w_vis - w_links));
		g_display.setXmlParam("w", integerToString(0 - 1 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - w_seek - w_vis));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.show();
		g_vis.show();
/*		if(w_links != 0) g_links.show();
		else g_links.hide();
*/
	}
//	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons + w_seek + w_links)
	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons + w_seek)
	{
//		if(g_links != null) g_links.setXmlParam("x", integerToString(x_seek - w_links));
		g_seek.setXmlParam("x", integerToString(x_seek));
		g_seek.setXmlParam("w", integerToString(w_seek));
		g_volume.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons));
		g_cbuttons.setXmlParam("x", integerToString(x_cbuttons));
		g_display.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons + w_volume));
//		g_display.setXmlParam("w", integerToString(0 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - w_seek - w_links));
		g_display.setXmlParam("w", integerToString(0 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - w_seek));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.show();
		g_vis.hide();
/*		if(w_links != 0) g_links.show();
		else g_links.hide();
*/
	}
	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons + w_seek)
	{
		g_seek.setXmlParam("x", integerToString(x_seek));
		g_seek.setXmlParam("w", integerToString(w_seek));
		g_volume.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons));
		g_cbuttons.setXmlParam("x", integerToString(x_cbuttons));
		g_display.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons + w_volume));
		g_display.setXmlParam("w", integerToString(0 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume - w_seek));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.show();
		g_vis.hide();
//		g_links.hide();
	}
	else if (w >= x_cbuttons + w_cbuttons + w_display + w_volume + w_sysbuttons)
	{
		g_cbuttons.setXmlParam("x", integerToString(x_cbuttons));
		g_volume.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons));
		g_display.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons + w_volume));
		g_display.setXmlParam("w", integerToString(0 - x_cbuttons - w_cbuttons - w_sysbuttons - w_volume));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.show();
		g_seek.hide();
		g_vis.hide();
//		g_links.hide();
	}
	else if (w >= x_cbuttons + w_cbuttons + w_display + w_sysbuttons)
	{
		g_cbuttons.setXmlParam("x", integerToString(x_cbuttons));
		g_volume.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons));
		g_display.setXmlParam("x", integerToString(x_cbuttons + w_cbuttons));
		g_display.setXmlParam("w", integerToString(0 - x_cbuttons - w_cbuttons - w_sysbuttons));
		g_display.setXmlParam("relatw", "1");
		g_display.show();
		g_volume.hide();
		g_seek.hide();
		g_vis.hide();
//		g_links.hide();
	}
	else
	{
		g_cbuttons.setXmlParam("x", integerToString(x_cbuttons));
		g_volume.hide();
		g_display.hide();
		g_seek.hide();
		g_vis.hide();
//		g_links.hide();
	}
}