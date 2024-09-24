#pragma once
#include "metadata/ifc_metadata_editor.h"


template <class metadata_t>
class MetadataEditorChain : public metadata_t
{
public:
	MetadataEditorChain()
	{
		parent_metadata=0;
	}

	~MetadataEditorChain()
	{
		if (parent_metadata)
			parent_metadata->Release();
	}
	
	static bool Unhandled(ns_error_t ret)
	{
		return (ret == NErr_NotImplemented || ret == NErr_Empty || ret == NErr_Unknown);
	}

	ns_error_t SetParentMetadata(ifc_metadata_editor *new_parent_metadata)
	{
		if (parent_metadata)
			parent_metadata->Release();
		parent_metadata=new_parent_metadata;
		if (parent_metadata)
			parent_metadata->Retain();
		return NErr_Success;
	}
protected:
	ifc_metadata_editor *parent_metadata;
private:
	int WASABICALL MetadataEditor_Save()
	{
		if (parent_metadata)
			return parent_metadata->Save();
		else
			return NErr_NotImplemented;
	}

	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
	{
		bool known=false;
		if (parent_metadata && parent_metadata->SetField(field, index, value) == NErr_Success)
			known=true;
		if (metadata_t::MetadataEditor_SetField(field, index, value) == NErr_Success)
			known=true;

		if (known)
			return NErr_Success;
		else
			return NErr_Unknown;
	}

	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
	{
		bool known=false;
		if (parent_metadata && parent_metadata->SetInteger(field, index, value) == NErr_Success)
			known=true;
		if (metadata_t::MetadataEditor_SetInteger(field, index, value) == NErr_Success)
			known=true;

		if (known)
			return NErr_Success;
		else
			return NErr_Unknown;
	}

	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value)
	{
		bool known=false;
		if (parent_metadata && parent_metadata->SetReal(field, index, value) == NErr_Success)
			known=true;
		if (metadata_t::MetadataEditor_SetReal(field, index, value) == NErr_Success)
			known=true;

		if (known)
			return NErr_Success;
		else
			return NErr_Unknown;
	}
	
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags)
	{
		bool known=false;
		if (parent_metadata && parent_metadata->SetArtwork(field, index, data, flags) == NErr_Success)
			known=true;
		if (metadata_t::MetadataEditor_SetArtwork(field, index, data, flags) == NErr_Success)
			known=true;

		if (known)
			return NErr_Success;
		else
			return NErr_Unknown;
	}
};