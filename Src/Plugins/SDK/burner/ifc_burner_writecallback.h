#ifndef NULLSOFT_BURNER_IFC_BURNER_WRITECALLBACK_H
#define NULLSOFT_BURNER_IFC_BURNER_WRITECALLBACK_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class ifc_burner_writecallback : public Dispatchable
{
protected:
	ifc_burner_writecallback() {}
	~ifc_burner_writecallback() {}
public:
	int OnStatus(uint32_t sectorsWritten, uint32_t totalSectors); // return 1 to abort or 0 to continue
	int Finished(); 
	enum
	{
		BURNER_WRITECALLBACK_ONSTATUS = 0,
		BURNER_WRITECALLBACK_FINISHED = 1,
	};
};


inline int ifc_burner_writecallback::OnStatus(uint32_t sectorsWritten, uint32_t totalSectors)
{
	return _call(BURNER_WRITECALLBACK_ONSTATUS, (int)0, sectorsWritten, totalSectors);
}

inline int ifc_burner_writecallback::Finished()
{
	return _call(BURNER_WRITECALLBACK_FINISHED, (int)0);
}

#endif