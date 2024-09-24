#ifndef NULLSOFT_FEEDSH
#define NULLSOFT_FEEDSH

#include "ifc_podcast.h"
#include "Item.h"
#include <vector>

class Channel : public ifc_podcast
{
public:
	typedef std::vector<RSS::Item> ItemList;
	Channel();
	Channel(const Channel &copy);
	const Channel &operator =(const Channel &copy);
	~Channel();
	void SortByTitle(), SortByMedia(), SortByMediaTime(), SortByDate(), SortByMediaSize();
	bool operator == (const Channel &compare);
	//void operator = (const Channel &copy);
	void UpdateFrom(const Channel &copy);

	unsigned int ttl;
	__time64_t updateTime, lastUpdate;
	int autoDownloadEpisodes;
	bool autoUpdate;
	bool useDefaultUpdate;
	bool autoDownload;
	bool needsRefresh; 
	// TODO: std::wstring downloadLocation;
	ItemList items;

	void SetURL(const wchar_t *val);
	void SetTitle(const wchar_t *val);
	void SetLink(const wchar_t *val);
	void SetDescription(const wchar_t *val);

	wchar_t *url, *title, *link, *description;

public: // ifc_podcast interface
	int GetTitle(wchar_t *str, size_t len);

private:
	void Init();
	void Reset();


protected:
	RECVS_DISPATCH;
};

#endif