#ifndef NULLSOFT_XMLWRITERH
#define NULLSOFT_XMLWRITERH
#include "Wire.h"
#include "Downloaded.h"
void SaveChannels(const wchar_t *fileName, ChannelList &channels);

void SaveSettings(const wchar_t *fileName, DownloadList &downloads);
#endif