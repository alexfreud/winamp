#include <lib/std.mi>
#include "attribs.m"

Function reset();
Function createNotifier(boolean cancel);
Function showNotifier(Int w);
Function onNext();
function cancelAnimation();

Function Int fillNextTrackInfo(String corneroverride);
Function Int fillCustomInfo(String customstring);
Function prepareAlbumArtNotifier();

Function checkPref(int bypassfs);

Function String getArtist();

Function onAlbumArt(Boolean success);

Global Container notifier_container;
Global Layout notifier_layout;
Global Timer notifier_timer;
Global String last_autotitle, cur_status;

Global AlbumArtLayer cover; 
Global boolean handleAACalback = false;

Global Boolean triggerBug; //BUGFIX remove this
Global Timer fallbackTempFix; //BUGFIX remove this

Global Boolean b_tohide = 0;
Global int width = 0;
Global int left = 0;

#define WEBCOVER_SHOUTCAST "winamp.cover.shoutcast"
#define FORCE_BUG_MODE 

// ------------------------------------------------------------------------------
// init
// ------------------------------------------------------------------------------
System.onScriptLoaded() {
	initAttribs();
	notifier_timer = new Timer;
	fallbackTempFix = new Timer; //BUGFIX remove this
	fallbackTempFix.setDelay(3000); //BUGFIX remove this / Time that onAlbumArtLoaded have to execute before bug mode is ON 
}

// ------------------------------------------------------------------------------
// shutdown
// ------------------------------------------------------------------------------
System.onScriptUnloading() {
	delete notifier_timer;
	delete fallbackTempFix; //BUGFIX remove this
}

// ------------------------------------------------------------------------------
// called by the system when the global hotkey for notification is pressed
// ------------------------------------------------------------------------------
System.onShowNotification() {
	//if (checkPref(1)) return; --mp: if we push the hotkey, we want to show the notifier, no matter what the pref settings are.
	createNotifier(false);
	if (getStatus() == STATUS_PLAYING) cur_status = "Playing";
	if (getStatus() == STATUS_PAUSED) cur_status = "Playback Paused";
	if (getStatus() == STATUS_STOPPED) cur_status = "Playback Stopped";
	prepareAlbumArtNotifier();
	complete; // prevents other scripts from getting the message
	return 1; // tells anybody else that might watch the returned value that, yes, we implemented that
}

// ------------------------------------------------------------------------------
// called by the system when the title for the playing item changes, this could be the result of the player
// going to the next track, or of an update in the track title
// ------------------------------------------------------------------------------
System.onTitleChange(String newtitle) {
	if (last_autotitle == newtitle) return;
	if (StrLeft(newtitle, 1) == "[") {
	if (StrLeft(newtitle, 7) == "[Buffer" ||
		StrLeft(newtitle, 4) == "[ICY") return;
	}
	last_autotitle = newtitle;
	onNext();

	fallbackTempFix.stop(); //BUGFIX remove later
	fallbackTempFix.start(); //BUGFIX remove later
}

// ------------------------------------------------------------------------------
// called by the system when the user clicks the next button
// ------------------------------------------------------------------------------
onNext() {
	if (checkPref(0)) return;
	createNotifier(false);
	cur_status = "";
	prepareAlbumArtNotifier();
}

// ------------------------------------------------------------------------------
// called by the system when the user clicks the play button
// ------------------------------------------------------------------------------
System.onPlay() {
	if (checkPref(0)) return;
	createNotifier(false);
	cur_status = "Playing";
	prepareAlbumArtNotifier();
}

// ------------------------------------------------------------------------------
// called by the system when the user clicks the pause button
// ------------------------------------------------------------------------------
System.onPause() {
	if (checkPref(0)) return;
	DebugString("onPause",9);
	createNotifier(true);
	showNotifier(fillCustomInfo("Playback Paused"));
}

// ------------------------------------------------------------------------------
// called by the system when the user clicks the pause button again
// ------------------------------------------------------------------------------
System.onResume() {
	if (checkPref(0)) return;
	DebugString("onResume",9);
	createNotifier(false);
	cur_status = "Resuming Playback";
	prepareAlbumArtNotifier();
}

// ------------------------------------------------------------------------------
// called by the system when the user clicks the play button
// ------------------------------------------------------------------------------
System.onStop() {
	if (checkPref(0)) return;
	createNotifier(true);
	showNotifier(fillCustomInfo("End of Playback"));
}

// ------------------------------------------------------------------------------
// checks if we should display anything
// ------------------------------------------------------------------------------
Int checkPref(int bypassfs) {
	if (!bypassfs && notifier_disablefullscreen_attrib.getData() == "1" && isVideoFullscreen()) return 1;
	if (notifier_never_attrib.getData() == "1") return 1;
	if (notifier_minimized_attrib.getData() == "1" && !isMinimized()) return 1;
	if (notifier_windowshade_attrib.getData() == "1") {
		if (isMinimized()) return 0;
		Container c = getContainer("main");
		if (!c) return 1;
		Layout l = c.getCurLayout();
		if (!l) return 1;
		if (l.getId() != "shade") return 1;
	}
	return 0;
}

// ------------------------------------------------------------------------------
// fade in/out completed
// ------------------------------------------------------------------------------
notifier_layout.onTargetReached() {
	int a = notifier_layout.getAlpha();
	if (a == 255) {
		notifier_timer.setDelay(StringToInteger(notifier_holdtime_attrib.getData()));
		notifier_timer.start();
	}
	else if (a == 0) {
		reset();
	}
}

// ------------------------------------------------------------------------------
// hold time elapsed
// ------------------------------------------------------------------------------
notifier_timer.onTimer() {
	stop();
	if (notifier_layout.isTransparencySafe()) {
		notifier_layout.setTargetA(0);
		notifier_layout.setTargetSpeed(StringToInteger(notifier_fadeouttime_attrib.getData()) / 1000);
		notifier_layout.gotoTarget();
	} else {
		reset();
	}
}

// ------------------------------------------------------------------------------
// when notifier is clicked, bring back the app from minimized state if its minimized and focus it
// ------------------------------------------------------------------------------
notifier_layout.onLeftButtonDown(int x, int y) {
	cancelAnimation();

	restoreApplication();
	activateApplication();
	/*if (notifier_opennowplaying_attrib.getData() == "1")
	{
		String artist = getArtist();
		if (artist == "") return;
		System.navigateUrlBrowser("http://client.winamp.com/nowplaying/artist/?icid=notifiermodern&artistName=" + artist);	
	}*/

	reset();
}

notifier_layout.onRightButtonUp(int x, int y) {
	cancelAnimation();
	reset();
	complete;
	return;
}

//TODO merge w/ code below
String getArtist ()
{
	String artist = getPlayItemMetaDataString("artist");
	if (artist == "") artist = getPlayItemMetaDataString("uvox/artist");
	if (artist == "") artist = getPlayItemMetaDataString("cbs/artist");
	if (artist == "") artist = getPlayItemMetaDataString("streamtitle");
	if (artist == "") artist = getPlayItemDisplayTitle();

	return artist;
}

// ------------------------------------------------------------------------------
// close the notifier window, destroys the container automatically because it's dynamic
// ------------------------------------------------------------------------------
reset() {
	notifier_container.close();
	notifier_container = NULL;
	notifier_layout = NULL;
	handleAACalback = FALSE;
}

// ------------------------------------------------------------------------------
createNotifier(boolean cancel) {
	if (notifier_container == NULL) {
		notifier_container = newDynamicContainer("notifier");
		if (!notifier_container) return; // reinstall duh!
		if (isDesktopAlphaAvailable())
			notifier_layout = notifier_container.getLayout("desktopalpha");
		else
			notifier_layout = notifier_container.getLayout("normal");
		if (!notifier_layout) return; // reinstall twice, man
	} else if (cancel) {
		cancelAnimation();
	}
}

cancelAnimation()
{
	notifier_layout.cancelTarget();
	notifier_timer.stop();
}

// ------------------------------------------------------------------------------
showNotifier(int w) {
	w = w + 32;

	Layout m = getContainer("main").getCurLayout();
	int x = left = getViewportWidthFromGuiObject(m) + getViewportLeftFromGuiObject(m) - w - 2;
	int y = getViewportHeightFromGuiObject(m) + getViewportTopFromGuiObject(m) - 80 - 2;

	// show if not there or if already shown then lets resize on the fly (bento style)
	if (!notifier_layout.isVisible()) {
		notifier_layout.resize(x, y, w, 80);
	}
	else {
		notifier_layout.resize(notifier_layout.getguiX(), y, notifier_layout.getGuiW(), 80);
	}

	if (notifier_layout.isTransparencySafe()) {
		notifier_layout.show();
		notifier_layout.setTargetA(255);
		notifier_layout.setTargetX(x);
		notifier_layout.setTargetY(y);
		notifier_layout.setTargetW(w);
		notifier_layout.setTargetH(80);
		notifier_layout.setTargetSpeed(StringToInteger(notifier_fadeintime_attrib.getData()) / 1000);
		notifier_layout.gotoTarget();
	} else {
		notifier_layout.setAlpha(255);
		notifier_layout.show();
		notifier_timer.setDelay(StringToInteger(notifier_holdtime_attrib.getData()));
		notifier_timer.start();
	}
}

// ------------------------------------------------------------------------------

prepareAlbumArtNotifier()
{
	if (!notifier_layout) return;
	Group g_albumart = notifier_layout.findObject("notifier.albumart");

	DebugString("prepareAlbumArtNotifier: handleAACalback="+integerToString(handleAACalback), 9);
	if (g_albumart)
	{
		cover = g_albumart.findObject("notifier.cover");
		DebugString("prepareAlbumArtNotifier: cover.isLoading="+integerToString(cover.isLoading()), 9);
		DebugString("prepareAlbumArtNotifier: cover.isInvalid="+integerToString(cover.isInvalid()), 9);
		handleAACalback = true;
		cover.refresh();
	}
}

cover.onAlbumArtLoaded(boolean success)
{
	/*
	Created a seperate function for the code that was here because for some reason I couldn't force this
	event (from the fallbackTempFix.onTimer) with cover.onAlbumArtLoaded(success) after the Winamp bug appears.
	Weird, yes.
	*/
	FORCE_BUG_MODE onAlbumArt(success); 
}

// ------------------------------------------------------------------------------
Int fillNextTrackInfo(String corneroverride) {
	Int maxv = 0;
	Int stream = 0;

	if (!notifier_layout) return 0;

	Group g_text = notifier_layout.findObject("notifier.text");
	Group g_albumart = notifier_layout.findObject("notifier.albumart");

	Text plentry = g_text.findObject("plentry");
	Text nexttrack = g_text.findObject("nexttrack");
	Text _title = g_text.findObject("title");
	Text album = g_text.findObject("album");
	Text artist = g_text.findObject("artist");
	Text endofplayback = notifier_layout.findObject("endofplayback");

	DebugString("got callback for " + getPlayItemString(), 0);

	// Get Stream Name - if no stream returns ""
	string s = getPlayItemMetaDataString("streamname");
	string stype = getPlayItemMetaDataString("streamtype"); //"streamtype" will return "2" for SHOUTcast and "5" for SHOUTcast 2
	if (stype == "2" || stype == "5") stream = 1;

	if (endofplayback) endofplayback.hide();

	if (plentry)
	{
		plentry.setText(integerToString(getPlaylistIndex()+1)+translate(" of ")+integerToString(getPlaylistLength()));
		plentry.show();
	}
	if (nexttrack) {
		if (corneroverride == "") {
			if (!stream) {
				if (!isVideo())
				{
					nexttrack.setText("New track");
				}
				else
				{
					nexttrack.setText("New video");
				}
			}
			else
			{
				nexttrack.setText("On air");
			}
		}
		else
		{
			nexttrack.setText(corneroverride);
		}
		nexttrack.show();
	}

	string set_artist = "";
	string set = "";
	if (_title) {
		String str;
		if (!stream)
		{
			_title.setXmlParam("ticker", "0");
			_title.setXmlParam("display", "");

			str = getPlayitemMetaDataString("title"); 
			if (str == "") str = getPlayitemDisplayTitle();
			String l = getPlayItemMetaDataString("length");
			if (l != "") {
				str += " (" + integerToTime(stringtointeger(l)) + ")";
			}
			_title.setText(str);
		}
		else
		{
			if (str = getPlayItemMetaDataString("streamtitle") != "")
			{
				int v = strsearch(str, " - "); // We divide the string by a " - " sublimiter - no luck for old / wrong tagged stations
				if (v > 0) {
					set_artist = strleft (str, v); // Store artist
					string str = strright (str, strlen(str) - 3 - v);
					_title.setText(str);
				}
				else
				{
					_title.setXmlParam("ticker", "1"); // These titles can be _very_ long
					_title.setText(str);
				}
			} else
			{
				_title.setXmlParam("ticker", "1");
				_title.setXmlParam("display", "songtitle");
				_title.setText("");
			}
		}
		_title.show();
	}

	if (artist) {
		if (!stream) {
			if (isVideo())
			{
				artist.setText("");
			}
			else
			{
				artist.setText(getPlayitemMetaDataString("artist"));
			}
		}
		else
		{
			// Perhaps we've stored the artist before?
			if (set_artist != "")
			{
				artist.setText(set_artist);
			}
			// Then display the station name
			else if (s != "")
			{
				artist.setText(s);
			}
			// So, we've had no luck - just display a static text :(
			else
			{
				if (isVideo())
				{
					artist.setText("Internet TV"); 
				}
				else
				{
					artist.setText("Internet Radio");
				}
			}
		}
		artist.show();
	}

	if (album) { 
		String str;
		if (!stream && !isVideo()) {
			album.setXmlParam("display", "");
			str = getPlayitemMetaDataString("album");
			String l = getPlayitemMetaDataString("track");
			if (l != "" && l != "-1") str += " (" + translate("Track ") + l + ")";
			album.setText(str);
		}
		else
		{
			album.setXmlParam("display", "");
			// we have divided the songname - let's display the station name
			if (set_artist != "" && s != "")
			{
				album.setText(s);
			}
			// no luck either...
			else
			{
				album.setText("");
				album.setXmlParam("display", "songinfo_localise");
			}
		}
		album.show();
	}

	// Album Art Stuff

	Layer webcover;
	if (g_albumart)
	{
		cover = g_albumart.findObject("notifier.cover");
		webcover = g_albumart.findObject("notifier.webcover");
	}

	Boolean showAlbumArt = FALSE;

	if (cover != NULL && webcover != NULL && notifier_artworkinnotification_attrib.getData() == "1")
	{
		if (stream)
		{
			if(stype == "2" || stype == "5" && cover.isInvalid())
			{
				webcover.setXMLParam("image", WEBCOVER_SHOUTCAST);
				cover.hide();
				webcover.show();
				showAlbumArt = TRUE;
			}
			else if(stype == "5" && !cover.isInvalid())
			{
				webcover.hide();
				cover.show();
				showAlbumArt = TRUE;
			}
		}
		else
		{
			if (cover.isInvalid()) // Check if the album art obj shows a pic
			{
				showAlbumArt = FALSE;
			}
			else
			{
				webcover.hide();
				cover.show();
				showAlbumArt = TRUE;
			}
		}
	}

	if (showAlbumArt)
	{
		if (g_albumart) g_albumart.show();
		if (g_text) g_text.setXmlParam("x", "75");
		if (g_text) g_text.setXmlParam("w", "-95");
	}
	else
	{
		if (g_albumart) g_albumart.hide();
		if (g_text) g_text.setXmlParam("x", "15");
		if (g_text) g_text.setXmlParam("w", "-35");
	}

	if (g_text) g_text.show();

	maxv = artist.getAutoWidth();
	if (maxv < album.getAutoWidth()) maxv = album.getAutoWidth();
	if (maxv < _title.getAutoWidth()) maxv = _title.getAutoWidth();
	if (maxv < (nexttrack.getAutoWidth() + plentry.getAutoWidth() - 5) ) maxv = nexttrack.getAutoWidth() + plentry.getAutoWidth() - 5;
	if (maxv < 128) maxv = 128;
	Layout m = getContainer("main").getCurLayout();
	if (maxv > getViewportWidthFromGuiObject(m)/4) maxv = getViewportWidthFromGuiObject(m)/4;

	width = maxv;

	return maxv + ( showAlbumArt * 91 );
}

// ------------------------------------------------------------------------------
Int fillCustomInfo(String customtext)
{
	Group p = notifier_layout;
	Group g_text = p.findObject("notifier.text");
	Group g_albumart = p.findObject("notifier.albumart");
	Text plentry = p.findObject("plentry");
	Text nexttrack = p.findObject("nexttrack");
	Text _title = p.findObject("title");
	Text album = p.findObject("album");
	Text artist = p.findObject("artist");
	Text endofplayback = p.findObject("endofplayback");

	if (g_text) { g_text.hide(); }
	if (g_albumart) g_albumart.hide();

	if (plentry) { plentry.hide(); }
	if (nexttrack) nexttrack.hide();
	if (_title) { _title.hide(); }
	if (artist) { artist.hide(); }
	if (album) { album.hide(); }

	if (endofplayback != NULL /*&& s_endofplayback  != NULL*/) {
		endofplayback.setText(translate(customtext)+" ");
		//s_endofplayback.setText(translate(customtext)+" ");
		int aw = endofplayback.getAutoWidth();
		endofplayback.show();
		//s_endofplayback.show();
		if (aw > 128)
			return aw;
	}
	return 128;
}

//BUGFIX remove this timer later
fallbackTempFix.onTimer() //As soon as this timer runs, bug mode is ON ;)
{
	if (checkPref(0)) return;

	if (!notifier_layout) onNext();

	if(!triggerBug)
	{
		triggerBug = true;

		onAlbumArt(cover.isInvalid()); //First time we see the bug
		fallbackTempFix.setDelay(30);
		
		DebugString("Hello Bug", 9);
	}
	else if(triggerBug && !cover.isLoading()) onAlbumArt(cover.isInvalid());
}

onAlbumArt(Boolean success){
	fallbackTempFix.stop(); //BUGFIX remove later

	DebugString("onAlbumArtLoaded: success="+integerToString(success), 9);
	DebugString("onAlbumArtLoaded: handleAACalback="+integerToString(handleAACalback), 9);
	DebugString("onAlbumArtLoaded: cover.isLoading="+integerToString(cover.isLoading()), 9);
	DebugString("onAlbumArtLoaded: cover.isInvalid="+integerToString(cover.isInvalid()), 9);
	if (!handleAACalback || !notifier_layout /*|| isLoading()*/)
	{
		return;
	}

	handleAACalback = cover.isLoading();
	cancelAnimation();
	showNotifier(fillNextTrackInfo(cur_status));
}
