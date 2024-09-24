/*---------------------------------------------------
-----------------------------------------------------
Filename:	syncbutton.m
Version:	1.0

Type:		maki
Date:		25. Jun. 2007 - 14:04 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>

Function String getArtist ();

Global Button nowplaying;

System.onScriptLoaded ()
{
	nowplaying = getScriptGroup().getObject("nowplaying");
}

nowplaying.onLeftClick ()
{
	String artist = getArtist();
	if (artist == "") return;

	String icid = "winshadeiconmodern";

	System.navigateUrlBrowser("http://client.winamp.com/nowplaying/artist/?icid="+ icid +"&artistName=" + artist);
}

String getArtist ()
{
	String artist = getPlayItemMetaDataString("artist");
	if (artist == "") artist = getPlayItemMetaDataString("uvox/artist");
	if (artist == "") artist = getPlayItemMetaDataString("cbs/artist");
	if (artist == "") artist = getPlayItemMetaDataString("streamtitle");
	if (artist == "") artist = getPlayItemDisplayTitle();
	
	return artist;
}