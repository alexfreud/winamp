#ifndef _PFC_CFG_VAR_H_
#define _PFC_CFG_VAR_H_

#ifndef NOVTABLE
#define NOVTABLE _declspec(novtable)
#endif

#include "string.h"
#include "string_unicode.h"

//unicode in reg functions is NOT working

class NOVTABLE cfg_var
{
private:
	string var_name;
	static cfg_var * list;
	cfg_var * next;
	static HKEY reg_open(const char * regname);
protected:
	cfg_var(const char * name) : var_name(name) {next=list;list=this;};

	const char * var_get_name() {return var_name;}

	//override me
	virtual void read(HKEY hk)=0;
	virtual void write(HKEY hk)=0;
	virtual void reset()=0;

	//helper
	int reg_get_struct_size(HKEY hk);
	bool reg_read_struct(HKEY hk,void * ptr,UINT size);
	void reg_write_struct(HKEY hk,const void * ptr,UINT size);
	void reg_write_int(HKEY hk,int val);
	int reg_read_int(HKEY hk,int def);

public:
	static void config_read(const char *inifile, const char *section);
	static void config_write(const char *inifile, const char *section);
	static void config_reset();
};

class cfg_int : private cfg_var
{
private:
	int val,def;
	virtual void read(HKEY hk);
	virtual void write(HKEY hk);
	virtual void reset() {val=def;}
public:
	cfg_int(const wchar_t* name, int v) : cfg_var(string_utf8(name)) { val = def = v; }
	cfg_int(const char * name,int v) : cfg_var(name) {val=def=v;}
	operator int() const {return val;}
	int operator=(int v) {return val=v;}
	inline int get_def() {return def;}

};

class cfg_string : private cfg_var
{
private:
	string val,def;
	virtual void read(HKEY hk);
	virtual void write(HKEY hk);
	virtual void reset() {val=def;}
public:
	cfg_string(const char * name,const char * v) : cfg_var(name), val(v), def(v) {}
	operator const char * () const {return val;}
	const char * operator=(const char* v) {val=v;return val;}
	string & get_string() {return val;}
	void s_SetDlgItemText(HWND wnd,int id) {val.s_SetDlgItemText(wnd,id);}
	void s_GetDlgItemText(HWND wnd,int id) {val.s_GetDlgItemText(wnd,id);}
};

#ifdef PFC_UNICODE

class cfg_string_w : private cfg_var
{
private:
	string_w val,def;
	virtual void read(HKEY hk);
	virtual void write(HKEY hk);
	virtual void reset() {val=def;}
public:
	cfg_string_w(const char * name,const WCHAR * v) : cfg_var(name), val(v), def(v) {}
	cfg_string_w(const char * name,const char * v) : cfg_var(name), val(string_w(v)), def(string_w(v)) {}
	operator const WCHAR * () const {return val;}
	void operator=(const WCHAR* v) {val=v;}
	string_w & get_string() {return val;}
	void s_SetDlgItemText(HWND wnd,int id) {val.s_SetDlgItemText(wnd,id);}
	void s_GetDlgItemText(HWND wnd,int id) {val.s_GetDlgItemText(wnd,id);}
};

#endif

template<class T>
class cfg_struct_t : private cfg_var
{
private:
	T val,def;

	virtual void read(HKEY hk)
	{
		reg_read_struct(hk,&val,sizeof(T));
	}

	virtual void write(HKEY hk)
	{
		T temp = def;
		reg_read_struct(hk,&temp,sizeof(T));
		if (memcmp(&temp,&val,sizeof(T)))
			reg_write_struct(hk,&val,sizeof(T));
	}

	virtual void reset() {val=def;}
public:
	cfg_struct_t(const char * name,const T& v) : cfg_var(name) {val=def=v;}
	cfg_struct_t(const char * name,int filler) : cfg_var(name) {memset(&val,filler,sizeof(T));memset(&def,filler,sizeof(T));}
	T& get_val() {return val;}
	operator T&() {return val;}
	T* operator=(const T& v) {val=v; return (T*)&val;}
};

#endif