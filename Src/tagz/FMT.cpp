#include "tagz.h"
#include "functions.h"
#include <windows.h>
#include <shlwapi.h>
#include "api__tagz.h"
#include "resource.h"

void CopyChar(LPTSTR dest, LPCTSTR src)
{
	LPTSTR end = CharNext(src);
	int count = (int)(end-src);
	if (count <= 0) return;
	while (count--)
	{
		*dest++=*src++;
	}
}

static int CharacterCompare(LPCTSTR ch1, LPCTSTR ch2)
{
	TCHAR str1[3]={0,0,0}, str2[3]={0,0,0};

	CopyChar(str1, ch1);
	CopyChar(str2, ch2);

	return memcmp(str1, str2, 3*sizeof(TCHAR));
}

static bool skipshit(wchar_t ** _p, wchar_t *skipCharacters)
{ 
	LPTSTR p = *_p;
	int paranCount = 0, bracketCount = 0;
	while (p && *p)
	{
		if (!paranCount && !bracketCount && skipCharacters)
		{
			LPTSTR skipChar = skipCharacters;
			while (skipChar && *skipChar)
			{
				if (!CharacterCompare(skipChar, p))
					break;
				skipChar++;
			}
			if (skipChar && *skipChar)
				break;
		}

		if (*p == L'\'')
		{
			p = CharNext(p);
			while (p && *p && *p != L'\'')
				p = CharNext(p);
			if (!p || p && !*p)
				return 0;
		}
		else if (*p == L'(') 
		{
			paranCount++;
		}
		else if (*p == L')')
		{
			if (--paranCount < 0)
				return 0;
		}
		else if (*p == L'[')
		{
			bracketCount++;
		}
		else if (*p == L']')
		{
			if (--bracketCount < 0) 
				return 0;
		}
		p = CharNext(p);
	}
	*_p = p;
	return *p && !paranCount && !bracketCount;
}

void FMT::run()
{
	if (!spec)
	{
		Error();
		return ;
	}
	while (spec && *spec)
	{
		if (*spec == L'%')
		{
			spec = CharNext(spec);
			if (spec && *spec == L'%')
			{
				str.AddChar(L'%');
				spec = CharNext(spec);
				continue;
			}

			wchar_t *s1 = CharNext(spec);
			while (s1 && *s1 && *s1 != L'%')
				s1 = CharNext(s1);

			if (!s1 || !*s1)
			{
				Error();
				break;
			}

			*s1 = 0;
			wchar_t *tag = tagProvider->GetTag(spec, parameters);
			*s1 = L'%';
			if (tag && tag[0])
			{
				found++;
				str.AddString(tag);
			}
			else
			{
				// this isgay
				//str.AddString(_TX("?"));
			}
			if (tag)
				tagProvider->FreeTag(tag);
			spec = CharNext(s1);
		}
		else if (*spec == L'$')
		{
			spec = CharNext(spec);
			if (spec && *spec == L'$')
			{
				str.AddChar(L'$');
				spec = CharNext(spec);
				continue;
			}
			wchar_t *s1 = CharNext(spec);
			while (s1 && *s1 && *s1 != L'(')
				s1 = CharNext(s1);

			if (!s1 || !*s1)
			{
				Error();
				break;
			}
			wchar_t *s2 = CharNext(s1);
			if (!skipshit(&s2, L")"))
			{
				Error();
				break;
			}
			if (!s2 || !*s2)
			{
				Error();
				break;
			}
			wchar_t *p = CharNext(s1);
			wchar_t *temp[64] = {0};
			size_t temp_f[64] = {0};
			size_t nt = 0;
			wchar_t *p1 = CharNext(s1);
			while (p && p <= s2 && nt < 64)
			{
				if (!skipshit(&p, L",)"))
				{
					Error();
					return ;
				}
				if (p > s2 || (*p != L',' && *p != L')'))
				{
					Error(WASABI_API_LNGSTRINGW(IDS_INTERNAL_ERROR));
					return ;
				}
				wchar_t bk = *p;
				*p = 0;
				temp[nt] = _FMT(p1, &temp_f[nt]);
				nt++;
				*p = bk;;
				p1 = CharNext(p);
				p = CharNext(p);
			}
			*s1 = 0;
			size_t n;
			TEXTFUNC fn = 0;
			for (n = 0;FUNCS[n].func;n++)
			{
				if (!StrCmpI(spec, FUNCS[n].name))
				{
					fn = FUNCS[n].func;
					break;
				}
			}
			*s1 = L'(';
			if (fn)
			{
				fn(nt, temp, temp_f, &str, vars);
			}
			else
			{
				str.AddString(WASABI_API_LNGSTRINGW(IDS_UNKNOWN_FUNCTION));
			}
			for (n = 0;n < nt;n++)
				free(temp[n]);
			spec = CharNext(s2);
		}
		else if (*spec == L'\'')
		{
			spec = CharNext(spec);
			if (spec && *spec == L'\'')
			{
				str.AddChar(L'\'');
				spec++;
				continue;
			}
			LPTSTR s1 = CharNext(spec);
			while (s1 && *s1 && *s1 != L'\'')
				s1 = CharNext(s1);
			if (!s1 || !*s1)
			{
				Error();
				break;
			}
			*s1 = 0;
			str.AddString(spec);
			*s1 = L'\'';
			spec = CharNext(s1);
		}
		else if (*spec == L'[')
		{
			spec = CharNext(spec);
			LPTSTR s1 = spec;
			if (!skipshit(&s1, L"]"))
			{
				Error();
				break;
			}
			wchar_t bk = *s1;
			*s1 = 0;
			FMT fmt(this, spec);
			fmt.run();
			if (fmt.found)
			{
				wchar_t *zz = fmt;
				str.AddString(zz);
				found += fmt.found;
				free(zz);
			}
			*s1 = bk;
			spec = CharNext(s1);
		}
		else if (*spec == L']' || *spec == L'(' || *spec == L')')
		{
			Error();
			break;
		}
		else
		{
			str.AddDBChar(spec);
			spec = CharNext(spec);
		}
	}
}

FMT::FMT(const wchar_t *p_spec, ifc_tagprovider *_tagProvider, ifc_tagparams *_parameters, VarList * _vars)
		: vars(_vars), tagProvider(_tagProvider), parameters(_parameters),
		found(0)
{
	org_spec = spec = _wcsdup(p_spec);
}

void FMT::Open(const wchar_t *p_spec, ifc_tagprovider *_tagProvider, ifc_tagparams *_parameters, VarList * _vars)
{
	vars=_vars;
	tagProvider = _tagProvider;
	parameters = _parameters;
	org_spec = spec = _wcsdup(p_spec);
}

FMT::operator LPTSTR()
{
	run();
	return str.GetBuf();
}

void FMT::Error(wchar_t *e)
{
	str.Reset();
	str.AddString(e ? e : WASABI_API_LNGSTRINGW(IDS_SYNTAX_ERROR_IN_STRING));
}

FMT::FMT(FMT* base, wchar_t *_spec)
{
	vars = base->vars;
	found = 0;
	org_spec = 0;
	tagProvider = base->tagProvider;
	parameters=base->parameters;
	spec = _spec;
}

wchar_t * FMT::_FMT(wchar_t * s, size_t *f)
{
	FMT fmt(this, s);
	wchar_t * c = (wchar_t *)fmt;
	if (f) *f = fmt.found;
	found += fmt.found;
	return c;
}

FMT::~FMT()
{
	if (org_spec)
		free(org_spec);
}