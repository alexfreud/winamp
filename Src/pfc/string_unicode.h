#ifndef _PFC_STRING_UNICODE_H_
#define _PFC_STRING_UNICODE_H_

#include "string.h"

class string_w : public string_base<WCHAR>
{
public:
	string_w() {}
	string_w(HWND w) {s_GetWindowText(w);}
	string_w(const WCHAR * z) {set_string(z);}
	void add_string_a(const char * c);
	void set_string_a(const char * c);
	string_w(const char * z) {set_string_a(z);}
	string_w(const string_w & z) {set_string(z);}
	string_w(const string_a & z) {set_string_a(z);}
	void add_string_utf8(const char * z);
	void set_string_utf8(const char * z);
	void s_GetWindowText(HWND w);
	inline void from_window(HWND w) {s_GetWindowText(w);}
	void s_SetWindowText(HWND w);
	const WCHAR * operator=(const WCHAR * s) {set_string(s);return get_ptr();}
	const WCHAR * operator+=(const WCHAR * s) {add_string(s);return get_ptr();}
	const WCHAR * operator=(const char * s) {set_string_a(s);return get_ptr();}
	const WCHAR * operator+=(const char * s) {add_string_a(s);return get_ptr();}
	const WCHAR * operator=(string_w & s) {set_string(s);return get_ptr();}
	const WCHAR * operator+=(string_w & s) {add_string(s);return get_ptr();}
	inline void s_GetDlgItemText(HWND w,int id) {s_GetWindowText(GetDlgItem(w,id));}
	inline void s_SetDlgItemText(HWND w,int id) {s_SetWindowText(GetDlgItem(w,id));}
	bool reg_read(HKEY hk,const WCHAR * name);
	void reg_write(HKEY hk,const WCHAR * name);

	static bool test_os();
};

class string_reg : public string_w
{
public:
	string_reg(HKEY hk,const WCHAR * name) {reg_read(hk,name);}
	string_reg(HKEY hk,const char * name) {reg_read(hk,string_w(name));}
};


class string_printf_w : public string_w
{
public:
	string_printf_w(const WCHAR * fmt,...);
};


class string_utf8 : public string_a
{
private:
	void convert(const WCHAR * foo);
public:
	string_utf8(const WCHAR * foo) {convert(foo);}
	string_utf8(const char * foo) {convert(string_w(foo));}
};

#endif //_PFC_STRING_UNICODE_H_