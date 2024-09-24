#ifndef NULLSOFT_UTILITY_CONFIGH
#define NULLSOFT_UTILITY_CONFIGH
#include <string>
#include <map>
#include <windows.h>

typedef std::wstring tstring;

namespace Nullsoft
{
	namespace Utility
	{

		class ConfigItemBase
		{
		public:
			ConfigItemBase(const wchar_t *_appName, const wchar_t * _fileName, LPCTSTR _keyName)
				: appName(nullptr), fileName(nullptr), keyName(nullptr)
			{
				appName = _appName;
				fileName = _fileName;
				keyName = _keyName;
			}

			~ConfigItemBase()
			{
				
			}

			const wchar_t *appName;
			const wchar_t *fileName;
			const wchar_t *keyName;
		};

		template <class config_t>
		class ConfigItem : public ConfigItemBase
		{
		public:
			ConfigItem(const wchar_t *_appName, const wchar_t * _fileName, LPCTSTR _keyName) : ConfigItemBase(_appName, _fileName, _keyName)
			{
			}

			~ConfigItem() {}

			void operator =(config_t input)
			{
				WritePrivateProfileStruct(appName,
					keyName,
					(void *) & input,
					sizeof(input),
					fileName);
			}

			operator config_t()
			{
				config_t temp;

				memset(&temp, 0, sizeof(temp));
				GetPrivateProfileStruct(appName,
					keyName,
					&temp,
					sizeof(temp),
					fileName);
				return temp;
			}
		};


		template <>
		class ConfigItem<TCHAR *> : public ConfigItemBase
		{
		public:
			ConfigItem(const wchar_t *_appName, const wchar_t * _fileName, LPCTSTR _keyName) : ConfigItemBase(_appName, _fileName, _keyName)
			{
			}

			~ConfigItem(){}

			void operator =(LPCTSTR input)
			{
				WritePrivateProfileString(appName,
					keyName,
					input,
					fileName);
			}
			void GetString(LPTSTR str, size_t len)
			{
				GetPrivateProfileString(appName, keyName, TEXT(""), str, len, fileName);
			}
		};

		template <>
		class ConfigItem<int> : public ConfigItemBase
		{
		public:
			ConfigItem(const wchar_t *_appName, const wchar_t * _fileName, LPCTSTR _keyName) : ConfigItemBase(_appName, _fileName, _keyName), def(0)
			{
			}

			~ConfigItem() {}

			void operator =(int input)
			{
				TCHAR tmp[(sizeof(int) / 2) * 5 + 1]; // enough room to hold for 16,32 or 64 bit ints, plus null terminator
				wsprintf(tmp, TEXT("%d"), input);
				WritePrivateProfileString(appName,
					keyName,
					tmp,
					fileName);
			}

			operator int ()
			{
				return GetPrivateProfileInt(appName, keyName, def, fileName);
			}

			void SetDefault(int _def)
			{
				def = _def;
			}
			int def;
		};

		class Config
		{
		public:
			Config() : appName(nullptr),fileName(nullptr)
			{
			}

			~Config()
			{
				if (appName != nullptr)
				{
					free(appName);
					appName = nullptr;
				}
				if (fileName != nullptr)
				{
					free(fileName);
					fileName = nullptr;
				}
			}

			void SetFile(LPCTSTR iniFile, LPCTSTR _appName)
			{
				if (appName != nullptr)
				{
					free(appName);
					appName = nullptr;
				}
				if (fileName != nullptr)
				{
					free(fileName);
					fileName = nullptr;
				}
				appName = _wcsdup(_appName);
				fileName = _wcsdup(iniFile);
			}

			ConfigItem<int> cfg_int(LPCTSTR keyName, int def)
			{
				ConfigItem<int> item(appName, fileName, keyName);
				item.SetDefault(def);
				return item;
			}

			ConfigItem<TCHAR *> cfg_str(LPCTSTR keyName)
			{
				return ConfigItem<TCHAR *>(appName, fileName, keyName);
			}

			ConfigItem<GUID> cfg_guid(LPCTSTR keyName)
			{
				return ConfigItem<GUID>(appName, fileName, keyName);

			}
			ConfigItem<__int64> cfg_int64(LPCTSTR keyName)
			{
				ConfigItem<__int64> item(appName, fileName, keyName);
				return item;
			}

			wchar_t *appName, *fileName;
		};
	}
}
#endif
