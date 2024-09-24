#pragma once
#ifndef _CACHE_H
#define _CACHE_H

#include "unicode/uniString.h"
#include "streamData.h"

struct cacheItem
{
	time_t generatedGZIP;
	time_t generatedRaw;
	uniString::utf8 headerGZIP;
	uniString::utf8 headerRaw;
	uniString::utf8 responseGZIP;
	uniString::utf8 responseRaw;

	cacheItem() : generatedGZIP(0), generatedRaw(0)
	{
	}

	~cacheItem()
	{
		generatedGZIP = generatedRaw = 0;
		headerGZIP.clear();
		headerRaw.clear();
		responseGZIP.clear();
		responseRaw.clear();
	}
};

typedef std::map<size_t, cacheItem *> CacheMap_t;

void AddorUpdateCache(cacheItem *item, const time_t now, const bool compressed,
					  const uniString::utf8 &header, const uniString::utf8 &response,
					  CacheMap_t &cache, AOL_namespace::mutex &lock,
					  const streamData::streamID_t sid);

bool GetFromCache(const cacheItem *item, AOL_namespace::mutex &lock,
				  const time_t now, const bool compressed,
				  const bool headRequest, uniString::utf8 &response,
				  const int limit = 1);

void DeleteCache(CacheMap_t &cache);
void DeleteAllCaches();

extern CacheMap_t m_xmlStatsCache, m_xmlStatisticsCache, m_jsonStatsCache,
				  m_jsonStatisticsCache, m_7Cache, m_PLSCache, m_M3UCache,
				  m_ASXCache, m_QTLCache, m_XSPFCache, m_xmlTracksCache,
				  m_jsonTracksCache, m_xmlPlayedCache, m_jsonPlayedCache,
				  m_htmlPlayedCache, m_streamArtCache, m_playingArtCache,
				  m_crossdomainCache;
#endif
