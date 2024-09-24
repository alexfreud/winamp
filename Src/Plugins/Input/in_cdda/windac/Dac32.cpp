// ----------------------------------------------
// -       DAC32.DLL Implementations Datei      -
// - Written 1996-1998 by Christoph Schmelnik   -
// ----------------------------------------------

// Version 1.33 : 18.01.1998
// Changes:
// Added speed selection support for all current Plextor drives
//
// Version 1.40 : 24.02.1998
// Changes:
// Set correct direction flags, to work with NT device IO interface and ATAPI drives
// Changed main CD detection to TestUnitReady
// Removed CD detection from Audio Status Info
// Added hopefully correct read command for Matsushita/Panasonic drives 
// Added Parameters to CDAC class to allow the disabling of the audio test and to spin up the drive for a specified time
// in seconds to avoid spin up problems on some drives. Both parameters have default values so it behaves like the old version
// without the additional parameters
// Added Parameter to the constructor of CWaveSave to disable writing any Headers. Also this parameter has a default to work like
// before.
// Added virtual function in CDAC to report buffer underruns in Burst Copy Mode
// For the last feature an immediate parameter in WaitCDDA is added
// GetLastSense function added to return the sense information for the last read audio command
// Configuration in CMapDrive extended by new features
// Added function to CD Class to read Media Cataloge Number
//
// Version 1.41 : 02.05.1998
// Changes:
// New GetInfoEx() function in CMapDrive, to allow a better result checking.
// Bugfixed error handling in CWaveSave and CDAC regarding write errors
//
// Version 1.42 : 02.08.1998
// Changes:
// Added GetLastReadableAddress function to get the last readable Sektor of a session.
// Added a flag in the drive properties for the function.
// Added this function to the CDAC object.
//
// Version 1.43 : 23.12.1998
// Changes:
// Added Wave and DAC classes are now available in a MT version, the old versions will not longer be updated.
//
// Version 1.44 : 10.03.1999
// Changes:
// Added Support for current Plextor CDROM drives and CD-Writers.
// Added Support for Jukeboxes
// Changed Handling of the Ringbuffer
//
// Version 1.45 : 15.08.1999
// Changes:
// Added Enhanced error detection for Plextor drives.
// Several Bugfixes (initialising under NT and Ringbuffer specific)
//
// Version 1.45-Build 11 : 11.11.1999
// Changes:
// Added a check for the MaxSektor parameter in CBaseWaveMT to avoid Program failures even if the applications provides an invalid value.
// Changed source to comile with Borland compiler
// Added MMC-2 Type which will be default for Sony CD-Writers 140 and 928
// Skip Virtual CD devices in Bus scan
// Fixed Array out of bound bug in drive detection.

//limit the read to 64k, because of a bug in the Adaptec drivers
#define Bug64

//limit the number of retries at error to 5
#define MAXTRIES 5
#include "dac32.h"
#include <stdlib.h>
#include <assert.h>
#include <process.h>

#ifdef __BORLANDC__	// pgo (to make it compatible with Bormand compiler)
#define	_stricmp	_stricmp
#define	_strlwr	strlwr
#endif

// ----------------------------------------------------------------------------------------
// - Implementation of the private copy operators                                         -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Check the syntax of program                                                 -
// ----------------------------------------------------------------------------------------

CBaseCD& CBaseCD::operator = (const CBaseCD &other)
{
	assert(!&other);
	return (*this);
}

CSCSICD& CSCSICD::operator = (const CSCSICD &other)
{
	assert(!&other);
	return (*this);
}

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CCDAdress                                     -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Eliminate the errors at compile time                                        -
// ----------------------------------------------------------------------------------------
void CCDAdress::SetRedbook(long Value)
{
	Adresse=Value >> 24;
	Adresse+=((Value >> 16) & 255)*75;
	Adresse+=((Value >> 8) & 255)*4500;
};

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CMapInfo                                      -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Implement the information of the TypeMappings                               -
// ----------------------------------------------------------------------------------------
CMapInfo::CMapInfo()
{
	strncpy(TypNamen[0],"TOSHIBA ", 9);
	strncpy(TypNamen[1],"SONY    ", 9);
	strncpy(TypNamen[2],"NEC     ", 9);
	strncpy(TypNamen[3],"HITACHI ", 9);
	strncpy(TypNamen[4],"YAMAHA  ", 9);
	strncpy(TypNamen[5],"PIONEER ", 9);
	strncpy(TypNamen[6],"IBM     ", 9);
	strncpy(TypNamen[7],"PLEXTOR ", 9);
	strncpy(TypNamen[8],"PHILIPS ", 9);
	strncpy(TypNamen[9],"GRUNDIG ", 9);
	strncpy(TypNamen[10],"HP      ", 9);
	strncpy(TypNamen[11],"IMS     ", 9);
	strncpy(TypNamen[12],"MITSUMI ", 9);
	strncpy(TypNamen[13],"ATAPI   ", 9);
	strncpy(TypNamen[14],"TOSHNEW ", 9);
	strncpy(TypNamen[15],"RICOH   ", 9);
	strncpy(TypNamen[16],"MATSHITA", 9);
	strncpy(TypNamen[17],"PLASMON ", 9);
	strncpy(TypNamen[18],"KODAK   ", 9);
	strncpy(TypNamen[19],"TEAC    ", 9);
	strncpy(TypNamen[20],"CyberDrv", 9);
	strncpy(TypNamen[21],"MMC-2   ", 9);	// pgo
	int const t[]={CDTYPE_TOSHIBA,CDTYPE_SONY,CDTYPE_NEC,CDTYPE_SONY,CDTYPE_SONY,CDTYPE_SONY,
		CDTYPE_SONY,CDTYPE_PLEXTOR,CDTYPE_PHILIPS,CDTYPE_PHILIPS,CDTYPE_PHILIPS,CDTYPE_PHILIPS,
		CDTYPE_PHILIPS,CDTYPE_ATAPI,CDTYPE_TOSHNEW,CDTYPE_RICOH,CDTYPE_MATSHITA,CDTYPE_PHILIPS,
		CDTYPE_PHILIPS,CDTYPE_SONY,CDTYPE_CYBERDRV,
		CDTYPE_CYBERDRV};	// pgo
	for (int i=0; i<MaxMappings; i++)
		TypMapping[i]=t[i];
}

char *CMapInfo::GetTypName(int Index)
{
	return TypNamen[Index];
};

int CMapInfo::GetTypMapping(int Index)
{
	return TypMapping[Index];
};

int CMapInfo::GetTypMappingRev(int CDType)
{
	int Index=0;
	while ((Index<MaxMappings) &&
		   (TypMapping[Index]!=CDType))
		Index++;
	return Index;
};

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CMapDrive                                     -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Administration of the drive configuration                                   -
// ----------------------------------------------------------------------------------------
CMapDrive::CMapDrive(BOOL bDoReset)
{
	First=0;
	
	m_hDriveEvent=INVALID_HANDLE_VALUE;
	wchar_t szEventName[32] = {0};
	wsprintf(szEventName,L"%X",this);
	m_hDriveEvent=CreateEvent(NULL,TRUE,FALSE,szEventName);
	if (bDoReset)
		Reset();
};

CMapDrive::~CMapDrive()
{
	DeleteAll();
	if (m_hDriveEvent!=INVALID_HANDLE_VALUE)
		CloseHandle(m_hDriveEvent);
};

void CMapDrive::DeleteAll()
{
	TDriveInfoMem *Akt;
	Akt=First;
	while (First)
	{
		Akt=First;
		First=Akt->Next;
		delete Akt;
	}
};

void CMapDrive::Reset()
{
	TDriveInfoMem *Akt,*Last;
	DeleteAll();
	BYTE SCSIType;
	TDriveInfo Info = {0};
	int DType = DTYPE_UNKNOWN;
	char Revision[5] = {0};
	char ManagerID[17] = {0};
	char HAID[17] = {0};

	THAUnique HAUnique;
	MEMORYSTATUS MemStat;
	MemStat.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus((LPMEMORYSTATUS)&MemStat);
	CMapInfo MapInfo;
	int HostNum;
	for (HostNum=0; HostNum<NumberOfHostAdapters; HostNum++)
	{
		memset(Revision,0,sizeof(Revision));
		memset(ManagerID,0,sizeof(ManagerID));
		memset(HAID,0,sizeof(HAID));
		memset(&HAUnique,0,sizeof(HAUnique));
		if (HAInquiry(HostNum,ManagerID,HAID,HAUnique))
		{
			if (_stricmp(HAID, "fastcdmp") == 0) //pgo: Skip Virtual CD Adapter in Bus Scan
			   continue;
			HostAdapterMemory[HostNum]=HAUnique.MaximumTransferLen;
			for (int IDNum=0; IDNum<8; IDNum++)
				for (int LUNNum=0; LUNNum<=MAXLUN; LUNNum++)
				{
					DType=GetDeviceInfo(HostNum,IDNum,LUNNum,SCSIType,Info.VendorID,Info.ProductID,Revision,m_hDriveEvent);
					if ((DType==DTYPE_CROM)||
						(DType==DTYPE_WORM))
					{
						if (SCSIType)
						{
							int i;
							for (i=0; (i<MaxMappings) && _stricmp(Info.VendorID,MapInfo.GetTypName(i)); i++);
							if (i>=MaxMappings) i=1;  //pgo: avoid array out of bound
							Info.Type=i;
						}
						else
							Info.Type=MapInfo.GetTypMappingRev(CDTYPE_ATAPI);

						if (MapInfo.GetTypMapping(Info.Type)==CDTYPE_TOSHIBA)
						{
							char szNumbers[17] = {0};
							for (size_t i=0; i<strlen(Info.ProductID); i++)
								if (isdigit(Info.ProductID[i]))
									strncat(szNumbers,&Info.ProductID[i],1);
							int nProductID=atoi(szNumbers);
							if (((nProductID>3800) &&
								 (nProductID<4000)) ||
								((nProductID>5700) &&
								 (nProductID<10000)))
								Info.Type=MapInfo.GetTypMappingRev(CDTYPE_TOSHNEW);
						}
						// pgo
						else if (MapInfo.GetTypMapping(Info.Type)==CDTYPE_SONY)
						{
							if (SCSIType)
							{
								char szNumbers[17] = {0};
								for (size_t i=0; i<strlen(Info.ProductID); i++)
									if (isdigit(Info.ProductID[i]))
										strncat(szNumbers,&Info.ProductID[i],1);
								int nProductID=atoi(szNumbers);
								if ((nProductID == 140) || (nProductID == 928))
									Info.Type=MapInfo.GetTypMappingRev(CDTYPE_CYBERDRV);
							}
						}
						Info.ID=IDNum;
						Info.LUN=LUNNum;
						Info.HostAdapterNumber=HostNum;
						Info.Mode=ModeBurst;
						Info.MaxSektors=MemStat.dwTotalPhys/16/2352;
						if (Info.MaxSektors>(HostAdapterMemory[HostNum]/2352))
							Info.MaxSektors=HostAdapterMemory[HostNum]/2352;
#ifdef Bug64
						if (Info.MaxSektors>27) Info.MaxSektors=27;
#else
						if (Info.MaxSektors>446) Info.MaxSektors=446;
#endif
						if (!Info.MaxSektors) Info.MaxSektors=26;

						Info.SynchSektors=3;
						Info.Speed=0;
						Info.PerformDATest=DATEST_FIRSTTRACK;
						Info.SpinUpMode=SPINUP_NEVER;
						Info.dwSpinUpTime=5;
						Info.bUseLastReadableAddress=FALSE;
						Info.bUseC2ErrorInfo=FALSE;
						Info.bSpinDown=FALSE;
						Akt=new TDriveInfoMem;
						Akt->Info=Info;
						Akt->Next=0;
						if (!First) First=Akt;
						else Last->Next=Akt;
						//Last must be set to Akt
						Last=Akt;
					}
				}
		}
	}
	if (RunningNT)
	{
		//check all drives for direct access
		int Index, Length;
		wchar_t DriveList[128] = {0}, *pDrives = 0;

		Length=GetLogicalDriveStrings(128,DriveList);
		if (Length)
		{
			pDrives=DriveList;
			Index=0;
			while (pDrives && *pDrives && (Index<Length))
			{
				if (GetDriveType(pDrives)==DRIVE_CDROM)
				{
					wchar_t CDDevice[10]=L"\\\\.\\x:";
					HANDLE hDrive;
					CharLower(pDrives);
					//_strlwr(pDrives);
					CDDevice[4]=pDrives[0];
					int IDNum=pDrives[0]-'a';
					
					//For Windows NT 5 use other file flags
					OSVERSIONINFO osver;
					memset( &osver, 0x00, sizeof(osver) );
					osver.dwOSVersionInfoSize = sizeof(osver);
					GetVersionEx( &osver );

					DWORD dwOpenFlags = GENERIC_READ;
					if ( (osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion > 4) ) dwOpenFlags |= GENERIC_WRITE;
	
					hDrive=CreateFile(CDDevice,dwOpenFlags,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
					if (hDrive!=INVALID_HANDLE_VALUE)
					{
						//CT> added correct host/id/lun guessing
						extern int getSCSIIDFromDrive(char driveletter, int *host, int *id, int *lun);
						int m_lun;
						getSCSIIDFromDrive((char)pDrives[0],&HostNum,&IDNum,&m_lun);

						DType=GetDeviceInfo(HostNum,IDNum,m_lun,SCSIType,Info.VendorID,Info.ProductID,Revision,hDrive);
						if ((DType==DTYPE_CROM)||
							(DType==DTYPE_WORM))
						{
							SCSIMaxBlocks(hDrive,&HostAdapterMemory[HostNum]);
							if (SCSIType)
							{
								int i;
								for (i=0; (i<MaxMappings) && _stricmp(Info.VendorID,MapInfo.GetTypName(i)); i++);
								if (i>=MaxMappings) i=1;  //pgo: avoid array out of bound
								Info.Type=i;
							}
							else
								Info.Type=MapInfo.GetTypMappingRev(CDTYPE_ATAPI);
							if (MapInfo.GetTypMapping(Info.Type)==CDTYPE_TOSHIBA)
							{
								char szNumbers[17];
								szNumbers[0]=0;
								for (size_t i=0; i<strlen(Info.ProductID); i++)
									if (isdigit(Info.ProductID[i]))
										strncat(szNumbers,&Info.ProductID[i],1);
								int nProductID=atoi(szNumbers);
								if (((nProductID>3800) &&
									 (nProductID<4000)) ||
									((nProductID>5700) &&
									 (nProductID<10000)))
									Info.Type=MapInfo.GetTypMappingRev(CDTYPE_TOSHNEW);
							}
							// pgo
							else if (MapInfo.GetTypMapping(Info.Type)==CDTYPE_SONY)
							{
								if (SCSIType)
								{
									char szNumbers[17] = {0};
									for (size_t i=0; i<strlen(Info.ProductID); i++)
										if (isdigit(Info.ProductID[i]))
											strncat(szNumbers,&Info.ProductID[i],1);
									int nProductID=atoi(szNumbers);
									if ((nProductID == 140) || (nProductID == 928))
										Info.Type=MapInfo.GetTypMappingRev(CDTYPE_CYBERDRV);
								}
							}
							Info.ID=IDNum;
							Info.LUN=0;
							Info.HostAdapterNumber=HostNum;
							Info.Mode=ModeNormal;
							Info.MaxSektors=MemStat.dwTotalPhys/16/2352;
							if (Info.MaxSektors>(HostAdapterMemory[HostNum]/2352))
								Info.MaxSektors=HostAdapterMemory[HostNum]/2352;
#ifdef Bug64
							if (Info.MaxSektors>27) Info.MaxSektors=27;
#else
							if (Info.MaxSektors>446) Info.MaxSektors=446;
#endif
							if (!Info.MaxSektors ||
								(MapInfo.GetTypMapping(Info.Type)==CDTYPE_ATAPI))
								Info.MaxSektors=26;
							Info.SynchSektors=3;
							Info.Speed=0;
							Info.PerformDATest=DATEST_FIRSTTRACK;
							Info.SpinUpMode=SPINUP_NEVER;
							Info.dwSpinUpTime=5;
							Info.bUseLastReadableAddress=FALSE;
							Info.bUseC2ErrorInfo=FALSE;
							Info.bSpinDown=FALSE;
							Akt=new TDriveInfoMem;
							Akt->Info=Info;
							Akt->Next=0;
							if (!First) First=Akt;
							else Last->Next=Akt;
							//Last must be set to Akt
							Last=Akt;
							HostNum++;
						}
						CloseHandle(hDrive);
					}
				}
				pDrives+=4;
				Index+=4;
			}
		}
	}
};

int CMapDrive::GetMaxDrives()
{
	int i=0;
	TDriveInfoMem *Akt;
	Akt=First;
	while (Akt)
	{
		i++;
		Akt=Akt->Next;
	}
	return i;
};

TDriveInfo &CMapDrive::GetInfo(int index)
{
	int i=0;
	TDriveInfoMem *Akt;
	Akt=First;
	while ((Akt) && (i<index))
	{
		i++;
		Akt=Akt->Next;
	}
	return Akt->Info;
};

BOOL CMapDrive::GetInfoEx(int index, TDriveInfo *&pInfo)
{
	int i=0;
	TDriveInfoMem *Akt;
	Akt=First;
	while ((Akt) && (i<index))
	{
		i++;
		Akt=Akt->Next;
	}
	if (!Akt)
		return FALSE;
	pInfo=&Akt->Info;
	return TRUE;
};

void CMapDrive::DeleteInfo(int index)
{
	int i=0;
	TDriveInfoMem *Akt,*Prev;
	Akt=First;
	Prev=0;
	while ((Akt) && (i<index))
	{
		i++;
		Prev=Akt;
		Akt=Akt->Next;
	}
	if (!Akt) return;
	if (Prev) Prev->Next=Akt->Next;
	else First=Akt->Next;
	delete Akt;
};

int CMapDrive::GetMaxHostAdapters()
{
	return NumberOfHostAdapters;
};

int CMapDrive::GetSupportedHostAdapterMemory(int index)
{
	if ((index<NumberOfHostAdapters) &&
		(index>=0))
		return HostAdapterMemory[index];
	else
		return -1;
};

void CMapDrive::SetSupportedHostAdapterMemory(int index,int Memory)
{
	if ((index<NumberOfHostAdapters) &&
		(index>=0))
		HostAdapterMemory[index]=Memory;
}

int CMapDrive::GetMaxSektors(int HostAdapterNumber)
{
#ifdef Bug64
	int Result=HostAdapterMemory[HostAdapterNumber]/2352;
	if (Result>27)
		Result=27;
	return Result;
#else
	return HostAdapterMemory[HostAdapterNumber]/2352;
#endif
};

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CMapInfoJuke                                  -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Implement the information of the TypeMappings                               -
// ----------------------------------------------------------------------------------------
CMapInfoJuke::CMapInfoJuke()
{
	strncpy(TypNamen[0],"SONY    ", 9);
	strncpy(TypNamen[1],"PIONEER ", 9);
	int const t[]={JUKETYPE_SONY,JUKETYPE_PIONEER};
	for (int i=0; i<MaxMappingsJuke; i++)
		TypMapping[i]=t[i];
}

char *CMapInfoJuke::GetTypName(int Index)
{
	return TypNamen[Index];
};

int CMapInfoJuke::GetTypMapping(int Index)
{
	return TypMapping[Index];
};

int CMapInfoJuke::GetTypMappingRev(int JukeType)
{
	int Index=0;
	while ((Index<MaxMappingsJuke) &&
		   (TypMapping[Index]!=JukeType))
		Index++;
	return Index;
};

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CMapJuke                                      -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Administration of the jukebox configuration                                 -
// ----------------------------------------------------------------------------------------
CMapJuke::CMapJuke(BOOL bDoReset)
{
	First=0;
	
	m_hJukeEvent=INVALID_HANDLE_VALUE;
	wchar_t szEventName[32] = {0};
	wsprintf(szEventName,L"%X",this);
	m_hJukeEvent=CreateEvent(NULL,TRUE,FALSE,szEventName);
	if (bDoReset)
		Reset();
};

CMapJuke::~CMapJuke()
{
	DeleteAll();
	if (m_hJukeEvent!=INVALID_HANDLE_VALUE)
		CloseHandle(m_hJukeEvent);
};

void CMapJuke::DeleteAll()
{
	TJukeInfoMem *Akt;
	Akt=First;
	while (First)
	{
		Akt=First;
		First=Akt->Next;
		if (Akt->Info.pConnectedDrives)
			delete Akt->Info.pConnectedDrives;
		delete Akt;
	}
};

void CMapJuke::Reset()
{
	TJukeInfoMem *Akt,*Last;
	DeleteAll();
	BYTE SCSIType = 0;
	TJukeInfo Info = {0};
	int DType = DTYPE_UNKNOWN;
	char Revision[5] = {0};
	char ManagerID[17] = {0};
	char HAID[17] = {0};

	THAUnique HAUnique;
	CMapInfoJuke MapInfo;
	for (int HostNum=0; HostNum<NumberOfHostAdapters; HostNum++)
	{
		memset(Revision,0,sizeof(Revision));
		memset(ManagerID,0,sizeof(ManagerID));
		memset(HAID,0,sizeof(HAID));
		memset(&HAUnique,0,sizeof(HAUnique));
		if (HAInquiry(HostNum,ManagerID,HAID,HAUnique))
		{
			for (int IDNum=0; IDNum<8; IDNum++)
			{
				for (int LUNNum=0; LUNNum<=MAXLUN; LUNNum++)
				{
					memset(&Info,0,sizeof(Info));
					DType=GetDeviceInfo(HostNum,IDNum,LUNNum,SCSIType,Info.VendorID,Info.ProductID,Revision,m_hJukeEvent);
					if (DType==DTYPE_JUKE)
					{
						int i;
						for (i=0; (i<MaxMappingsJuke) && _stricmp(Info.VendorID,MapInfo.GetTypName(i)); i++);
						if (_stricmp(Info.VendorID,MapInfo.GetTypName(i))) i=JUKETYPE_SONY;
						Info.Type=i;
						switch (Info.Type)
						{
							case JUKETYPE_PIONEER :
								Info.MaxDrives=4; //currently only data for the 500x changer implemeneted
								Info.MaxDiscs=500;
								break;
							case JUKETYPE_SONY :
							default:
								Info.MaxDrives=2;
								Info.MaxDiscs=100;
								break;
						}
						Info.pConnectedDrives=new int[Info.MaxDrives];
						for (int nDriveNum=0; nDriveNum<Info.MaxDrives; nDriveNum++)
							Info.pConnectedDrives[nDriveNum]=-1;
						Info.ID=IDNum;
						Info.LUN=LUNNum;
						Info.HostAdapterNumber=HostNum;
						
						Akt=new TJukeInfoMem;
						Akt->Info=Info;
						Akt->bIsWorking=FALSE;
						Akt->Next=0;
						if (!First) First=Akt;
						else Last->Next=Akt;
						//Last must be set to Akt
						Last=Akt;
					}
				}
			}
		}
	}
};

int CMapJuke::GetMaxJukes()
{
	int i=0;
	TJukeInfoMem *Akt;
	Akt=First;
	while (Akt)
	{
		i++;
		Akt=Akt->Next;
	}
	return i;
};

TJukeInfo &CMapJuke::GetInfo(int index)
{
	int i=0;
	TJukeInfoMem *Akt;
	Akt=First;
	while ((Akt) && (i<index))
	{
		i++;
		Akt=Akt->Next;
	}
	return Akt->Info;
};

BOOL CMapJuke::IsWorking(int index)
{
	int i=0;
	TJukeInfoMem *Akt;
	Akt=First;
	while ((Akt) && (i<index))
	{
		i++;
		Akt=Akt->Next;
	}
	return Akt->bIsWorking;
};

void CMapJuke::SetWorking(int index,BOOL bIsWorking)
{
	int i=0;
	TJukeInfoMem *Akt;
	Akt=First;
	while ((Akt) && (i<index))
	{
		i++;
		Akt=Akt->Next;
	}
	Akt->bIsWorking=bIsWorking;
};

void CMapJuke::DeleteInfo(int index)
{
	int i=0;
	TJukeInfoMem *Akt,*Prev;
	Akt=First;
	Prev=0;
	while ((Akt) && (i<index))
	{
		i++;
		Prev=Akt;
		Akt=Akt->Next;
	}
	if (!Akt) return;
	if (Prev) Prev->Next=Akt->Next;
	else First=Akt->Next;
	if (Akt->Info.pConnectedDrives)
		delete Akt->Info.pConnectedDrives;
	delete Akt;
};

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CJukeBox                                      -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Control the basic JukeBox functions over ASPI								  -
// ----------------------------------------------------------------------------------------
CJukeBox::CJukeBox (TJukeInfo &xInfo):Config(xInfo)
{
	m_hJukeEvent=INVALID_HANDLE_VALUE;
	wchar_t szEventName[32] = {0};
	wsprintf(szEventName,L"%X",this);
	m_hJukeEvent=CreateEvent(NULL,TRUE,FALSE,szEventName);
	assert(m_hJukeEvent);
}

CJukeBox::~CJukeBox()
{
	if (m_hJukeEvent!=INVALID_HANDLE_VALUE)
		CloseHandle(m_hJukeEvent);
}

BOOL CJukeBox::MoveMedium(int Source,int Destination)
{
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case JUKETYPE_PIONEER :
			if ((Source<1) || (Source>Config.MaxDiscs))
				if ((Source<0x4000) || (Source>=(0x4000+Config.MaxDrives)))
					return FALSE;
			if ((Destination<1) || (Destination>Config.MaxDiscs))
				if ((Destination<0x4000) || (Destination>=(0x4000+Config.MaxDrives)))
					return FALSE;
			break;
		case JUKETYPE_SONY :
		default:
			if ((Source<1) || (Source>Config.MaxDiscs))
			{
				if ((Source<0x4000) || (Source>=(0x4000+Config.MaxDrives)))
					return FALSE;
				else
					Source-=0x3fff;
			}
			else
				Source+=10;
			if ((Destination<1) || (Destination>Config.MaxDiscs))
			{
				if ((Destination<0x4000) || (Destination>=(0x4000+Config.MaxDrives)))
					return FALSE;
				else
					Destination-=0x3fff;
			}
			else
				Destination+=10;
			break;
	}

	while (!TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hJukeEvent));
	TOpcode OpC;
	OpC[0]=0xa5;
	OpC[1]=Config.LUN>>5;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=Source/256;
	OpC[5]=Source%256;
	OpC[6]=Destination/256;
	OpC[7]=Destination%256;
	OpC[8]=0x00;
	OpC[9]=0x00;
	OpC[10]=0x00;
	OpC[11]=0x00;
	return ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,12,NULL,0,m_hJukeEvent);
}

// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CBaseCD                                       -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: The drive independent functions to access the CD-ROM drives                 -
// ----------------------------------------------------------------------------------------
int CBaseCD::Lasterror()
{
	int h;
	h=Error;
	Error=CDOK;
	return h;
}

int CBaseCD::ReadFirstTrackInfo(TTrackList &Infos)
{
	if (!FirstTrack) return 0;
	AktTrack=FirstTrack;
	Infos=AktTrack->Info;
	return 1;
}

int CBaseCD::ReadNextTrackInfo(TTrackList &Infos)
{
	if ((!FirstTrack) ||
	    (!AktTrack->Next)) return 0;
	AktTrack=AktTrack->Next;
	Infos=AktTrack->Info;
	return 1;
}

int CBaseCD::ReadPrevTrackInfo(TTrackList &Infos)
{
	if ((!FirstTrack) ||
	    (!AktTrack->Prev)) return 0;
	AktTrack=AktTrack->Prev;
	Infos=AktTrack->Info;
	return 1;
}

int CBaseCD::ReadTrackInfo(TTrackList &Infos)
{
	if ((!FirstTrack) ||
		(Infos.TrackNummer<1)) return 0;
	if (!AktTrack)
		AktTrack=FirstTrack;
	if (AktTrack->Info.TrackNummer==Infos.TrackNummer)
	{
		Infos=AktTrack->Info;
		return 1;
	}
	while ((AktTrack->Info.TrackNummer>Infos.TrackNummer) &&
		   (AktTrack->Prev))
		AktTrack=AktTrack->Prev;
	while ((AktTrack->Info.TrackNummer<Infos.TrackNummer) &&
	       (AktTrack->Next))
		AktTrack=AktTrack->Next;
	if (AktTrack->Info.TrackNummer!=Infos.TrackNummer) return 0;
	Infos=AktTrack->Info;
	return 1;
}

int CBaseCD::ReadMaxTracks()
{
	TTrackListeMem *Laeufer;
	if (!FirstTrack) return 0;
	Laeufer=AktTrack;
	while (Laeufer->Next)
		Laeufer=Laeufer->Next;
	return Laeufer->Info.TrackNummer;
}

void CBaseCD::DeleteTrackList()
{
	while (FirstTrack)
	{
		AktTrack=FirstTrack->Next;
		delete FirstTrack;
		FirstTrack=AktTrack;
	}
}


// ----------------------------------------------------------------------------------------
// - Implementation of the class members of CSCSICD                                       -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purpose: Control the basic CDROM functions over ASPI								  -
// ----------------------------------------------------------------------------------------
CSCSICD::CSCSICD (char drive, TDriveInfo &xInfo):Config(xInfo)
{
	m_bSpeedTableInitialized=FALSE;
	FirstTrack=0;
	NECRotationSpeed = 0;	// pgo
	Changed=FALSE;
	CDPresentLast=TRUE;
	Error=CDOK;
	m_hDriveEvent=INVALID_HANDLE_VALUE;
	memset(&m_SenseInfo,0,sizeof(TSenseInfo));
	if ((Config.HostAdapterNumber>=NumberOfHostAdapters) &&
		RunningNT)
	{
		DWORD dwFlags;
		OSVERSIONINFO	osver;
		wchar_t CDDevice[10]=L"\\\\.\\x:";
		CDDevice[4]=(wchar_t)drive;//(Config.HostAdapterNumber + 'A'/* + 1*/);
		//For Windows NT 5 use other file flags
		memset( &osver, 0x00, sizeof(osver) );
		osver.dwOSVersionInfoSize = sizeof(osver);
		GetVersionEx( &osver );

		// if Win2K or greater, add GENERIC_WRITE
		dwFlags = GENERIC_READ;
		if ( (osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion > 4) ) dwFlags |= GENERIC_WRITE;
		   
		m_hDriveEvent = CreateFile( CDDevice, dwFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	}
	else
	{
		wchar_t szEventName[32] = {0};
		wsprintf(szEventName,L"%X",this);
		m_hDriveEvent=CreateEvent(NULL,TRUE,FALSE,szEventName);
		assert(m_hDriveEvent);
	}
	TDriveStatus DInfo=Get_DriveStatus();
	if (DInfo.CDPresent) ReRead();
}

CSCSICD::~CSCSICD()
{
	DeleteTrackList();
	if (m_hDriveEvent!=INVALID_HANDLE_VALUE)
		CloseHandle(m_hDriveEvent);
}

void CSCSICD::PrepareCDDA()
{
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
			{
				memset(&ModeData,0,sizeof(ModeData));
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,0,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				ModeData[0]=0;
				TDriveMode ModeSelectData;
				ModeSelectData[0]=0x00;
				ModeSelectData[1]=0x00;
				ModeSelectData[2]=0x00;
				ModeSelectData[3]=0x08;
				ModeSelectData[4]=0x82;
				ModeSelectData[5]=0x00;
				ModeSelectData[6]=0x00;
				ModeSelectData[7]=0x00;
				ModeSelectData[8]=0x00;
				ModeSelectData[9]=0x00;
				ModeSelectData[10]=0x09;
				ModeSelectData[11]=0x30;
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSelectData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_TOSHNEW :
			{
				memset(&ModeData,0,sizeof(ModeData));
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,15,0x20,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				ModeData[0]=0;
				TDriveMode ModeSelectData;
				ModeSelectData[0]=0x00;
				ModeSelectData[1]=0x00;
				ModeSelectData[2]=0x00;
				ModeSelectData[3]=0x08;
				ModeSelectData[4]=0x82;
				ModeSelectData[5]=0x00;
				ModeSelectData[6]=0x00;
				ModeSelectData[7]=0x00;
				ModeSelectData[8]=0x00;
				ModeSelectData[9]=0x00;
				ModeSelectData[10]=0x09;
				ModeSelectData[11]=0x30;

				ModeSelectData[12]=0x20;
				ModeSelectData[13]=0x01;
				ModeSelectData[14]=(ModeData[14] & 0xcf)|0x10;
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSelectData,15,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_NEC :
			{
				memset(&ModeData,0,sizeof(ModeData));
				if (!addModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				NECRotationSpeed=ModeData[6] & 0x20;
				ModeData[6]=ModeData[6]|0x20;
				if (!addModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_PHILIPS :
		case CDTYPE_MATSHITA :
			{
				memset(&ModeData,0,sizeof(ModeData));
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,0,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				ModeData[0]=0;
				TDriveMode ModeSelectData;
				ModeSelectData[0]=0x00;
				ModeSelectData[1]=0x00;
				ModeSelectData[2]=0x00;
				ModeSelectData[3]=0x08;
				ModeSelectData[4]=0x00;
				ModeSelectData[5]=0x00;
				ModeSelectData[6]=0x00;
				ModeSelectData[7]=0x00;
				ModeSelectData[8]=0x00;
				ModeSelectData[9]=0x00;
				ModeSelectData[10]=0x09;
				ModeSelectData[11]=0x30;
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSelectData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
	}
}

void CSCSICD::ReadCDDA(CCDAdress StartSektor,long Sektoranzahl,void *Buffer,BOOL bUseC2ErrorInfo)
{
	TOpcode OpC;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=LOBYTE(HIWORD(StartSektor.GetHSG()));
	OpC[4]=HIBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[5]=LOBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[6]=0x00;
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
		case CDTYPE_TOSHNEW :
		{
			OpC[0]=0x28;
			OpC[7]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[8]=LOBYTE(LOWORD(Sektoranzahl));
			OpC[9]=0x00;
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,Buffer,Sektoranzahl*2352,ReadSRB,m_hDriveEvent);
			break;
		}
		case CDTYPE_SONY :
		case CDTYPE_RICOH :
		case CDTYPE_PLEXTOR :
		{
			OpC[0]=0xD8;
			OpC[7]=0x00;
			OpC[8]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[9]=LOBYTE(LOWORD(Sektoranzahl));
			// benski
			if (bUseC2ErrorInfo)
				OpC[10]=0x04;
			else
				OpC[10]=0x00;
			OpC[11]=0x00;
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,12,Buffer,(bUseC2ErrorInfo)?(Sektoranzahl*2646):(Sektoranzahl*2352),ReadSRB,m_hDriveEvent);
			break;
		}
		case CDTYPE_NEC :
		{
			OpC[0]=0xD4;
			OpC[7]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[8]=LOBYTE(LOWORD(Sektoranzahl));
			OpC[9]=0x00;
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,Buffer,Sektoranzahl*2352,ReadSRB,m_hDriveEvent);
			break;
		}
		case CDTYPE_MATSHITA :
		{
			OpC[0]=0xD4;
			OpC[7]=0x00;
			OpC[8]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[9]=LOBYTE(LOWORD(Sektoranzahl));
			OpC[10]=0x00;
			OpC[11]=0x00;
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,12,Buffer,Sektoranzahl*2352,ReadSRB,m_hDriveEvent);
			break;
		}
		case CDTYPE_PHILIPS :
		{
			OpC[0]=0x28;
			OpC[7]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[8]=LOBYTE(LOWORD(Sektoranzahl));
			OpC[9]=0x00;
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,Buffer,Sektoranzahl*2352,ReadSRB,m_hDriveEvent);
			break;
		}
		case CDTYPE_ATAPI :
		case CDTYPE_CYBERDRV :
		{
			OpC[0]=0xBE;
			OpC[1]=0x04;
			OpC[7]=HIBYTE(LOWORD(Sektoranzahl));
			OpC[8]=LOBYTE(LOWORD(Sektoranzahl));
			OpC[9]=0xF0;
			// benski
						if (bUseC2ErrorInfo)
							OpC[9]|=2; // flag 2 is supposed to mean check for C2 error info
			OpC[10]=0x00;
			OpC[11]=0x00;
			// with C2 error info, our sector size is now 2646 bytes
			FillSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,12,Buffer,bUseC2ErrorInfo?(Sektoranzahl*2646):(Sektoranzahl*2352),ReadSRB,m_hDriveEvent);
			break;
		}
	}
	ExecuteSCSIRequest(ReadSRB,m_hDriveEvent);
	StartReadTime=GetTickCount();
}

BOOL CSCSICD::WaitCDDA(BOOL bImmediate)
{
	BYTE Status=WaitSCSIRequest(ReadSRB,m_hDriveEvent,bImmediate);
	if ((Status!=SS_PENDING) &&
		(Status!=SS_COMP))
	{
		memcpy(&m_SenseInfo,&ReadSRB.SenseArea,SENSE_LEN);
		Error=CDASPIError;
	}
	if ((Status==SS_PENDING) &&
		!bImmediate)
	{
		DWORD AktReadTime=GetTickCount();
		if (abs((long long)AktReadTime-StartReadTime)>12000)
		{
			AbortSCSIRequest(ReadSRB);
			Error=CDTimeOut;
			Status=SS_COMP;
		}
	}
	return (Status!=SS_PENDING);
}

void CSCSICD::FinishCDDA()
{
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
			{
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_TOSHNEW :
			{
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,15,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_NEC :
			{
				ModeData[6]=ModeData[6]|NECRotationSpeed;
				if (!addModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
		case CDTYPE_PHILIPS :
		case CDTYPE_MATSHITA :
			{
				if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeData,12,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				break;
			}
	}
}

void CSCSICD::SortWaveData(DWORD *Data,int Samples)
{
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_PHILIPS :
//      RICOH Drives doesn't seem to swap the bytes
//		This has been evaluated with the CD-RW drives
//		So they are even just handled like SONY drives, but i don't remove this type
//		case CDTYPE_RICOH :
			{
				for (int i=0; i<Samples; i++)
					Data[i]=((Data[i]&0xff00ff00)>>8)|
						((Data[i]&0x00ff00ff)<<8);
				break;
			}
	}
}

CCDAdress CSCSICD::GetErrorAdress()
{
	CCDAdress h;
	h.SetHSG(0);
	if ((Error!=CDOK)&&
		(ReadSRB.SRB_TargStat==STATUS_CHKCOND))
		h.SetHSG((ReadSRB.SenseArea[3]<<24)+
				 (ReadSRB.SenseArea[4]<<16)+
				 (ReadSRB.SenseArea[5]<<8)+
				 ReadSRB.SenseArea[6]);
	return h;
}

void CSCSICD::Play_Audio(CCDAdress StartSektor,long Sektoranzahl)
{
	while (!TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hDriveEvent));
	TOpcode OpC;
	OpC[0]=0x45;
	OpC[1]=0x00;
	OpC[2]=HIBYTE(HIWORD(StartSektor.GetHSG()));
	OpC[3]=LOBYTE(HIWORD(StartSektor.GetHSG()));
	OpC[4]=HIBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[5]=LOBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[6]=0x00;
	OpC[7]=HIBYTE(LOWORD(Sektoranzahl));
	OpC[8]=LOBYTE(LOWORD(Sektoranzahl));
	OpC[9]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,10,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

void CSCSICD::Stop_Audio()
{
	while (!TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hDriveEvent));
	TOpcode OpC;
/*	OpC[0]=0x1b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
*/
	OpC[0]=0x2b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=0x00;
	OpC[9]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,10,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

void CSCSICD::Pause_Audio()
{
	while (!TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hDriveEvent));
	TOpcode OpC;
	OpC[0]=0x4b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=0x00;
	OpC[9]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,10,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

void CSCSICD::Resume_Audio()
{
	while (!TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hDriveEvent));
	TOpcode OpC;
	OpC[0]=0x4b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=0x01;
	OpC[9]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,10,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

TDriveStatus CSCSICD::Get_DriveStatus()
{
	TDriveStatus h = {0};
/*	TOpcode OpC;
	TQChannelInfo ChannelInfo;
	BOOL b;
	memset(&ChannelInfo,0,sizeof(ChannelInfo));
	OpC[0]=0x42;
	OpC[1]=0x02;
	OpC[2]=0x40;
	OpC[3]=0x01;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=sizeof(ChannelInfo);
	OpC[9]=0x00;
	if (MapInfo.GetTypMapping(Config.Type)==CDTYPE_ATAPI)
	{
		OpC[10]=0;
		OpC[11]=0;
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,12,(void *)&ChannelInfo,sizeof(ChannelInfo),m_hDriveEvent);
	}
	else
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,10,(void *)&ChannelInfo,sizeof(ChannelInfo),m_hDriveEvent);
	if (b && ChannelInfo.DataLen)
*/
	if (TestUnitReady(Config.HostAdapterNumber,Config.ID,Config.LUN,m_hDriveEvent))
		h.CDPresent=TRUE;
	else
		h.CDPresent=FALSE;
	if (h.CDPresent!=CDPresentLast) Changed=TRUE;
	CDPresentLast=h.CDPresent;
	return h;
}

BOOL CSCSICD::MediaChanged()
{
	BOOL h;
	h=Changed;
	//if (CDPresentLast) 
		Changed=FALSE;
	return h;
}

TAudioStatus CSCSICD::Get_AudioStatus_Info()
{
	TAudioStatus h;
	TOpcode OpC;
	TQChannelInfo ChannelInfo;
	BOOL b;
	memset(&ChannelInfo,0,sizeof(ChannelInfo));
	OpC[0]=0x42;
	OpC[1]=0x02;
	OpC[2]=0x40;
	OpC[3]=0x01;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=0x10;
	OpC[9]=0x00;
	h.Pause=FALSE;
	h.IsPlaying=FALSE;
	h.IsDone=FALSE;
	h.PlayError=FALSE;
	if (MapInfo.GetTypMapping(Config.Type)==CDTYPE_ATAPI)
	{
		OpC[10]=0;
		OpC[11]=0;
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,12,(void *)&ChannelInfo,16,m_hDriveEvent);
	}
	else
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,(void *)&ChannelInfo,16,m_hDriveEvent);
	if (b && ChannelInfo.DataLen)
	{
//		if (!CDPresentLast) Changed=TRUE;
//		CDPresentLast=TRUE;
		switch (ChannelInfo.AudioStatus)
		{
			case 0x11 : h.IsPlaying=TRUE; break;
			case 0x12 : h.Pause=TRUE; break;
			case 0x13 : h.IsDone=TRUE; break;
			case 0x14 : h.PlayError=TRUE; break;
		}
		h.AbsSektor.SetRedbook(ChannelInfo.AbsCDAdress);
		h.RelSektor.SetRedbook(ChannelInfo.RelTrackAdress);
		h.TrackNummer=ChannelInfo.TrackNumber;
	}
	else
	{
//		if (CDPresentLast) Changed=TRUE;
//		CDPresentLast=FALSE;
		h.PlayError=TRUE;
		Error=CDNoCD;
	}
	return h;
}

void CSCSICD::Get_MediaCatalogNumber(char szUPC[16])
{
	TOpcode OpC;
	BYTE Info[24] = {0};
	BOOL b;
	szUPC[0]=0;
	OpC[0]=0x42;
	OpC[1]=0x02;
	OpC[2]=0x40;
	OpC[3]=0x02;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=sizeof(Info);
	OpC[9]=0x00;
	if (MapInfo.GetTypMapping(Config.Type)==CDTYPE_ATAPI)
	{
		OpC[10]=0;
		OpC[11]=0;
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,12,(void *)&Info,sizeof(Info),m_hDriveEvent);
	}
	else
		b=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,(void *)&Info,sizeof(Info),m_hDriveEvent);
	if (b && (Info[8]&0x80))
	{
		BOOL bIsEmpty=TRUE;
		for (int i=0; i<15; i++)
		{
			BYTE Value=Info[i+9];
			if (Value)
				bIsEmpty=FALSE;
			if (Value<10)
				Value+=0x30;
			szUPC[i]=Value;
		}
		szUPC[15]=0;
		if (bIsEmpty)
			szUPC[0]=0;
	}
}

void CSCSICD::EjectDisk()
{
	TOpcode OpC;
	OpC[0]=0x1b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x02;
	OpC[5]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,6,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

void CSCSICD::LockDoor(int Lock)
{
	TOpcode OpC;
	OpC[0]=0x1e;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=((BYTE) Lock) & 1;
	OpC[5]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,6,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

void CSCSICD::CloseTray()
{
	TOpcode OpC;
	OpC[0]=0x1b;
	OpC[1]=0x00;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x03;
	OpC[5]=0x00;
	if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,0,OpC,6,NULL,0,m_hDriveEvent))
		Error=CDDriveNotReady;
}

int Swap(int value)
{
	int result=(value & 0xff000000)>>24;
	result|=(value & 0x00ff0000)>>8;
	result|=(value & 0x0000ff00)<<8;
	result|=(value & 0x000000ff)<<24;
	return result;
}

void CSCSICD::ReRead()
{
	DeleteTrackList();
	m_bSpeedTableInitialized=FALSE;
	TTOCHeader TOCHeader;
	memset(&TOCHeader,0,sizeof(TOCHeader));
	TOpcode OpC;
	OpC[0]=0x43;
	OpC[1]=0;
	OpC[2]=0;
	OpC[3]=0;
	OpC[4]=0;
	OpC[5]=0;
	OpC[6]=0;
	OpC[7]=HIBYTE(sizeof(TTOCHeader));
	OpC[8]=LOBYTE(sizeof(TTOCHeader));
	OpC[9]=0;
	
	BOOL r=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,(void*)&TOCHeader,sizeof(TOCHeader),m_hDriveEvent);
	if (r && TOCHeader.FirstTrack && TOCHeader.LastTrack)
	{
		TTrackListeMem *Last;
		CCDAdress Ende;
		Last=FirstTrack;
		for (int i=TOCHeader.FirstTrack; i<=TOCHeader.LastTrack; i++)
		{
			AktTrack=new TTrackListeMem;
			//AbsCDAdress
			//AdrCtrl
			AktTrack->Info.TrackNummer=TOCHeader.Info[i-1].TrackNummer;
			AktTrack->Info.StartSektor.SetHSG(Swap(TOCHeader.Info[i-1].AbsCDAdress)+150);
			Ende.SetHSG(Swap(TOCHeader.Info[i].AbsCDAdress)+150);
			AktTrack->Info.Laenge=Ende.GetHSG()-AktTrack->Info.StartSektor.GetHSG();
			AktTrack->Info.StartSektor.SetHSG(AktTrack->Info.StartSektor.GetHSG()-150);
			if (TOCHeader.Info[i-1].AdrCtrl & 8)
				AktTrack->Info.Flags.AudioChannels=4;
			else
				AktTrack->Info.Flags.AudioChannels=2;
			AktTrack->Info.Flags.PreEmphasis=(TOCHeader.Info[i-1].AdrCtrl & 1);
			AktTrack->Info.Flags.DataTrack=(TOCHeader.Info[i-1].AdrCtrl & 4);
			AktTrack->Info.Flags.CopyProhibeted=!(TOCHeader.Info[i-1].AdrCtrl & 2);
			AktTrack->Prev=Last;
			AktTrack->Next=0;
			if (FirstTrack)
				Last->Next=AktTrack;
			else
				FirstTrack=AktTrack;
			Last=AktTrack;
		}
		// check for CD-Extra
		if (AktTrack)
		{
			if (AktTrack->Info.Flags.DataTrack)
			{
				Last=AktTrack->Prev;
				if (Last && !Last->Info.Flags.DataTrack)
				{
					if (Last->Info.Laenge>11400)
						Last->Info.Laenge-=11400;
				}
			}
		}
	}
	else 
	{
		if (CDPresentLast) Changed=TRUE;
		CDPresentLast=FALSE;
		Error=CDDriveNotReady;
	}
}

CCDAdress CSCSICD::GetLastReadableAddress(CCDAdress StartSektor)
{
	CCDAdress LastSektor;
	BYTE RetVal[8] = {0};
	TOpcode OpC;
	OpC[0]=0x25;
	OpC[1]=0;
	OpC[2]=HIBYTE(HIWORD(StartSektor.GetHSG()));
	OpC[3]=LOBYTE(HIWORD(StartSektor.GetHSG()));
	OpC[4]=HIBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[5]=LOBYTE(LOWORD(StartSektor.GetHSG()));
	OpC[6]=0;
	OpC[7]=0;
	OpC[8]=1; //Set PMI Bit
	OpC[9]=0;
	
	BOOL r=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,10,(void*)&RetVal,sizeof(RetVal),m_hDriveEvent);
	if (!r)
		return StartSektor;
	LastSektor.SetHSG((RetVal[0]<<24)+(RetVal[1]<<16)+(RetVal[2]<<8)+RetVal[3]);
	return LastSektor;
}

int CSCSICD::GetMaxSektors()
{
	return Config.MaxSektors;
};

int CSCSICD::GetSynchSektors()
{
	return Config.SynchSektors;
};

int CSCSICD::GetMode()
{
	return Config.Mode;
};

int CSCSICD::GetSpeed()
{
	return Config.Speed;
};

TSenseInfo CSCSICD::GetSense()
{
	TSenseInfo SenseInfo;
	memset(&SenseInfo,0,sizeof(SenseInfo));
	TOpcode OpC;
	OpC[0]=0x03;
	OpC[1]=0;
	OpC[2]=0;
	OpC[3]=0;
	OpC[4]=sizeof(SenseInfo);
	OpC[5]=0;
	
	BOOL r=ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_IN,OpC,6,(void*)&SenseInfo,sizeof(SenseInfo),m_hDriveEvent);
	if (!r)
		memset(&SenseInfo,0,sizeof(SenseInfo));
	return SenseInfo;
}

TSenseInfo CSCSICD::GetLastSenseInfo()
{
	TSenseInfo Info=m_SenseInfo;
	memset(&m_SenseInfo,0,sizeof(TSenseInfo));
	return Info;
}

void CSCSICD::InitSpeedTable()
{
	if (m_bSpeedTableInitialized)
		return;
	SupportedSpeeds=0;
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
			break;
		case CDTYPE_TOSHNEW :
		{
			SpeedTable[0]=2352*75;
			SpeedTable[1]=2352*75*4;
			SpeedTable[2]=2352*75*4;
			SpeedTable[3]=-2;
			SupportedSpeeds=4;
			break;
		}
		case CDTYPE_SONY :
		case CDTYPE_RICOH :
		case CDTYPE_ATAPI :
		case CDTYPE_CYBERDRV :
		{
			int LastSpeed=GetCurrentSpeed();
			int Speed=65532000;
			BOOL bFound=FALSE;
			while (!bFound && (Speed>0))
			{
				SetCurrentSpeed(Speed);
				if (Lasterror()==CDOK)
				{
					int ResultingSpeed=GetCurrentSpeed();
					if (Lasterror()==CDOK)
					{
						bFound=FALSE;
						for (int i=0; i<SupportedSpeeds; i++)
						{
							if (SpeedTable[i]==ResultingSpeed)
								bFound=TRUE;
						}
						if (!bFound)
						{
							SpeedTable[SupportedSpeeds]=ResultingSpeed;
							SupportedSpeeds++;
							Speed=ResultingSpeed-2352*75;
						}
						else
						{
							Speed-=(2352*75);
							bFound=FALSE;
						}
					}
				}
				else
					Speed=0;
			}
			if (SupportedSpeeds>1)
			{
				//Swap entries
				for (int i=0; i<(SupportedSpeeds/2); i++)
				{
					int Help=SpeedTable[i];
					SpeedTable[i]=SpeedTable[SupportedSpeeds-1-i];
					SpeedTable[SupportedSpeeds-1-i]=Help;
				}
			}
			SetCurrentSpeed(LastSpeed);
			break;
		}
		case CDTYPE_PLEXTOR :
		{
			int LastSpeed=GetCurrentSpeed();
			for (int index=1; index<=20; index++)
			{
				SetCurrentSpeed(index*2352*75);
				if (Lasterror()==CDOK)
				{
					int Speed=GetCurrentSpeed();
					if (Lasterror()==CDOK)
					{
						BOOL found=FALSE;
						for (int i=0; i<SupportedSpeeds; i++)
						{
							if (SpeedTable[i]==Speed)
								found=TRUE;
						}
						if (!found)
						{
							SpeedTable[SupportedSpeeds]=Speed;
							SupportedSpeeds++;
						}
					}
				}
			}
			SetCurrentSpeed(LastSpeed);
			break;
		}
		case CDTYPE_NEC :
		{
			break;
		}
		case CDTYPE_PHILIPS :
		case CDTYPE_MATSHITA :
		{
			int LastSpeed=GetCurrentSpeed();
			SpeedTable[0]=-1;
			SupportedSpeeds++;
			for (int index=1; index<8; index++)
			{
				SetCurrentSpeed(2352*75*index);
				if (Lasterror()==CDOK)
				{
					int Speed=GetCurrentSpeed();
					if ((Lasterror()==CDOK) &&
						(Speed==2352*75*index))
					{
						SpeedTable[SupportedSpeeds]=2352*75*index;
						SupportedSpeeds++;
					}
				}
			}
			SetCurrentSpeed(LastSpeed);
			break;
		}
	}
	m_bSpeedTableInitialized=TRUE;
}

BYTE CSCSICD::GetSupportedSpeeds()
{
	return SupportedSpeeds;
}

BOOL IsOldPhilips(TDriveInfo *pConfig)
{
	BOOL bResult=FALSE;
	if (strstr(pConfig->ProductID,"2000"))
		bResult=TRUE;
	if (strstr(pConfig->ProductID,"4020"))
		bResult=TRUE;
	return bResult;
}

//return the identifictaion number for the plextor models
//defined values:
#define PX4X		0
#define PX6X		1
#define PX8X		2
#define PX12X		3
#define PX20X		4
#define PX32X		5
#define PXR412		6
#define PX40X		7

DWORD GetPlextorModel(TDriveInfo *pConfig)
{
	DWORD dwResult=PX40X;
	char szId[7] = {0};
	strncpy(szId,&pConfig->ProductID[10],6);
	szId[6]=0;
	if (!_stricmp(szId,"W4220T"))
	{
		dwResult=PXR412;
	} 
	else
	{
		if (!_stricmp(szId,"W8220T"))
		{
			dwResult=PXR412;
		} 
		else
		{
			szId[5]=0;
			if (!_stricmp(szId,"R412C"))
			{
				dwResult=PXR412;
			} 
			else
			{
				if (!_stricmp(szId,"R820T"))
				{
					dwResult=PXR412;
				} 
				else
				{
					szId[2]=0;
					if (!strcmp(szId,"40"))
						dwResult=PX40X;
					else
					{
						if (!strcmp(szId,"32"))
							dwResult=PX32X;
						else
						{
							if (!strcmp(szId,"20"))
								dwResult=PX20X;
							else
							{
								if (!strcmp(szId,"12"))
									dwResult=PX12X;
								else
								{
									if (!isdigit(szId[1]))
										szId[1]=0;
									if (!strcmp(szId,"8"))
										dwResult=PX8X;
									else
									{
										if (!strcmp(szId,"6"))
											dwResult=PX6X;
										else
										{
											if (!strcmp(szId,"4"))
												dwResult=PX4X;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return dwResult;
}

int CSCSICD::GetCurrentSpeed()
{
	TDriveMode ModeSenseData;
	int Speed=0;
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
		{
			Speed=0;
			break;
		}
		case CDTYPE_TOSHNEW :
		{
			if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,15,0x20,m_hDriveEvent))
			{
				Error=CDASPIError;
				return 0;
			}
			int Index=(ModeSenseData[14] & 0x30)>>4;
			switch (Index)
			{
				case 0 :
					Speed=2352*75;
					break;
				case 1 :
					Speed=2352*75*4;
					break;
				case 2 :
					Speed=2352*75*4;
					break;
				case 3 :
					Speed=-2;
					break;
			}
			break;
		}
		case CDTYPE_SONY :
		case CDTYPE_RICOH :
		case CDTYPE_CYBERDRV :
		{
			if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,34,0x2A,m_hDriveEvent))
			{
				Error=CDASPIError;
				return 0;
			}
			Speed=(ModeSenseData[26]*256+ModeSenseData[27])*1000;
			break;
		}
		case CDTYPE_ATAPI :
		{
			if (!ATAPIModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,34,0x2A,m_hDriveEvent))
			{
				Error=CDASPIError;
				return 0;
			}
			if ((ModeSenseData[8]&0x3F)==0x2A)
				Speed=(ModeSenseData[22]*256+ModeSenseData[23])*1000;
			else if ((ModeSenseData[4]&0x3F)==0x2A)
				Speed=(ModeSenseData[18]*256+ModeSenseData[19])*1000;
			else Speed=-1;
			break;
		}
		case CDTYPE_PLEXTOR :
		{
			DWORD dwModel=GetPlextorModel(&Config);
			if (dwModel!=PXR412)
			{
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,16,0x31,m_hDriveEvent))
				{
					Error=CDASPIError;
					return 0;
				}
				int Index=ModeSenseData[14];
				switch (dwModel)
				{
					case PX4X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX6X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*4;
								break;
							case 2 :
								Speed=2352*75*6;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX8X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							case 3 :
								Speed=2352*75*8;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX12X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							case 3 :
								Speed=2352*75*8;
								break;
							case 4 :
								Speed=2352*75*8;
								break;
							case 5 :
								Speed=2352*75*12;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX20X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							case 3 :
								Speed=2352*75*8;
								break;
							case 4 :
								Speed=2352*75*8;
								break;
							case 5 :
								Speed=-2;
								break;
							case 6 :
								Speed=2352*75*12;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX32X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							case 3 :
								Speed=2352*75*8;
								break;
							case 4 :
								Speed=2352*75*8;
								break;
							case 5 :
								Speed=2352*75*8;
								break;
							case 6 :
								Speed=2352*75*14;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
					case PX40X :
					{
						switch (Index)
						{
							case 0 :
								Speed=2352*75;
								break;
							case 1 :
								Speed=2352*75*2;
								break;
							case 2 :
								Speed=2352*75*4;
								break;
							case 3 :
								Speed=2352*75*8;
								break;
							case 4 :
								Speed=2352*75*8;
								break;
							case 5 :
								Speed=2352*75*10;
								break;
							case 6 :
								Speed=2352*75*17;
								break;
							default :
								Speed=-1;
								break;
						}
						break;
					}
				}
			}
			else
			{
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,32,0x2A,m_hDriveEvent))
				{
					Error=CDASPIError;
					return 0;
				}
				Speed=(ModeSenseData[26]*256+ModeSenseData[27])*1000;
			}
			break;
		}
		case CDTYPE_NEC :
		{
			if (!addModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,12,m_hDriveEvent))
			{
				Error=CDASPIError;
				return 0;
			}
//			Speed=ModeSenseData[6] & 0x20;
			break;
		}
		case CDTYPE_PHILIPS :
		case CDTYPE_MATSHITA :
		{
			if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,20,0x23,m_hDriveEvent))
			{
				Error=CDASPIError;
				return 0;
			}
			int Index;
			if (IsOldPhilips(&Config))
				Index=ModeSenseData[14];
			else
				Index=ModeSenseData[16];
			switch (Index)
			{
				case 0 :
					Speed=-1;
					break;
				case 1 :
					Speed=2352*75;
					break;
				case 2 :
					Speed=2352*75*2;
					break;
				case 4 :
					Speed=2352*75*4;
					break;
				case 6 :
					Speed=2352*75*6;
					break;
				case 8 :
					Speed=2352*75*8;
					break;
				default :
					Speed=-2;
			}
			break;
		}
		default :
			Speed=0;
	}
	return Speed;
}

void CSCSICD::SetCurrentSpeed(int Speed)
{
	TDriveMode ModeSenseData;
	if (Speed==0)
		return;
	switch (MapInfo.GetTypMapping(Config.Type))
	{
		case CDTYPE_TOSHIBA :
			break;
		case CDTYPE_TOSHNEW :
		{
			if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,15,0x20,m_hDriveEvent))
			{
				Error=CDASPIError;
				return;
			}
			ModeSenseData[0]=0;
			int Index;
			switch (Speed)
			{
				case -2 :
					Index=3;
					break;
				case 2352*75 :
					Index=0;
					break;
				case 2352*75*4 :
					Index=1;
					break;
				default :
					Index=2;
			}
			ModeSenseData[14]=(ModeSenseData[14] & 0xcf)|(Index<<4);
			if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,15,m_hDriveEvent))
				Error=CDASPIError;
			break;
		}
		case CDTYPE_SONY :
		case CDTYPE_RICOH :
		case CDTYPE_CYBERDRV :
		case CDTYPE_ATAPI :
		{
			TOpcode OpC;
			OpC[0]=0xbb;
			OpC[1]=0x00;
			OpC[4]=0x00;
			OpC[5]=0x00;
			OpC[6]=0x00;
			OpC[7]=0x00;
			OpC[8]=0x00;
			OpC[9]=0x00;
			OpC[10]=0x00;
			OpC[11]=0x00;
			int NewSpeed=Speed/1000;
			int ModSpeed=NewSpeed*1000;
			int Direction=0;
			int AktSpeed=0;
			int Counter=0;
			do
			{
				Error=CDOK;
				Counter++;
				NewSpeed+=Direction;
				OpC[2]=NewSpeed/256;
				OpC[3]=NewSpeed%256;
				if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_OUT,OpC,12,NULL,0,m_hDriveEvent))
					Error=CDASPIError;
				// Is the speed really the one we have selected?
				AktSpeed=GetCurrentSpeed();
				if (!Direction)
				{
					if (AktSpeed<Speed)
						Direction=1;
					else if (AktSpeed>Speed)
						Direction=-1;
				}
			}
			while ((AktSpeed!=ModSpeed) && NewSpeed && (Counter<3));
			break;
		}
		case CDTYPE_PLEXTOR :
		{
			DWORD dwModel=GetPlextorModel(&Config);
			if (dwModel!=PXR412)
			{
				if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,16,0x31,m_hDriveEvent))
				{
					Error=CDASPIError;
					return;
				}
				int Index;
				switch (dwModel)
				{
					case PX4X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case -1 :
							case 2352*75*4 :
								Index=2;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX6X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*4 :
								Index=1;
								break;
							case -1 :
							case 2352*75*6 :
								Index=2;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX8X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case 2352*75*4 :
								Index=2;
								break;
							case -1 :
							case 2352*75*8 :
								Index=3;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX12X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case 2352*75*4 :
								Index=2;
								break;
							case 2352*75*8 :
								Index=3;
								break;
							case -1 :
							case 2352*75*12 :
								Index=5;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX20X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case 2352*75*4 :
								Index=2;
								break;
							case 2352*75*8 :
								Index=3;
								break;
							case -1 :
							case 2352*75*12 :
								Index=6;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX32X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case 2352*75*4 :
								Index=2;
								break;
							case 2352*75*8 :
								Index=3;
								break;
							case -1 :
							case 2352*75*14 :
								Index=6;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
					case PX40X :
					{
						switch (Speed)
						{
							case 2352*75 :
								Index=0;
								break;
							case 2352*75*2 :
								Index=1;
								break;
							case 2352*75*4 :
								Index=2;
								break;
							case 2352*75*8 :
								Index=3;
								break;
							case 2352*75*10 :
								Index=5;
								break;
							case -1 :
							case 2352*75*17 :
								Index=6;
								break;
							default :
								Index=-1;
								break;
						}
						break;
					}
				}
				if (Index>=0)
				{
					ModeSenseData[14]=Index;
					if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,16,m_hDriveEvent))
						Error=CDASPIError;
				}
				else
					Error=CDASPIError;
			}
			else
			{
				TOpcode OpC;
				OpC[0]=0xbb;
				OpC[1]=0x00;
				OpC[4]=0xff;
				OpC[5]=0xff;
				OpC[6]=0x00;
				OpC[7]=0x00;
				OpC[8]=0x00;
				OpC[9]=0x00;
				OpC[10]=0x00;
				OpC[11]=0x00;
				int NewSpeed=Speed/1000;
				int ModSpeed=NewSpeed*1000;
				int Direction=0;
				int AktSpeed=0;
				int Counter=0;
				do
				{
					Error=CDOK;
					Counter++;
					NewSpeed+=Direction;
					OpC[2]=NewSpeed/256;
					OpC[3]=NewSpeed%256;
					if (!ExecuteSCSIRequest(Config.HostAdapterNumber,Config.ID,Config.LUN,SRB_DIR_OUT,OpC,12,NULL,0,m_hDriveEvent))
						Error=CDASPIError;
					// Is the speed really the one we have selected?
					AktSpeed=GetCurrentSpeed();
					if (!Direction)
					{
						if (AktSpeed<Speed)
							Direction=1;
						else if (AktSpeed>Speed)
							Direction=-1;
					}
				}
				while ((AktSpeed!=ModSpeed) && NewSpeed && (Counter<3));
			}
			break;
		}
		case CDTYPE_NEC :
		{
			if (!addModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,12,m_hDriveEvent))
			{
				Error=CDASPIError;
				return;
			}
//			Speed=ModeSenseData[6] & 0x20;
			if (!addModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,12,m_hDriveEvent))
				Error=CDASPIError;
			break;
		}
		case CDTYPE_PHILIPS :
		case CDTYPE_MATSHITA :
		{
			if (!ModeSense(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,20,0x23,m_hDriveEvent))
			{
				Error=CDASPIError;
				return;
			}
			ModeSenseData[0]=0;
			int Index;
			switch (Speed)
			{
				case -2 :
				case -1 :
					Index=0;
					break;
				case 2352*75 :
					Index=1;
					break;
				case 2352*75*2 :
					Index=2;
					break;
				case 2352*75*4 :
					Index=4;
					break;
				case 2352*75*6 :
					Index=6;
					break;
				case 2352*75*8 :
					Index=8;
					break;
				default :
					Index=0;
			}
			if (IsOldPhilips(&Config))
				ModeSenseData[14]=Index;
			else
				ModeSenseData[16]=Index;
			if (!ModeSelect(Config.HostAdapterNumber,Config.ID,Config.LUN,ModeSenseData,20,m_hDriveEvent))
				Error=CDASPIError;
			break;
		}
	}
}

int CSCSICD::GetSpeed(BYTE Index)
{
	if (Index<SupportedSpeeds)
		return SpeedTable[Index];
	return 0;
}


// ----------------------------------------------------------------------------------------
// - Implementation of general functions                                                  -
// -                                                                                      -
// - Author: Christoph Schmelnik														  -
// - Purposee: variour initialisations                                                    -
// ----------------------------------------------------------------------------------------

extern "C" DWORD NtScsiSendASPI32Command( LPSRB lpsrb );
int LoadASPI2()
{
	OSVERSIONINFO VersionInfo;
	VersionInfo.dwOSVersionInfoSize=sizeof(VersionInfo);
	GetVersionEx(&VersionInfo);
	if (VersionInfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
		RunningNT=TRUE;
	else
		RunningNT=FALSE;
	ASPIInstalled=TRUE;

	hDLL=LoadLibrary(L"WNASPI32.DLL"); // load DLL
	
	if (hDLL==0)
	{
    if(RunningNT) {
      // ok, let's try to see if we can use NT's internal SCSI manager
      extern int NtScsiInit( void );
      int nb;
      if(nb=NtScsiInit()) {
        NumberOfHostAdapters=nb;
      	SendASPI32Command=(SRBPROC)&NtScsiSendASPI32Command;
        return TRUE;
      }
    }
		ASPIInstalled=FALSE;
 		return FALSE;
	}
	GetASPI32SupportInfo=(VOIDPROC)GetProcAddress(hDLL,"GetASPI32SupportInfo"); // get Address
	SendASPI32Command=(SRBPROC)GetProcAddress(hDLL,"SendASPI32Command"); // get Address
	if (GetASPI32SupportInfo==NULL)
	{ 
   		ASPIInstalled=FALSE;
		return FALSE;
	}
	if (SendASPI32Command==NULL)
	{ 
   		ASPIInstalled=FALSE;
		return FALSE;
	}
	int r=GetASPI32SupportInfo();
	if (HIBYTE(r)!=SS_COMP)
	{
  		ASPIInstalled=FALSE;
		return FALSE;
	}
	NumberOfHostAdapters=LOBYTE(r);
	return TRUE;
}

int LoadASPI() {
  int ret=0;
  __try {
    ret=LoadASPI2();
  } __except(EXCEPTION_EXECUTE_HANDLER)
  {
    ret=0;
  }
  return ret;
}


int FreeASPI()
{
  extern int NtScsiDeInit( void );
  NtScsiDeInit();
	if (hDLL) FreeLibrary(hDLL);
  hDLL=NULL;
	return TRUE;
}


int CheckASPI()
{
	return ASPIInstalled;
}