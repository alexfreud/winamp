/*---------------------------------------------------
-----------------------------------------------------
Filename:	about.m
Version:	1.0

Type:		maki
Date:		03. Jul. 2006 - 22:40 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>

#include nibbles/nibbles.m

Global Layer tgnibbles;
Global Group sg;
Global Timer change;
Global int counter;
Global Group g1, g2, g3, g4, g5, g6, g7, g8;
Function blend (guiobject in, guiobject out);

System.onScriptLoaded ()
{
	sg = getScriptGroup();
	tgnibbles = sg.getObject("toggle");
	nibbles = sg.getObject("nibbles");

	init_nibbles();

	g1 = sg.getObject("about.text1");
	g2 = sg.getObject("about.text2");
	g3 = sg.getObject("about.text3");
	g4 = sg.getObject("about.text4");
	g5 = sg.getObject("about.text5");
	g6 = sg.getObject("about.text6");
	g7 = sg.getObject("about.text7");
	g8 = sg.getObject("about.text8");

	change = new Timer;
	change.setDelay(3300);
}

system.onScriptUnloading ()
{
	delete change;
}


tgnibbles.onLeftButtonDblClk (int x, int y)
{
	if (nibbles.isVisible()) nibbles.hide();
	else if (!nibbles.isVisible()) nibbles.show();
}

sg.onSetVisible (Boolean onoff)
{
	if (onoff)
	{
		change.start();
		g1.setXmlParam("x", "400");
		g1.canceltarget();
		g1.setAlpha(255);
		g1.setTargetX(25);
		g1.setTargetSpeed(1.3);
		g1.gotoTarget();
	}
	else
	{
		change.stop();
	}
	
}


change.onTimer ()
{
	counter++;
	if (counter > 7) counter = 0;
	if (counter == 0)
	{
		blend(g1, g8);
	}
	else if (counter == 1)
	{
		blend(g2, g1);
	}
	else if (counter == 2)
	{
		blend(g3, g2);
	}
	else if (counter == 3)
	{
		blend(g4, g3);
	}
	else if (counter == 4)
	{
		blend(g5, g4);
	}
	else if (counter == 5)
	{
		blend(g6, g5);
	}
	else if (counter == 6)
	{
		blend(g7, g6);
	}
	else if (counter == 7)
	{
		blend(g8, g7);
	}
}

blend (guiobject in, guiobject out)
{
	out.canceltarget();
	out.setTargetA(0);
	out.setTargetX(-25);
	out.setTargetSpeed(2);
	out.gotoTarget();
	in.canceltarget();
	in.setXmlParam("x", "400");
	in.setAlpha(255);
	in.setTargetX(25);
	in.setTargetA(255);
	in.setTargetSpeed(1.5);
	in.gotoTarget();
}

