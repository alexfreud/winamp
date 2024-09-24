#ifndef NULLSOFT_AGAVE_METADATA_IFC_METADATAREADER_H
#define NULLSOFT_AGAVE_METADATA_IFC_METADATAREADER_H

#include <bfc/dispatch.h>

class ifc_metadataReader : public Dispatchable
{
protected:
	ifc_metadataReader()
	{}
	~ifc_metadataReader()
	{}
public:
	/* If there are multiple values for the same field, these functions will concatenate the values in some
	 manner as defined by the language pack */
	int GetField( const wchar_t *field, wchar_t *destination, size_t destinationCch );
	int GetFieldByKey( uint32_t field_key, wchar_t *destination, size_t destinationCch );

	int GetIndexedField( const wchar_t *field, uint32_t index, wchar_t *destination, size_t destinationCch );
	int GetIndexedFieldByKey( uint32_t field_key, uint32_t index, wchar_t *destination, size_t destinationCch );

	class ifc_fieldEnumerator : public Dispatchable
	{
	protected:
		ifc_fieldEnumerator();
		~ifc_fieldEnumerator();

	public:
		const wchar_t *GetField();
		uint32_t GetKey();
		int Next();

		DISPATCH_CODES
		{
			DISP_GETFIELD = 0,
			DISP_GETKEY   = 1,
			DISP_NEXT     = 2,
		};

	};

	enum
	{
		ENUMERATORFLAG_MULTIPLE_FIELDS         = 0, // if multiple values of the same field are present, return each one
		ENUMERATORFLAG_REMOVE_DUPLICATE_FIELDS = 1, // don't enumerate duplicate fields (with differing values)
	};

	ifc_fieldEnumerator *GetFieldEnumerator( int flags = ENUMERATORFLAG_MULTIPLE_FIELDS ); // Enumerate fields present in this file

	DISPATCH_CODES
	{
		DISP_GETFIELD             = 0,
		DISP_GETFIELDKEY          = 1,
		DISP_GETINDEXEDFIELD      = 2,
		DISP_GETINDEXEDFIELDBYKEY = 3,
		DISP_GETFIELDENUMERATOR   = 4,
	};

	enum
	{
		NOT_IMPLEMENTED = -1,
		SUCCESS         = 0,
		FAILURE         = 1,
		END_OF_ITERATOR = 2,
	};
};

inline int ifc_metadataReader::GetField(const wchar_t *field, wchar_t *destination, size_t destinationCch)
{
	return _call(DISP_GETFIELD, (int)ifc_metadataReader::NOT_IMPLEMENTED, field, destination, destinationCch);
}

inline int ifc_metadataReader::GetFieldByKey(uint32_t field_key, wchar_t *destination, size_t destinationCch)
{
	return _call(DISP_GETFIELDKEY, (int)ifc_metadataReader::NOT_IMPLEMENTED, field_key, destination, destinationCch);
}

inline int ifc_metadataReader::GetIndexedField(const wchar_t *field, uint32_t index, wchar_t *destination, size_t destinationCch)
{
	return _call(DISP_GETINDEXEDFIELD, (int)ifc_metadataReader::NOT_IMPLEMENTED, field, index, destination, destinationCch);
}

inline int ifc_metadataReader::GetIndexedFieldByKey(uint32_t field_key, uint32_t index, wchar_t *destination, size_t destinationCch)
{
	return _call(DISP_GETINDEXEDFIELDBYKEY, (int)ifc_metadataReader::NOT_IMPLEMENTED, field_key, index, destination, destinationCch);
}

inline ifc_metadataReader::ifc_fieldEnumerator *ifc_metadataReader::GetFieldEnumerator(int flags)
{
	return _call(DISP_GETFIELDENUMERATOR, (ifc_metadataReader::ifc_fieldEnumerator *)0, flags);
}

inline const wchar_t *ifc_metadataReader::ifc_fieldEnumerator::GetField()
{
	return _call(DISP_NEXT, (const wchar_t *)0);
}

inline uint32_t ifc_metadataReader::ifc_fieldEnumerator::GetKey()
{
	return _call(DISP_NEXT, (int)ifc_metadataReader::NOT_IMPLEMENTED);
}

inline int ifc_metadataReader::ifc_fieldEnumerator::Next()
{
	return _call(DISP_NEXT, (int)END_OF_ITERATOR);
}
#endif