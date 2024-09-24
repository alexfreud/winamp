#ifndef __OSWNDHOST_H
#define __OSWNDHOST_H

#include <bfc/dispatch.h>

// {7050AACF-2731-4319-AF32-4ECE4CC8BDC4}
static const GUID osWndHostGuid =
    { 0x7050aacf, 0x2731, 0x4319, { 0xaf, 0x32, 0x4e, 0xce, 0x4c, 0xc8, 0xbd, 0xc4 } };


class OSWndHost : public Dispatchable
{
public:
	void oswndhost_host(HWND oswnd);
	void oswndhost_unhost();
	void oswndhost_setRegionOffsets(RECT *r);

	DISPATCH_CODES
	{
	    OSWNDHOST_OSWNDHOST_HOST = 0,
	    OSWNDHOST_OSWNDHOST_UNHOST = 5,
	    OSWNDHOST_OSWNDHOST_SETREGIONOFFSETS = 10,
	};
};

inline void OSWndHost::oswndhost_host(HWND oswnd)
{
	_voidcall(OSWNDHOST_OSWNDHOST_HOST, oswnd);
}

inline void OSWndHost::oswndhost_unhost()
{
	_voidcall(OSWNDHOST_OSWNDHOST_UNHOST);
}

inline void OSWndHost::oswndhost_setRegionOffsets(RECT *r)
{
	_voidcall(OSWNDHOST_OSWNDHOST_SETREGIONOFFSETS, r);
}

class OSWndHostI : public OSWndHost
{
public:
	virtual void oswndhost_host(HWND oswnd) = 0;
	virtual void oswndhost_unhost() = 0;
	virtual void oswndhost_setRegionOffsets(RECT *r) = 0;

protected:
	RECVS_DISPATCH;
};


#endif
