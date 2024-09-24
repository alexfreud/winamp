#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "tagz.h"

#ifdef TAGZ_UNICODE
#define _TX(X) L##X
#define t_strdup _wcsdup
#define t_strlen wcslen
#define t_strnicmp _wcsnicmp
#define t_strtoul wcstoul
#define t_stricmp _wcsicmp
#define t_strstr wcsstr

#define sprintf swprintf
#else
#define _TX(X) X
#define t_strdup _strdup
#define t_strlen strlen
#define t_strnicmp _strnicmp
#define t_strtoul strtoul
#define t_stricmp _stricmp
#define t_strstr strstr

#endif

int t_atoi(T_CHAR * c)
{
	if (c[0]=='-') return -(int)t_strtoul(c+1,0,10);
	else return (int)t_strtoul(c,0,10);
}

#define TABSIZE(x) (sizeof(x)/sizeof(x[0]))
/*
char * strndup(char * p1,UINT n)
{
	char * r=(char*)malloc(n+1);
	if (r)
	{
		memcpy(r,p1,n);
		r[n]=0;
	}
	return n;
}
*/

class String
{
private:
	T_CHAR * data;
	UINT size, used;
public:
	String() {data=0;size=0;used=0;}
	void AddChar(T_CHAR c)
	{
		if (!data)
		{
			data=(T_CHAR*)malloc((size=512)*sizeof(T_CHAR));
			used=0;
		}
		else if (size==used)
		{
			UINT old_size = size;
			T_CHAR * old_data = data;
			size<<=1;
			data=(T_CHAR*)realloc((char*)data,size*sizeof(T_CHAR));
			if (!data)
			{
				size = old_size;
				data = old_data;
				return;
			}
		}
		if (data) data[used++]=c;
	}
	void AddInt(int i)
	{
		T_CHAR foo[16];
		sprintf(foo,_TX("%i"),i);
		AddString(foo);
	}
	void AddString(const T_CHAR * z)
	{
		while(*z) {AddChar(*z);z++;}
	}
	void AddString(String & s)
	{
		AddString(s.Peek());
	}
	~String()
	{
		if (data) free(data);
	}
	T_CHAR * GetBuf()
	{
		if (!data) return t_strdup(_TX(""));
		T_CHAR * r=(T_CHAR*)realloc(data,(used+1)*sizeof(T_CHAR));
		r[used]=0;
		data=0;
		return r;
	}
	T_CHAR operator[](UINT i)
	{
		if (!data || i>=used) return 0;
		else return data[i];
	}
	UINT Len() {return data ? used : 0;}
	void Reset()
	{
		if (data) {free(data);data=0;}
	}
	const T_CHAR * Peek()
	{
		AddChar(0);
		used--;
		return data;
	}
	T_CHAR * _strdup()
	{
		return ::t_strdup(Peek());
	}
};




static int separator(T_CHAR x)
{
	if (!x || x==' ') return 1;
	if (x=='\'' || x=='_') return 0;
#ifdef TAGZ_UNICODE
	return !iswalnum(x);
#else
	return !isalnum(x);
#endif
}

static int sepcmp(T_CHAR* src,T_CHAR* val)
{
	UINT l=t_strlen(val);
	return !t_strnicmp(src,val,l) && separator(src[l]);
}

static char roman_num[]=
{
	'I','V','X','L','C','D','M'
};


static int is_roman(T_CHAR * ptr)//could be more smart i think
{
	if (ptr[0]==']' && ptr[1]=='[' && separator(ptr[2])) return 1;
	while(!separator(*ptr))
	{
		UINT n;
		bool found=0;
		for(n=0;n<TABSIZE(roman_num);n++)
		{
			if (*ptr==roman_num[n]) {found=1;break;}
		}
		if (!found) return 0;
		ptr++;
	}
	return 1;
}

static int need_full(T_CHAR* ptr)
{
	if (is_roman(ptr)) return 1;
	if (sepcmp(ptr,_TX("RPG"))) return 1;
	while(!separator(*ptr))
	{
		if (*ptr<='0' || *ptr>='9') return 0;
		ptr++;
	}
	return 1;
}

class VarList
{
private:
	typedef struct tagVAR
	{
		tagVAR * next;
		T_CHAR * name;
		T_CHAR * value;
	} VAR;
	VAR * vars;
public:
	VarList() {vars=0;}
	~VarList()
	{
		VAR * p=vars;
		while(p)
		{
			VAR *n=p->next;
			free(p->name);
			free(p->value);
			delete p;
			p=n;
		}
	}
	T_CHAR * Get(T_CHAR * name)
	{
		VAR * p=vars;
		while(p)
		{
			if (!t_stricmp(name,p->name)) return p->value;
			p=p->next;
		}
		return 0;
	}
	void Put(T_CHAR * name,T_CHAR* value)
	{
		VAR * p=vars;
		while(p)
		{
			if (!t_stricmp(name,p->name))
			{
				free(p->value);
				p->value=t_strdup(value);
				return;
			}
			p=p->next;
		}
		p=new VAR;
		p->next=vars;
		vars=p;
		p->value=t_strdup(value);
		p->name=t_strdup(name);
	}
};

typedef void (*TEXTFUNC)(UINT n_src,T_CHAR **src,UINT*,String &out,VarList & vars);


#define MAKEFUNC(X) static void X(UINT n_src,T_CHAR ** src,UINT *found_src,String &out,VarList &vars)

/*
void Blah(UINT n_src,T_CHAR ** src,UINT*,String &out)
{
	out.AddString("blah");
}

void Nop(UINT n_src,T_CHAR ** src,UINT *,String &out)
{
	UINT n;
	for(n=0;n<n_src;n++) out.AddString(src[n]);
}
*/

MAKEFUNC(Get)
{
	if (n_src>=1)
	{
		T_CHAR * p=vars.Get(src[0]);
		if (p) out.AddString(p);
	}
}

MAKEFUNC(Put)
{
	if (n_src>=2)
	{
		vars.Put(src[0],src[1]);
		out.AddString(src[1]);
	}
}

MAKEFUNC(PutQ)
{
	if (n_src>=2)
	{
		vars.Put(src[0],src[1]);
	}
}

MAKEFUNC(If)
{
	if (n_src!=3) out.AddString(_TX("[INVALID $IF SYNTAX]"));
	else
	{
		out.AddString(src[found_src[0] ? 1 : 2]);
	}
}

MAKEFUNC(Iflonger)
{
	if (n_src!=4) out.AddString(_TX("[INVALID $IFLONGER SYNTAX]"));
	else
	{
		out.AddString(src[(int)t_strlen(src[0])>t_atoi(src[1]) ? 2 : 3]);
	}
}

MAKEFUNC(Ifgreater)
{
	if (n_src!=4) out.AddString(_TX("[INVALID $IFGREATER SYNTAX]"));
	else
	{
		out.AddString(src[t_atoi(src[0])>t_atoi(src[1]) ? 2 : 3]);
	}
}

MAKEFUNC(Upper)
{
	if (n_src>=1)
	{
		T_CHAR * s=src[0];
		while(*s)
		{
			out.AddChar(toupper(*(s++)));
		}
	}
}

MAKEFUNC(Lower)
{
	if (n_src>=1)
	{
		T_CHAR * s=src[0];
		while(*s)
		{
			out.AddChar(tolower(*(s++)));
		}
	}
}

MAKEFUNC(Pad)
{
	if (n_src>=2)
	{
		T_CHAR *fill=_TX(" ");
		if (n_src>=3 && src[2][0]) fill=src[2];
		int num=t_atoi(src[1]);
		T_CHAR *p=src[0];
		while(*p) {out.AddChar(*(p++));num--;}
		UINT fl=t_strlen(fill);
		while(num>0) {out.AddChar(fill[(--num)%fl]);}
	}
}

MAKEFUNC(Cut)
{
	if (n_src>=2)
	{
		UINT num=t_atoi(src[1]);
		T_CHAR *p=src[0];
		while(*p && num) {out.AddChar(*(p++));num--;}
	}
}

MAKEFUNC(PadCut)
{
	if (n_src>=2)
	{
		T_CHAR *fill=_TX(" ");
		if (n_src>=3 && src[2][0]) fill=src[3];
		int num=t_atoi(src[1]);
		T_CHAR *p=src[0];
		while(*p && num>0) {out.AddChar(*(p++));num--;}
		UINT fl=t_strlen(fill);
		while(num>0) {out.AddChar(fill[(--num)%fl]);}
	}
}

MAKEFUNC(Abbr)
{//abbr(string,len)
	if (n_src==0 || (n_src>=2 && (int)t_strlen(src[0])<t_atoi(src[1]))) return;
	T_CHAR * meta=src[0];
	bool w=0,r=0;
	while(*meta)
	{
		bool an=!separator(*meta) || *meta==']' || *meta=='[';
		if (w && !an)
		{
			w=0;
		}
		else if (!w && an)
		{
			w=1;
			if (!sepcmp(meta,_TX("a")) && !sepcmp(meta,_TX("the")))
			{
				r=need_full(meta)?1:0;
				out.AddChar(*meta);
			}
			else r=0;
		}
		else if (w && r)
		{
			out.AddChar(*meta);
		}
		meta++;
	}
}



MAKEFUNC(Caps)
{
	if (n_src<1) return;
	T_CHAR* sp=src[0];
	int sep=1;
	while(*sp)
	{
		T_CHAR c=*(sp++);
		int s = separator(c);
		if (!s && sep)
			c=toupper(c);
		else if (!sep) c=tolower(c);
		sep=s;
		out.AddChar(c);
	}
}

MAKEFUNC(Caps2)
{
	if (n_src<1) return;
	T_CHAR* sp=src[0];
	int sep=1;
	while(*sp)
	{
		T_CHAR c=*(sp++);
		int s = separator(c);
		if (!s && sep)
			c=toupper(c);
		sep=s;
		out.AddChar(c);
	}
}

MAKEFUNC(Longest)
{
	T_CHAR * ptr=0;
	UINT n,m=0;
	for(n=0;n<n_src;n++)
	{
		UINT l=t_strlen(src[n]);
		if (l>m) {m=l;ptr=src[n];}
	}
	if (ptr) out.AddString(ptr);
}

MAKEFUNC(Shortest)
{
	T_CHAR * ptr=0;
	UINT n,m=(UINT)(-1);
	for(n=0;n<n_src;n++)
	{
		UINT l=t_strlen(src[n]);
		if (l<m) {m=l;ptr=src[n];}
	}
	if (ptr) out.AddString(ptr);
}

MAKEFUNC(Num)
{
	if (n_src==2)
	{
		T_CHAR tmp[16];
		T_CHAR tmp1[16];
		sprintf(tmp1,_TX("%%0%uu"),t_atoi(src[1]));
		sprintf(tmp,tmp1,t_atoi(src[0]));
		out.AddString(tmp);
	}
}

MAKEFUNC(Hex)
{
	if (n_src==2)
	{
		T_CHAR tmp[16];
		T_CHAR tmp1[16];
		sprintf(tmp1,_TX("%%0%ux"),t_atoi(src[1]));
		sprintf(tmp,tmp1,t_atoi(src[0]));
		out.AddString(tmp);
	}
}

MAKEFUNC(StrChr)
{
	if (n_src==2)
	{
		T_CHAR * p=src[0];
		T_CHAR s=src[1][0];
		while(*p && *p!=s) p++;
		if (*p==s) out.AddInt(1+p-src[0]);
		else out.AddChar('0');
	}
}

MAKEFUNC(StrRChr)
{
	if (n_src==2)
	{
		T_CHAR * p=src[0],*p1=0;
		T_CHAR s=src[1][0];
		while(*p)
		{
			if (*p==s) p1=p;
			p++;
		}
		if (p1) out.AddInt(1+p1-src[0]);
		else out.AddChar('0');

	}
}

MAKEFUNC(StrStr)
{
	if (n_src==2)
	{
		T_CHAR * p=t_strstr(src[0],src[1]);
		if (p) out.AddInt(1+p-src[0]);
		else out.AddChar('0');
	}
}

MAKEFUNC(SubStr)
{
	int n1,n2;
	if (n_src<2) return;
	n1=t_atoi(src[1]);
	if (n_src>=3)
	{
		n2=t_atoi(src[2]);
	}
	else n2=n1;
	if (n1<1) n1=1;
	if (n2>=n1)
	{
		n1--;
		n2--;
		while(n1<=n2 && src[0][n1])
		{
			out.AddChar(src[0][n1++]);
		}
	}
}

MAKEFUNC(Len)
{
	if (n_src>=1) out.AddInt(t_strlen(src[0]));
}

MAKEFUNC(FileName)
{
	if (n_src<1) return;
	T_CHAR * p=src[0];
	T_CHAR * p1=0;
	while(*p)
	{
		if (*p=='\\' || *p=='/') p1=p;
		p++;
	}
	if (p1) p1++;
	else p1=p;
	while(*p1 && *p1!='.') out.AddChar(*(p1++));
}

MAKEFUNC(Add)
{
	UINT n;
	int s=0;
	for(n=0;n<n_src;n++)
	{
		s+=t_atoi(src[n]);
	}
	out.AddInt(s);
}

MAKEFUNC(Sub)
{
	if (n_src>=1)
	{
		UINT n;
		int s=t_atoi(src[0]);
		for(n=1;n<n_src;n++)
		{
			s-=t_atoi(src[n]);
		}
		out.AddInt(s);
	}
}

MAKEFUNC(Mul)
{
	UINT n;
	int s=1;
	for(n=0;n<n_src;n++)
	{
		s*=t_atoi(src[n]);
	}
	out.AddInt(s);
}
				
MAKEFUNC(Div)
{
	if (n_src>=1)
	{
		UINT n;
		int s=t_atoi(src[0]);
		for(n=1;n<n_src;n++)
		{
			int t=t_atoi(src[n]);
			if (t) s/=t;
			else t=0;
		}
		out.AddInt(s);
	}
}

MAKEFUNC(Mod)
{
	if (n_src>=1)
	{
		UINT n;
		int s=t_atoi(src[0]);
		for(n=1;n<n_src;n++)
		{
			int t=t_atoi(src[n]);
			if (t) s%=t;
			else t=0;
		}
		out.AddInt(s);
	}
}

MAKEFUNC(Max)
{
	if (!n_src) return;
	int m=t_atoi(src[0]);
	UINT n;
	for(n=1;n<n_src;n++)
	{
		int t=t_atoi(src[n]);
		if (t>m) m=t;
	}
	out.AddInt(m);
}

MAKEFUNC(Min)
{
	if (!n_src) return;
	int m=t_atoi(src[0]);
	UINT n;
	for(n=1;n<n_src;n++)
	{
		int t=t_atoi(src[n]);
		if (t<m) m=t;
	}
	out.AddInt(m);
}

MAKEFUNC(Null)
{
}

struct
{
	TEXTFUNC func;
	const T_CHAR * name;
} FUNCS[]={
//	Blah,"blah",
//	Nop,"nop",
	If,_TX("if"),
	Upper,_TX("upper"),
	Lower,_TX("lower"),
	Pad,_TX("pad"),
	Cut,_TX("cut"),
	PadCut,_TX("padcut"),
	Abbr,_TX("abbr"),
	Caps,_TX("caps"),
	Caps2,_TX("caps2"),
	Longest,_TX("longest"),
	Shortest,_TX("shortest"),
	Iflonger,_TX("iflonger"),
	Ifgreater,_TX("ifgreater"),
	Num,_TX("num"),Num,_TX("dec"),
	Hex,_TX("hex"),
	StrChr,_TX("strchr"),
	StrChr,_TX("strlchr"),
	StrRChr,_TX("strrchr"),
	StrStr,_TX("strstr"),
	SubStr,_TX("substr"),
	Len,_TX("len"),
	Add,_TX("add"),
	Sub,_TX("sub"),
	Mul,_TX("mul"),
	Div,_TX("div"),
	Mod,_TX("mod"),
	FileName,_TX("filename"),
	Min,_TX("min"),
	Max,_TX("max"),
	Get,_TX("get"),
	Put,_TX("put"),
	PutQ,_TX("puts"),
	Null,_TX("null"),
};


class FMT
{
private:
	String str;
	VarList *vars;
	T_CHAR * spec;
	TAGFUNC f;
	TAGFREEFUNC ff;
	void * fp;
	T_CHAR * org_spec;
	int found;

	void Error(T_CHAR *e=0) {str.Reset();str.AddString(e ? e : _TX("[SYNTAX ERROR IN FORMATTING STRING]"));}

	T_CHAR * _FMT(T_CHAR * s,UINT *f=0)
	{
		FMT fmt(this,s);
		T_CHAR * c=(T_CHAR*)fmt;
		if (f) *f=fmt.found;
		found+=fmt.found;
		return c;
	}

	static bool skipshit(T_CHAR** _p,T_CHAR *bl)
	{
		T_CHAR * p=*_p;
		int bc1=0,bc2=0;
		while(*p)
		{
			if (!bc1 && !bc2 && bl)
			{
				T_CHAR *z=bl;
				while(*z)
				{
					if (*z==*p) break;
					z++;
				}
				if (*z) break;				
			}
			if (*p=='\'')
			{
				p++;
				while(*p && *p!='\'') p++;
				if (!*p) return 0;
			}
			else if (*p=='(') bc1++;
			else if (*p==')')
			{
				if (--bc1<0) return 0;
			}
			else if (*p=='[') bc2++;
			else if (*p==']')
			{
				if (--bc2<0) return 0;
			}
			p++;
		}
		*_p=p;
		return *p && !bc1 && !bc2;
	}

	void run()
	{
		if (!spec) {Error();return;}
		while(*spec)
		{
			if (*spec=='%')
			{
				spec++;
				if (*spec=='%') {str.AddChar('%');spec++;continue;}
				T_CHAR* s1=spec+1;
				while(*s1 && *s1!='%') s1++;
				if (!*s1) {Error();break;}
				*s1=0;
				T_CHAR * tag=f(spec,fp);
				*s1='%';
				//if (!tag) tag=tag_unknown;
				if (tag && tag[0])
				{
					found++;
					str.AddString(tag);
				}
				else
				{
					str.AddString(_TX("?"));
				}
				if (tag && ff) ff(tag,fp);
				spec=s1+1;
			}
			else if (*spec=='$')
			{
				spec++;
				if (*spec=='$') {str.AddChar('$');spec++;continue;}
				T_CHAR * s1=spec+1;
				while(*s1 && *s1!='(') s1++;
				if (!*s1) {Error();break;}
				T_CHAR * s2=s1+1;
				if (!skipshit(&s2,_TX(")"))) {Error();break;}
				if (!*s2) {Error();break;};
				T_CHAR * p=s1+1;
				T_CHAR* temp[64];
				UINT temp_f[64];
				UINT nt=0;
				T_CHAR * p1=s1+1;
				while(p<=s2 && nt<64)
				{
					if (!skipshit(&p,_TX(",)"))) {Error();return;}
					if (p>s2 || (*p!=',' && *p!=')')) {Error(_TX("internal error"));return;}
					T_CHAR bk=*p;
					*p=0;
					temp[nt]=_FMT(p1,&temp_f[nt]);
					nt++;
					*p=bk;;
					p1=p+1;
					p++;
				}
				*s1=0;
				UINT n;
				TEXTFUNC fn=0;
				for(n=0;n<TABSIZE(FUNCS);n++)
				{
					if (!t_stricmp(spec,FUNCS[n].name)) {fn=FUNCS[n].func;break;}
				}
				*s1='(';
				if (fn)
				{
					fn(nt,temp,temp_f,str,*vars);
				}
				else
				{
					str.AddString(_TX("[UNKNOWN FUNCTION]"));
				}
				for(n=0;n<nt;n++) free(temp[n]);
				spec=s2+1;
			}
			else if (*spec=='\'')
			{
				spec++;
				if (*spec=='\'') {str.AddChar('\'');spec++;continue;}
				T_CHAR * s1=spec+1;
				while(*s1 && *s1!='\'') s1++;
				if (!*s1) {Error();break;}
				*s1=0;
				str.AddString(spec);
				*s1='\'';
				spec=s1+1;
			}
			else if (*spec=='[')
			{
				spec++;
				T_CHAR * s1=spec;
				if (!skipshit(&s1,_TX("]"))) {Error();break;}
				T_CHAR bk=*s1;
				*s1=0;
				FMT fmt(this,spec);
				fmt.run();
				if (fmt.found)
				{
					str.AddString(fmt);
					found+=fmt.found;
				}
				*s1=bk;
				spec=s1+1;
			}
			else if (*spec==']' || *spec=='(' || *spec==')') {Error();break;}
			else
			{
				str.AddChar(*spec);
				spec++;
			}
		}
	}

	FMT(FMT* base,T_CHAR * _spec)
	{
		vars=base->vars;
		found=0;
		org_spec=0;
		f=base->f;
		ff=base->ff;
		fp=base->fp;
		spec=_spec;
	}
public:
	FMT(T_CHAR * p_spec,TAGFUNC _f,TAGFREEFUNC _ff,void * _fp,VarList * _vars)
	{
		vars=_vars;
		found=0;
		org_spec=spec=t_strdup(p_spec);
		f=_f;
		ff=_ff;
		fp=_fp;
	}
	operator T_CHAR*()
	{
		run();
		return str.GetBuf();
	}
	~FMT()
	{
		if (org_spec) free(org_spec);
	}
};

extern "C"
{

UINT tagz_format(T_CHAR * spec,TAGFUNC f,TAGFREEFUNC ff,void *fp,T_CHAR* out,UINT max)
{
	T_CHAR * zz=tagz_format_r(spec,f,ff,fp);
	UINT r=0;
	while(r<max-1 && zz[r])
	{
		out[r]=zz[r];
		r++;
	}
	out[r]=0;
	free(zz);
	return r;
}	

T_CHAR * tagz_format_r(T_CHAR* spec,TAGFUNC f,TAGFREEFUNC ff,void * fp)
{
	VarList vars;
	return FMT(spec,f,ff,fp,&vars);
}

//char tagz_manual[]="TODO: WTFM";

char tagz_manual[]="Syntax reference: \n"
	"\n"
	"* %tagname% - inserts field named <tagname>, eg. \"%artist%\"\n"
	"* $abbr(x) - inserts abbreviation of x, eg. \"$abbr(%album%)\" - will convert album name of \"Final Fantasy VI\" to \"FFVI\"\n"
	"* $abbr(x,y) - inserts abbreviation of x if x is longer than y characters; otherwise inserts full value of x, eg. \"$abbr(%album%,10)\"\n"
	"* $lower(x), $upper(x) - converts x to in lower/uppercase, eg. \"$upper(%title%)\"\n"
	"* $num(x,y) - displays x number and pads with zeros up to y characters (useful for track numbers), eg. $num(%tracknumber%,2)\n"
	"* $caps(x) - converts first letter in every word of x to uppercase, and all other letters to lowercase, eg. \"blah BLAH\" -> \"Blah Blah\"\n"
	"* $caps2(x) - similar to $caps, but leaves uppercase letters as they are, eg. \"blah BLAH\" -> \"Blah BLAH\"\n"
	"* $if(A,B,C) - if A contains at least one valid tag, displays B, otherwise displays C; eg. \"$if(%artist%,%artist%,unknown artist)\" will display artist name if present; otherwise will display \"unknown artist\"; note that \"$if(A,A,)\" is equivalent to \"[A]\" (see below)\n"
	"* $longest(A,B,C,....) - compares lengths of output strings produced by A,B,C... and displays the longest one, eg. \"$longest(%title%,%comment%)\" will display either title if it's longer than comment; otherwise it will display comment\n"
	"* $pad(x,y) - pads x with spaces up to y characters\n"
	"* $cut(x,y) - truncates x to y characters\n"
	"* $padcut(x,y) - pads x to y characters and truncates to y if longer\n"
	"* [ .... ] - displays contents of brackets only if at least one of fields referenced inside has been found, eg. \"%artist% - [%album% / ]%title%\" will hide [] block if album field is not present\n"
	"* \' (single quotation mark) - outputs raw text without parsing, eg, \'blah$blah%blah[][]\' will output the contained string and ignore all reserved characters (%,$,[,]) in it; you can use this feature to insert square brackets for an example.\n"
	"\n"
	"eg. \"[%artist% - ][$abbr(%album%,10)[ %tracknumber%] / ]%title%[ %streamtitle%]\"\n";

}