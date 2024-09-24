#ifndef _commonmacros_H_
#define _commonmacros_H_

#ifndef NDEBUG
#define DIAG(x) OutputDebugString(x);
#else
#define DIAG(x)
#endif
 
/**
 * The __TODO__ macro provides a way to notate source code in a way that the 
 * compiler will recognize and print out the file and line number in the Build
 * window. This should be used like so:
 *
 * #pragma message(__TODO__"Message here")
 */
#define __TODO_STR2__(str) #str
#define __TODO_STR__(str) __TODO_STR2__(str)
#define __TODO__ __FILE__ "(" __TODO_STR__(__LINE__)") : TODO: "

/* nocopy macros makes an uncopyable object use as follows
	class foobar
	{
		nocopy(foobar)

	public:
		// other methods here
	};
*/
#define nocopy(c) private: c(const c &/*v*/) { ASSERT(0); } c& operator=(const c &/*v*/) { ASSERT(0); return *this;}

// various forget macros that delete objects and zero them out safely.
#define forget(x) { if (x) { delete x; x = 0; } }
#define forgetArray(x) { if (x) { delete [] x; x = 0; } }
#define safeRelease(x) { if (x) { x->Release(); x = 0; } }
#define forgetHandleNULL(x) { if (x) { ::CloseHandle(x); x = NULL; } }

#ifdef WIN32
#define forgetHandleInvalid(x) { if (x != INVALID_HANDLE_VALUE) ::CloseHandle(x); x = INVALID_HANDLE_VALUE; }
#define forgetGDIObject(x) { if (x) ::DeleteObject(x); x = NULL; }
#define forgetGDIIcon(x) { if (x) ::DestroyIcon(x); x = NULL; }

#ifdef HRESULT
// I do this so often, I turned it into a macro
struct bad_hresult
{
	HRESULT err;
	bad_hresult(HRESULT herr): err(herr){}
};
#endif

#define checkCOMReturn(x) { HRESULT err = x; if (FAILED(err)) return err; }
#define checkCOMThrow(x)  { HRESULT err = x; if (FAILED(err)) throw bad_hresult(err); }
#endif

// so I don't have to include nsvlib.h all the time
#ifndef NSV_MAKETYPE
#define NSV_MAKETYPE(A,B,C,D) ((A) | ((B)<<8) | ((C)<<16) | ((D)<<24))
#endif

#endif
