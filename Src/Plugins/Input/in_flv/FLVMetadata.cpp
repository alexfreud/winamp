#include "FLVMetadata.h"
#include "FLVUtil.h"
#include <windows.h>

/*
(c) 2006 Nullsoft, Inc.
Author: Ben Allison benski@nullsoft.com 
*/

/*

type - uint8 -
length - uint16

*/


AMFType *MakeObject(uint8_t type)
{
	switch(type)
	{
	case AMFType::TYPE_DOUBLE: // double
		return new AMFDouble;
	case AMFType::TYPE_BOOL: // bool
		return new AMFBoolean;
	case AMFType::TYPE_STRING: // string
		return new AMFString;
	case AMFType::TYPE_OBJECT: // object
		return new AMFObj;
	case AMFType::TYPE_MOVIE: // movie (basically just a URL)
		return new AMFString;
	case AMFType::TYPE_NULL: // null
		return 0;
	case AMFType::TYPE_REFERENCE: // reference
		return 0;
	case AMFType::TYPE_MIXEDARRAY:
		return new AMFMixedArray;
	case AMFType::TYPE_TERMINATOR:
		return new AMFTerminator;
	case AMFType::TYPE_ARRAY:
		return new AMFArray;
	case AMFType::TYPE_DATE: // date
		return new AMFTime;
	case AMFType::TYPE_LONG_STRING: // long string
		return new AMFLongString;
	case AMFType::TYPE_XML: // XML
		return 0;
	default:
		return 0;
	}
}

FLVMetadata::FLVMetadata()
{
}

FLVMetadata::~FLVMetadata()
{
	for ( FLVMetadata::Tag *tag : tags )
		delete tag;
}

bool FLVMetadata::Read(uint8_t *data, size_t size)
{
	// TODO: there can be multiple name/value pairs so we could read them all
	while(size)
	{
		uint8_t type=*data; 
		data++; 	
		size--;

		if (type == 0 && size >= 2 && data[0] == 0 && data[1] == AMFType::TYPE_TERMINATOR) // check for terminator
			return true; // array is done

		if (type != AMFType::TYPE_STRING) // first entry is a string, verify this
			return false;  // malformed, lets bail

		FLVMetadata::Tag *tag = new FLVMetadata::Tag;
		
		// read name
		size_t skip = tag->name.Read(data, size);
		data+=skip;
		size-=skip;

		type=*data; 
		data++; 	
		size--;

		if (type != AMFType::TYPE_MIXEDARRAY) // second entry is an associative array, verify this
		{
			delete tag;
			return false; // malformed, lets bail
		}

		tag->parameters = new AMFMixedArray; // we're new'ing this because we need to reference count
		skip = tag->parameters->Read(data, size);
		data+=skip;
		size-=skip;
		tags.push_back(tag);
	}

	return true;
}

FLVMetadata::Tag::Tag() : parameters(0)
{
}
		FLVMetadata::Tag::~Tag()
		{
			if (parameters)
				parameters->Release();
		}