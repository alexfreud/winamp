#include <nx/nxfile.h>
#include <libopenmpt/libopenmpt.h>
#include <assert.h>

static size_t openmpt_nxfile_read(void *stream, void *dst, size_t bytes) 
{
	nx_file_t f = (nx_file_t)stream;
	size_t bytes_read;
	ns_error_t err = NXFileRead(f, dst, bytes, &bytes_read);
	if (err != NErr_Success) {
		return 0;
	}
	return bytes_read;
}

static int openmpt_nxfile_seek(void *stream, int64_t offset, int whence)
{
	nx_file_t f = (nx_file_t)stream;
	uint64_t position;
	if (whence == OPENMPT_STREAM_SEEK_SET) {
		position = offset;
	} else if (whence == OPENMPT_STREAM_SEEK_CUR) {
		ns_error_t err = NXFileTell(f, &position);
		if (err != NErr_Success) {
			return -1;
		}
		position += offset;
	} else if (whence == OPENMPT_STREAM_SEEK_END) {
		assert(0);
	} else {
		return -1;
	}
	ns_error_t err = NXFileSeek(f, position);
	if (err = NErr_Success) {
		return 0;
	} else {
		return -1;
	}
}

static int64_t openmpt_nxfile_tell(void *stream)
{
		nx_file_t f = (nx_file_t)stream;
		uint64_t position;
		if (NXFileTell(f, &position) == NErr_Success) {
			return (int64_t)position;
		} else {
			return -1;
		}
}

openmpt_stream_callbacks openmpt_stream_get_nxfile_callbacks(void)
{
	openmpt_stream_callbacks retval;
	memset( &retval, 0, sizeof( openmpt_stream_callbacks ) );
	retval.read = openmpt_nxfile_read;
	retval.seek = openmpt_nxfile_seek;
	retval.tell = openmpt_nxfile_tell;
	return retval;
}

openmpt_module *OpenMod(const wchar_t *filename)
{
	openmpt_module * mod = 0;

	nx_string_t nx_filename=0;
	nx_uri_t nx_uri=0;
	NXStringCreateWithUTF16(&nx_filename, filename);
	NXURICreateWithNXString(&nx_uri, nx_filename);
	NXStringRelease(nx_filename);

	nx_file_t f=0;
	ns_error_t nserr;

	nserr = NXFileOpenZip(&f, nx_uri, NULL);
	if (nserr != NErr_Success) {
		nserr = NXFileOpenFile(&f, nx_uri, nx_file_FILE_read_binary);
	}
	NXURIRelease(nx_uri);
	if (nserr != NErr_Success) {
		return 0;
	}

	mod = openmpt_module_create(openmpt_stream_get_nxfile_callbacks(), f, NULL, NULL, NULL);
	NXFileRelease(f);
	return mod;
}