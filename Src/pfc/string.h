#ifndef _PFC_STRING_H_
#define _PFC_STRING_H_

template <class myChar> class string_base
{
private:
	myChar * ptr;
	size_t size,used;
	void makespace(size_t s)
	{
		if (size<s)
		{
			do size<<=1; while(size<s);
			ptr=(myChar*)realloc(ptr,size*sizeof(myChar));
		}
	}

	static int mylen(const myChar * p) {int r=0;while(p[r]) r++;return r;}
public:
	const myChar * get_ptr() const {return ptr;}
	void add_char(myChar c)
	{
		makespace(used+2);
		ptr[used++]=c;
		ptr[used]=0;
	}
	string_base()
	{
		used=0;
		size=8;
		ptr=(myChar*)malloc(size*sizeof(myChar));
		ptr[0]=0;
	}

	~string_base() { if (ptr) free(ptr);}

	operator const myChar*() const {return get_ptr();}
	
	const myChar & operator*() const {return *get_ptr();}

	size_t length() const {return used;}
	int is_empty() const {return used==0;}

	void add_string(const myChar * c)
	{
		int d=mylen(c);
		makespace(used+d+1);
		memcpy(ptr+used,c,sizeof(myChar)*d);
		used+=d;
		ptr[used]=0;
	}

	void add_string_n(const myChar * c,int count)
	{
		makespace(used+count+1);
		memcpy(ptr+used,c,sizeof(myChar)*count);
		ptr[used+count]=0;
		used+=mylen(ptr+used);
	}

	void reset() {truncate(0);}
	void truncate(size_t x) {if (used>x) {used=x;ptr[x]=0;}}

	void set_string(const myChar * s) {reset();add_string(s);}

	myChar *buffer_get(size_t n)
	{
		makespace(n+1);
		memset(ptr,0,size);
		return ptr;
	}

	inline void buffer_done() {used=mylen(ptr);}

	void set_char(int offset,myChar c)//hack for some ghey routines
	{
		if (!c) truncate(offset);
		else if (offset<used) ptr[offset]=c;
	}

	int find_first(myChar c)	//return -1 if not found
	{
		int n;
		for(n=0;n<used;n++)
		{
			if (ptr[n]==c) return n;
		}
		return -1;
	}
	int find_last(myChar c)
	{
		int n;;
		for(n=used-1;n>=0;n--)
		{
			if (ptr[n]==c) return n;
		}
		return -1;
	}

	int replace_char(myChar c1,myChar c2)
	{
		int rv=0;
		int n;
		for(n=0;n<used;n++)
		{
			if (ptr[n]==c1) {ptr[n]=c2;rv++;}
		}
		return rv;
	}

};

template<class myChar>
class string_buffer
{
private:
	string_base<myChar> * parent;
	myChar * data;
public:
	string_buffer(string_base<myChar> & s,UINT siz) {parent=&s;data=s.buffer_get(siz);}
	~string_buffer() {parent->buffer_done();}
	operator myChar* () {return data;}
};

#define string_buffer_w string_buffer<WCHAR>
#define string_buffer_a string_buffer<char>

class string_w;
#define string string_a

class string_a : public string_base<char>
{
public:
	string_a() {}
	string_a(HWND w) {s_GetWindowText(w);}
	string_a(const char* z) {set_string(z);}
	string_a(const WCHAR* z) {set_string_w(z);}
	string_a(const string_a& z) {set_string(z);}
	string_a(const string_w& z);
	void add_string_w(const WCHAR * c);
	void set_string_w(const WCHAR * c);
	void s_GetWindowText(HWND w);
	inline void from_window(HWND w) {s_GetWindowText(w);}
	void s_SetWindowText(HWND w);
	const char * operator=(const char * s) {set_string(s);return get_ptr();}
	const char * operator+=(const char * s) {add_string(s);return get_ptr();}
	const char * operator=(const WCHAR * s) {set_string_w(s);return get_ptr();}
	const char * operator+=(const WCHAR * s) {add_string_w(s);return get_ptr();}
	const char * operator=(string_a & s) {set_string(s);return get_ptr();}
	const char * operator+=(string_a & s) {add_string(s);return get_ptr();}
	inline void s_GetDlgItemText(HWND w,int id) {s_GetWindowText(GetDlgItem(w,id));}
	inline void s_SetDlgItemText(HWND w,int id) {s_SetWindowText(GetDlgItem(w,id));}
	bool reg_read(HKEY hk,const char * name);
	void reg_write(HKEY hk,const char * name);
};

class string_printf_a : public string_a
{
public:
	string_printf_a(const char * fmt,...);
};

#define string_printf string_printf_a

template<class myChar>
class string_file_title : public string_base<myChar>
{
public:
	string_file_title(const myChar * fn)
	{
		const myChar * ptr=fn,*dot=0,*src=fn;
		while(*ptr)
		{
			if (*ptr=='\\' || *ptr=='/' || *ptr==':') src=ptr+1;
			else if (*ptr=='.') dot=ptr;
			ptr++;
		}

		while(*src && (!dot || src<dot)) add_char(*(src++));
	}
};

#define string_f2t_a string_file_title<char>
#define string_f2t_w string_file_title<WCHAR>

template<class myChar>
class string_extension : public string_base<myChar>
{
public:
	string_extension(const myChar * foo)
	{
		const myChar * ptr = 0;
		while(*foo)
		{
			if (*foo == '.') ptr = foo;
			foo++;
		}
		if (ptr) set_string(ptr+1);
	}
};

#define string_extension_a string_extension<char>
#define string_extension_w string_extension<WCHAR>

#endif //_PFC_STRING_H_