#pragma once
#include <bfc/dispatch.h>
class ifc_audioFileEncoder : public Dispatchable
{
protected:
	ifc_audioFileEncoder() {}
	~ifc_audioFileEncoder() {}
public:
	/*
	@param frame - frame number, optional (pass 0 every time if you don't have it).  
	               it is used for special purposes and is not generally needed.
  @param data - the input data (audio)
	@param data_len - number of valid bytes in data
	@param data_used - on return, set to the number of audio bytes read

	note for implementors:
	it is highly recommended that implementation use all available input data, but it is not required.
	
	note for users:
	if not all input data is read, you need to pass in the old data again
	but have more available.  This is likely to happen when the block size of the codec
	does not match your buffer size
	*/
	int Feed(uint32_t frame, const void *data, size_t data_len, size_t *data_used);

	// same as above, but used when you have no more input to pass
	// you can pass more audio data if you like (preferred if possible)
	// or pass 0 for data & data_len if you passed all your data to Feed()
	// note to implementors:
	//  for many implementations: close your encoder (e.g. vorbis) but not your file writer (e.g. ogg)
	// note to users:
	//  you still need to call Finish().
	int Finish(uint32_t frame, const void *data, size_t data_len, size_t *data_used);

	// call this to do whatever necessary 
	// this is a separate call than Finish() because of time-constraints and the killswitch
	// for many implementations: close your file writer object here, write seek table, etc.
	// note to users:
	//  after this function succeeds, you may use the file (write metadata, etc)
	int Finalize(int *killswitch);

	// @param block_size - on return, set to the 'natural' number of bytes that the audio encoder expects
	// can be helpful for users of this object to do more efficient I/O
	// optional (check the return value when you call this!)
	// but recommended
	int GetBlockSize(size_t *block_size);

	// @param fill_size - on return, set to the number of bytes needed to fill the audio encoder's current block
	// might be 0 for some types (WAV).
	// can be helpful for users of this object to do more efficient I/O
	// optional (check the return value when you call this!)
	// don't implement unless it's easy
	int GetFillSize(size_t *fill_size);

	// @param bytes_written - on return, set to the number of encoded bytes written so far.
	// used by the CD ripper to show a 'real time' bitrate
	// optional (check the return value when you call this!)
	// don't implement unless it's easy
	int GetBytesWritten(size_t *bytes_written);
};