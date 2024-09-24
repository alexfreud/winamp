#include <precomp.h>

#include "selectfile.h"

#include <api/wnd/popup.h>
#include <api/service/svcs/svc_filesel.h>

SelectFile::SelectFile(ifc_window *parent, const wchar_t *menu_prefix, const wchar_t *menu_suffix)
		: parentWnd(parent),
		prefix_str(menu_prefix),
		suffix_str(menu_suffix)
{
	svc = NULL;
	ASSERT(parent != NULL);
	xpos = ypos = 0;
	pos_set = 0;
}

SelectFile::~SelectFile()
{
	if (svc) WASABI_API_SVC->service_release(svc);
	types.deleteAll();
}

void SelectFile::setDefaultDir(const wchar_t *dir)
{
	default_dir = dir;
}

const wchar_t *SelectFile::getDirectory()
{
	if (svc) return svc->getDirectory();
	return NULL;
}

void SelectFile::setIdent(const wchar_t *id)
{
	ident = id;
}

void SelectFile::setPopPosition(int x, int y)
{
	xpos = x;
	ypos = y;
	pos_set = 1;
}

int SelectFile::runSelector(const wchar_t *type, int allow_multiple, const wchar_t *extlist)
{
	if (svc) WASABI_API_SVC->service_release(svc); svc = NULL;
	types.deleteAll();
	if (type == NULL)
	{
		int pos = 0;
		for (;;)
		{
			waServiceFactory *wasvc = WASABI_API_SVC->service_enumService(WaSvc::FILESELECTOR, pos++);
			if (wasvc == NULL) break;
			svc_fileSelector *sfs = castService<svc_fileSelector>(wasvc);
			const wchar_t *pref = sfs->getPrefix();
			if (pref != NULL)
				types.addItem(new StringW(pref));
			WASABI_API_SVC->service_release(sfs);
		}
		if (types.getNumItems() <= 0) return 0;	// none?!

		PopupMenu *pop = new PopupMenu(parentWnd);
		for (int i = 0; i < types.getNumItems(); i++)
		{
			StringW str;
			str += prefix_str;
			str += types[i]->getValue();
			str += suffix_str;
			pop->addCommand(str, i);
		}
		int cmd = pos_set ? pop->popAtXY(xpos, ypos) : pop->popAtMouse();
		StringW *s = types[cmd];
		if (s == NULL) return 0;
		type = *s;
	}
	ASSERT(type != NULL);

	saved_type = type;

	svc = FileSelectorEnum(type).getFirst();
	ASSERT(svc != NULL);	// we just enumed it
	if (extlist != NULL) svc->setExtList(extlist);
	//FUCKO : need to set open vs. save as
	WASABI_API_WND->pushModalWnd();
	int r = svc->runSelector(parentWnd, FileSel::OPEN, allow_multiple,
	                         ident.isempty() ? type : ident.getValue(),
	                         default_dir);
	WASABI_API_WND->popModalWnd();
	ASSERT(svc != NULL);	// we just enumed it
	return r;
}

const wchar_t *SelectFile::getType()
{
	return saved_type;
}

int SelectFile::getNumFiles()
{
	return svc ? svc->getNumFilesSelected() : 0;
}

const wchar_t *SelectFile::enumFilename(int n)
{
	return svc ? svc->enumFilename(n) : NULL;
}
