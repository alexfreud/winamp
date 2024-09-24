#ifndef NULLOSFT_MLSTRING_HEADER
#define NULLOSFT_MLSTRING_HEADER

#include <windows.h>

class MLString
{
public:
	MLString(void);
    ~MLString(void);
	MLString(const wchar_t* string);
	MLString(unsigned int cchBuffer);
protected:
	MLString(const MLString &copy);
   

public:
	HRESULT Set(const wchar_t* string, unsigned int cchLength);
	HRESULT Append(const wchar_t* string, unsigned int cchLength);
    const wchar_t* Get(void) { return (cchLen) ? buffer : NULL; }
	void Clear(void) { cchLen = 0; }
	unsigned int GetLength(void) { return cchLen; }
	HRESULT Format(const wchar_t *format, ...);
	HRESULT CopyTo(MLString *destination);

	HRESULT Set(const wchar_t* string) { return Set(string, lstrlenW(string)); }
	HRESULT Append(const wchar_t* string) { return Append(string, lstrlenW(string)); }

	// buffer
	HRESULT Allocate(unsigned int cchNewSize);
	void Compact(void);
	wchar_t* GetBuffer(void) { return buffer; }
	unsigned int GetBufferLength(void) { return allocated; }
	void UpdateBuffer(void) { cchLen = lstrlenW(buffer); }

	operator const wchar_t *() { return buffer; }
	operator wchar_t *() { return buffer; }
	wchar_t& operator [](unsigned int index) { return buffer[index]; }
	MLString& operator = (const wchar_t *source) { (source) ? Set(source, lstrlenW(source)) : Clear(); return *this;}
	MLString& operator + (const wchar_t *source) { if (source) Append(source, lstrlenW(source)); return *this;}
	MLString& operator += (const wchar_t *source) { if (source) Append(source, lstrlenW(source)); return *this;}
	MLString& operator = (MLString *source) { (source) ? source->CopyTo(this) : Clear(); return *this;}
	MLString& operator + (MLString *source) { if (source) Append(source->GetBuffer(), source->GetLength()); return *this;}
	MLString& operator += (MLString *source) { if (source) Append(source->GetBuffer(), source->GetLength()); return *this;}

protected:
	wchar_t			*buffer;
	unsigned int	cchLen;
	unsigned int	allocated;

	static void			*heap;
};

#endif // NULLOSFT_STRING_HEADER