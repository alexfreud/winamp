#ifndef NULLSOFT_FILETYPESH
#define NULLSOFT_FILETYPESH

#include <vector>
#include "../nu/AutoLock.h"
#include "AutoChar.h"

class FileType
{
public:
	enum
	{
		AUDIO = 0,
		VIDEO = 1,
	};
	FileType() : wtype(0), type(0), description(0), isProtocol(false), avType(AUDIO)
	{
	}

	FileType(wchar_t *_type, wchar_t *_description, bool _protocol, int _avType)
	{
		wtype=_wcsdup(_type);
		type = _wcsdup(_type);
		description = _wcsdup(_description);
		isProtocol = _protocol;
		avType = _avType;
	}
	~FileType()
	{
		free(type);
		free(wtype);
		free(description);
	}
	FileType(const FileType &copy) :wtype(0), type(0), description(0)
	{
		operator =(copy);
	}
	void operator =(const FileType &copy)
	{
		
		if (copy.wtype)
			wtype=_wcsdup(copy.wtype);
		if (copy.type)
			type = _wcsdup(copy.type);
		if (copy.description)
			description = _wcsdup(copy.description);
		isProtocol = copy.isProtocol;
		avType = copy.avType;
	}
	wchar_t *type;
	wchar_t *wtype;
	wchar_t *description;
	bool isProtocol;
	int avType; // audio or video
};

class FileTypes
{
public:
	FileTypes()
		: typesString(0),
		typeGuard(GUARDNAME("FileTypes::typeGuard"))
	{}
	~FileTypes()
	{
		free(typesString);
		typesString=0;
	}
	int GetAVType(const wchar_t *ext);
	bool IsSupportedURL(const wchar_t *fn);
	bool IsDefault();
	void LoadDefaults();
	void ReadConfig();
	void SaveConfig();
	void CheckVideo();

	typedef std::vector<FileType> TypeList;
	void SetTypes(TypeList &newTypes);
	TypeList types;
	Nullsoft::Utility::LockGuard typeGuard;
private:
	char *typesString;
	void ResetTypes();
	void AddType(const char *type, const char *description);
};

extern FileTypes fileTypes;
#endif