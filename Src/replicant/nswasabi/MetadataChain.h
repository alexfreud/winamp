#pragma once
#include "metadata/ifc_metadata.h"


template <class metadata_t>
class MetadataChain : public metadata_t
{
public:
	MetadataChain()
	{
		parent_metadata=0;
	}

	~MetadataChain()
	{
		if (parent_metadata)
			parent_metadata->Release();
	}
	
	static bool Unhandled(ns_error_t ret)
	{
		return (ret == NErr_NotImplemented || ret == NErr_Empty || ret == NErr_Unknown);
	}

	ns_error_t SetParentMetadata(ifc_metadata *new_parent_metadata)
	{
		if (parent_metadata)
			parent_metadata->Release();
		parent_metadata=new_parent_metadata;
		if (parent_metadata)
			parent_metadata->Retain();
		return NErr_Success;
	}
protected:
	ifc_metadata *parent_metadata;
private:
	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value)
	{
		ns_error_t ret = metadata_t::Metadata_GetField(field, index, value);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetField(field, index, value);
		else
			return ret;

	}

	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value)
	{
		ns_error_t ret = metadata_t::Metadata_GetInteger(field, index, value);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetInteger(field, index, value);
		else
			return ret;

	}

	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value)
	{
		ns_error_t ret = metadata_t::Metadata_GetReal(field, index, value);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetReal(field, index, value);
		else
			return ret;

	}

	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
	{
		ns_error_t ret = metadata_t::Metadata_GetArtwork(field, index, artwork, flags);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetArtwork(field, index, artwork, flags);
		else
			return ret;

	}

	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)
	{
		ns_error_t ret = metadata_t::Metadata_GetBinary(field, index, data);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetBinary(field, index, data);
		else
			return ret;

	}

	ns_error_t WASABICALL Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata)
	{
		ns_error_t ret = metadata_t::Metadata_GetMetadata(field, index, metadata);
		if (Unhandled(ret) && parent_metadata)
			return parent_metadata->GetMetadata(field, index, metadata);
		else
			return ret;

	}
	// TODO: ns_error_t WASABICALL Metadata_Serialize(nx_data_t *data) { return NErr_NotImplemented; }
};
