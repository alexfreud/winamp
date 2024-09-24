#ifndef __WASABI_API_READERCALLBACK_H
#define __WASABI_API_READERCALLBACK_H

#include <bfc/dispatch.h>

class NOVTABLE api_readercallback : public Dispatchable
{
public:
	void metaDataReader_onData(const char *data, int size);

	enum 
	{
	    METADATAREADERONDATA = 10,
	};
};

inline void api_readercallback::metaDataReader_onData(const char *data, int size)
{
	_voidcall(METADATAREADERONDATA, data, size);
}

#endif
