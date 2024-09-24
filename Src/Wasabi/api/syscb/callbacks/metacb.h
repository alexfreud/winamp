#pragma once
#include <api/syscb/callbacks/syscbi.h>

namespace MetadataCallback {
	enum {
		FILE_UPDATED = 10,
		ART_UPDATED = 20,
		FILE_MAY_UPDATE = 30,
	};
};

#define METADATACALLBACK_PARENT SysCallbackI
class MetadataCallbackI : public METADATACALLBACK_PARENT 
{
protected:
	MetadataCallbackI() { }

public:
	virtual void metacb_FileUpdated(const wchar_t *filename) { }
	virtual void metacb_ArtUpdated(const wchar_t *filename) { }
	virtual void metacb_FileMayUpdate(const wchar_t *filename) { }

private:
	virtual FOURCC syscb_getEventType() { return SysCallback::META; }

	virtual int syscb_notify(int msg, intptr_t param1, intptr_t param2) {
		switch (msg) {
			case MetadataCallback::FILE_UPDATED:
				metacb_FileUpdated((const wchar_t *)param1);
			break;
			case MetadataCallback::ART_UPDATED:
				metacb_ArtUpdated((const wchar_t *)param1);
			break;
			case MetadataCallback::FILE_MAY_UPDATE:
				metacb_FileMayUpdate((const wchar_t *)param1);
			break;
			default: return 0;
		}
		return 1;
	}
};