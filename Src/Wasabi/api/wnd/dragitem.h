#ifndef _DRAGITEM_H
#define _DRAGITEM_H

#include <bfc/dispatch.h>

// has 1 or more pointers to a named data type
class NOVTABLE DragItem : public Dispatchable
{
public:
	const wchar_t *getDatatype() { return _call(GETDATATYPE, L""); }
	int getNumData() { return _call(GETNUMDATA, 0); }
	void *getDatum(int pos = 0) { return _call(GETDATUM, (void*)NULL, pos); }

protected:
	DISPATCH_CODES
	{
	    GETDATATYPE = 100,
	    GETNUMDATA = 200,
	    GETDATUM = 300
	};
};

#endif
