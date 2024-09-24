#include "NXFileObject.h"
#include <new>
#include "minizip/unzip.h"
#include <nx/nxfile.h>
#include <assert.h>

class NXZipFile : NXFileObject
{
public:
	NXZipFile(unzFile zip_file);
	~NXZipFile();

	/* NXFileObject implementation */
	ns_error_t Read(void *buffer, size_t bytes_requested, size_t *bytes_read);
	ns_error_t Write(const void *buffer, size_t bytes);
	ns_error_t Seek(uint64_t position);
	ns_error_t Tell(uint64_t *position);
	ns_error_t PeekByte(uint8_t *byte);
	ns_error_t Sync();
	ns_error_t Truncate();
	// TODO(benski) implementation EOF
	// TODO(benski) implement region locking
private:
	unzFile zip_file;
};

NXZipFile::NXZipFile(unzFile zip_file) : zip_file(zip_file)
{
}

NXZipFile::~NXZipFile()
{
	if (zip_file) {
		unzCloseCurrentFile(zip_file);
		unzClose(zip_file);
		zip_file=0;
	}
}

/* NXFileObject implementation */
ns_error_t NXZipFile::Read(void *buffer, size_t bytes_requested, size_t *bytes_read)
{
	int zret = unzReadCurrentFile(zip_file, buffer, (unsigned int)bytes_requested);
	if (zret == 0) {
		if (bytes_read) {
			*bytes_read = 0;
		}
		return NErr_EndOfFile;
	} else if (zret > 0) {
		if (bytes_read) {
			*bytes_read = (size_t)zret;
		}
		return NErr_Success;
	} else {
		if (bytes_read) {
			*bytes_read = 0;
		}
		return NErr_Error;
	}
}

ns_error_t NXZipFile::Write(const void *buffer, size_t bytes)
{
	return NErr_NotImplemented;
}

ns_error_t NXZipFile::Seek(uint64_t position)
{
	// TODO(benski) error check)
	unzSetOffset64(zip_file, position);
	return NErr_Success;
}

ns_error_t NXZipFile::Tell(uint64_t *position)
{
	*position = unzGetOffset64(zip_file);
	return NErr_Success;
}

ns_error_t NXZipFile::PeekByte(uint8_t *byte)
{
	return NErr_NotImplemented;
}

ns_error_t NXZipFile::Sync()
{
	return NErr_NotImplemented;
}

ns_error_t NXZipFile::Truncate()
{
	return NErr_NotImplemented;
}

static voidpf ZCALLBACK unzip_nxfile_open OF((voidpf opaque, const void*  filename, int mode))
{
	nx_file_t f;
	if (NXFileOpenFile(&f, (nx_uri_t)filename, nx_file_FILE_read_binary) != NErr_Success) {
		return 0;
	}
	return f;
}

static uLong ZCALLBACK unzip_nxfile_read OF((voidpf opaque, voidpf stream, void* buf, uLong size))
{
	nx_file_t f = (nx_file_t)stream;
	size_t bytes_read;
	if (NXFileRead(f, buf, size, &bytes_read) != NErr_Success) {
		return 0;
	}
	return (uLong)bytes_read;
}

static int ZCALLBACK unzip_nxfile_close OF((voidpf opaque, voidpf stream))
{
	NXFileRelease((nx_file_t)stream);
	return 0;
}


static ZPOS64_T ZCALLBACK unzip_nxfile_tell OF((voidpf opaque, voidpf stream))
{
	nx_file_t f = (nx_file_t)stream;
	uint64_t position;
	if (NXFileTell(f, &position) == NErr_Success) {
		return (int64_t)position;
	} else {
		return -1;
	}
}

static long ZCALLBACK unzip_nxfile_seek OF((voidpf opaque, voidpf stream, ZPOS64_T offset, int whence))
{
		nx_file_t f = (nx_file_t)stream;
	uint64_t position;
	if (whence == SEEK_SET) {
		position = offset;
	} else if (whence == SEEK_CUR) {
		ns_error_t err = NXFileTell(f, &position);
		if (err != NErr_Success) {
			return -1;
		}
		position += offset;
	} else if (whence == SEEK_END) {
		uint64_t length;
		NXFileLength(f, &length);
		position = length + offset;
	} else {
		return -1;
	}
	ns_error_t err = NXFileSeek(f, position);
	if (err == NErr_Success) {
		return 0;
	} else {
		return -1;
	}
}
#if 0
    open64_file_func    zopen64_file;
    read_file_func      zread_file;
    write_file_func     zwrite_file;
    tell64_file_func    ztell64_file;
    seek64_file_func    zseek64_file;
    close_file_func     zclose_file;
    testerror_file_func zerror_file;
#endif

ns_error_t NXFileOpenZip(nx_file_t *out_file, nx_uri_t filename, nx_string_t extension_hint)
{
#if 0
	typedef struct zlib_filefunc_def_s
{
    open_file_func      zopen_file;
    read_file_func      zread_file;
    write_file_func     zwrite_file;
    tell_file_func      ztell_file;
    seek_file_func      zseek_file;
    close_file_func     zclose_file;
    testerror_file_func zerror_file;
    voidpf              opaque;
} zlib_filefunc_def;
#endif

	zlib_filefunc64_def file_func = {0, };
	file_func.zopen64_file = unzip_nxfile_open;
		file_func.zread_file = unzip_nxfile_read;
		file_func.ztell64_file = unzip_nxfile_tell;
		file_func.zseek64_file = unzip_nxfile_seek;
		file_func.zclose_file = unzip_nxfile_close;

	unzFile zip_file = unzOpen2_64(filename, &file_func);
	if (zip_file == NULL) {
		return NErr_Error;
	}

	unzGoToFirstFile(zip_file);
	// TODO(benski): look for filename with extension_hint as extension
	// TODO(benski): search for anything with extension
	unzOpenCurrentFile(zip_file);

	NXZipFile *nx_zip_file = new (std::nothrow) NXZipFile(zip_file);
	if (!nx_zip_file) {
		unzCloseCurrentFile(zip_file);
		unzClose(zip_file);
		return NErr_OutOfMemory;
	}
	*out_file = (nx_file_t)nx_zip_file;
	return NErr_Success;
}
