#ifndef NULLSOFT_WINAMP_VIDEOFEEDFACTORY_H
#define NULLSOFT_WINAMP_VIDEOFEEDFACTORY_H

#include <api/service/waservicefactorybase.h>
#include <api/service/services.h>
#include <api/skin/feeds/api_textfeed.h>
#include "api.h"

// {BD838BA9-1CE8-4f73-BBC0-58DA5168517F}
static const GUID videoFeedFactoryGUID = 
{ 0xbd838ba9, 0x1ce8, 0x4f73, { 0xbb, 0xc0, 0x58, 0xda, 0x51, 0x68, 0x51, 0x7f } };


class VideoTextFeedFactory : public waServiceBase<svc_textFeed, VideoTextFeedFactory> {
public:
  VideoTextFeedFactory() : waServiceBase<svc_textFeed, VideoTextFeedFactory>(videoFeedFactoryGUID) {}
  static const char *getServiceName() { return "Video Text Feed"; }
  svc_textFeed *getService() { return videoTextFeed; }
  static FOURCC getServiceType() { return WaSvc::TEXTFEED; }
};

// {87291C37-EC56-476b-B813-ED0F6F90C3B7}
static const GUID playlistFeedFactory = 
{ 0x87291c37, 0xec56, 0x476b, { 0xb8, 0x13, 0xed, 0xf, 0x6f, 0x90, 0xc3, 0xb7 } };


class PlaylistTextFeedFactory : public waServiceBase<svc_textFeed, PlaylistTextFeedFactory> {
public:
  PlaylistTextFeedFactory() : waServiceBase<svc_textFeed, PlaylistTextFeedFactory>(playlistFeedFactory) {}
  static const char *getServiceName() { return "Playlist Text Feed"; }
  svc_textFeed *getService() { return playlistTextFeed; }
  static FOURCC getServiceType() { return WaSvc::TEXTFEED; }
};

#endif