#include "main.h"
#include "adts.h"
#include <memory.h>
#include <malloc.h>
#include <xutility>
#include <assert.h>
#include <shlwapi.h>
#include <foundation/error.h>
#include "../nu/RingBuffer.h"
#include <api/service/waservicefactory.h>

// {19450308-90D7-4E45-8A9D-DC71E67123E2}
static const GUID adts_aac_guid = 
{ 0x19450308, 0x90d7, 0x4e45, { 0x8a, 0x9d, 0xdc, 0x71, 0xe6, 0x71, 0x23, 0xe2 } };

// {4192FE3F-E843-445c-8D62-51BE5EE5E68C}
static const GUID adts_mp2_guid = 
{ 0x4192fe3f, 0xe843, 0x445c, { 0x8d, 0x62, 0x51, 0xbe, 0x5e, 0xe5, 0xe6, 0x8c } };

class GapCutter
{
public:
	GapCutter() {}

	void SetEndSize( int postSize );
	void SetSize( int preSize, int postSize );
	void Flush( int time_in_ms );
	int  Write( void *dest, void *input, size_t inputBytes );

private:
	RingBuffer ringBuffer;
	
	int        preCut     = 0;
	int        preCutSize = 0;
};

void GapCutter::SetEndSize(int postSize)
{
	if (postSize < 0)
		postSize = 0;

	if (postSize)
	{
		ringBuffer.Reset();
		ringBuffer.reserve(postSize);
	}
}

void GapCutter::SetSize( int preSize, int postSize )
{
	if ( preSize < 0 )
		preSize = 0;

	if ( postSize < 0 )
		postSize = 0;

	SetEndSize( postSize );

	preCutSize = preSize;
	preCut     = preSize;
}

void GapCutter::Flush( int time_in_ms )
{
	//		if (time_in_ms == 0) // TODO: calculate actual delay if we seek within the encoder delay area
	preCut = preCutSize; // reset precut size if we seek to the start

	ringBuffer.clear();
}

int GapCutter::Write( void *dest, void *input, size_t inputBytes ) // returns # of bytes written
{
	int bytesWritten = 0;
	unsigned __int8 *in  = (unsigned __int8 *)input;
	unsigned __int8 *out = (unsigned __int8 *)dest;
	// cut pre samples, if necessary

	intptr_t pre = min( preCut, (intptr_t)inputBytes );
	in         += pre;
	inputBytes -= pre;
	preCut     -= (int)pre;

	if ( !inputBytes )
		return bytesWritten;

	size_t   remainingFill = ringBuffer.avail();
	intptr_t fillWrite     = min( (intptr_t)( inputBytes - remainingFill ), (intptr_t)ringBuffer.size() ); // only write fill buffer if we've got enough left to fill it up

	if ( fillWrite > 0 )
	{
		size_t written = ringBuffer.read( out, fillWrite );

		bytesWritten += (int)written;
		out          += written;
	}

	remainingFill = ringBuffer.avail();
	
	int outWrite = (int)max( 0, (intptr_t)( inputBytes - remainingFill ) );
	if ( outWrite )
		memcpy( out, in, outWrite );

	bytesWritten += outWrite;
	in           += outWrite;
	inputBytes   -= outWrite;

	if ( inputBytes )
		ringBuffer.write( in, inputBytes );

	return bytesWritten;
}


struct ExtendedRead
{
	ExtendedRead()                                                    { memset(&data, 0, sizeof(data)); }
	~ExtendedRead()
	{
		file.Close();
		if ( decoder )
		{
			decoder->Close();
			decoder->Release();
		}
	}
	
	bool Open( const wchar_t *fn, int *size, int *bps, int *nch, int *srate, bool useFloat );

	adts      *decoder     = NULL;
	int        bits        = 0;
	size_t     initialData = 0;
	int        frameSize   = 0;

	GapCutter  cutter;
	CGioFile   file;

#define DATA_SIZE (6*4*2*2*1152)
	unsigned char data[DATA_SIZE];
};

bool ExtendedRead::Open(const wchar_t *fn, int *size, int *bps, int *nch, int *srate, bool useFloat)
{
	if (file.Open(fn, config_max_bufsize_k) != NErr_Success)
		return false;

	int downmix = 0;
	bool allowsurround = 1;
	if (*nch == 1)
	{
		downmix = 1;
		allowsurround = 0;
	}
	else if (*nch == 2)
	{
		allowsurround = 0;
	}

	if (useFloat)
		bits=32;
	else if (*bps == 24)
		bits = 24;
	else
	{
		bits = 16;
		*bps = 16;
	}

	wchar_t *ext = PathFindExtensionW(fn);
	if (!_wcsicmp(ext, L".vlb"))
	{
		return false;
	}
	else if (!_wcsicmp(ext,  L".aac") ||  !_wcsicmp(ext,  L".apl"))
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(adts_aac_guid);
		if (factory)
			decoder = (adts *)factory->getInterface();
	}
	else
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(adts_mp2_guid);
		if (factory)
			decoder = (adts *)factory->getInterface();
	}

	if (!decoder)
		return false; 

	decoder->Initialize(!!downmix, 0, allowsurround, bits, false, useFloat);
	decoder->Open(&file);
	size_t bitrate;
	bool done=false;
	while (!done)
	{
		switch (decoder->Sync(&file, data, sizeof(data), &initialData, &bitrate))
		{
		case adts::SUCCESS:
			done=true;
			break;
		case adts::FAILURE:
		case adts::ENDOFFILE:
			return false;
		case adts::NEEDMOREDATA:
			break;
		}
	}

	size_t numBits = 0;
	decoder->GetOutputParameters(&numBits, nch, srate);
	*bps = bits = (int)numBits;
	frameSize = bits / 8 * *nch;
	if (config_gapless)
		cutter.SetSize((file.prepad + (int)decoder->GetDecoderDelay())*frameSize, (file.postpad - (int)decoder->GetDecoderDelay())*frameSize);

	if (file.m_vbr_samples) // exact number of samples in the LAME header, how nice :)
		*size = (int)file.m_vbr_samples*frameSize;
	else if (file.m_vbr_ms) // if we know the milliseconds accurately
		*size = MulDiv(*srate * frameSize, file.m_vbr_ms, 1000);  // our size should be mostly accurate
	else // no helpful info to go on
	{
		// just guess based on bitrate and content length
		bitrate=decoder->GetCurrentBitrate();
		int len_ms = MulDiv(file.GetContentLength(), 8, (int)bitrate);
		*size = MulDiv(*srate * frameSize, len_ms, 1000);  
	}

	return true;
}

extern "C"
{
	//returns handle!=0 if successful, 0 if error
	//size will return the final nb of bytes written to the output, -1 if unknown
	__declspec(dllexport) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
	{
		ExtendedRead *ext = new ExtendedRead;
		if (ext)
		{
			if (ext->Open(fn, size, bps, nch, srate, false))
				return reinterpret_cast<intptr_t>(ext);
			delete ext;
		}
		return 0;
	}

	__declspec(dllexport) intptr_t winampGetExtendedRead_openW_float(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
	{
		ExtendedRead *ext = new ExtendedRead;
		if (ext)
		{
			if (ext->Open(fn, size, bps, nch, srate, true))
				return reinterpret_cast<intptr_t>(ext);
			delete ext;
		}
		return 0;
	}

	//returns nb of bytes read. -1 if read error (like CD ejected). if (ret==0), EOF is assumed
	__declspec(dllexport) size_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;
		int copied = 0;
		if (ext)
		{
			len -= (len % ext->frameSize); // only do whole frames
			while (len)
			{
				size_t toMove = min(len, ext->initialData);
				int toCopy = ext->cutter.Write(dest, ext->data, toMove);

				if (ext->initialData != toMove)
					memmove(ext->data, ext->data + toMove, ext->initialData - toMove);

				ext->initialData -= toMove;
				len -= toCopy;
				copied += toCopy;
				dest += toCopy;

				if (!ext->initialData)
				{
					size_t written = 0, bitrate, endCut = 0;
					int ret = ext->decoder->Decode(&ext->file, ext->data, DATA_SIZE, &written, &bitrate, &endCut);
					if (config_gapless && endCut)
						ext->cutter.SetEndSize((int)(endCut - ext->decoder->GetDecoderDelay())*ext->frameSize);
					ext->initialData = written;
					if (/*ret != adts::SUCCESS && */!ext->initialData && (copied || ret == adts::ENDOFFILE))
						return copied;

					if (ret == adts::FAILURE)
						return -1;
				}
			}
		}
		return copied;
	}

	// return nonzero on success, zero on failure.
	__declspec(dllexport) int winampGetExtendedRead_setTime(intptr_t handle, int millisecs)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;
		if (ext)
		{
			if (!ext->file.IsSeekable()) return 0; // not seekable

			int br = ext->file.GetAvgVBRBitrate();
			if (!br) br = (int)ext->decoder->GetCurrentBitrate();
			if (!br) return 0; // can't find a valid bitrate

			ext->cutter.Flush(millisecs); // fucko?
			ext->decoder->Flush(&ext->file);

			ext->file.Seek(millisecs,br);
			return 1;
		}
		return 0;
	}

	__declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;
		if (ext) delete ext;
	}
}