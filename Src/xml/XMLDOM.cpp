#include "XMLDOM.h"

XMLDOM::XMLDOM() : curtext(0), curtext_len(0), curNode(0)
{
	xmlNode = new XMLNode;
	curNode = xmlNode;
}

XMLDOM::~XMLDOM()
{
	if (curtext)
	{
		free(curtext);
		curtext = 0;
		curtext_len = 0;
	}
	delete xmlNode;
}

void XMLDOM::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	XMLNode *newNode = new XMLNode;

	int num = (int)params->getNbItems();
	for (int i = 0;i < num;i++)
		newNode->SetProperty(params->getItemName(i), params->getItemValue(i));

	newNode->parent = curNode;
	curNode->SetContent_Own(curtext);
	curtext = 0;

	curNode->AddNode(xmltag, newNode);
	curNode = newNode;
}

void XMLDOM::EndTag(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	curNode->AppendContent(curtext);

	if (curtext)
	{
		free(curtext);
		curtext = 0;
		curtext_len = 0;
	}

	curNode = curNode->parent;
}

void XMLDOM::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	if (str && *str)
	{
		if (curtext)
		{
			size_t len = wcslen(str), new_len = len + curtext_len;
			wchar_t* newcurtext = (wchar_t *)realloc(curtext, (new_len+1)*sizeof(wchar_t));
			if (newcurtext)
			{
				curtext = newcurtext;
				wcsncpy(curtext+curtext_len, str, len);
				*(curtext+curtext_len+len) = 0;
				curtext_len = new_len;
			}
			else
			{
				newcurtext = (wchar_t *)malloc((new_len+1)*sizeof(wchar_t));
				if (newcurtext)
				{
					memcpy(newcurtext, curtext, curtext_len*sizeof(wchar_t));
					free(curtext);
					curtext = newcurtext;

					wcsncpy(curtext+curtext_len, str, len);
					*(curtext+curtext_len+len) = 0;
					curtext_len = new_len;
				}
			}
		}
		else
		{
			curtext_len = wcslen(str);
			curtext = (wchar_t *)malloc((curtext_len+1)*sizeof(wchar_t));
			if (curtext)
				memcpy(curtext, str, (curtext_len+1)*sizeof(wchar_t));
		}
	}
}

const XMLNode *XMLDOM::GetRoot() const
{
	return xmlNode;
}

#define CBCLASS XMLDOM
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONENDELEMENT, EndTag)
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
#undef CBCLASS