#pragma once
#include "foundation/dispatch.h"
#include "nx/nxstring.h"
#include "foundation/error.h"
#include "metadata/MetadataKeys.h"
#include "nx/nxdata.h"
#include "metadata/types.h"
class NOVTABLE ifc_metadata_editor : public Wasabi2::Dispatchable
{
protected:
	ifc_metadata_editor() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_metadata_editor() {}
public:
	enum
	{
		INDEX_DEFAULT = -1,
	};

	int Save() { return MetadataEditor_Save(); }

	/* Fields */
	int SetField(int field, unsigned int index, nx_string_t value) { return MetadataEditor_SetField(field, index, value); }
	int SetInteger(int field, unsigned int index, int64_t value) { return MetadataEditor_SetInteger(field, index, value); }
	int SetReal(int field, unsigned int index, double value) { return MetadataEditor_SetReal(field, index, value); }

	/* Art */
	int SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags=DATA_FLAG_ALL) { return MetadataEditor_SetArtwork(field, index, data, flags); }

	/* Binary Data */

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL MetadataEditor_Save()=0;

	virtual int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)=0;
	virtual int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)=0;
	virtual int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value)=0;
	
	virtual int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags)=0;
};
