// ----------------------------------------------
// -           DAC32.DLL Header Datei           -
// - Written 1996-1998 by Christoph Schmelnik   -
// ----------------------------------------------

// Changes
// ===========
// Version 1.2 :
// -CMapDrive supports now up to 34 Host Adapters, because now it could access the drives under
// NT with the DeviceControl Interface
// The required information for this is coded as:
// Drives accessed over the new Interface have a HostAdapterNumber above or equal to the
// NumberOfHostAdapters (global Information).
// The ID consists of the good old drive number, like in DOS.
// The LUN is not used for the configuration, but it is used to hold the open handle to the device
// The Burstmodus couldn't be used with the new access mode, so the default setting for those
// drives is the Normal-mode. (For those drives is no difference between Burst- and Normal-mode)
// -LoadASPI checks now the Windows Version and hold this result global in the DLL
// -The Constructor of CSCSCD opens the handles to the new devices
// -The destructor closes those handles
// Interface changes are not required but some values must be handled differnt to avoid
// misconfiguring, e.g. it is not allowed to configure the new drives ID, LUN, and HostAdapterNumber
// -A bug in CWaveSave regarding conversion to mixed mono has been fixed.
//
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
// Fixed GetRedBook operator in CCDAdress
// Added function to CD Class to read Media Cataloge Number
//
// Version 1.41 : 28.04.1998
// Changes:
// New GetInfoEx() function in CMapDrive, to allow a better result checking.
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


#ifndef _DAC32_H
#define _DAC32_H

#ifndef STRICT
#define STRICT               // Use strct typechecking
#define WIN32_LEAN_AND_MEAN  // compile only important Headerfiles
#endif

#include <windows.h>
#include <stdio.h>
#include "aspifunc.h"

/*#ifdef DLL
#define DACDLL __declspec(dllexport)
#else
#define DACDLL __declspec(dllimport)
#endif*/
#define DACDLL  

#ifdef _DEBUG
#define DBGOUT(sz) OutputDebugString(sz)
#else
#define DBGOUT(sz)
#endif

//The Errormessages for the CD and DAC Classes
#define CDOK              0x000
#define CDUnknownDrive    0x001
#define CDDriveNotReady   0x002
#define CDUnknownCommand  0x003
#define CDSeekError       0x006
#define CDSectorNotFound  0x008
#define CDReadError       0x00B
#define CDGeneralError    0x00C
#define CDNoCD            0x00E
#define CDIllegalCDChange 0x00F
#define CDDriveNotFound   0x100
#define CDNoMemory        0x101
#define CDDACUnable       0x102
#define CDASPIError       0x103
#define CDUserBreak       0x104
#define CDTimeOut         0x105

//The Errormessage for the wave classes
#define WAVEFileOpenError	0x200
#define WAVEFileWriteError	0x201
#define WAVEChannelError	0x202
#define WAVEBitsError		0x203
#define WAVEFreqError		0x204
#define WAVENameError 		0x205
#define WAVEMemoryError		0x206

//The supported base drive types
#define CDTYPE_TOSHIBA	0
#define CDTYPE_SONY		1
#define CDTYPE_NEC		2
#define CDTYPE_PHILIPS	3
#define CDTYPE_ATAPI	4
#define CDTYPE_TOSHNEW  5
#define CDTYPE_RICOH    6
#define CDTYPE_MATSHITA 7
#define CDTYPE_PLEXTOR  8
#define CDTYPE_CYBERDRV 9

#define JUKETYPE_SONY    0
#define JUKETYPE_PIONEER 1

//Amount of predefined drive mappings (relationship between real drives and base drive types)
#define MaxMappings 22	// pgo

//Amount of predefined jukebox mappings (relationship between real jukeboxes and base jukebox types)
#define MaxMappingsJuke 2

//The possible Copy modes
#define ModeNormal 0
#define ModeSynch  1
#define ModeBurst  2

//The possible Values for the DA Test
#define DATEST_ALLWAYS		0
#define DATEST_FIRSTTRACK	1
#define DATEST_NEVER		2

//The possible SpinUp modes
#define SPINUP_ALLWAYS		0
#define SPINUP_FIRSTTRACK	1
#define SPINUP_NEVER		2

// Amount of DWORD for the synchronisation
#define SynLen 512

// The Class for the Addressformats
class DACDLL CCDAdress
{
public:
	void SetRedbook(long Value);
	void SetHSG(long Value)
	{
		Adresse=Value;
	};
	long GetHSG()
	{
		return Adresse;
	};
	long GetRedbook()
	{
		long t;
		t=(Adresse % 75)<<24;
		t+=((Adresse / 75) % 60)<<16;
		t+=((Adresse / 75) / 60)<<8;
		return t;
	};
	CCDAdress &operator = (const CCDAdress &other)
	{
		Adresse=other.Adresse;
		return (*this);
	};
	CCDAdress &operator + (const long Value)
	{
		Adresse+=Value;
		return (*this);
	};
	CCDAdress &operator - (const long Value)
	{
		Adresse-=Value;
		return (*this);
	};
	CCDAdress &operator += (const long Value)
	{
		Adresse+=Value;
		return (*this);
	};
	CCDAdress &operator -= (const long Value)
	{
		Adresse-=Value;
		return (*this);
	};
private:
	long Adresse;
};

// Typendeclarations
struct TDriveStatus
{
	int DoorOpen;                 //Door open/closed
	int DoorLocked;               //Door locked/unlocked
	int Cooked_RAW;               //supports Cooked and RAW or Cooked
	int Read_Write;               //supports read and write
	int Data_Audio_Video;         //supports Data/Audio/Video or only Data
	int Interleave;               //supports Interleave regarding ISO
	int CommandPrefetch;          //supports Command Prefetching
	int AudioChannelManipulation; //supports Audio-Channel Manipulation
	int HSG_Redbook;              //supports HSG and Redbook Addressing or only HSG
	int CDPresent;                //CD inserted or not
	int RWSupport;                //supports R-W-Sub Channels
};

struct TAudioStatus
{
	BOOL Pause;						//Play is paused
	BOOL IsPlaying;					//CD is playing
	BOOL IsDone;					//Play is stopped
	BOOL PlayError;					//Play completed with error
	int TrackNummer;				//Number of actual track
	CCDAdress AbsSektor,RelSektor;	//Startsector and Endsector of last/next Play
};
	
struct TTrackFlag
{
	BYTE AudioChannels;   //Amount of Audio Channels (2/4)
    BOOL PreEmphasis;     //Audio Channel with or without...
    BOOL DataTrack;       //Data track or Audio track
    BOOL CopyProhibeted;  //Digital copy prohibited
};

struct TTrackList
{
	BYTE TrackNummer; 		//Number of Track
    CCDAdress StartSektor;	//First sector in HSG Format
    long Laenge;			//Amount of Sectors
    TTrackFlag Flags;
};

struct TTrackListeMem
{
	TTrackList Info;
	TTrackListeMem *Prev,*Next;
};

struct TDriveInfo
{
	int ID;
	int LUN;
	int HostAdapterNumber;
	int Type;
	int MaxSektors;
	int SynchSektors;
	int Mode;
	char VendorID[9];
	char ProductID[17];
	int Speed;
	int PerformDATest;
	int SpinUpMode;
	DWORD dwSpinUpTime;
	BOOL bUseLastReadableAddress;
	BOOL bUseC2ErrorInfo;
	BOOL bSpinDown;
};

struct TJukeInfo
{
	int ID;
	int LUN;
	int HostAdapterNumber;
	int Type;
	int MaxDrives;
	int *pConnectedDrives;
	int MaxDiscs;
	char VendorID[9];
	char ProductID[17];
};

// The class with the infos for the type mapping
class DACDLL CMapInfo
{
public:
	CMapInfo();
	char *GetTypName(int Index);
	int GetTypMapping(int Index);
	int GetTypMappingRev(int CDType);
private:
	char TypNamen[MaxMappings][9];
	int TypMapping[MaxMappings];
};

// The class with the infos for the type mapping
class DACDLL CMapInfoJuke
{
public:
	CMapInfoJuke();
	char *GetTypName(int Index);
	int GetTypMapping(int Index);
	int GetTypMappingRev(int JukeType);
private:
	char TypNamen[MaxMappingsJuke][9];
	int TypMapping[MaxMappingsJuke];
};

// The base class (pure virtual) for the CD access
class DACDLL CBaseCD
{
public:
    int Lasterror();
	virtual void PrepareCDDA()=0;
	virtual void ReadCDDA(CCDAdress StartSektor,long Sektoranzahl,void *Buffer,BOOL bUseC2ErrorInfo=FALSE)=0;
    virtual BOOL WaitCDDA(BOOL bImmediate=FALSE)=0;
	virtual void FinishCDDA()=0;
    virtual void SortWaveData(DWORD *Data,int Samples)=0;
	virtual CCDAdress GetErrorAdress()=0;
    virtual void Play_Audio(CCDAdress StartSektor,long Sektoranzahl)=0;
    virtual void Stop_Audio()=0;
    virtual void Pause_Audio()=0;
    virtual void Resume_Audio()=0;
    virtual TDriveStatus Get_DriveStatus()=0;
    virtual BOOL MediaChanged()=0;
	virtual void Get_MediaCatalogNumber(char szUPC[16])=0;
    virtual void EjectDisk()=0;
    virtual void LockDoor(int Lock)=0;
    virtual void CloseTray()=0;
    virtual void ReRead()=0;
    virtual CCDAdress GetLastReadableAddress(CCDAdress StartSektor)=0;
    virtual int GetMaxSektors()=0;
    virtual int GetSynchSektors()=0;
	virtual int GetMode()=0;
	virtual int GetSpeed()=0;
	virtual void InitSpeedTable()=0;
	virtual BYTE GetSupportedSpeeds()=0;
	virtual int GetCurrentSpeed()=0;
	virtual void SetCurrentSpeed(int Speed)=0;
	virtual int GetSpeed(BYTE Index)=0;
    int ReadFirstTrackInfo(TTrackList &Infos);
    int ReadNextTrackInfo(TTrackList &Infos);
    int ReadPrevTrackInfo(TTrackList &Infos);
    int ReadTrackInfo(TTrackList &Infos);
    int ReadMaxTracks();
protected:
    int Error,BusyFlag,DoneFlag;
    TTrackListeMem *FirstTrack,*AktTrack;
    void DeleteTrackList();
private:
	CBaseCD& operator = (const CBaseCD &other);
};

// The class for the access to SCSI drives
class DACDLL CSCSICD:public CBaseCD
{
public:
	CSCSICD (char drive, TDriveInfo &xInfo);
	~CSCSICD();
	virtual void PrepareCDDA();
	virtual void ReadCDDA(CCDAdress StartSektor,long Sektoranzahl,void *Buffer,BOOL bUseC2ErrorInfo=FALSE);
    virtual BOOL WaitCDDA(BOOL bImmediate=FALSE);
	virtual void FinishCDDA();
    virtual void SortWaveData(DWORD *Data,int Samples);
    virtual CCDAdress GetErrorAdress();
    virtual void Play_Audio(CCDAdress StartSektor,long Sektoranzahl);
    virtual void Stop_Audio();
    virtual void Pause_Audio();
    virtual void Resume_Audio();
    virtual TDriveStatus Get_DriveStatus();
    virtual BOOL MediaChanged();
    virtual TAudioStatus Get_AudioStatus_Info();
	virtual void Get_MediaCatalogNumber(char szUPC[16]);
    virtual void EjectDisk();
    virtual void LockDoor(int Lock);
    virtual void CloseTray();
    virtual void ReRead();
    virtual CCDAdress GetLastReadableAddress(CCDAdress StartSektor);
    virtual int GetMaxSektors();
    virtual int GetSynchSektors();
	virtual int GetMode();
	virtual int GetSpeed();
	virtual void InitSpeedTable();
	virtual BYTE GetSupportedSpeeds();
	virtual int GetCurrentSpeed();
	virtual void SetCurrentSpeed(int Speed);
	virtual int GetSpeed(BYTE Index);
	TDriveInfo &GetInfo()
	{
		return Config;
	};
	TSenseInfo GetSense();
	TSenseInfo GetLastSenseInfo();

private:
	CSCSICD& operator = (const CSCSICD &other);
	
	TDriveInfo &Config;		 // Drive Configuration		
	BOOL CDPresentLast,Changed; // Helpvariables for the MediaChanged function
	SRB_ExecSCSICmd ReadSRB; // SCSI Commando Block
	TDriveMode ModeData;
	BYTE NECRotationSpeed;
	CMapInfo MapInfo;
	DWORD StartReadTime;
	int SpeedTable[256];
	BYTE SupportedSpeeds;
	HANDLE m_hDriveEvent;
	TSenseInfo m_SenseInfo;
	BOOL m_bSpeedTableInitialized;
};

// The base class for saving/converting the audio data
class DACDLL CBaseWave
{
public:
	int Lasterror();
	virtual void WritePuffer(long Samples,DWORD *Buffer)=0;
protected:
	int Error;				//last occured error
private:
	CBaseWave& operator = (const CBaseWave &other);
};

typedef struct
{
	BOOL bIsUsed;				//Is Buffer used?
	BOOL bReady;				//Is Nuffer ready to write?
	int nSamples;				//Number of Samples in Buffer.
	int nZeroSamples;			//Number of Silence Samples to insert before the buffer
	DWORD *dwBuffer;		//Buffer for Audio Data
} WAVEBUFFER, *PWAVEBUFFER;

typedef struct _WAVEBUFFERLIST
{
	PWAVEBUFFER pWaveBuffer;
	_WAVEBUFFERLIST *pNext;
} WAVEBUFFERLIST, *PWAVEBUFFERLIST;


typedef struct
{
	HANDLE hEvent;
	LPVOID pData;
} TWAVEMTSTRUCT;

// The base class for saving/converting the audio data as its own thread
class DACDLL CBaseWaveMT
{
public:
	CBaseWaveMT(DWORD dwBufferSize,BOOL bUseHighPriority=FALSE,int MaxSektors=27,DWORD dwExtraBytes=0);
	~CBaseWaveMT();

	void SetFadeInOut(int dwTotalSamples,int dwFadeSamples);

	PWAVEBUFFER GetBuffer();	//returns NULL if no Buffer is available
	void SignalBuffer();		//signal if a buffer is filled;
	int Lasterror();
	double GetBufferFullRatio();
	DWORD GetBytesInBuffer();
	
	virtual void WritePuffer(long Samples,DWORD *Buffer)=0;

	friend unsigned _stdcall WaveThreadProc(LPVOID pUserData);

protected:
	int Error;				//last occured error

	void StartThread(); //call this from your own initialisation function
	BOOL WriteData();
	void StopThread(BOOL bImmediate=FALSE); //call this from your own cleanup function
	
private:
	CBaseWaveMT& operator = (const CBaseWaveMT &other);

	DWORD m_Nullen[256];
	PWAVEBUFFERLIST m_pFirstBuffer, m_pReadBuffer, m_pWriteBuffer;
	TWAVEMTSTRUCT m_WaveInfo;
	BOOL m_bStopThread,m_bAbortThread,m_bIsWorking;
	HANDLE m_hWaveThread;
	BOOL m_bUseHighPriority;
	int m_dwTotalSamples,m_dwFadeSamples,m_dwCurrentSample;
	int m_nBufferNum,m_nReadBufferNum,m_nWriteBufferNum;
	int m_nMaxSektors;
};

// The class for saving audio data in a wave file
class DACDLL CWaveSave:public CBaseWave
{
public:
	CWaveSave(const char *DateiName,BYTE Freq,BYTE Channels,BYTE Bits,BOOL bWriteHeaders=TRUE);
	~CWaveSave();
	virtual void WritePuffer(long Samples,DWORD *Buffer);
private:
	void WMono8(long Samples,DWORD *Buffer);
	void WStereo8(long Samples,DWORD *Buffer);
	void WMono16(long Samples,DWORD *Buffer);
	void WStereo16(long Samples,DWORD *Buffer);
	void WLR8(long Samples,int Mode,DWORD *Buffer);
	void WLR16(long Samples,int Mode,DWORD *Buffer);

	CWaveSave& operator = (const CWaveSave &other);
	
	BYTE ConvertType;		//CodeNumber of the conversion type
	FILE *Datei;			//file variable to access the wave file
	WORD *DPM16;			//Pointer to the file data buffer
	BYTE *DPM8;				//Pointer to the file data buffer
	DWORD *DPS16;			//Pointer to the file data buffer
	BYTE *DPS8;				//Pointer to the file data buffer
	int DatenCount;			//Counter of data in buffer
	long WaveBytes;			//Counts all written bytes
	BYTE SAdd;				//Value to increment the source counter
	BOOL m_bWriteHeaders;	//Write Headers of Wave file
};

// The class for saving audio data in a wave file in its own thread
class DACDLL CWaveSaveMT:public CBaseWaveMT
{
public:
	CWaveSaveMT(DWORD dwBufferSize,BOOL bUseHighPriority=FALSE,int MaxSektors=27,DWORD dwExtraBytes=0):CBaseWaveMT(dwBufferSize,bUseHighPriority,MaxSektors,dwExtraBytes)
	{
	};
	void Init(const char *DateiName,BYTE Freq,BYTE Channels,BYTE Bits,BOOL bWriteHeaders=TRUE);
	void Done(BOOL bImmediate=FALSE);
	virtual void WritePuffer(long Samples,DWORD *Buffer);
private:
	void WMono8(long Samples,DWORD *Buffer);
	void WStereo8(long Samples,DWORD *Buffer);
	void WMono16(long Samples,DWORD *Buffer);
	void WStereo16(long Samples,DWORD *Buffer);
	void WLR8(long Samples,int Mode,DWORD *Buffer);
	void WLR16(long Samples,int Mode,DWORD *Buffer);

	CWaveSave& operator = (const CWaveSave &other);
	
	BYTE ConvertType;		//CodeNumber of the conversion type
	FILE *Datei;			//file variable to access the wave file
	WORD *DPM16;			//Pointer to the file data buffer
	BYTE *DPM8;				//Pointer to the file data buffer
	DWORD *DPS16;			//Pointer to the file data buffer
	BYTE *DPS8;				//Pointer to the file data buffer
	int DatenCount;			//Counter of data in buffer
	long WaveBytes;			//Counts all written bytes
	BYTE SAdd;				//Value to increment the source counter
	BOOL m_bWriteHeaders;	//Write Headers of Wave file
};

// The class for copying the audio data from CD.
class DACDLL CDAC
{
public:
	CDAC(CBaseCD *pDrive,CBaseWave *pWave,
		 CCDAdress Start,long Laenge,BOOL xKillZeros,BOOL bPerformDATest=TRUE,DWORD dwSpinUpTime=0,BOOL bUseLastReadableAddress=FALSE);
	~CDAC();
	int Lasterror();
	void Copy();
	int Errors();
	void StopCopy();

	// The following member functions are declared as virtual. They could be used to display
	// information to the user. They do nothing by default.
	virtual void WriteInit();
	virtual void WritePercent(float Percent);
	virtual void WriteReading();
	virtual void WriteReadingEnd();
	virtual void WriteSynch();
	virtual void WriteSynchEnd();
	virtual void WriteFlushing();
	virtual void WriteFlushingEnd();
	virtual void WriteSynchError();
	virtual void WriteBufferUnderrun(CCDAdress Start);
	virtual void WriteReRead(CCDAdress Start,long Laenge);
	virtual void WriteSektorsSkipped(CCDAdress Start,long Laenge);
	virtual void WriteDone();
	virtual void OnIdle();

protected:
	CCDAdress StartSektor,StartOld;
	int SynchErrors,Error;
	long Anzahl,AnzahlOld,Remain;

private:
	CBaseCD *m_pCD;
	BOOL RunCopy,KillFirst,KillLast,KillZero,Found;
	DWORD m_dwSpinUpTime;
	CBaseWave *m_pWaveSave;
	long SektorAnzahl,Retries,SynchDiff,ZeroCount;
	DWORD *MemBlocks[2];
	int BlockCount,S_Offset;
	int SpeedSave;
	DWORD m_Nullen[256];

	void ReadCDDA(CCDAdress Start,long Sektoranzahl,void *Buffer);
	void FlushWave();
	void FlushSynch(int Samples,DWORD *Data);
	void MakeTable(DWORD *Werte,DWORD *Table);
	int SynchSearch(DWORD *String1,DWORD *String2,DWORD *Table);
	void SynchWave();

	CDAC& operator = (const CDAC &other);
};

// The class for copying the audio data from CD.
class DACDLL CDACMT
{
public:
	CDACMT(CBaseCD *pDrive,CBaseWaveMT *pWave,
		 CCDAdress Start,long Laenge,BOOL xKillZeros,BOOL bPerformDATest=TRUE,DWORD dwSpinUpTime=0,BOOL bUseHighPriority=FALSE,BOOL bUseC2ErrorInfo=FALSE,BOOL bSpinDown=FALSE,BOOL bUseLastReadableAddress=FALSE);
	~CDACMT();
	int Lasterror();
	void Copy();
	int Errors();
	void StopCopy();

	// The following member functions are declared as virtual. They could be used to display
	// information to the user. They do nothing by default.
	virtual void WriteInit();
	virtual void WritePercent(float Percent);
	virtual void WriteReading();
	virtual void WriteReadingEnd();
	virtual void WriteSynch();
	virtual void WriteSynchEnd();
	virtual void WriteFlushing();
	virtual void WriteFlushingEnd();
	virtual void WriteSynchError();
	virtual void WriteBufferUnderrun(CCDAdress Start);
	virtual void WriteReRead(CCDAdress Start,long Laenge);
	virtual void WriteSektorsSkipped(CCDAdress Start,long Laenge);
	virtual void WriteDone();
	virtual void OnIdle(BOOL bReturnFast=TRUE);

	friend unsigned _stdcall DACThreadProc(LPVOID pUserData);
protected:
	CCDAdress StartSektor,StartOld;
	int SynchErrors,Error;
	long Anzahl,AnzahlOld,Remain;

	void CopyMT();
private:
	CBaseCD *m_pCD;
	BOOL RunCopy,KillFirst,KillLast,KillZero,Found;
	DWORD m_dwSpinUpTime;
	CBaseWaveMT *m_pWaveSave;
	BOOL m_bUseC2ErrorInfo;
	BOOL m_bSpinDown;
	long SektorAnzahl,Retries,SynchDiff,ZeroCount;
	PWAVEBUFFER MemBlocks[2];
	int BlockCount,S_Offset;
	int SpeedSave;
	int m_MaxSektors;
	int m_SynchSektors;

	void ReadCDDA(CCDAdress Start,long Sektoranzahl,void *Buffer);
	void FlushWave(int nMax=2);
	void FlushSynch(int Samples,PWAVEBUFFER Data);
	void MakeTable(DWORD *Werte,DWORD *Table);
	int SynchSearch(DWORD *String1,DWORD *String2,DWORD *Table);
	void SynchWave();

	CDACMT& operator = (const CDACMT &other);

	HANDLE m_hDACThread;
	BOOL m_bUseHighPriority;
};

// The class for configuring the SCSI CDROM drives
class DACDLL CMapDrive
{
public:
	CMapDrive(BOOL bDoReset=TRUE);
	~CMapDrive();
	void Reset();
	int GetMaxDrives();
	TDriveInfo &GetInfo(int index);
	BOOL GetInfoEx(int index, TDriveInfo *&pInfo);
	void DeleteInfo(int index);
	int GetMaxHostAdapters();
	int GetSupportedHostAdapterMemory(int index);
	void SetSupportedHostAdapterMemory(int index,int Memory);
	int GetMaxSektors(int HostAdapterNumber);
protected:
	void DeleteAll();

private:
	struct TDriveInfoMem
	{
		TDriveInfo Info;
		TDriveInfoMem *Next;
	};
	TDriveInfoMem *First;
	int HostAdapterMemory[34];
	HANDLE m_hDriveEvent;
};


// The class for configuring the SCSI Jukeboxes
class DACDLL CMapJuke
{
public:
	CMapJuke(BOOL bDoReset=TRUE);
	~CMapJuke();
	void Reset();
	int GetMaxJukes();
	TJukeInfo &GetInfo(int index);
	BOOL IsWorking(int index);
	void SetWorking(int index,BOOL bIsWorking);
	void DeleteInfo(int index);
protected:
	void DeleteAll();

private:
	struct TJukeInfoMem
	{
		TJukeInfo Info;
		BOOL bIsWorking;
		TJukeInfoMem *Next;
	};
	TJukeInfoMem *First;
	HANDLE m_hJukeEvent;
};

//The class to access Jukeboxes
class DACDLL CJukeBox
{
public:
	CJukeBox (TJukeInfo &xInfo);
	~CJukeBox();
	//use following defines to address an item in the jukebox:
	//drive0..x : 0x4000...0x400x
	//storage1..xxx : 0x0001...0x0xxx
	BOOL MoveMedium(int Source,int Destination);
	TJukeInfo &GetInfo()
	{
		return Config;
	};

private:
	TJukeInfo &Config;		 // Drive Configuration		
	CMapInfoJuke MapInfo;
	HANDLE m_hJukeEvent;
};


// ----------------------
// function declarations
// ----------------------

// initialize and deinatialize the WNASPI32.DLL and some internal flags
int DACDLL LoadASPI();
int DACDLL FreeASPI();

int DACDLL CheckASPI();



#endif //_DAC32_H