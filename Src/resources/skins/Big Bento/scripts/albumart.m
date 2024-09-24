/*---------------------------------------------------
-----------------------------------------------------
Filename:	albumart.m
Version:	1.1

Type:		maki
Date:		20. Sep. 2007 - 16:54 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#include <lib/std.mi>
#include <lib/com/songinfo.m>

#define WEBCOVER_SHOUTCAST "winamp.cover.shoutcast"

Function loadFileInfo();
//Function loadPlaylistArtWork();

Global Int plArtRetries = 0;
Global Group scriptGroup;
Global AlbumArtLayer l_albumart;
Global String notfoundImage = "winamp.cover.notfound.xxl";

System.onScriptLoaded ()
{
	scriptGroup = getScriptGroup();
	l_albumart = scriptGroup.findObject(getToken(getParam(), ",", 0));
	notfoundImage = getToken(getParam(), ",", 1);
	loadFileInfo();
}

l_albumart.onRightButtonDown (int x, int y)
{
	popupmenu p = new popupmenu;

	p.addCommand("Refresh Album Art", 1, 0, 0);
	String path = getPath(getPlayItemMetaDataString("filename"));
	if(path != "")
	{
		p.addCommand("Open Folder", 2, 0, 0);
	}

	int result = p.popatmouse();
	delete p;

	if (result == 1)
	{
		l_albumart.refresh();
	}
	else if (result == 2)
	{
		if(path != "")
		{
			System.navigateUrl(path);
		}
		else
		{
			String url = getPlayItemMetaDataString("streamurl");
			if(url != "")
			{
				System.navigateUrl(url);
			}
		}
	}
}

l_albumart.onLeftButtonDblClk (int x, int y)
{
	String path = getPath(getPlayItemMetaDataString("filename"));
	if(path != "")
	{
		System.navigateUrl(path);
	}
	else
	{
		String url = getPlayItemMetaDataString("streamurl");
		if(url != "")
		{
			System.navigateUrl(url);
		}
	}
}

loadFileInfo ()
{
	songinfo_reload(); // refresh vars
	plArtRetries = 0;
	l_albumart.setXMLParam("notfoundImage", notfoundImage);

	//debugInt(songinfo_isStream);
	if (songinfo_isStream)
	{
		// setCover either from a supplied url or from in-stream artwork or default to a generic image
		if (songinfo_streamAlbumArt != "")
		{
			l_albumart.setXMLParam("image", songinfo_streamAlbumArt);
			l_albumart.setXMLParam("notfoundImage", notfoundImage);
		}

		if(songinfo_streamType == SONGINFO_STREAMTYPE_SHOUTCAST || songinfo_streamType == SONGINFO_STREAMTYPE_SHOUTCAST2)
		{
			if(songinfo_streamType == SONGINFO_STREAMTYPE_SHOUTCAST2)
			{
				if(l_albumart.isInvalid() && plArtRetries < 5)
				{
					if(!plArtRetries)
					{
						l_albumart.setXMLParam("notfoundImage", WEBCOVER_SHOUTCAST);
					}
					plArtRetries += 1;
					l_albumart.refresh();
				}
			}
			else
			{
				l_albumart.setXMLParam("notfoundImage", WEBCOVER_SHOUTCAST);
			}
		}
	}
}

// Hide branding of we start playback
System.onPlay ()
{
	loadFileInfo();
}

System.onTitleChange (String newtitle)
{
	loadFileInfo();
}