#ifndef NULLSOFT_AUTOCHARH
#define NULLSOFT_AUTOCHARH

class AutoChar
{
public:
	AutoChar(const wchar_t *convert) : allocated(false), narrow(0)
	{
		// review maybe CP_UTF8?
		
		int size = WideCharToMultiByte(CP_ACP, 0, convert, -1, 0, 0, NULL, NULL);
		if (!size)
			return;

		narrow = new char[size];
		allocated=true;

		if (!WideCharToMultiByte(CP_ACP, 0, convert, -1, narrow, size, NULL, NULL))
		{
			delete [] narrow;
			narrow=0;
			allocated=false;
		}
	}
	~AutoChar()
	{
		if (allocated)
		{
			delete [] narrow;
			narrow=0;
			allocated=false;
		}
	}
	operator char *()
	{
		return narrow;
	}
private:
	bool allocated;
	char *narrow;
};

#endif