#ifndef AUTOWIDEH
#define AUTOWIDEH

class AutoWide
{
public:
	AutoWide(const char *convert) : allocated(false), wide(0)
	{
		// review maybe CP_UTF8?
		int size = MultiByteToWideChar(CP_ACP, 0, convert, -1, 0,0);
		if (!size)
			return;

		wide = new unsigned short[size];
		allocated=true;
		if (!MultiByteToWideChar(CP_ACP, 0, convert, -1, wide,size))
		{
			delete wide;
			wide=0;
			allocated=false;
		}
	}
	~AutoWide()
	{
		if (allocated)
		{
			delete wide;
			wide=0;
			allocated=false;
		}
	}
	operator unsigned short *()
	{
		return wide;
	}
private:
	bool allocated;
	unsigned short *wide;
};

#endif