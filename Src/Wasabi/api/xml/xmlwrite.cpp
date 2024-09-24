#include <precomp.h>

#include "xmlwrite.h"

#include <bfc/wasabi_std.h>
#include <bfc/string/bfcstring.h>
#include <bfc/parse/paramparser.h>

#if 0
static unsigned char xmltypecheck[256] = 
{
#define BT_COLON BT_NMSTRT
#include "latin1tab.h"
#undef BT_COLON
#include "asciitab.h"
        };
#endif

#define EFPRINTF (nohicharconversion ? eutf8fprintf : efprintf)

#include "../nu/AutoChar.h"
XMLWrite::XMLWrite(const wchar_t *filename, const wchar_t *doctype, const wchar_t *dtddoctype, int no_hi_chars_conversion)
{
	nohicharconversion = no_hi_chars_conversion;
	FILE *f =	_wfopen(filename, WF_WRITE_BINARY);
	Init(f, doctype, dtddoctype);
}

XMLWrite::XMLWrite(FILE *file, const wchar_t *doctype, const wchar_t *dtddoctype, int no_hi_chars_conversion)
{
	nohicharconversion = no_hi_chars_conversion;
	Init(file, doctype, dtddoctype);
}

void XMLWrite::Init(FILE *file, const wchar_t *doctype, const wchar_t *dtddoctype)
{
	fp = file;
	ASSERT(fp != NULL);   // sheet, need exceptions here
		indenter.setValue(L"");
	utf8fprintf(fp, L"<?xml version=\"1.0\" encoding='UTF-8' standalone=\"yes\"?>\n");
	if (dtddoctype != NULL)
		utf8fprintf(fp, L"<!DOCTYPE %s>\n", dtddoctype);
	pushCategory(doctype, 1, 0);
}

XMLWrite::~XMLWrite()
{
	popCategory(1, 0);
	fflush(fp);
	fclose(fp);
	ASSERT(titles.peek() == 0);
}

void XMLWrite::comment(const wchar_t *comment)
{
	utf8fprintf(fp, L"<!-- %s -->\n", comment);
}

void XMLWrite::pushCategory(const wchar_t *title, int wantcr, int wantindent)
{
	if (wantindent)
	{
		utf8fprintf(fp, L"%s<%s>%s", indenter.getValue(), title, wantcr ? L"\n" : L"");
	}
	else
		utf8fprintf(fp, L"<%s>%s", title, wantcr ? L"\n" : L"");
	indenter+=L"  ";
	ParamParser pp(title, L" ");
	titles.push(WCSDUP(pp.enumItem(0)));
}

void XMLWrite::pushCategoryAttrib(const wchar_t *title, int nodata)
{
	utf8fprintf(fp, L"%s<%s", indenter.getValue(), title);
	indenter+=L"  ";
	titles.push(nodata ? NULL : WCSDUP(title));
}

void XMLWrite::writeCategoryAttrib(const wchar_t *title, const int val)
{
	utf8fprintf(fp, L" %s=\"%d\"", title, val);
}

void XMLWrite::writeCategoryAttrib(const wchar_t *title, const wchar_t *str)
{
	if (!str)
		str = L"";
	utf8fprintf(fp, L" %s=\"", title);
	EFPRINTF(fp, L"%s", str);
	utf8fprintf(fp, L"\"");
}

void XMLWrite::closeCategoryAttrib(int wantcr)
{
	if (titles.top() == NULL)
		utf8fprintf(fp, L" /");
	utf8fprintf(fp, L">%s", wantcr ? L"\n" : L"");
}

void XMLWrite::writeAttribEmpty(const wchar_t *title, int wantcr, int wantindent)
{
	if (wantindent)
		utf8fprintf(fp, L"%s<%s/>%s", 	indenter.getValue(), title, wantcr ? L"\n" : L"");
	else
		utf8fprintf(fp, L"<%s/>%s", title, wantcr ? L"\n" : L"");
}

void XMLWrite::writeAttrib(const wchar_t *title, const wchar_t *text, int wantcr, int wantindent)
{
	if (text && *text)
	{
		if (wantindent)
			utf8fprintf(fp, L"%s<%s>", 	indenter.getValue(), title);
		else
			utf8fprintf(fp, L"<%s>", title);
		EFPRINTF(fp, L"%s", text);
		utf8fprintf(fp, L"</%s>%s", title, wantcr ? L"\n" : L"");
	}
	else
	{
		writeAttribEmpty(title, wantcr, wantindent);
	}
}

void XMLWrite::writeAttrib(const wchar_t *title, int val, int wantcr, int wantindent)
{
	if (wantindent)
		utf8fprintf(fp, L"%s<%s>%d</%s>%s", indenter.getValue(), title, val, title, wantcr ? L"\n" : L"");
	else
		utf8fprintf(fp, L"<%s>%d</%s>%s", title, val, title, wantcr ? L"\n" : L"");
}

int XMLWrite::popCategory(int wantcr, int wantindent)
{
	indenter.trunc(-2);
	wchar_t *title;
	int r = titles.pop(&title);
	if (!r) return 0;
	if (title != NULL)
	{
		if (wantindent)
			utf8fprintf(fp, L"%s</%s>%s", indenter.getValue(), title, wantcr ? L"\n" : L"");
		else
			utf8fprintf(fp, L"</%s>%s", title, wantcr ? L"\n" : L"");
		FREE(title);
	}
	return titles.peek();
}

int XMLWrite::utf8fprintf(FILE *fp, const wchar_t *format, ...)
{
		va_list v;
	StringW outstr;
	va_start(v, format);
	outstr.va_sprintf(format, v);
	va_end(v);

#ifdef _WIN32
	AutoChar utf8(outstr, CP_UTF8);
#else
#warning port me
  AutoChar utf8(outstr);
#endif
	const char *data = (const char *)utf8; // to make the next line less messay
	fwrite(data, STRLEN(data), 1, fp);
	return 0;
}

int XMLWrite::eutf8fprintf(FILE *fp, const wchar_t *format, ...)
{
	va_list v;
	StringW outstr;
	va_start(v, format);
	outstr.va_sprintf(format, v);
	va_end(v);

#ifdef _WIN32
	AutoChar utf8(outstr, CP_UTF8);
#else
#warning port me
  AutoChar utf8(outstr);
#endif
	
	const char *data = (const char *)utf8; // to make the next line less messay
	while (data && *data)
	{
		size_t cur_length=0;
		while (data[cur_length] && data[cur_length] != '<' && data[cur_length] != '>' && data[cur_length] != '&' && data[cur_length] != '\"' && data[cur_length] != '\'')
		{
			cur_length++;
		}
		fwrite(data, cur_length, 1, fp);
		data += cur_length;
		if (*data)
		{
			// if we get here, it was a special character
			switch(*data)
			{
				case '<': fwrite("&lt;", 4, 1, fp); break;
				case '>': fwrite("&gt;", 4, 1, fp); break;
				case '&': fwrite("&amp;", 5, 1, fp); break;
				case '\"': fwrite("&quot;", 6, 1, fp); break;
				case '\'': fwrite("&apos;", 6, 1, fp); break;
			}
			data++;
		}
	};
	
	return 0;
}

int XMLWrite::efprintf(FILE *fp, const wchar_t *format, ...)
{
	va_list v;
	// http://www.w3.org/TR/REC-xml#syntax
	int bcount = 0;
	StringW outstr;
	va_start(v, format);
	outstr.va_sprintf(format, v);
	va_end(v);
	size_t n = outstr.len();
	for (size_t i = 0; i != n; i++)
	{
		wchar_t c = outstr.getValue()[i];
		switch (c)
		{
		case '<': fwrite("&lt;", 4, 1, fp); bcount += 4; break;
		case '>': fwrite("&gt;", 4, 1, fp); bcount += 4; break;
		case '&': fwrite("&amp;", 5, 1, fp); bcount += 5; break;
		case '\"': fwrite("&quot;", 6, 1, fp); bcount += 6; break;
		case '\'': fwrite("&apos;", 6, 1, fp); bcount += 6; break;
		default:
			//        if (xmltypecheck[c] != 0)
			{
				// TODO: benski> optimize by scanning for the next character to be escaped (or NULL)
				size_t numChars=1;
				while (1)
				{
					wchar_t check = outstr.getValue()[i+numChars];
					if (check == 0
						|| check == '<'
						|| check == '>'
						|| check == '&'
						|| check == '\''
						|| check == '\"')
						break;
					numChars++;
				}
				const wchar_t *str = outstr.getValue() + i;
				int len = WideCharToMultiByte(CP_UTF8, 0, str, (int)numChars, 0, 0, 0, 0);
				char *utf8 = (char *)malloc(len);
				WideCharToMultiByte(CP_UTF8, 0, str, (int)numChars, utf8, len, 0, 0);
				fwrite(utf8, len, 1, fp);
				free(utf8);
				bcount+=(int)numChars;
				i+=(numChars-1);
			}
			break;
		}
	}
	return bcount;
}
