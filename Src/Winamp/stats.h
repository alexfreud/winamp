#pragma once
#include "api_stats.h"
#include <api/service/services.h>

class Stats : public api_stats
{
public:
	static const char *getServiceName() { return "Anonymous Statistics API"; }
	static const GUID getServiceGuid() { return AnonymousStatsGUID; }	
	static FOURCC getServiceType() { return WaSvc::UNIQUE; }
public:
	Stats();
	void Init();
	void Write();

	/* API implementation */
	void SetStat(int stat, int value);
	void IncrementStat(int stat);
	void GetStats(int stats[NUM_STATS]) const;
	void SetString(const char *key, const wchar_t *value);
	void GetString(const char *key, wchar_t *value, size_t value_cch) const;

private:
	int values[NUM_STATS];
protected:
	RECVS_DISPATCH;
};

extern Stats stats;