#include "xml/ifc_xmlcallback.h"
#include "nx/nxmutablestring.h"

/* this one is an xml callback that just saves the last encountered string */

class XMLString : public ifc_xmlcallback
{
public:
	XMLString();
	~XMLString();
	void Reset();
	nx_string_t GetString();
	
private:
		/* XML callbacks */
	void WASABICALL XMLCallback_OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes);
	void WASABICALL XMLCallback_OnCharacterData(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, const nsxml_char_t *characters, size_t num_characters);

	nx_mutable_string_t data;	
};

