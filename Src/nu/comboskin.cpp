#include "comboskin.h"
#include "../nu/MediaLibraryInterface.h"



ComboSkin::ComboSkin(HWND hwnd)
		: token(0)
{
	token = mediaLibrary.SkinComboBox(hwnd);
}

ComboSkin::~ComboSkin()
{
	mediaLibrary.UnskinComboBox(token);
}
