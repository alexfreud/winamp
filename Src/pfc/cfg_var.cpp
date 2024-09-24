#define STRICT
#include <windows.h>
#include "cfg_var.h"
#include "string_unicode.h"

static const char *m_inifile, *m_section;

int cfg_var::reg_read_int(HKEY hk,int def)
{

  return GetPrivateProfileIntA(m_section,var_get_name(),def,m_inifile);
}

void cfg_var::reg_write_int(HKEY hk,int val)
{
/*	long temp=val;
	RegSetValueEx(hk,var_get_name(),0,REG_DWORD,(const BYTE*)&temp,4);*/
  char tmp[512] = {0};
  wsprintfA(tmp,"%d",val);
  WritePrivateProfileStringA(m_section,var_get_name(),tmp,m_inifile);
}

void cfg_var::reg_write_struct(HKEY hk,const void * ptr,UINT size)
{

  WritePrivateProfileStructA(m_section,var_get_name(),(void *)ptr,size,m_inifile);
}

bool cfg_var::reg_read_struct(HKEY hk,void * ptr,UINT size)
{
  GetPrivateProfileStructA(m_section,var_get_name(),ptr,size,m_inifile);
  return 1;
}

int cfg_var::reg_get_struct_size(HKEY hk)
{
	DWORD sz=0,t=0;
	if (RegQueryValueExA(hk,var_get_name(),0,&t,0,&sz)!=ERROR_SUCCESS) return 0;
	return sz;
}

bool string_a::reg_read(HKEY hk,const char * name)
{
  char tmp[4096] = {0};
  GetPrivateProfileStringA(m_section,name,"|||",tmp,sizeof(tmp)-1,m_inifile);
  if(strstr(tmp,"|||")==tmp) return 0;
  lstrcpyA(buffer_get(strlen(tmp)+1),tmp);
	buffer_done();
	return 1;
}

void string_a::reg_write(HKEY hk,const char * name)
{
	WritePrivateProfileStringA(m_section,name,(const char*)*this,m_inifile);
}


cfg_var * cfg_var::list=0;

/*HKEY cfg_var::reg_open(const char * regname)
{
	HKEY hk;
	RegCreateKey(HKEY_CURRENT_USER,regname,&hk);
	return hk;
}*/


void cfg_var::config_read(const char *inifile, const char *section)
{
	HKEY hk = 0; //reg_open(regname);
  m_inifile=inifile;
  m_section=section;
	cfg_var * ptr;
	for(ptr = list; ptr; ptr=ptr->next) ptr->read(hk);
	//RegCloseKey(hk);
}

void cfg_var::config_write(const char *inifile, const char *section)
{
  HKEY hk = 0; //reg_open(regname);
  m_inifile=inifile;
  m_section=section;
	cfg_var * ptr;
	for(ptr = list; ptr; ptr=ptr->next) ptr->write(hk);
	//RegCloseKey(hk);
}

void cfg_var::config_reset()
{
	cfg_var * ptr;
	for(ptr = list; ptr; ptr=ptr->next) ptr->reset();
}

void cfg_int::read(HKEY hk)
{
	val = reg_read_int(hk,def);
}

void cfg_int::write(HKEY hk)
{
	if (val!=reg_read_int(hk,def))
		reg_write_int(hk,val);
}

void cfg_string::read(HKEY hk)
{
	string_a temp;
	if (temp.reg_read(hk,var_get_name())) val=temp;
}

void cfg_string::write(HKEY hk)
{
	string_a temp = def;
	string_a name = var_get_name();

	if (!temp.reg_read(hk,name) || lstrcmpA(val,temp))
		val.reg_write(hk,name);
}

#ifdef PFC_UNICODE

void cfg_string_w::read(HKEY hk)
{
	string_w temp;
	if (temp.reg_read(hk,string_w(var_get_name()))) val=temp;
}

void cfg_string_w::write(HKEY hk)
{
	string_w temp = def;
	string_w name = var_get_name();
	string_w val_w = val;

	if (!temp.reg_read(hk,name) || wcscmp(val_w,temp))
		val_w.reg_write(hk,name);
}

#endif