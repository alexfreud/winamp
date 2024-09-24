#include "main.h"
#include "FileTypes.h"
#include "Config.h"
#include "../nu/Config.h"
#include "resource.h"
#include <strsafe.h>

FileTypes fileTypes;
extern  Nullsoft::Utility::Config wmConfig;

bool FileTypes::IsSupportedURL(const wchar_t *fn)
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	TypeList::iterator itr;
	for (itr=types.begin();itr!=types.end();itr++)
	{
		if (itr->isProtocol && 	!_wcsnicmp(fn, itr->type, wcslen(itr->type)))
			return true;
	}
	return false;
}

bool FileTypes::IsDefault()
{
	return true;
}

void FileTypes::LoadDefaults()
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	types.clear();
	types.push_back(FileType(L"WMA", WASABI_API_LNGSTRINGW(IDS_WMA_AUDIO_FILE), false, FileType::AUDIO));
	if (!config_no_video)
	{
		types.push_back(FileType(L"WMV", WASABI_API_LNGSTRINGW(IDS_WMA_VIDEO_FILE), false, FileType::VIDEO));
		types.push_back(FileType(L"ASF", WASABI_API_LNGSTRINGW(IDS_ASF_STREAM), false, FileType::VIDEO));
	}
	types.push_back(FileType(L"MMS://", WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_STREAM), true, FileType::VIDEO));
	types.push_back(FileType(L"MMSU://", WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_STREAM), true, FileType::VIDEO));
	types.push_back(FileType(L"MMST://", WASABI_API_LNGSTRINGW(IDS_WINDOWS_MEDIA_STREAM), true, FileType::VIDEO));
}

void FileTypes::ReadConfig()
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	int numTypes = wmConfig.cfg_int(L"numtypes", -1);
	if (numTypes!=-1)
	{
		for (size_t i=0;i!=numTypes;i++)
		{
			wchar_t type[1024] = {0}, description[1024] = {0}, temp[64] = {0};

			StringCchPrintf(temp, 64, L"type%u", i);
			wmConfig.cfg_str(temp).GetString(type, 1024);
			StringCchPrintf(temp, 64, L"description%u", i);
			wmConfig.cfg_str(temp).GetString(description, 1024);
			StringCchPrintf(temp, 64, L"protocol%u", i);
			bool protocol = !!wmConfig.cfg_int(temp, 0);
			StringCchPrintf(temp, 64, L"avtype%u", i);
			int avtype = !!wmConfig.cfg_int(temp, 0);
			if (!(config_no_video && !protocol && avtype==FileType::VIDEO)) // if we havn't explicity disabled video support
				types.push_back(FileType(type, description, protocol, avtype));
		}
	}
	else
		fileTypes.LoadDefaults();

	ResetTypes();
	numTypes=types.size();
	for (size_t i=0;i!=numTypes;i++)
	{
		if (!types[i].isProtocol)
		AddType(AutoChar(types[i].type), AutoChar(types[i].description));
	}

	CheckVideo();
}

void FileTypes::SetTypes(TypeList &newTypes)
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	types=newTypes;
	ResetTypes();
	int numTypes=types.size();
	for (size_t i=0;i!=numTypes;i++)
	{
		if (!types[i].isProtocol)
			AddType(AutoChar(types[i].type), AutoChar(types[i].description));
	}

	CheckVideo();
}
void FileTypes::SaveConfig()
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	wmConfig.cfg_int(L"numtypes", -1) = types.size();
	for (size_t i=0;i!=types.size();i++)
	{
		wchar_t temp[64] = {0};

		StringCchPrintf(temp, 64, L"type%u", i);
		wmConfig.cfg_str(temp) = types[i].wtype;
		StringCchPrintf(temp, 64, L"description%u", i);
		wmConfig.cfg_str(temp) = types[i].description;
		StringCchPrintf(temp, 64, L"protocol%u", i);
		wmConfig.cfg_int(temp, 0) = types[i].isProtocol;
		StringCchPrintf(temp, 64, L"avtype%u", i);
		wmConfig.cfg_int(temp, 0) = types[i].avType;
	}		
}

void FileTypes::CheckVideo()
{
	bool videoPresent=false;
	Nullsoft::Utility::AutoLock lock(typeGuard);
	//wmConfig.cfg_int(L"numtypes", -1) = types.size();
	for (size_t i=0;i!=types.size();i++)
		if (types[i].avType == FileType::VIDEO)
			videoPresent=true;

	config_no_video = !videoPresent;
}

void FileTypes::ResetTypes()
{
	free(typesString);
	typesString=0;
}

void FileTypes::AddType(const char *extension, const char *description)
{
	size_t oldSize=0, size=0;

	if (typesString)
	{
		char *temp=typesString;
		while (temp && *temp++)
		{
			size++;
			if (*temp == 0)
			{
				size++;
				temp++;
			}
		} ;
		oldSize=size;
	}
	size += lstrlenA(extension)+1;
	size += lstrlenA(description)+1;

	char *newTypes = (char *)calloc(size+1, sizeof(char));
	if (newTypes)
	{
		memcpy(newTypes, typesString, oldSize);
		free(typesString);
		typesString=newTypes;
		newTypes += oldSize;
		size-=oldSize;
		StringCchCopyA(newTypes, size, extension);
		int extSize = lstrlenA(extension)+1;
		newTypes+=extSize;
		size-=extSize;
		StringCchCopyA(newTypes, size, description);
		newTypes+=lstrlenA(description)+1;
		*newTypes=0;
		plugin.FileExtensions = typesString;
	}
}

int FileTypes::GetAVType(const wchar_t *ext)
{
	Nullsoft::Utility::AutoLock lock(typeGuard);
	for (size_t i=0;i!=types.size();i++)
	{
		if (!types[i].isProtocol && !lstrcmpi(types[i].type, ext))
			return types[i].avType;
	}
	return -1;
}