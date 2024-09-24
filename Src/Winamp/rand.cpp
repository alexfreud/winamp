/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "random.h"
#include <wincrypt.h>
/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti = N + 1; /* mti==N+1 means mt[N] is not initialized */

static CRITICAL_SECTION randCS;
/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
	mt[0] = s & 0xffffffffUL;
	for (mti = 1; mti < N; mti++)
	{
		mt[mti] =
		    (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
		mt[mti] &= 0xffffffffUL;
	}
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

static const unsigned long mag01[2] = {0x0UL, MATRIX_A};
/* mag01[x] = x * MATRIX_A  for x=0,1 */

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
	EnterCriticalSection(&randCS);
	unsigned long y;

	if (mti >= N)
	{ /* generate N words at one time */
		int kk;
		// benski> CUT - because we call init_genrand() as the first thing in WinMain
		//if (mti == N+1)   /* if init_genrand() has not been called, */
		// benski> CUT init_genrand(5489UL); /* a default initial seed is used */

		for (kk = 0;kk < N - M;kk++)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk < N - 1;kk++)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	LeaveCriticalSection(&randCS);

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);
	
	return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
	return (long)(genrand_int32() >> 1);
}

static HCRYPTPROV GetKeySet()
{
	HCRYPTPROV   hCryptProv;
	LPCSTR UserName = "WinampKeyContainer";  // name of the key container

	if(CryptAcquireContextA(&hCryptProv, UserName, NULL, PROV_RSA_FULL, 0))
	{
		return hCryptProv;
	} 
	else if(CryptAcquireContextA(&hCryptProv, UserName, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) 
	{
		return hCryptProv;
	}
	else
		return 0;
}

Random::Random()
{
	InitializeCriticalSectionAndSpinCount(&randCS, 1000);

	HCRYPTPROV hCryptProv = GetKeySet();
	if (hCryptProv)
	{
		unsigned long seed_data[8];
		if (CryptGenRandom(hCryptProv, sizeof(seed_data), (PBYTE)seed_data)) 
		{
			init_by_array(seed_data, 8);
		}
		CryptReleaseContext(hCryptProv,0);
	}
	else
	{
		LARGE_INTEGER performance_counter;
		QueryPerformanceCounter(&performance_counter);

		unsigned long seed_data[4] = {(unsigned long)performance_counter.HighPart, (unsigned long)performance_counter.LowPart, (unsigned long)GetTickCount64(), (unsigned long)GetCurrentProcessId()};
		init_by_array(seed_data, 4);
	}
}

int warand()
{
	return genrand_int31();
}

float warandf()
{
	return genrand_int32()*(1.0f / 4294967295.0f);
}


RandomGenerator Random::GetFunction()
{
	return warand;
}

UnsignedRandomGenerator Random::GetUnsignedFunction()
{
	return genrand_int32;
}

int Random::GetNumber()
{
	return genrand_int32();
}

int Random::GetPositiveNumber()
{
	return genrand_int31();
}

/* generates a random number on [0,1]-real-interval */
float Random::GetFloat()
{
	return genrand_int32()*(1.0f / 4294967295.0f);
	/* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
float Random::GetFloat_LessThanOne()
{
	return genrand_int32()*(1.0f / 4294967296.0f);
	/* divided by 2^32 */
}
/* generates a random number on (0,1)-real-interval */
float Random::GetFloat_LessThanOne_NotZero()
{
	return (((float)genrand_int32()) + 0.5f)*(1.0f / 4294967296.0f);
} // (0-1)

/* generates a random number on [0,1) with 53-bit resolution*/
double Random::GetDouble()
{
	unsigned long a = genrand_int32() >> 5, b = genrand_int32() >> 6;
	return (a*67108864.0 + b)*(1.0 / 9007199254740992.0);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Random
START_DISPATCH;
CB(API_RANDOM_GETFUNCTION, GetFunction)
CB(API_RANDOM_GETFUNCTION_UNSIGNED, GetUnsignedFunction)
CB(API_RANDOM_GETNUMBER, GetNumber)
CB(API_RANDOM_GETPOSITIVENUMBER, GetPositiveNumber)
CB(API_RANDOM_GETFLOAT, GetFloat)
CB(API_RANDOM_GETFLOAT2, GetFloat_LessThanOne)
CB(API_RANDOM_GETFLOAT3, GetFloat_LessThanOne_NotZero)
CB(API_RANDOM_GETDOUBLE, GetDouble)
END_DISPATCH