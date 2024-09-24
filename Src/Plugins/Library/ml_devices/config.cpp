#include "main.h"
#include "./config.h"

#include <strsafe.h>

#define CONFIG_SUFFIX		L"Plugins\\ml"
#define CONFIG_FILE			L"ml_devices.ini"

static const char *
Config_GetPath(BOOL ensureExist)
{
	static const char *configPath = NULL;
	if (NULL == configPath)
	{
		const wchar_t *userPath;
		wchar_t buffer[MAX_PATH * 2] = {0};

		if (NULL == WASABI_API_APP)
			return NULL;

		userPath = WASABI_API_APP->path_getUserSettingsPath();
		if (NULL == userPath)
			return NULL;
		
		if (0 != PathCombine(buffer, userPath, CONFIG_SUFFIX))
		{
			if ((FALSE == ensureExist || SUCCEEDED(Plugin_EnsurePathExist(buffer))) &&
				FALSE != PathAppend(buffer,CONFIG_FILE))
			{
				configPath = String_ToAnsi(CP_UTF8, 0, buffer, -1, NULL, NULL);
			}
		}
	}

	return configPath;
}


unsigned long 
Config_ReadString(const char *section,  const char *key,  const char *defaultValue, char *returnedString, unsigned long size)
{
	return GetPrivateProfileStringA(section, key, defaultValue, returnedString, size, Config_GetPath(FALSE));
}

unsigned int
Config_ReadInt(const char *section, const char *key, int defaultValue)
{
	return GetPrivateProfileIntA(section, key, defaultValue, Config_GetPath(FALSE));
}

BOOL
Config_ReadBool(const char *section, const char *key, BOOL defaultValue)
{
	char buffer[32] = {0};
	int length = Config_ReadString(section, key, NULL, buffer, ARRAYSIZE(buffer));
	
	if (0 == length) 
		return defaultValue;
	
	if (1 == length)
	{
		switch(*buffer)
		{
			case '0':
			case 'n':
			case 'f':
				return FALSE;
			case '1':
			case 'y':
			case 't':
				return TRUE;
		}
	}
	else
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "yes", -1, buffer, length) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "true", -1, buffer, length))
		{
			return TRUE;
		}

		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "no", -1, buffer, length) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "false", -1, buffer, length))
		{
			return FALSE;
		}
	}

	if (FALSE != StrToIntExA(buffer, STIF_SUPPORT_HEX,  &length))
		return (0 != length);

	return defaultValue;
}

BOOL
Config_WriteString(const char *section, const char *key, const char *value)
{
	const char *configPath = Config_GetPath(TRUE);
	if (NULL == configPath || '\0' == *configPath) 
		return FALSE;
		
	return (0 != WritePrivateProfileStringA(section, key, value, configPath));
}

BOOL
Config_WriteInt(const char *section, const char *key, int value)
{
	char buffer[32] = {0};
	if (FAILED(StringCchPrintfA(buffer, ARRAYSIZE(buffer), "%d", value)))
		return FALSE;

	return Config_WriteString(section, key, buffer);
}

BOOL
Config_WriteBool(const char *section, const char *key, BOOL value)
{
	return Config_WriteString(section, key, (FALSE != value) ? "yes" : "no"); 
}
