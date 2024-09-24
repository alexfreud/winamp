#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "metadata/ifc_metadata_editor.h"

class ifc_filemetadata_editor : public Wasabi2::Dispatchable
{
protected:
	ifc_filemetadata_editor() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_filemetadata_editor() {}
public:
	ns_error_t GetMetdataObject(ifc_metadata_editor **metadata) { return FileMetadata_GetMetdataObject(metadata); }
	ns_error_t Save(nx_file_t file) { return FileMetadata_Save(file); }
	ns_error_t RequireTempFile() { return FileMetadata_RequireTempFile(); }
	ns_error_t SaveAs(nx_file_t destination, nx_file_t source) { return FileMetadata_SaveAs(destination, source); }
	void Close() { FileMetadata_Close(); }

	ns_error_t WantID3v2(int *position) { return FileMetadata_WantID3v2(position); }
	ns_error_t WantID3v1() { return FileMetadata_WantID3v1(); }
	ns_error_t WantAPEv2(int *position) { return FileMetadata_WantAPEv2(position); }
	ns_error_t WantLyrics3() { return FileMetadata_WantLyrics3(); }
	
	enum
	{
		DISPATCHABLE_VERSION=0,

		TAG_POSITION_INDIFFERENT=0,
		TAG_POSITION_PREPENDED=1,
		TAG_POSITION_APPENDED=2,
	};
protected:
	virtual ns_error_t WASABICALL FileMetadata_GetMetdataObject(ifc_metadata_editor **metadata)=0;
	virtual ns_error_t WASABICALL FileMetadata_Save(nx_file_t file) { return NErr_NotImplemented; }
	virtual ns_error_t WASABICALL FileMetadata_RequireTempFile()=0;	
	virtual ns_error_t WASABICALL FileMetadata_SaveAs(nx_file_t destination, nx_file_t source) { return NErr_NotImplemented; }
	virtual void WASABICALL FileMetadata_Close() {}

	virtual ns_error_t WASABICALL FileMetadata_WantID3v2(int *position) { return NErr_False; }
	virtual ns_error_t WASABICALL FileMetadata_WantID3v1() { return NErr_False; }
	virtual ns_error_t WASABICALL FileMetadata_WantAPEv2(int *position) { return NErr_False; }
	virtual ns_error_t WASABICALL FileMetadata_WantLyrics3() { return NErr_False; }
	
};
