#ifndef NULLSOFT_UTILITY_ALIASH
#define NULLSOFT_UTILITY_ALIASH

template <class ptr_t>
class Alias
{
public:
	Alias()
		:pointer(0)
	{
	}
	Alias(ptr_t *_pointer) : pointer(_pointer)
	{
	}
	Alias(const Alias<ptr_t> &copy)
	{
		pointer=copy.pointer;
	}
	~Alias()
	{
		pointer=0;
	}
	ptr_t *operator = (ptr_t *copy)
	{
		pointer=copy;
		return copy;
	}

	ptr_t *operator ->()
	{
		return pointer;
	}

	operator bool()
	{
		return !!pointer;
	}

	bool operator == (ptr_t *compare)
	{
		return pointer == compare;
	}
/*
	operator ptr_t *()
	{
		return pointer;
	}
	*/

private:
	ptr_t *pointer;
};

#endif