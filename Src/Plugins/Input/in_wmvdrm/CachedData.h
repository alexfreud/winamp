#ifndef CACHEDDATAH
#define CACHEDDATAH

template <class DataType>
class CachedData
{
public:
	CachedData() : cached(false)
	{
	}
	CachedData(DataType _data) : cached(true), data(_data)
	{
	}
	operator DataType()
	{
		return data;
	}

	bool IsCached()
	{
		return cached;
	}
	void ClearCache()
	{
		cached=false;
	}
	DataType *operator &()
	{
		if (cached)
			return &data;
		else
			return 0;
	}

	template <class FromType>
	bool operator =(FromType from)
	{
		data = from;
		cached=true;
		return cached;
	}

private:
	bool cached;
	DataType data;
};

#endif