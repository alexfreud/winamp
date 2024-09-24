#ifndef NULLSOFT_FLVMETADATA_H
#define NULLSOFT_FLVMETADATA_H

#include "AMFObject.h"
#include <vector>

class FLVMetadata
{
public:
	FLVMetadata();
	~FLVMetadata();
	bool Read(uint8_t *data, size_t size);
	struct Tag
	{
		Tag(); 
		~Tag();
		
		AMFString name;
		AMFMixedArray *parameters; // needs to be pointer so we can refcount
	};
	std::vector<Tag*> tags;
	
};
#endif