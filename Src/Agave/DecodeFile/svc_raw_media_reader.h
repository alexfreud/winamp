#pragma once

#include <bfc/dispatch.h>
#include "ifc_raw_media_reader.h"
#include <bfc/error.h>

class svc_raw_media_reader : public Dispatchable
{
protected:
	svc_raw_media_reader() {}
	~svc_raw_media_reader() {}
public:
	static FOURCC getServiceType() { return svc_raw_media_reader::SERVICETYPE; }
	int CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader);
public:
	DISPATCH_CODES
	{
		CREATERAWMEDIAREADER = 0,
	};

	
	enum
	{
		SERVICETYPE = MK4CC('r','a','w','m')
	};
};

inline int svc_raw_media_reader::CreateRawMediaReader(const wchar_t *filename, ifc_raw_media_reader **reader)
{
	return _call(CREATERAWMEDIAREADER, (int)NErr_NotImplemented, filename, reader);
}

