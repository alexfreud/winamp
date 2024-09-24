#ifndef NULLSOFT_DISCINFO_HEADER
#define NULLSOFT_DISCINFO_HEADER

#include <windows.h>


// disc data array size (see decalrations in discInfo.cpp)
#define DISC_DATA_COUNT			0x0009

#define TEXT_BUFFER_SIZE 64



class DiscInfo
{
public:
	DiscInfo(void);
	DiscInfo(const wchar_t *info);
	~DiscInfo(void);

public:
	
	BOOL SetStringInfo(const wchar_t *info);
	const wchar_t* GetStringInfo(void);

	DWORD GetMedium(void);
	DWORD GetMediumType(void);
	DWORD GetMediumFormat(void);
	BOOL  GetProtectedDVD(void);
	BOOL  GetErasable(void);
	DWORD GetTracksNumber(void);
	DWORD GetSectorsUsed(void);
	DWORD GetSectorsFree(void);
	void SetSerialNumber(int serialNumber);
	int GetSerialNumber(void);

	BOOL GetRecordable(void);

	const wchar_t* GetMediumText(void);
	const wchar_t* GetMediumTypeText(void);
	const wchar_t* GetMediumFormatText(void);
	const wchar_t* GetProtectedDVDText(void);
	const wchar_t* GetErasableText(void);
	const wchar_t* GetTracksNumberText(void);
	const wchar_t* GetSectorsUsedText(void);
	const wchar_t* GetSectorsFreeText(void);

	const wchar_t* GetRecordableText(void);

protected:
	void ResetData(void);

private:
	wchar_t *strData;
	int	 serialNum;
	wchar_t buffer[TEXT_BUFFER_SIZE];
	DWORD data[DISC_DATA_COUNT];

};

#endif // NULLSOFT_DISCINFO_HEADER