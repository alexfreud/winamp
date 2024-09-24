#ifndef _SELECTFILE_H
#define _SELECTFILE_H

#include <bfc/common.h>

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

class svc_fileSelector;
class ifc_window;

class SelectFile
{
public:
	SelectFile(ifc_window *parent, const wchar_t *menu_prefix = NULL, const wchar_t *menu_suffix = NULL);
	~SelectFile();

	void setDefaultDir(const wchar_t *dir);	// default dir to use
	const wchar_t *getDirectory(); // return base directory after ok clicked
	void setIdent(const wchar_t *id);	// unless you saved one under this id

	void setPopPosition(int x, int y);	// in screen coords

	int runSelector(const wchar_t *type = NULL, int allow_multiple = FALSE, const wchar_t *extlist = NULL);	// if NULL, generate popup
	const wchar_t *getType();

	int getNumFiles();
	const wchar_t *enumFilename(int n);

private:
	int xpos, ypos;
	int pos_set;
	ifc_window *parentWnd;
	svc_fileSelector *svc;
	PtrList<StringW> types;
	StringW prefix_str, suffix_str;
	StringW default_dir, ident;
	StringW saved_type;
};

#endif
