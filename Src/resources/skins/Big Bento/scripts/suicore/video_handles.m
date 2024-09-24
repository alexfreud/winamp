/*---------------------------------------------------
-----------------------------------------------------
Filename:	video_handles.m
Version:	2.0

Type:		maki
Date:		28. Okt. 2006 - 16:30 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
Depending Files:
		scripts/suicore.maki
-----------------------------------------------------
---------------------------------------------------*/

#ifndef included
#error This script can only be compiled as a #include
#endif

#define SKINTWEAKS_CFGPAGE "{0542AFA4-48D9-4c9f-8900-5739D52C114F}"
#define VIDEO_CONFIG_GROUP "{2135E318-6919-4bcf-99D2-62BE3FCA8FA6}"

Function initVideo();
Function disablePSOVC();
Function enablePSOVC();

Global Timer PSOVCTimer;
Global string psovc_save;

Global WinampConfigGroup cfg_Video;

Global Boolean lastWasVideo;

initVideo ()
{
    play_auto_fs_video = 0;
    
	PSOVCTimer = new Timer;
	PSOVCTimer.setDelay(1000);

	cfg_Video = WinampConfig.getGroup(VIDEO_CONFIG_GROUP);
}

/** Prevent video playback to stop after the wnd is hidden */

disablePSOVC()
{
	debugString("[suicore.m] " + "--> disabling stop on video close",0 );
	ConfigItem item = Config.getItem(SKINTWEAKS_CFGPAGE);
	if (item)
	{
		ConfigAttribute attr = item.getAttribute("Prevent video playback Stop on video window Close");
		if (attr) psovc_save = attr.getData();
		if (attr) attr.setData("1");
	}
	PSOVCTimer.start();
	debugString("[suicore.m] " + "--> PSOVCTimer.started();",0 );
}

enablePSOVC()
{
	debugString("[suicore.m] " + "--> enabling stop on video close",0 );
	PSOVCTimer.stop();
	ConfigItem item = Config.getItem(SKINTWEAKS_CFGPAGE);
	if (item)
	{
		ConfigAttribute attr = item.getAttribute("Prevent video playback Stop on video window Close");
		if (attr) attr.setData(psovc_save);
	}
	debugString("[suicore.m] " + "--> PSOVCTimer.stopped();",0 );
}

PSOVCTimer.onTimer()
{
	enablePSOVC();
}

System.onPlay()
{
    // needed to handle video auto fullscreen on play
    boolean auto_fs = cfg_Video.getBool("auto_fs");
    if (auto_fs) play_auto_fs_video = 1;
    else play_auto_fs_video = 0;
    
    // removed for debugging aims
	// if (isVideo() && cfg_Video.getBool("autoopen") && !sui_video.isVisible()) dc_showVideo();
}

System.onPause()
{
    play_auto_fs_video = 0;
}

System.onResume()
{
    play_auto_fs_video = 0;
    
    // removed for debugging aims
	// if (isVideo() && cfg_Video.getBool("autoopen") && !sui_video.isVisible()) dc_showVideo();
}

System.onStop()
{
    play_auto_fs_video = 0;
}

System.onTitleChange(String newtitle)
{
    // needed to handle video auto fullscreen on play
    boolean auto_fs = cfg_Video.getBool("auto_fs");
    if (auto_fs) play_auto_fs_video = 1;
    else play_auto_fs_video = 0;
    
    /* removed for debugging aims
	if (startup)
	{
		lastWasVideo = isVideo();
		return;
	}
	
	if (isVideo() && cfg_Video.getBool("autoopen") && !sui_video.isVisible() && getStatus() != 0) dc_showVideo();
	else if (!isVideo() && cfg_Video.getBool("autoclose") && sui_video.isVisible() && lastWasVideo) showML();
	lastWasVideo = isVideo();
    */
}

//S/ystem.onTitleChange (String newtitle)
//{
	/*if (startup)
	{
		lastWasVideo = isVideo();
		return;
	}*/
	
	/*if (!startup && isVideo() && cfg_Video.getBool("autoopen") && !sui_video.isVisible() && getStatus() != 0)
	{
		callbackTimer.stop();
		tempDisable.stop();
		switchToVideo();
		debug("");
	}*/
	//else if (!isVideo() && cfg_Video.getBool("autoclose") && sui_video.isVisible() && lastWasVideo) showML();
	//lastWasVideo = isVideo();
//}