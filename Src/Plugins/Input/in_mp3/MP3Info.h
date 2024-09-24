#ifndef NULLSOFT_IN_MP3_MP3_INFO_H
#define NULLSOFT_IN_MP3_MP3_INFO_H

#include "Metadata.h"
#include <windows.h>

class MP3Info 
{
public:
	MP3Info(const wchar_t *fn);

	char mpeg_description[1024];

	bool	isOld();
	
	void get_file_info();
	int write_id3v1();
	int remove_id3v1();
	void display_id3v1(HWND hwndDlg);
	void get_id3v1_values(HWND hwndDlg);
	void display_id3v2(HWND hwndDlg);
	void get_id3v2_values(HWND hwndDlg);
	void write_id3v2(HWND hwndDlg);
	void do_enable_id3v1(HWND hwndDlg, int en);
	void do_enable_id3v2(HWND hwndDlg, int en);
	
	BOOL CALLBACK id3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

	int setExtendedFileInfoW(const char *data, wchar_t *val);
	int writeExtendedFileInfo();
	
	bool IsMe(const wchar_t *fn)
	{
		return !lstrcmpW(file, fn);
	}
protected:
	// Keep track of file timestamp for file system change notification handling
	FILETIME		last_write_time;

private:
	void SetField(const wchar_t *value, wchar_t *&v2, char *v1, size_t v1size);
	Metadata metadata;	
	wchar_t file[MAX_PATH];
};


#endif