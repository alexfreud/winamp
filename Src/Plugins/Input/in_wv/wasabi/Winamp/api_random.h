#ifndef NULLSOFT_API_RANDOM_H
#define NULLSOFT_API_RANDOM_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

typedef int (*RandomGenerator)(void);
typedef unsigned long (*UnsignedRandomGenerator)(void);

class api_random : public Dispatchable
{
protected:
	api_random() {}
	~api_random() {}
public:
	RandomGenerator  GetFunction();
	UnsignedRandomGenerator GetUnsignedFunction();
	int GetNumber();
	int GetPositiveNumber();
	float GetFloat(); // [0-1]
	float GetFloat_LessThanOne(); // [0-1)
	float GetFloat_LessThanOne_NotZero(); // (0-1)
	double GetDouble(); // [0-1)
public:
	DISPATCH_CODES
	{
	    API_RANDOM_GETFUNCTION = 10,
				API_RANDOM_GETFUNCTION_UNSIGNED = 11,
	    API_RANDOM_GETNUMBER = 20,
	    API_RANDOM_GETPOSITIVENUMBER = 30,
	    API_RANDOM_GETFLOAT = 40,
	    API_RANDOM_GETFLOAT2 = 41,
	    API_RANDOM_GETFLOAT3 = 42,
	    API_RANDOM_GETDOUBLE = 50,
	};
};

inline RandomGenerator api_random::GetFunction()
{
	return _call(API_RANDOM_GETFUNCTION, (RandomGenerator )0);
}
inline UnsignedRandomGenerator api_random::GetUnsignedFunction()
{
	return _call(API_RANDOM_GETFUNCTION_UNSIGNED, (UnsignedRandomGenerator )0);
}

inline int api_random::GetNumber()
{
	return _call(API_RANDOM_GETNUMBER, 0);
}
inline int api_random::GetPositiveNumber()
{
	return _call(API_RANDOM_GETPOSITIVENUMBER, 0);
}
inline float api_random::GetFloat()
{
	return _call(API_RANDOM_GETFLOAT, 0.f);
}
inline float api_random::GetFloat_LessThanOne()
{
	return _call(API_RANDOM_GETFLOAT2, 0.f);
}
inline float api_random::GetFloat_LessThanOne_NotZero()
{
	return _call(API_RANDOM_GETFLOAT3, 0.f);
}
inline double api_random::GetDouble()
{
	return _call(API_RANDOM_GETDOUBLE, 0.);
}

// {CB401CAB-CC10-48f7-ADB7-9D1D24B40E0C}
static const GUID randomApiGUID = 
{ 0xcb401cab, 0xcc10, 0x48f7, { 0xad, 0xb7, 0x9d, 0x1d, 0x24, 0xb4, 0xe, 0xc } };


#endif

