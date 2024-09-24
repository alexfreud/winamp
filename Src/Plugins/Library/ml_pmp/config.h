#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

class C_Config
{
  public:
    C_Config(wchar_t *ini,wchar_t *section=L"ml_pmp", C_Config * globalWrite=NULL);
    ~C_Config();
    void  WriteInt(wchar_t *name, int value, wchar_t *section=NULL);
    wchar_t *WriteString(wchar_t *name, wchar_t *string, wchar_t *section=NULL);
    int   ReadInt(wchar_t *name, int defvalue, wchar_t *section=NULL);
    wchar_t *ReadString(wchar_t *name, wchar_t *defvalue, wchar_t *section=NULL);
    wchar_t *GetIniFile(){return m_inifile;}
  private:
    wchar_t m_strbuf[8192];
    wchar_t *m_inifile;
    wchar_t *m_section;
    C_Config * globalWrite;
};

#endif//_C_CONFIG_H_
