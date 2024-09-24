#pragma once
template <class myChar>
class string_base
{
private:
	myChar * ptr;
	UINT size,used;
	void makespace(UINT s);
	static UINT mylen(const myChar * p) {UINT r=0;while(p[r]) r++;return r;}
public:
	void AddChar(myChar c)
	{
		makespace(used+2);
		ptr[used++]=c;
		ptr[used]=0;
	}
	string_base()
	{
		used=0;
		size=128;
		ptr=(myChar*)malloc(size*sizeof(myChar));
		ptr[0]=0;
	}

	~string_base() { if (ptr) free(ptr);}

	operator const myChar*() const {return ptr;}
	
	const myChar & operator*() const {return *ptr;}

	UINT Length() const {return used;}

	void AddString(const myChar * c)
	{
		UINT d=mylen(c);
		makespace(used+d+1);
		memcpy(ptr+used,c,sizeof(myChar)*d);
		used+=d;
		ptr[used]=0;
	}

	void Reset() {Truncate(0);}
	void Truncate(UINT x) {if (used>x) {used=x;ptr[x]=0;}}

	void SetString(const myChar * s) {Reset();AddString(s);}

	myChar * BufferStart(UINT n)
	{
		makespace(n+1);
		memset(ptr,0,size);
		return ptr;
	}

	inline void BufferDone() {used=mylen(ptr);}

	void SetChar(UINT offset,myChar c)//hack for some ghey routines
	{
		if (!c) Truncate(offset);
		else if (offset<used) ptr[offset]=c;
	}
};

template<class myChar>
class StringTemp
{
private:
	string_base<myChar> * parent;
	myChar * data;
public:
	StringTemp(string_base<myChar> & s,UINT siz) {parent=&s;data=s.BufferStart(siz);}
	~StringTemp() {parent->BufferDone();}
	operator myChar* () {return data;}
};

#define StringTempW StringTemp<WCHAR>
#define StringTempA StringTemp<char>

class StringW;

class String : public string_base<char>
{
public:
	String() {}
	String(HWND w) {s_GetWindowText(w);}
	String(const char* z) {SetString(z);}
	String(const WCHAR* z) {SetStringW(z);}
	String(const String& z) {SetString(z);}
	String(const StringW& z);
	void AddStringW(const WCHAR * c);
	void SetStringW(const WCHAR * c);
	void s_GetWindowText(HWND w);
	void s_SetWindowText(HWND w);
	void operator=(const char * s) {SetString(s);}
	void operator+=(const char * s) {AddString(s);}
	void operator=(String & s) {SetString(s);}
	void operator+=(String & s) {AddString(s);}
	inline void s_GetDlgItemText(HWND w,UINT id) {s_GetWindowText(GetDlgItem(w,id));}
	inline void s_SetDlgItemText(HWND w,UINT id) {s_SetWindowText(GetDlgItem(w,id));}
};

class StringW : public string_base<WCHAR>
{
public:
	StringW() {}
	StringW(HWND w) {s_GetWindowText(w);}
	StringW(const WCHAR * z) {SetString(z);}
	void AddStringA(const char * c);
	void SetStringA(const char * c);
	StringW(const char * z) {SetStringA(z);}
	StringW(const StringW & z) {SetString(z);}
	StringW(const String & z) {SetStringA(z);}
	void s_GetWindowText(HWND w);
	void s_SetWindowText(HWND w);
	void operator=(const WCHAR * s) {SetString(s);}
	void operator+=(const WCHAR * s) { if (s) AddString(s);}
	void operator=(StringW & s) {SetString(s);}
	void operator+=(StringW & s) {AddString(s);}
	inline void s_GetDlgItemText(HWND w,UINT id) {s_GetWindowText(GetDlgItem(w,id));}
	inline void s_SetDlgItemText(HWND w,UINT id) {s_SetWindowText(GetDlgItem(w,id));}
	bool reg_read(const char *name);
	void reg_write(const char *name);
};


class StringPrintf : public String
{
public:
	StringPrintf(const char * fmt,...);
};

class StringPrintfW : public StringW
{
public:
	StringPrintfW(const WCHAR * fmt,...);
};

template<class myChar>
class StringF2T : public string_base<myChar>
{
public:
	StringF2T(const myChar * fn)
	{
		const myChar * ptr=fn,*dot=0,*src=fn;
		while(ptr && *ptr)
		{
			if (*ptr=='\\' || *ptr=='/' || *ptr==':') src=ptr+1;
			else if (*ptr=='.') dot=ptr;
			ptr++;
		}

		while(src && *src && (!dot || src<dot)) AddChar(*(src++));
	}
};

#define StringF2T_A StringF2T<char>
#define StringF2T_W StringF2T<WCHAR>