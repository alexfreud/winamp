#include <precomp.h>
#include <bfc/wasabi_std.h>
#include "XMLAutoInclude.h"
#include <bfc/file/wildcharsenum.h>
#include <api/util/varmgr.h>

void LoadXmlFile(obj_xml *parser, const wchar_t *filename);

XMLAutoInclude::XMLAutoInclude(obj_xml *_parser, const wchar_t *_path)
{
	path=_path;
	parser = _parser;
	if (parser)
	{
		parser->xmlreader_registerCallback(L"*\finclude", this);
	}
}

XMLAutoInclude::~XMLAutoInclude()
{
	if (parser)
	{
		parser->xmlreader_unregisterCallback(this);
	}
}
static const wchar_t *varmgr_translate(const wchar_t *str)
{
	static StringW ret;
	StringW *s = PublicVarManager::translate_nocontext(str);
	if (s)
	{
		ret.swap(s);
		delete s;
		return ret.getValueSafe();
	}
	return str;
}
void XMLAutoInclude::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t *includeFile = params->getItemValue(L"file");
	const wchar_t *trans = /*WASABI_API_WNDMGR->*/varmgr_translate(includeFile);
	if (trans)
	{
		//const char *path = WASABI_API_SKIN->getSkinsPath();
		if (!Wasabi::Std::isRootPath(trans))
			includeFn=StringPathCombine(path, trans);
		else
			includeFn=trans;
	}

}

void XMLAutoInclude::Include(const wchar_t *filename)
{
	if (filename && *filename)
	{
		parser->xmlreader_interrupt();
		StringW oldPath = path;

		const wchar_t *file = Wasabi::Std::filename(filename);
		int fnlen = wcslen(file);
		path = filename;
		path.trunc(-fnlen);
		LoadXmlFile(parser, filename);
		path = oldPath;
		parser->xmlreader_resume();
	}
}

void XMLAutoInclude::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (!includeFn.isempty())
	{
		WildcharsEnumerator e(includeFn);
		if (e.getNumFiles() > 0) // if we're including multiple files
		{
			for (int i = 0;i < e.getNumFiles();i++)
			{
				Include(e.enumFile(i));
			}
		}
		else
			Include(includeFn);
	}

}

