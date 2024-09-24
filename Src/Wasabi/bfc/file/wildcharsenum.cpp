// NONPORTABLE NONPORTABLE NONPORTABLE
#include "precomp_wasabi_bfc.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "wildcharsenum.h"
#include <bfc/parse/pathparse.h>
#include <bfc/parse/paramparser.h>
#include <bfc/file/readdir.h>

WildcharsEnumerator::WildcharsEnumerator(const wchar_t *_selection) : selection(_selection) 
{
	// Then scan.
	rescan();
}

WildcharsEnumerator::~WildcharsEnumerator() {
	finddatalist.deleteAll();
}

int WildcharsEnumerator::getNumFiles() 
{
	return finddatalist.getNumItems();
}

const wchar_t *WildcharsEnumerator::enumFile(int n) {
	StringW path = finddatalist.enumItem(n)->path;
	if (!path.isempty()) 
	{
		enumFileString = StringPathCombine(path.getValue(), finddatalist.enumItem(n)->filename.getValue());
		return enumFileString;
	}
	return finddatalist.enumItem(n)->filename;
}

void WildcharsEnumerator::rescan() 
{
	finddatalist.removeAll();
	ParamParser pp(selection, L";");
	for (int is = 0; is < pp.getNumItems(); is++)
	{
		StringW _selection = pp.enumItem(is);

		PathParserW parse(_selection);
		StringW path = L"";
		StringW mask = L"";

		for (int i=0;i<parse.getNumStrings()-1;i++)
			path.AppendFolder(parse.enumString(i));

		mask = parse.getLastString();

		// enum files and store a list
		ReadDir rd(path, mask, true);
		while (rd.next()) {
			finddatalist.addItem(new find_entry(rd.getPath(), rd.getFilename()));
		}
	}
}

int WildcharsEnumerator::isWildchars(const wchar_t *filename) 
{
	return (wcschr(filename, '*') || wcschr(filename, '?'));
}
