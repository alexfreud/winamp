#ifndef NULLSOFT_WINAMP_RANDOM_H
#define NULLSOFT_WINAMP_RANDOM_H

#include "api_random.h"
#include <windows.h> // for DWORD


class Random : public api_random
{
public:
	static const char *getServiceName() { return "Random Number API"; }
	static const GUID getServiceGuid() { return randomApiGUID; }	
public:
	Random();
	RandomGenerator GetFunction();
	UnsignedRandomGenerator GetUnsignedFunction();
	int GetNumber();
	int GetPositiveNumber();
	float GetFloat(); // [0-1]
	float GetFloat_LessThanOne(); // [0-1)
	float GetFloat_LessThanOne_NotZero(); // (0-1)
	double GetDouble(); // [0-1)
protected:
	RECVS_DISPATCH;

	

};

#endif