#ifndef NULLSOFT_AGAVE_IFC_AUDIOSTREAM_H
#define NULLSOFT_AGAVE_IFC_AUDIOSTREAM_H

#include <bfc/dispatch.h>

class ifc_audiostream : public Dispatchable
{
protected:
	ifc_audiostream() {}
	~ifc_audiostream() {}
public:
	/* returns number of bytes written to buffer.
	 * a return value of 0 means EOF
	 */
	size_t ReadAudio(void *buffer, size_t sizeBytes); 

	size_t ReadAudio(void *buffer, size_t, int *killswitch, int *errorCode);
	/* Seeks to a point in the stream in milliseconds
	 * returns TRUE if successful, FALSE otherwise
	 */
	int SeekToTimeMs(int millisecs);

	/* returns 1 if this stream is seekable using SeekToTime, 0 otherwise
	*/
	int CanSeek();
public:
	DISPATCH_CODES
	{
	    IFC_AUDIOSTREAM_READAUDIO = 10,
			IFC_AUDIOSTREAM_READAUDIO2 = 11,
			IFC_AUDIOSTREAM_SEEKTOTIMEMS = 20,
			IFC_AUDIOSTREAM_CANSEEK = 30,
	};
};

inline size_t ifc_audiostream::ReadAudio(void *buffer, size_t sizeBytes)
{
	return _call(IFC_AUDIOSTREAM_READAUDIO, (size_t)0, buffer, sizeBytes);
}

inline size_t ifc_audiostream::ReadAudio(void *buffer, size_t sizeBytes, int *killswitch, int *errorCode)
{
	void *params[4] = { &buffer, &sizeBytes, &killswitch, &errorCode};
	size_t retval;

	if (_dispatch(IFC_AUDIOSTREAM_READAUDIO2, &retval, params, 4))
		return retval;
	else
	{
		*errorCode=0;
		return ReadAudio(buffer, sizeBytes);
	}	
}

inline int ifc_audiostream::SeekToTimeMs(int millisecs)
{
	return _call(IFC_AUDIOSTREAM_SEEKTOTIMEMS, (int)0, millisecs);
}

inline int ifc_audiostream::CanSeek()
{
	return _call(IFC_AUDIOSTREAM_CANSEEK, (int)0);
}

#endif
