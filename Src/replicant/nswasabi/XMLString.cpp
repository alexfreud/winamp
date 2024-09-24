#include "XMLString.h"

/* TODO: make and use some sort of nx_mutable_string_t object */
XMLString::XMLString() 
{
	data=0;
}

XMLString::~XMLString()
{
	NXMutableStringDestroy(data);
}


void XMLString::Reset()
{
	if (data)
		data->nx_string_data->len = 0; // TODO: make mutable string function for this
}

nx_string_t XMLString::GetString()
{
	// TODO: make mutable string function for this
	nx_string_t str = data->nx_string_data;
	data->nx_string_data = 0;
	NXMutableStringDestroy(data);
	data=0;
	return str;
}

void XMLString::XMLCallback_OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes)
{
	if (data)
		data->nx_string_data->len = 0; // TODO: make mutable string function for this
}

void XMLString::XMLCallback_OnCharacterData(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, const nsxml_char_t *characters, size_t num_characters)
{
	if (!data)
		data = NXMutableStringCreateFromXML(characters, num_characters);
	else
		NXMutableStringGrowFromXML(data, characters, num_characters);
}

