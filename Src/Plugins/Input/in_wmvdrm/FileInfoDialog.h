#ifndef NULLSOFT_FILEINFODIALOGH
#define NULLSOFT_FILEINFODIALOGH

#include "../nu/listview.h"
#include "WMInformation.h"
/*  CUT> we're now using the unified file info dlg. I'll leave this commented out incase we want to do an advanced tab later on.
class FileInfoDialog 
{
public:
	FileInfoDialog(HINSTANCE _hInstance, HWND parent, const wchar_t *fileName);
	~FileInfoDialog();
	void Init(HWND _hwnd);
	static INT_PTR WINAPI FileInfoProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	BOOL MetadataList_Notify(NMHDR *header);
	BOOL Edit_Notify(NMHDR *header);
	BOOL OnOk();
	BOOL OnCancel();
  bool WasEdited();
private:
	void FillAttributeList();
	void WriteAttributeList();
	void WriteAttributeListA();
	void FillEditBoxes();
	void WriteEditBoxes();
	bool Apply();
	void Revert();
	void FileInfoDialog::WriteEditBoxHelper(const wchar_t attrName[], DWORD IDC, wchar_t *&temp, int &size);
	bool AttributeInStandardEditor(const wchar_t *attrName);
	HWND fileInfoHWND;
	WMInformation *wmInfo;
	W_ListView attributeList;
	HINSTANCE hInstance;

	wchar_t *fileName;
	wchar_t *fileNameToShow;

  bool edited;

};
*/
#endif