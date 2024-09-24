#include "../nde_c.h"
#include "../DBUtils.h"

#include <atomic>

extern "C" void NDE_HeapInit();
extern "C" void NDE_HeapQuit();
static volatile std::atomic<std::size_t> _init_count = 0;

/* NDE_Init isn't thread safe, be aware
best to call on the main thread during initialization
*/
void NDE_Init()
{
	if ( _init_count.load() == 0 )
	{
		NDE_HeapInit();
		HMODULE klib = LoadLibraryW( L"Kernel32.dll" );
		if ( klib )
		{
			void *nls = GetProcAddress( klib, "FindNLSString" );
			if ( nls )
				*( (void **)&findNLSString ) = nls;
		}

		FreeModule( klib );
	}

	_init_count.fetch_add( 1 );
}

void NDE_Quit()
{
	if ( _init_count.fetch_sub( 1 ) == 0 )
		NDE_HeapQuit();
}