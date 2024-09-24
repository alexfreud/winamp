#ifndef __TRANSFER_THREAD_
#define __TRANSFER_THREAD_
#include <wchar.h>

class DeviceView;

#define STATUS_ERROR -1
#define STATUS_WAITING 0
#define STATUS_TRANSFERRING 1
#define STATUS_DONE 3
#define STATUS_CANCELLED 4

extern int SynchronousProcedureCall(void * p, ULONG_PTR dwParam);
extern BOOL RecursiveCreateDirectory(wchar_t* buf1); // from replaceVars.cpp
extern wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song); // from replaceVars.cpp

class CopyInst {
public:
	CopyInst() : status(STATUS_WAITING), res(0), dev(NULL), equalsType(-1), usesPreCopy(false), usesPostCopy(true), songid(NULL) {
		typeCaption[0] = 0;
		statusCaption[0] = 0;
		trackCaption[0] = 0;
		sourceDevice[0] = 0;
		destDevice[0] = 0;
		lastChanged[0] = 0;
		sourceFile[0] = 0;
	}
	virtual bool PreCopyAction() {return false;} // called in main window thread. Return true to skip.
	virtual bool CopyAction()=0; // Do the actual transfer, called in transfer thread. Return true if failed.
	virtual void PostCopyAction() {} // called in main window thread
	virtual void Cancelled() {} // the transfer has been cancelled
	virtual bool Equals(CopyInst * b)=0; // use equalsType to check if it is safe to cast b to your own type. Return true if equal.
	int status; // one of STATUS_*
	int res; // don't mess with this!
	DeviceView * dev;
	wchar_t typeCaption[128];
	wchar_t statusCaption[128];
	wchar_t trackCaption[128];
	wchar_t sourceDevice[128];
	wchar_t destDevice[128];
	wchar_t lastChanged[128];
	wchar_t sourceFile[MAX_PATH];
	int equalsType; // 0 for SongCopyInst, 1 for ReverseCopyInst, -1 for unknown
	bool usesPreCopy;
	bool usesPostCopy;
	songid_t songid;
};

class SongCopyInst : public CopyInst {
public:
	SongCopyInst(DeviceView * dev,itemRecordW * song);
	virtual ~SongCopyInst();
	itemRecordW song;
	virtual bool CopyAction();
	virtual void PostCopyAction();
	virtual void Cancelled();
	virtual bool Equals(CopyInst * b);
};

class PlaylistCopyInst : public SongCopyInst {
public:
	wchar_t plName[256];
	int plid;
	C_ItemList * plAddSongs;
	PlaylistCopyInst(DeviceView * dev, itemRecordW * song, wchar_t * plName0, int plid0);
	virtual ~PlaylistCopyInst();
	virtual bool PreCopyAction();
	virtual void PostCopyAction();
};

class ReverseCopyInst : public CopyInst {
public:
	ReverseCopyInst(DeviceView * dev, const wchar_t * filepath, const wchar_t * format, songid_t song, bool addToLibrary, bool uppercaseext);
	virtual bool CopyAction(); // Do the actual transfer, called in transfer thread. Return true if failed.
	virtual void PostCopyAction();
	virtual bool Equals(CopyInst * b); // use equalsType to check if it is safe to cast b to your own type. Return true if equal.
	wchar_t path[2048];
	bool uppercaseext;
};

// simple subclass which appends the copied filename to an m3u playlist file after copy.
class ReversePlaylistCopyInst : public ReverseCopyInst {
public:
	ReversePlaylistCopyInst(DeviceView * dev, const wchar_t * filepath, const wchar_t * format, songid_t song, wchar_t * playlistFile, wchar_t * playlistName, bool last,bool addToLibrary=true);
	virtual bool CopyAction();
	virtual void PostCopyAction();
	wchar_t playlistFile[MAX_PATH];
	wchar_t playlistName[128];
	bool last;
};

//DWORD WINAPI ThreadFunc_Transfer(LPVOID lpParam);

#endif //__TRANSFER_THREAD_