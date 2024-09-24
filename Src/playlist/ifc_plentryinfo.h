#ifndef NULLSOFT_IFC_PLENTRYINFO_H
#define NULLSOFT_IFC_PLENTRYINFO_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class ifc_plentryinfo : public Dispatchable
{
protected:
	ifc_plentryinfo()                                                 {}
	~ifc_plentryinfo()                                                {}

public:
	// TODO: you can't guarantee that this wchar_t * pointer will last, make a copy before calling again!
	const wchar_t *GetExtendedInfo( const wchar_t *parameter );

	DISPATCH_CODES
	{
		IFC_PLENTRYINFO_GETEXTENDEDINFO = 10,
	};
};

inline const wchar_t *ifc_plentryinfo::GetExtendedInfo( const wchar_t *parameter )
{
	return _call( IFC_PLENTRYINFO_GETEXTENDEDINFO, (const wchar_t *)0, parameter );
}

#endif