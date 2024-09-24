//------------------------------------------------------------------------
//
// iTunes XML Library Reader
// Copyright (C) 2003-2011 Nullsoft, Inc.
//
//------------------------------------------------------------------------

#include "loader.h"
#include <windows.h>
#include <commdlg.h>
#include <bfc/string/stringdict.h>


#define ATTRIB_TRUE  256
#define ATTRIB_FALSE 257

//------------------------------------------------------------------------
// xml tags
//------------------------------------------------------------------------
BEGIN_STRINGDICTIONARY(_itunestypes)
SDI(L"key", PLISTDATA_KEY);
SDI(L"dict", PLISTDATA_DICT);
SDI(L"integer", PLISTDATA_INTEGER);
SDI(L"string", PLISTDATA_STRING);
SDI(L"date", PLISTDATA_DATE);
SDI(L"array", PLISTDATA_ARRAY);
SDI(L"data", PLISTDATA_RAW);
SDI(L"true", ATTRIB_TRUE);
SDI(L"false", ATTRIB_FALSE);
END_STRINGDICTIONARY(_itunestypes, itunestypes)

//------------------------------------------------------------------------
plistLoader::plistLoader() : plistKey(L"root")
{
	m_context.push(this);
}

//------------------------------------------------------------------------
plistLoader::~plistLoader() 
{
}


//------------------------------------------------------------------------
// element opens (or singleton), push new key/dictionary/array, handle value singletons (true/false), defer job to closing tag for other data types
//------------------------------------------------------------------------
void plistLoader::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params) {
	m_cdata = 0;
	int a = itunestypes.getId(xmltag);
	switch (a) {
		case PLISTDATA_KEY:
			m_cdata = L"";
			m_context.push(new plistKey());
			m_dict.top()->addKey((plistKey *)m_context.top());
			break;
		case PLISTDATA_DICT:
			{
				m_dict.push(new plistDict());
				plistData *contextTop = m_context.top();
				contextTop->setData(static_cast<plistDict *>(m_dict.top()));
			}
			break;
		case PLISTDATA_ARRAY: 
			{
				plistArray *new_array = new plistArray;
				plistData *contextTop = m_context.top();
				contextTop->setData(new_array);
				m_context.push(new_array);
				m_dict.push(new_array);
				break;
			}
		case PLISTDATA_STRING:
			break;
		case PLISTDATA_INTEGER:
			break;
		case PLISTDATA_DATE:
			break;
		case PLISTDATA_RAW:
			break;
		case PLISTDATA_REAL:
			break;
		case ATTRIB_TRUE: {
			m_context.top()->setData(new plistBoolean(true));
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
		case ATTRIB_FALSE: {
			m_context.top()->setData(new plistBoolean(false));
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
	}
}

//------------------------------------------------------------------------
// element closes, set data for last key/array, pop key/array/dictionary from stack
//------------------------------------------------------------------------
void plistLoader::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag) {
	int a = itunestypes.getId(xmltag);
	switch (a) {
		case PLISTDATA_KEY:
			ASSERT(m_context.top()->getType() == PLISTDATA_KEY);
			((plistKey*)m_context.top())->setName(m_cdata);
			break;
		case PLISTDATA_DICT:
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			m_dict.pop();
			break;
		case PLISTDATA_ARRAY: {
			/*if (m_context.top()->getType() == PLISTDATA_KEY)*/ m_context.pop();
			//ASSERT(m_context.top()->getType() == PLISTDATA_ARRAY);
			//plistArray *a = (plistArray *)m_context.top();
			//m_context.pop();
			//m_context.top()->setData(a);
			m_dict.pop();
			break;
		}
		case PLISTDATA_STRING: {
			plistString *s = new plistString(m_cdata);
			m_context.top()->setData(s);
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
		case PLISTDATA_INTEGER: {
			plistInteger *i = new plistInteger();
			i->setString(m_cdata);
			m_context.top()->setData(i);
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
		case PLISTDATA_REAL: {
			plistReal *r = new plistReal();
			r->setString(m_cdata);
			m_context.top()->setData(r);
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
		case PLISTDATA_DATE: {
			plistDate *d = new plistDate();
			d->setString(m_cdata);
			m_context.top()->setData(d);
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
		case PLISTDATA_RAW: {
			plistRaw *r = new plistRaw();
			r->setString(m_cdata);
			m_context.top()->setData(r);
			if (m_context.top()->getType() == PLISTDATA_KEY) m_context.pop();
			break;
		}
	}
	m_cdata = 0;
}

//------------------------------------------------------------------------
// record c_data
//------------------------------------------------------------------------
void plistLoader::xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str) {
	m_cdata += str;
}

#define CBCLASS plistLoader
START_DISPATCH;
VCB(ONSTARTELEMENT, xmlReaderOnStartElementCallback)
VCB(ONCHARDATA, xmlReaderOnCharacterDataCallback)
VCB(ONENDELEMENT, xmlReaderOnEndElementCallback)
END_DISPATCH;
#undef CBCLASS