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
#include <lib/com/songinfo.m>

//nothing to see here

Function String getArtist ();

/*

Global Button search, nowplaying;
Global boolean isShade;

System.onScriptLoaded ()
{
	search = getScriptGroup().getObject("search");
	isShade = (search.getParentLayout().getId() == "shade");
	nowplaying = getScriptGroup().getObject("nowplaying");
}

search.onLeftClick ()
{
	String artist = getArtist();
	if (artist == "") return;

	getContainer("main").switchToLayout("normal");
	group sui = getContainer("main").getLayout("normal").findObject("sui.content");
	sui.sendAction ("browser_search", artist, 0, 0, 0, 0);
}

nowplaying.onLeftClick ()
{
	String artist = getArtist();
	if (artist == "") return;

	//getContainer("main").switchToLayout("normal");
	String icid = "fileinfoicon";
	if (isShade)
	{
		icid = "winshadeiconbento";
	}	
	System.navigateUrlBrowser("http://client.winamp.com/nowplaying/artist/?icid="+ icid +"&artistName=" + artist);
}

*/

String getArtist ()
{
	songinfo_reload();
	if (songinfo_isStream && songinfo_artist == "")
	{
		int v = strsearch(songinfo_streamtitle, " - ");
		if (v > 0)
		{
			return strleft(songinfo_streamtitle, v);
		}
		return songinfo_streamtitle;
	}
	
	return songinfo_artist;
}
