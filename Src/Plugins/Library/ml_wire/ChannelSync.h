#ifndef NULLSOFT_CHANNELSYNCH
#define NULLSOFT_CHANNELSYNCH

#include "Feeds.h"

/*
ChannelSync is a virtual base class (aka interface) used by the RSS downloader.
When you instantiate a downloader, you are required to give it a pointer to this interface.
 
The downloader will call:
 
	BeginChannelSync();
	for (;;) // however many it encounters
	  NewChannel(newChannel); // called once for each channel it downloads
	EndChannelSync();
 
If you have a class or data structure that wants updates, you'll have to mix-in
this interface or implement a data-structure-babysitter class (or however you want to deal with it)
 
See the "WireManager" for an example of how to use it
*/

class ChannelSync
{
public:
	virtual void BeginChannelSync() {}
	virtual void NewChannel(const Channel &newChannel) = 0;
	virtual void EndChannelSync() {}}
;

#endif