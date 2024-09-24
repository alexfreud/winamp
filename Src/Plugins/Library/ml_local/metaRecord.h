#ifndef NULLSOT_LOCALMEDIA_METARECORD_H
#define NULLSOT_LOCALMEDIA_METARECORD_H

#include <map>
#include <vector>
#include <string>

// Links db record to the metadata struct
typedef struct 
{
	int		dbColumnId;
	char	*recordKey;
} LM_RECORD_LINK;

// cache size (records count)
#define CACHE_SIZE 100;

class MetaData
{

// construcotrs
public:
	MetaData();
	~MetaData();

// methods
private:
	// gets record data from db to the cache
	int GetDbColumnsCount();
	void GetDBRecordToCache(int64 recordId);


public:
	// returns pointer to metadata
	const char* GetMetaData(const char *metaKey);

// fields
private:


}


#endif //NULLSOT_LOCALMEDIA_METARECORD_H