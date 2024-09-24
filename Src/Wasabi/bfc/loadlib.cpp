#include "precomp_wasabi_bfc.h"

#include "loadlib.h"

#if !defined(WIN32) && !defined(LINUX)
#error port me
#endif

Library::Library(const wchar_t *filename) : NamedW(filename)
{
	lib = NULL;
}

Library::~Library()
{
	unload();
}

int Library::load(const wchar_t *newname)
{
	if (lib != NULL && newname == NULL) 
		return 1;
	unload();
	if (newname != NULL) 
		setName(newname);

	const wchar_t *n = getName();
	ASSERT(n != NULL);
#ifdef WIN32
	__try {
	    lib = LoadLibraryW(n);
	} __except(EXCEPTION_EXECUTE_HANDLER)
	{
		// stupid DLL
		lib = NULL;
		OutputDebugString(L"exception while loading dll");
		OutputDebugStringW(newname);
		OutputDebugString(L"\n");
	}
#elif defined(LINUX)
	// Not using string to try to not use common/wasabi in Studio.exe
	char *conv = _strdup( getName() );
	int len = strlen( conv );
	if ( ! strcasecmp( conv + len - 4, ".dll" ) )
	{
		strcpy( conv + len - 4, ".so" );
	}

	lib = dlopen(conv, RTLD_LAZY);

	free( conv );
#else
#error port me!
#endif
	if (lib == NULL) return 0;
	return 1;
}

void Library::unload()
{
	if (lib != NULL)
	{
#ifdef WIN32
		FreeLibrary(lib);
#elif defined(LINUX)
		dlclose(lib);
#else
#error port me!
#endif

	}
	lib = NULL;
}

void *Library::getProcAddress(const char *procname)
{
	ASSERT(procname != NULL);
#if defined(WIN32)
	return GetProcAddress(lib, procname);
#elif defined(LINUX)
	return dlsym(lib, procname);
#else
#error port me!
#endif
}
