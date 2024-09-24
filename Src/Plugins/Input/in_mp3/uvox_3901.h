#pragma once
#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallback.h"


class Ultravox3901 : public ifc_xmlreadercallback
{
public:
	Ultravox3901();
	~Ultravox3901();
	int Parse(const char *xml_data);
	int GetExtendedData(const char *tag, wchar_t *data, int dataLen);
private:
	/* XML callbacks */
	void TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);

	obj_xml *parser;
	wchar_t title[256],artist[256],album[256],album_art_url[4096];
	RECVS_DISPATCH;
};