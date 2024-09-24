#pragma once

#include <bfc/dispatch.h>
#include <api/service/services.h>
class api_amgsucks : public Dispatchable
{
protected:
	api_amgsucks() {}
	~api_amgsucks() {}
public:
	int WriteAlbumArt(const void *data, size_t data_len, void **out, int *out_len);

	DISPATCH_CODES
	{
		WRITEALBUMART = 0,
	};
};

inline int api_amgsucks::WriteAlbumArt(const void *data, size_t data_len, void **out, int *out_len)
{
	return _call(WRITEALBUMART, (int)2, data, data_len, out, out_len);
}

// {E93907C8-8CFD-47dc-87FC-80B5B03716CB}
static const GUID amgSucksGUID =
{ 0xe93907c8, 0x8cfd, 0x47dc, { 0x87, 0xfc, 0x80, 0xb5, 0xb0, 0x37, 0x16, 0xcb } };


class AMGSucks : public api_amgsucks
{
public:
	static FOURCC getServiceType() { return WaSvc::UNIQUE; }
	static const char *getServiceName() { return "AMG Sucks"; }
	static GUID getServiceGuid() { return amgSucksGUID; }
private:
	int WriteAlbumArt(const void *data, size_t data_len, void **out, int *out_len);
protected:
	RECVS_DISPATCH;
};