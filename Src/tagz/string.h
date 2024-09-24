#ifndef NULLSOFT_TAGZ_STRINGH
#define NULLSOFT_TAGZ_STRINGH

#include <windows.h>

namespace tagz_
{
	class string
	{
	private:
		LPTSTR data;
		size_t size,used;
	public:
		string();
		void AddDBChar(LPTSTR c);
		void AddChar(TCHAR c);
		void AddInt(int i);
		void AddString(LPCTSTR z);
		void AddString(string &s);
		~string();
		LPTSTR GetBuf();
		TCHAR operator[](size_t i);
		size_t Len();
		void Reset();
		LPCTSTR Peek();
	};
}
#endif