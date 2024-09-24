#pragma once

class SafeSize
{
public:
	SafeSize()
	{
		value = 0;
		overflow = false;
	}

	void Add(size_t add)
	{
		if (!overflow)
		{
			value += add;
			if (value < add)
				overflow=true;
		}
	}

	void AddN(size_t size, size_t count)
	{
		if (!overflow)
		{
			size_t total = size * count;
			if (total < size)
			{
				overflow = true;
			}
			else
			{
				Add(total);
			}
		}
	}

	bool Overflowed() const
	{
		return overflow;
	}

	operator size_t ()
	{
		return value;
	}

private:
	size_t value;
	bool overflow;
};