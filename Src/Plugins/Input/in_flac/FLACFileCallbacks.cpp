#include "FLACFileCallbacks.h"


FLAC__StreamDecoderReadStatus FLAC_NXFile_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	nx_file_t file = FLAC_GetFile(client_data);
	size_t bytes_to_read = *bytes;
	size_t bytes_read=0;
	ns_error_t ret = NXFileRead(file, buffer, bytes_to_read, &bytes_read);
	*bytes = bytes_read;

	if (ret == NErr_EndOfFile)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;

	if (ret != NErr_Success)
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;

	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus FLAC_NXFile_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	nx_file_t file = FLAC_GetFile(client_data);
	if (NXFileSeek(file, absolute_byte_offset) == NErr_Success)
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	else
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}
	
FLAC__StreamDecoderTellStatus FLAC_NXFile_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	nx_file_t file = FLAC_GetFile(client_data);

	if (NXFileTell(file, absolute_byte_offset) == NErr_Success)
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	else
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
}

FLAC__StreamDecoderLengthStatus FLAC_NXFile_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	nx_file_t file = FLAC_GetFile(client_data);
	
	if (NXFileLength(file, stream_length) == NErr_Success)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	else
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}

FLAC__bool FLAC_NXFile_EOF(const FLAC__StreamDecoder *decoder, void *client_data)
{
	nx_file_t file = FLAC_GetFile(client_data);
	if (NXFileEndOfFile(file) == NErr_False)
		return false;
	else
		return true;		
}
