#ifndef NULLSOFT_DEFAULTSH
#define NULLSOFT_DEFAULTSH
#include <windows.h>
extern wchar_t defaultDownloadPath[MAX_PATH], serviceUrl[1024];
extern __time64_t updateTime;
extern int autoDownloadEpisodes;
extern bool autoUpdate;
extern bool autoDownload;
extern bool updateOnLaunch;
extern float htmlDividerPercent;
extern float channelDividerPercent;
extern int itemTitleWidth;
extern int itemDateWidth;
extern int itemMediaWidth;
extern int itemSizeWidth;

#define DOWNLOADSCHANNELWIDTHDEFAULT 200
#define DOWNLOADSITEMWIDTHDEFAULT 200
#define DOWNLOADSPROGRESSWIDTHDEFAULT 100
#define DOWNLOADSPATHWIDTHDEFAULTS 200

extern int downloadsChannelWidth;
extern int downloadsItemWidth;
extern int downloadsProgressWidth;
extern int downloadsPathWidth;

extern bool needToMakePodcastsView;

extern int currentItemSort;
extern bool itemSortAscending;
extern bool channelSortAscending;
extern int channelLastSelection;
void BuildDefaultDownloadPath(HWND);
#endif