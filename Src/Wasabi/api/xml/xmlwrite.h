#ifndef _XML_H
#define _XML_H

#include <bfc/wasabi_std.h>
#include <bfc/stack.h>
#include <bfc/string/StringW.h>

class XMLWrite
{
public:
	XMLWrite(const wchar_t *filename, const wchar_t *doctype, const wchar_t *dtddoctype = NULL, int no_hi_chars_conversion = 0);
	XMLWrite(FILE *file, const wchar_t *doctype, const wchar_t *dtddoctype = NULL, int no_hi_chars_conversion = 0);
	~XMLWrite();

	void Init(FILE *file, const wchar_t *doctype, const wchar_t *dtddoctype);

	void comment(const wchar_t *comment);

	void pushCategory(const wchar_t *title, int wantcr = 1, int wantindent = 1);

	void pushCategoryAttrib(const wchar_t *title, int nodata = FALSE);
	void writeCategoryAttrib(const wchar_t *title, const int val);
	void writeCategoryAttrib(const wchar_t *title, const wchar_t *val);
	void closeCategoryAttrib(int wantcr = 1);

	void writeAttribEmpty(const wchar_t *title, int wantcr = 1, int wantindent = 1);
	void writeAttrib(const wchar_t *title, const wchar_t *text, int wantcr = 1, int wantindent = 1);
	void writeAttrib(const wchar_t *title, int val, int wantcr = 1, int wantindent = 1);
	int popCategory(int wantcr = 1, int wantindent = 1);	// returns nonzero if more categories are open

	static int efprintf(FILE *fp, const wchar_t *format, ...);
	static int utf8fprintf(FILE *fp, const wchar_t *format, ...);
	static int eutf8fprintf(FILE *fp, const wchar_t *format, ...);

private:
	FILE *fp;
	StringW indenter;
//	int indent;
	Stack<wchar_t *>titles;
	int nohicharconversion;
};



#endif
