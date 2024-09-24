#ifndef NULLSOFT_DRIVEINFO_HEADER
#define NULLSOFT_DRIVEINFO_HEADER

#include "../nu/Map.h"
#include ".\discinfo.h"


typedef struct
{
	wchar_t		letter;
	int			typeCode;	
	wchar_t*	modelInfo; 
	int			busType;
	int			*pTypeList;		// all supported types
	int			nTypeList;		// number of supported tpyes
	DiscInfo	*disc;			// inserted disc info;
} OPTICAL_DRIVE;

#define MAX_FORMAT_DRIVE_STRING 512

class Drives
{
public:
	Drives(void);
	~Drives(void);

public:
	void AddDrive(wchar_t letter, unsigned int typeCode, wchar_t* description, const wchar_t *extInfo);
	void Clear(void);
	unsigned int GetCount(void);
	const OPTICAL_DRIVE* GetFirst(void);
	const OPTICAL_DRIVE* GetNext(void); // if returns NULL - means no more
	
	static BOOL IsRecorder(const OPTICAL_DRIVE *drive);

	static const wchar_t* GetTypeString(int typeCode);
	static const wchar_t* GetBusString(int busCode);
	static const wchar_t* GetFormatedString(const OPTICAL_DRIVE *drv, wchar_t *buffer, size_t size, BOOL useFullName = TRUE);
private:
	Map<wchar_t, OPTICAL_DRIVE> driveList;
	Map<wchar_t, OPTICAL_DRIVE>::const_iterator c_iter;
};

#endif  //NULLSOFT_DRIVEINFO_HEADER