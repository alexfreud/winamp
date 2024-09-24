#include <precomp.h>
#include "blending.h"
#include <bfc/std.h>

#if !defined(WIN32) && !defined(LINUX)
#error port me!
#endif

class BlenderInit
{
public:
	BlenderInit() { Blenders::init(); }
};
static BlenderInit blender_init;

void Blenders::init()
{
	if (!alphatable[127][127])
	{
		int i, j;
		for (j = 0;j < 256;j++)
			for (i = 0;i < 256;i++)
				alphatable[i][j] = (i * (j + 1)) >> 8;
#ifndef NO_MMX
		DWORD retval1, retval2;
#ifdef WIN32
		__try {
		    _asm {
		        mov eax, 1  // set up CPUID to return processor version and features
		        // 0 = vendor string, 1 = version info, 2 = cache info
		        _emit 0x0f  // code bytes = 0fh,  0a2h
		        _emit 0xa2
		        mov retval1, eax
		        mov retval2, edx
		    }
	} __except(EXCEPTION_EXECUTE_HANDLER) { retval1 = retval2 = 0;}
#else
		__asm__ volatile ( "movl $1, %%eax\n"
						   ".byte 15, 162\n"
						   "movl %%eax, %0\n"
						   "movl %%edx, %1\n"
					   : "=m" (retval1), "=m" (retval2)
								   :   // No inputs...
								   : "%eax", "%edx" );
#endif
		mmx_available = retval1 && (retval2 & 0x800000);
#endif	//ndef NO_MMX

	}
}

#ifndef NO_MMX
int Blenders::mmx_available = 0;
#endif

unsigned char Blenders::alphatable[256][256];
