#include "precomp.h"
#include <api.h>
#include "cursormgr.h"
#include <api/skin/skinelem.h>



OSCURSOR CursorMgr::requestCursor(const wchar_t *id)
{	
	// Martin> Register os cursors, perhaps find a better way to do this in basewnd.cpp	
	if (_wcsicmp(id, L"IDC_ARROW") == 0) return LoadCursor(NULL, IDC_ARROW);
	if (_wcsicmp(id, L"IDC_SIZENS") == 0) return LoadCursor(NULL, IDC_SIZENS);
	if (_wcsicmp(id, L"IDC_SIZEWE") == 0) return LoadCursor(NULL, IDC_SIZEWE);
	if (_wcsicmp(id, L"IDC_SIZENWSE") == 0) return LoadCursor(NULL, IDC_SIZENWSE);
	if (_wcsicmp(id, L"IDC_SIZENESW") == 0) return LoadCursor(NULL, IDC_SIZENESW);
	if (_wcsicmp(id, L"IDC_SIZEALL") == 0) return LoadCursor(NULL, IDC_SIZEALL);
	if (_wcsicmp(id, L"IDC_IBEAM") == 0) return LoadCursor(NULL, IDC_IBEAM);
	if (_wcsicmp(id, L"IDC_WAIT") == 0) return LoadCursor(NULL, IDC_WAIT);
	if (_wcsicmp(id, L"IDC_CROSS") == 0) return LoadCursor(NULL, IDC_CROSS);
	if (_wcsicmp(id, L"IDC_UPARROW") == 0) return LoadCursor(NULL, IDC_UPARROW);
	if (_wcsicmp(id, L"IDC_NO") == 0) return LoadCursor(NULL, IDC_NO);
	if (_wcsicmp(id, L"IDC_HAND") == 0) return LoadCursor(NULL, IDC_HAND);
	if (_wcsicmp(id, L"IDC_APPSTARTING") == 0) return LoadCursor(NULL, IDC_APPSTARTING);
	if (_wcsicmp(id, L"IDC_HELP") == 0) return LoadCursor(NULL, IDC_HELP);

	OSCURSOR cursor = WASABI_API_PALETTE->getCursor(id);
	return cursor;
}

