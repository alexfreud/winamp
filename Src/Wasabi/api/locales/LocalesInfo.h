#ifndef NULLSOFT_WASABI_LOCALESINFO_H
#define NULLSOFT_WASABI_LOCALESINFO_H

#include "../xml/ifc_xmlreadercallbackI.h" 
#include <bfc/string/StringW.h>
class obj_xml;
class LocaleItem
{
public:
	LocaleItem(const wchar_t *name) : name(name), language(name) { };
	const wchar_t *getName() { return name; }

	const wchar_t *getLanguage() { return language; }
	const wchar_t *getAuthor() { return author; }
protected:
	StringW name;
	StringW language;
	StringW author;
};


class LocalesInfosXmlReader : public LocaleItem, public ifc_xmlreadercallbackI // XmlReaderCallbackI
{
public:
	LocalesInfosXmlReader(const wchar_t *skinname);
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
private:
	obj_xml *parser;
};
#endif