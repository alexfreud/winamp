#include "precomp_wasabi_bfc.h"
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <unistd.h>
#endif

#include "bfc_assert.h"

#ifdef ASSERTS_ENABLED

static int in_assert = 0;

// we try to use wsprintf because it can make for a smaller .exe
#ifdef WIN32
#define ASSERT_SPRINTF wsprintfA
#elif defined(LINUX)
#define ASSERT_SPRINTF sprintf
#elif defined(__APPLE__)
#define ASSERT_SPRINTF sprintf
#else
#error port me!
#endif

#ifdef WIN32
static int isDebuggerPresent() {
	HINSTANCE kernel32 = LoadLibrary(L"kernel32.dll");
	if (kernel32 == NULL) return 0;
	BOOL (WINAPI *IsDebuggerPresent)() = NULL;
	IsDebuggerPresent = (BOOL (WINAPI *)())GetProcAddress(kernel32, "IsDebuggerPresent");
	if (IsDebuggerPresent == NULL) return 0;
	int ret = (*IsDebuggerPresent)();
	FreeLibrary(kernel32);
	return ret;
}
#endif

static void assert_kill()
{
#ifdef _WIN32
#ifndef _WIN64
	__asm { int 3 };
#else
	ExitProcess(0);
#endif
#endif
}
void _assert_handler(const char *reason, const char *file, int line) {
	char msg[4096] = {0};
	ASSERT_SPRINTF(msg, "Expression: %s\nFile: %s\nLine: %d\n", reason, file, line);

#ifdef WIN32
	OutputDebugStringA(msg);
	if (in_assert) 
	{
		assert_kill();
	}
	in_assert = 1;

	ATOM a = AddAtom(L"BYPASS_DEACTIVATE_MGR"); // so we don't need to call api->appdeactivation_setbypass

	if (isDebuggerPresent() || MessageBoxA(NULL, msg, "Assertion failed", MB_OKCANCEL|MB_TASKMODAL) == IDCANCEL) {
		assert_kill();
	} else
		ExitProcess(0);
	DeleteAtom(a);
#elif defined(LINUX)
	kill(getpid(), SIGSEGV );
#elif defined(__APPLE__)
	kill(getpid(), SIGSEGV );
#else
#error port me!
#endif
	in_assert = 0;
}

void _assert_handler_str(const char *string, const char *reason, const char *file, int line) 
{
	char msg[4096] = {0};
	ASSERT_SPRINTF(msg, "\"%s\"\nExpression: %s\nFile: %s\nLine: %d\n", string, reason ? reason : "", file, line);

#ifdef WIN32
	OutputDebugStringA(msg);
	if (in_assert) assert_kill();
	in_assert = 1;

	ATOM a = AddAtom(L"BYPASS_DEACTIVATE_MGR");
	if (isDebuggerPresent() || MessageBoxA(NULL, msg, "Assertion failed", MB_OKCANCEL|MB_TASKMODAL) == IDCANCEL) {
		assert_kill();
	} else
		ExitProcess(0);
	DeleteAtom(a);
#elif defined(LINUX)
	kill(getpid(), SIGSEGV );
#elif defined(__APPLE__)
	kill(getpid(), SIGSEGV );
#else
#error port me!
#endif
	in_assert = 0;
}

#endif
