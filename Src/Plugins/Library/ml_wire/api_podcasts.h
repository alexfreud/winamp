#ifndef NULLSOFT_API_ML_WIRE_PODCASTS_H
#define NULLSOFT_API_ML_WIRE_PODCASTS_H

#include <bfc/dispatch.h>

class ifc_podcastsubscribecallback;
class ifc_podcast;

class api_podcasts : public Dispatchable
{
protected:
	api_podcasts()                                                    {}
	~api_podcasts()                                                   {}

public:
	size_t GetNumPodcasts();
	ifc_podcast *EnumPodcast( size_t i );

	// TODO: locking mechanism like api_playlists
	// TODO: Remove playlists
	// TODO: int Subscribe(const wchar_t *url, ifc_podcastsubscribecallback *callback);
	/* TODO: method to download/parse a podcast channel w/o adding it to the list
			 maybe as part of a separate class? */

	enum
	{
		API_PODCASTS_GETNUMPODCASTS = 0,
		API_PODCASTS_ENUMPODCAST    = 1,
	};
};

inline size_t api_podcasts::GetNumPodcasts()
{
	return _call( API_PODCASTS_GETNUMPODCASTS, (size_t)0 );
}

inline ifc_podcast *api_podcasts::EnumPodcast( size_t i )
{
	return _call( API_PODCASTS_ENUMPODCAST, (ifc_podcast *)0, i );
}

// {4D2E9987-D955-45f0-A06F-371405E8B961}
static const GUID api_podcastsGUID = 
{ 0x4d2e9987, 0xd955, 0x45f0, { 0xa0, 0x6f, 0x37, 0x14, 0x5, 0xe8, 0xb9, 0x61 } };

#endif // !NULLSOFT_API_ML_WIRE_PODCASTS_H