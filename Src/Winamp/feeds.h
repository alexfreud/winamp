#ifndef NULLSOFT_WINAMP_FEEDS_H
#define NULLSOFT_WINAMP_FEEDS_H

#include "FeedBase.h"

class VideoTextFeed : public FeedBase
{
public:
	int hasFeed(const wchar_t *name);
	const wchar_t *getFeedText(const wchar_t *name);
	const wchar_t *getFeedDescription(const wchar_t *name);
	void UpdateText(const wchar_t *text, int length);
};

class PlaylistTextFeed : public FeedBase
{
public:
	int hasFeed(const wchar_t *name);
	const wchar_t *getFeedText(const wchar_t *name);
	const wchar_t *getFeedDescription(const wchar_t *name);
	void UpdateText(const wchar_t *text, int length);
};

#endif