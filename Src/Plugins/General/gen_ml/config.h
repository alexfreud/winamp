#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

class C_Config
{
public:
	C_Config( wchar_t *ini );
	~C_Config();

	void     WriteInt( wchar_t *name, int value );
	wchar_t *WriteString( wchar_t *name, wchar_t *string );
	char    *WriteString( char *name, char *string );

	int      ReadInt( wchar_t *name, int defvalue );
	wchar_t *ReadString( wchar_t *name, wchar_t *defvalue );
	char    *ReadString( const char *name, char *defvalue );
	char    *ReadString( const char *section_name, const char *key_name, char *defvalue );
	bool     ReadString( const char *name, const char *defvalue, char *storage, size_t len );          // returns true
	bool     ReadString( const wchar_t *name, const wchar_t *defvalue, wchar_t *storage, size_t len ); // returns true

	wchar_t *GetIniFile()                        { return m_inifile; }

private:
	wchar_t  m_strbuf[ 8192 ];
	char     m_strbufA[ 8192 ];
	wchar_t *m_inifile;
	char    *m_inifileA;
	wchar_t *m_section;
};

#endif//_C_CONFIG_H_