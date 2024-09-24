#ifndef _XLATSTR_H
#define _XLATSTR_H

#include <api/locales/localesmgr.h>

/**
  Provides string translation for the string
  used as the constructor parameter.
  
  The constructor will automatically lookup
  the translated value of the string it receives
  in the currently loaded locale.

  @short Translates a string using the currently loaded locale.
  @author Nullsoft
  @ver 1.0
  @see ComponentAPI::locales_getTranslation()
*/

class _ {
  public:
  /**
	Automatically looks up the translated value of the string
	it receives as a parameter in the currently loaded
	locale. The same string is returned if there's no
	translation.

	@param str String to be translated.
	@ret Translation found, Translated string; Translation not found, Input string;
  */

#if defined(WASABI_COMPILE_LOCALES)
		_(const wchar_t *str) { s=LocalesManager::getTranslation(str); }
#else
	_(const wchar_t *str) { s=str; }
#endif
	operator const wchar_t *() const { return s; }

  private:
	const wchar_t *s;
};


class __ {
  public:
  /**
	Automatically looks up the translated value of the string
	it receives as a parameter in the currently loaded
	locale. The same string is returned if there's no
	translation.

	@param str String to be translated.
	@ret Translation found, Translated string; Translation not found, Input string;
  */

#if defined(WASABI_COMPILE_LOCALES)
		__(const wchar_t *str) { s=LocalesManager::lookupString(str); }
#else
	__(const wchar_t *str) { s=str; }
#endif
	operator const wchar_t *() const { return s; }

  private:
	const wchar_t *s;
};


#endif