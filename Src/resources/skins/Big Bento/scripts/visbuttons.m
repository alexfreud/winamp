/*---------------------------------------------------
-----------------------------------------------------
Filename:	visbuttons.m
Version:	1.0
Type:		maki
Date:		16. Aug. 2007 - 23:54 
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
Global Button cfg, prv, nxt, rnd, rnda;
Global Boolean isBig;

System.onScriptLoaded ()
{
	initAttribs_Appearance();

	scriptGroup = getScriptGroup();

	cfg = scriptGroup.findObject("vis.cfg");
	prv = scriptGroup.findObject("vis.prv");
	nxt = scriptGroup.findObject("vis.nxt");
	rnd = scriptGroup.findObject("vis.rnd");
	rnda = scriptGroup.findObject("vis.rnd.active");

	isBig = (prv.getGuiX() == 31);
}

scriptGroup.onResize (int x, int y, int w, int h)
{
	updateObjectPosition(w);
}

/*

artist_info_buttons_attrib.onDataChanged ()
{
	updateObjectPosition(scriptGroup.getWidth());
}

updateObjectPosition (int w)
{
	if (isBig)
	{
		if (artist_info_buttons_attrib.getData() != "1")
		{
			w += 62;
		}

		if (w >= 248)
		{
			cfg.show();
			prv.show();
			rnd.show();
			rnda.show();
			nxt.show();
		}
		else if (w >= 217)
		{
			cfg.show();
			prv.show();
			rnd.show();
			rnda.show();
			nxt.hide();
		}
		else if (w >= 186)
		{
			cfg.show();
			prv.show();
			rnd.hide();
			rnda.hide();
			nxt.hide();
		}
		else if (w >= 155)
		{
			cfg.show();
			prv.hide();
			rnd.hide();
			rnda.hide();
			nxt.hide();
		}
		else
		{
			cfg.hide();
			prv.hide();
			rnd.hide();
			rnda.hide();
			nxt.hide();
		}
		return;
	}

	if (artist_info_buttons_attrib.getData() != "1")
	{
		w += 46;
	}

	if (w >= 192)
	{
		cfg.show();
		prv.show();
		rnd.show();
		rnda.show();
		nxt.show();
	}
	else if (w >= 168)
	{
		cfg.show();
		prv.show();
		rnd.show();
		rnda.show();
		nxt.hide();
	}
	else if (w >= 144)
	{
		cfg.show();
		prv.show();
		rnd.hide();
		rnda.hide();
		nxt.hide();
	}
	else if (w >= 120)
	{
		cfg.show();
		prv.hide();
		rnd.hide();
		rnda.hide();
		nxt.hide();
	}
	else
	{
		cfg.hide();
		prv.hide();
		rnd.hide();
		rnda.hide();
		nxt.hide();
	}

}*/