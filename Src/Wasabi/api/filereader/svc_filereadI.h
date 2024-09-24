#ifndef __WASABI_SVC_FILEREADI_H
#define __WASABI_SVC_FILEREADI_H

#include <api/service/svcs/svc_fileread.h>
#include <api/filereader/api_readercallback.h>
// derive from this one
class NOVTABLE svc_fileReaderI : public svc_fileReader
{
public:
	virtual int isMine(const wchar_t *filename, int mode = SvcFileReader::READ) { return -1; }
	virtual int open(const wchar_t *filename, int mode = SvcFileReader::READ) = 0; // return 1 on success
	virtual size_t read(int8_t *buffer, size_t length) = 0; // return number of bytes read (if < length then eof)
	virtual size_t write(const int8_t *buffer, size_t length) = 0; // return number of bytes written
	virtual void close() = 0; //must be safe to call even when not open

	virtual int canSetEOF() { return 0; }
	virtual int setEOF(uint64_t newlen) { return -1; }

	virtual void abort() { } // tells the reader to abort its current prebuffering/reader

	virtual int getLength() { return -1; } // return -1 on unknown/infinite
	virtual int getPos() = 0;

	virtual int canSeek() { return 0; }
	virtual int seek(uint64_t position) { return 0; }
	virtual uint64_t bytesAvailable(uint64_t requested) { return requested; }

	virtual int hasHeaders() { return 0; }
	virtual const char *getHeader(const char *header) { return (const char *)NULL; }

	virtual int exists(const wchar_t *filename) { return 0; } // return 1 if true, 0 if not, -1 if unknown

	virtual int remove(const wchar_t *filename) { return 0; } // return 1 on success, 0 on error

	virtual int removeUndoable(const wchar_t *filename) { return -1; }

	virtual int move(const wchar_t *filename, const wchar_t *destfilename) { return 0; } // return 1 on success, 0 on error

	virtual void setMetaDataCallback(api_readercallback *cb) { }

	virtual int canPrefetch() { return 1; } // return 1 if your reader should prefetch infos about the file in pledit
	// (HTTP reader will return 0 here for instance)

protected:
	RECVS_DISPATCH;
};


// derive from this one
class NOVTABLE MetaDataReaderCallbackI : public api_readercallback {
public:
  virtual void metaDataReader_onData(const char *data, int size)=0;

protected:
#undef CBCLASS
#define CBCLASS MetaDataReaderCallbackI
START_DISPATCH_INLINE;
      VCB(METADATAREADERONDATA, metaDataReader_onData);
END_DISPATCH;
#undef CBCLASS
};

#include <api/service/svcs/svc_redir.h>
#include <bfc/std_file.h> // for WA_MAX_PATH but this needs to be in a better place

#define MAX_FILEREADER_REDIRECT 256

// note: this class handles both redirection and calling open()
class FileReaderEnum : public SvcEnumT<svc_fileReader> {
public:
  FileReaderEnum(const wchar_t *filename, int mode=SvcFileReader::READ, int allow_redirect=FALSE) :
    fn(filename), m(mode)
  {
    if (allow_redirect) {
      for (int done = 0, c = 0; !done && c < MAX_FILEREADER_REDIRECT; done = 1, c++) {
        RedirectEnum re(fn);
        svc_redirect *svc;
        while ((svc = re.getNext(FALSE)) != NULL) {
          wchar_t buf[WA_MAX_PATH]=L"";
          if (svc->redirect(fn, L"Filename", buf, WA_MAX_PATH)) {
            fn = buf;
            done = 0;
          }
          re.getLastFactory()->releaseInterface(svc);
        }
      }
    }
  }
  virtual int testService(svc_fileReader *svc) {
    if (!svc->isMine(fn)) return 0;
    return !!svc->open(fn, m);
  }

private:
  const wchar_t *fn;
  int m;
};


#endif