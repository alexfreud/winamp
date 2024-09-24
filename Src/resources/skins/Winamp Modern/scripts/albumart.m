#include <lib/std.mi>
#include "attribs.m"

Global AlbumArtLayer waaa;
Global Layout aalayout;

System.onScriptLoaded()
{
	initAttribs();
	Container albumart = System.getContainer("winamp.albumart");
	aalayout = albumart.getLayout("normal");
	waaa = getScriptGroup().findObject(getParam());
}

system.onScriptUnloading ()
{
	if (!aalayout) return;
	setPrivateInt("Winamp Modern", "Album Art XPos", aalayout.getLeft());
	setPrivateInt("Winamp Modern", "Album Art YPos", aalayout.getTop());
}

aalayout.onStartup ()
{
	resize(getPrivateInt("Winamp Modern", "Album Art XPos", 0), getPrivateInt("Winamp Modern", "Album Art YPos", 0), getWidth(), getHeight());
}

aalayout.onSetVisible (Boolean onoff)
{
	if (!onoff)
	{
		albumart_visible_attrib.setData("0");
	}
	else
	{
		albumart_visible_attrib.setData("1");
	}
}

albumart_visible_attrib.onDataChanged ()
{
	if (getData() == "1")
	{
		aalayout.show();
	}
	else
	{
		aalayout.hide();
	}
}

System.onKeyDown(String key)
{
	if (key == "alt+a")
	{
		if (albumart_visible_attrib.getData() == "0")
				albumart_visible_attrib.setData("1");
		else
				albumart_visible_attrib.setData("0");
		complete;
	}
}

waaa.onRightButtonDown (int x, int y)
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
		waaa.refresh();
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

waaa.onLeftButtonDblClk (int x, int y)
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