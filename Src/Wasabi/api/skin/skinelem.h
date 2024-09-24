#ifndef _SKINELEM_H
#define _SKINELEM_H

#include <api/xml/xmlreader.h>
#include <tataki/region/region.h>

#include <api/skin/skinitem.h>
#include <api/xml/xmlparamsi.h>

typedef struct
{
	const wchar_t *tagname;
	int id;
	int needclosetag;
}
xml_elementtag;

class XmlElementTagComp
{
public:
	static int compareItem(void *p1, void *p2)
	{
		return WCSICMP(((xml_elementtag *)p1)->tagname, ((xml_elementtag *)p2)->tagname);
	}
	static int compareAttrib(const wchar_t *attrib, void *item)
	{
		return WCSICMP(attrib, ((xml_elementtag *)item)->tagname);
	}
};

enum {
    XML_ELEMENTTAG_UNKNOWN = 0,
    XML_ELEMENTTAG_ELEMENTS,
    XML_ELEMENTTAG_ELEMENTALIAS,
    XML_ELEMENTTAG_BITMAP,
    XML_ELEMENTTAG_COLOR,
    XML_ELEMENTTAG_BITMAPFONT,
    XML_ELEMENTTAG_TRUETYPEFONT,
    XML_ELEMENTTAG_CURSOR,

};


class ElementRegionServer;

/*typedef enum {
	SKIN_BITMAP_ELEMENT,
	SKIN_FONT_ELEMENT,
	SKIN_CURSOR_ELEMENT,
	SKIN_COLOR_ELEMENT
} SkinElementType;*/




class SkinElementsXmlReader : public XmlReaderCallbackI
{
public:
	void xmlReaderOnStartElementCallback(const wchar_t *xmltag, skin_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmltag);
};

class SkinElementsMgr
{
public:
	static void init();
	static void deinit();

	static void onBeforeLoadingSkinElements(const wchar_t *rootpath);
	static void onAfterLoadingSkinElements();

	static void onBeforeLoadingScriptElements(const wchar_t *name, int script_id);
	static void onAfterLoadingScriptElements();

	static void resetSkinElements();
	static void unloadScriptElements(int scriptid);

	static void xmlReaderOnStartElementCallback( const wchar_t *xmltag, skin_xmlreaderparams *params);
	static void _xmlReaderOnStartElementCallback( int tagid, const wchar_t *xmltag, skin_xmlreaderparams *params);
	static void xmlReaderOnEndElementCallback( const wchar_t *xmltag);

	static const wchar_t *getSkinRootpathFromIncludePath(const wchar_t *includepath, const wchar_t *def);
	static int elementEqual(const wchar_t *file1, const wchar_t *rootpath1,
	                        const wchar_t *file2, const wchar_t *rootpath2);



private:
	static SkinElementsXmlReader xmlreader;

	static int inelements;
	static int elementScriptId;

	static int oldid, oldinel;
	static StringW rootpath;
	static StringW original_rootpath;
	static StringW t_rootpath;
	static StringW last_includepath;
	static PtrList<StringW> rootpathstack;
	
	static PtrListQuickSorted<xml_elementtag, XmlElementTagComp> quickxmltaglist;
};



#endif
