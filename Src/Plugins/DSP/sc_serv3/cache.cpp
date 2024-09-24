#ifdef _WIN32
#include <winsock2.h>
#endif
#include <map>
#include "cache.h"

void AddorUpdateCache(cacheItem *item, const time_t now, const bool compressed,
					  const uniString::utf8 &header, const uniString::utf8 &response,
					  CacheMap_t &cache, AOL_namespace::mutex &lock,
					  const streamData::streamID_t sid)
{
	if (lock.timedLock(3000))
	{
		// if we've already got an instance then we update
		if (item == NULL)
		{
			item = cache[sid] = new cacheItem();
		}

		if (item != NULL)
		{
			if (compressed)
			{
				item->generatedGZIP = now;
				item->responseGZIP = response;
				item->headerGZIP = header;
			}
			else
			{
				item->generatedRaw = now;
				item->responseRaw = response;
				item->headerRaw = header;
			}
		}
		lock.unlock();
	}
}

bool GetFromCache(const cacheItem *item, AOL_namespace::mutex &lock,
				  const time_t now, const bool compressed,
				  const bool headRequest, uniString::utf8 &response,
				  const int limit)
{
	if (lock.timedLock(3000))
	{
		if (item != NULL)
		{
			if (((now - item->generatedGZIP) < limit) && compressed)
			{
				if (item->responseGZIP.size() > 0)
				{
					response = (!headRequest ? item->responseGZIP : item->headerGZIP);
					lock.unlock();
					return true;
				}
			}
			else if (((now - item->generatedRaw) < limit) && !compressed)
			{
				if (item->responseRaw.size() > 0)
				{
					response = (!headRequest ? item->responseRaw : item->headerRaw);
					lock.unlock();
					return true;
				}
			}
		}
		lock.unlock();
	}
	return false;
}

void DeleteCache(CacheMap_t &cache)
{
	if (!cache.empty())
	{
		for (CacheMap_t::const_iterator i = cache.begin(); i != cache.end(); ++i)
		{
			delete (*i).second;
		}
		cache.clear();
	}
}

void DeleteAllCaches()
{
	DeleteCache(m_xmlStatsCache);
	DeleteCache(m_xmlStatisticsCache);
	DeleteCache(m_jsonStatsCache);
	DeleteCache(m_jsonStatisticsCache);
	DeleteCache(m_7Cache);
	DeleteCache(m_PLSCache);
	DeleteCache(m_M3UCache);
	DeleteCache(m_ASXCache);
	DeleteCache(m_QTLCache);
	DeleteCache(m_XSPFCache);
	DeleteCache(m_xmlTracksCache);
	DeleteCache(m_jsonTracksCache);
	DeleteCache(m_xmlPlayedCache);
	DeleteCache(m_jsonPlayedCache);
	DeleteCache(m_htmlPlayedCache);
	DeleteCache(m_streamArtCache);
	DeleteCache(m_playingArtCache);
	DeleteCache(m_crossdomainCache);
}
