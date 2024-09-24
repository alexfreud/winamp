#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include "../nu/ns_wc.h"
#include "resource.h"
#include "wmsdk.h"       // for IWMWriterSink

#include "AudioCoderWMA.h"

#include <cassert>
#include <exception>

#include "../nu/AutoLock.h"
#include "../nu/AutoWide.h"
#include "../Winamp/strutil.h"
#include "../Agave/Language/api_language.h"

/* TODO: implement 2-pass encoding via IWMWriterPreprocess */

int config_bitrate, config_samplerate, config_nch;
// New globals for encoder query

class CustomIndexStatus : public IWMStatusCallback
{
public:
	CustomIndexStatus( HANDLE _done ) : done(_done), IWMStatusCallback(), refcount(1)
	{}

	// IUnknown methods
public:
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++refcount;
	}


	virtual ULONG STDMETHODCALLTYPE Release()
	{
		// If we go to zero, who cares?
		return --refcount;
	}


	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
	{
		HRESULT hRetval = E_NOINTERFACE;
		if (IID_IWMStatusCallback == iid)
		{
			*ppvObject = static_cast<IWMStatusCallback *>(this);
			hRetval = S_OK;
		}
		else
		{
			*ppvObject = NULL;
		}
		return hRetval;
	}

	// IWMStatusCallback methods
public:
	HRESULT STDMETHODCALLTYPE OnStatus( WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE* pValue, void* pvContext )
	{
		switch ( Status )
		{
		case WMT_CLOSED:
			// You may want to deal with the HRESULT value passed with the status.
			// If you do, you should do it here.

			// Signal the event.
			SetEvent(done);
			break;
		}
		return S_OK;
	}

protected:
	ULONG refcount;
	HANDLE done;
};


// Our custom buffer object, used by the writer sink.

AudioCoderWMA::AudioCoderWMA(int numchannels, int samplerate, int bitspersamp, configtype *cfg, char *configfile) : AudioCoder()
{
	lastByteCount=0;
	writerAdvanced=0;
	begin_writing = false;
	error = WMA_NO_ERROR;
	sink = NULL;

	// Get globals from Winamp.ini config file
	config_bitrate = cfg->config_bitrate;
	config_samplerate = cfg->config_samplesSec;
	config_nch = cfg->config_nch;

	timeunits_per_byte = ( ( (10000000.0) / (double)samplerate ) / (double)numchannels ) / ( (double)bitspersamp / 8.0 );
	//char t[100] = {0};
	//wsprintf(t,"%d", timeunits_per_byte);
	//::MessageBox(NULL, t, t, MB_OK);
	input_bytecount = 0;

	HRESULT hr = CreateAndConfigureWriter(numchannels, samplerate, bitspersamp, configfile);

	if ( FAILED(hr) )
	{
		error = WMA_CANT_CREATE_WRITER;
	}
}

AudioCoderWMA::~AudioCoderWMA()
{
	if (writer)
	{
		if ( begin_writing )
		{
			begin_writing = false;
			writer->EndWriting();
		}
		writer->Release();
		writer = NULL;
	}
	if (writerAdvanced)
	{
		writerAdvanced->Release();
		writerAdvanced=0;
	}	
	if (sink)
	{
		sink->Release();
		sink=0;
	}
}

int AudioCoderWMA::GetLastError()
{
	return error;
}

void AudioCoderWMA::PrepareToFinish()
{
	// We don't want to kill the objects here, because there might still be data in the pipeline.
	if (writer && begin_writing)
	{
		begin_writing = false;
		// Tell WM that we're done giving it input data.

		writer->EndWriting();
		// TODO: do we have to wait for this to finish?
	}
}

void AudioCoderWMA::OnFinished(const wchar_t *wfname)
{
	//
	// Okay, here we need to go back and index the file we just wrote so it's seekable.
	//

	// From:  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmform/htm/toconfiguretheindexer.asp
	// And:   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmform/htm/toindexanasffile.asp
	IWMIndexer* pBaseIndexer = NULL;
	IWMIndexer2* pMyIndexer = NULL;

	// Create an indexer.
	WMCreateIndexer(&pBaseIndexer);

	// Retrieve an IWMIndexer2 interface pointer for the indexer just created.
	pBaseIndexer->QueryInterface(IID_IWMIndexer2, (void **) & pMyIndexer);

	// Release the base indexer.
	pBaseIndexer->Release();
	pBaseIndexer = NULL;

	// Configure the indexer to create a timecode-based index.
	pMyIndexer->Configure(0,                          // Stream Number, use all.
		WMT_IT_PRESENTATION_TIME,   // Indexer type.
		NULL,                       // Index interval, use default.
		NULL);                     // Index type, use default.

	// Create an event for asynchronous calls.
	HANDLE done = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Give that to the status object
	CustomIndexStatus status( done );

	// Start the indexer.
	pMyIndexer->StartIndexing(tempFilename, &status, NULL);

	// Wait for the indexer to finish.
	WaitForSingleObject(done, INFINITE);

	// Release the remaining interface.
	pMyIndexer->Release();
	pMyIndexer = NULL;

	// Cleanup
	CloseHandle( done );
	DeleteFileW(wfname);
	MoveFileW(tempFilename, wfname);

}

int AudioCoderWMA::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{	
	HRESULT hr = S_OK;
	int retval = 0; // number of bytes written into "out"
	*in_used = 0;   // number of bytes read from "in"
	if ( !framepos && !in_avail )
	{
		int x = 0;
		x++;
	}
	assert(writer);

	// Hopefully, at the end of the stream, we still get called with "Encode" until we return 0?
	if (in_avail)
	{
		// Allocate an INSSBuffer of the proper size to hold all the data.
		INSSBuffer* pSample = NULL;
		if (FAILED(writer->AllocateSample(in_avail, &pSample))) return -1;

		// Get its internal memory buffer
		DWORD newBufferLength;
		pSample->GetLength(&newBufferLength);
		assert(newBufferLength == in_avail);

		BYTE *pdwBuffer = NULL;
		if (FAILED(pSample->GetBuffer(&pdwBuffer))) return -1;

		memcpy(pdwBuffer, in, in_avail); // Send all the available bytes in the input buffer into the IWMWriter,

		pSample->SetLength(in_avail); // Tell the buffer object how much we used

		QWORD timeunits = (QWORD)( (double)input_bytecount * timeunits_per_byte ); // Compute the current timecode
		// And stuff it into the writer
		hr = writer->WriteSample(0, timeunits, 0, pSample);
		if (FAILED(hr))
		{
		}
		else
		{
			// Increment the bytecount to be able to calculate the next timecode
			input_bytecount += in_avail;
			// And tell the host we used up all the available input data.
			*in_used = in_avail;
		}
		// Release immediately
		pSample->Release();
	}

	WM_WRITER_STATISTICS stats;
	writerAdvanced->GetStatistics(0, &stats);
	retval = (int)(stats.qwByteCount - lastByteCount);
	retval = min(retval, out_avail);
	lastByteCount+=retval;
	memset(out, 0, retval); // so we don't write random memory to disk

	return retval;
}

HRESULT AudioCoderWMA::SelectAndLoadResampler(int numchannels, int samplerate, int bitspersamp)
{
	DWORD inCount = 0;
	BOOL success = false;

	//wsprintf(junk,"IN Chan=%d, SRate=%d, BPS=%d", numchannels, samplerate,bitspersamp);
	//MessageBox(NULL, junk, "INPUT FMT", MB_OK);
	// First get the number of input streams
	HRESULT hr = writer->GetInputCount(&inCount);
	if(!FAILED(hr)){
		//wsprintf(junk, "Input Count = %d", inCount);
		//MessageBox(NULL, junk, "DEBUG", MB_OK);
		// Now get the number of input formats we can resample for
		DWORD fmtCount = 0;
		hr = writer->GetInputFormatCount(0, &fmtCount);
		if(!FAILED(hr)){
			//wsprintf(junk, "Format Count = %d", fmtCount);
			//MessageBox(NULL, junk, "DEBUG", MB_OK);
			// Now cycle through and find the one that matches our input fmt
			for(size_t i = 0;i < fmtCount;i++){
				IWMInputMediaProps* pProps = NULL;
				hr = writer->GetInputFormat(0, (DWORD)i, &pProps);
				if(!FAILED(hr)){
					DWORD cbSize = 0;
					// Get the size of the media type structure.
					pProps->GetMediaType(NULL, &cbSize);
					// Allocate memory for the media type structure.
					WM_MEDIA_TYPE* pType = (WM_MEDIA_TYPE*) new BYTE[cbSize];
					if(pType != NULL){
						WAVEFORMATEX* pwave = NULL;
						// Get the media type structure.
						hr = pProps->GetMediaType(pType, &cbSize);
						// Check that the format data is present.
						if (pType->cbFormat >= sizeof(WAVEFORMATEX)){
							pwave = (WAVEFORMATEX*)pType->pbFormat;
							//wsprintf(junk, "Cnannels = %d, SPerSec = %d, AvgBPS = %d, BPS = %d BALIGN = %d",
							//	pwave->nChannels,
							//	pwave->nSamplesPerSec,
							//	pwave->nAvgBytesPerSec,
							//	pwave->wBitsPerSample,
							//	pwave->nBlockAlign);
							//MessageBox(NULL, junk, "DEBUG", MB_OK);
						}
						else{
							break;
						}
						// Try to match the channels/samplerate/and bits/samp
						if((pwave->nChannels == numchannels) && (pwave->nSamplesPerSec == samplerate) && (pwave->wBitsPerSample == bitspersamp)){
							writer->SetInputProps(0, pProps);
							success = true;
							break;
						}
					}
				}
			}
		}
	}
	if(success != 1){
		wchar_t junk[FILETITLE_SIZE] = {0};
		wsprintfW(junk,WASABI_API_LNGSTRINGW(IDS_CANNOT_FIND_INPUT_FORMATTER),
				  numchannels, samplerate,bitspersamp);
		MessageBoxW(NULL, junk, WASABI_API_LNGSTRINGW(IDS_WARNING), MB_OK);
	}
	if (success)
		return S_OK;
	else if (FAILED(hr)) // if we have an error code, return it
		return hr;
	else 
		return E_FAIL;
}


HRESULT AudioCoderWMA::CreateAndConfigureWriter(WORD numchannels, WORD samplerate, WORD bitspersamp, char *configfile)
{
	// First, create the writer.
	HRESULT hr = WMCreateWriter( NULL, &writer );
	if ( !FAILED(hr) )
	{
		// Create and Configure a stream profile with the given wave limits.
		WAVEFORMATEX WaveLimits =
		{
			WAVE_FORMAT_PCM,
				numchannels,
				samplerate,
				samplerate * numchannels * bitspersamp / (DWORD)8,
				numchannels * bitspersamp / (DWORD)8,
				bitspersamp,
				0
		};
		IWMProfile* pProfile = NULL;
		hr = CreateAndConfigureProfile(&WaveLimits, &pProfile, configfile);
		if ( !FAILED(hr) )
		{
			// Set the profile into the writer
			hr = writer->SetProfile( pProfile );
			if ( !FAILED(hr) )
			{
				// Go get the input resampler and load it to the profile
				hr = SelectAndLoadResampler(numchannels, samplerate, bitspersamp);
				if (!FAILED(hr))
				{
					wchar_t tempPath[MAX_PATH] = {0};
					GetTempPathW(MAX_PATH,tempPath);
					GetTempFileNameW(tempPath, L"wma", 0, tempFilename);

					// Make the custom data sink object
					WMCreateWriterFileSink(&sink);
					//sink = new CustomWMWriterSink;
					if ( sink )
					{
						sink->Open(tempFilename);
						HRESULT hr;
						// From MSDN: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmform/htm/addingsinkstothewriter.asp
						IWMWriterSink* pSinkBase = NULL;
						hr = writer->QueryInterface( IID_IWMWriterAdvanced, (void **) & writerAdvanced );
						if ( !FAILED(hr) )
						{
							hr = sink->QueryInterface( IID_IWMWriterSink, (void**) & pSinkBase );
							if ( !FAILED(hr) )
							{
								// Stuff the custom data sink into the writer.
								hr = writerAdvanced->AddSink(pSinkBase);
								if ( !FAILED(hr) )
								{
									// And let the writer initialize itself for output.
									hr = writer->BeginWriting();
									if ( !FAILED(hr) )
									{
										begin_writing = true;
									}
								}
								else
								{
									error = WMA_CANT_ADD_SINK;
								}
							}
							else
							{
								error = WMA_CANT_QUERY_SINK_INTERFACE;
							}
						}
						else
						{
							error = WMA_CANT_QUERY_WRITER_INTERFACE;
						}
					}
					else
					{
						error = WMA_CANT_MAKE_CUSTOM_SINK;
					}
				}
			}
		}
	}

	return hr;
}

HRESULT AudioCoderWMA::CreateAndConfigureProfile(WAVEFORMATEX* pWaveLimits, IWMProfile** ppProfile, char *configfile)
{
	IWMProfileManager* pProfileMgr = NULL;

	// Instantiate a profile manager object.
	HRESULT hr = WMCreateProfileManager(&pProfileMgr);
	if ( !FAILED(hr) )
	{
		/* SAVE
		// Create the empty profile.
		//hr = pProfileMgr->CreateEmptyProfile(WMT_VER_9_0, ppProfile);
		if ( !FAILED(hr) ){
		IWMCodecInfo3 *codecInfo = NULL;
		hr = pProfileMgr->QueryInterface(&codecInfo);
		if(!FAILED(hr)){
		// Find the proper IWMStreamConfig that matches the WAVEFORMATEX data.
		IWMStreamConfig* pStreamConfig = NULL;
		//hr = FindAudioFormat(WMMEDIASUBTYPE_WMAudioV2, pProfileMgr, pWaveLimits, config_bitrate * 1000, FALSE, &pStreamConfig);
		hr = codecInfo->GetCodecFormat(WMMEDIATYPE_Audio, config_encOffset, config_formatOffset, &pStreamConfig);
		if ( !FAILED(hr) ){
		// Config the stream.
		// hr = pStreamConfig->SetBitrate( config_bitrate );
		hr = pStreamConfig->SetConnectionName( L"enc_wma" );
		hr = pStreamConfig->SetStreamName( L"enc_wma" );
		hr = pStreamConfig->SetStreamNumber( 1 );

		// Stuff it into the profile
		hr = (*ppProfile)->AddStream( pStreamConfig );
		}
		}
		}
		*/
		if ( !FAILED(hr) ){
			// Load the .prx file into the writer
			if(configfile == NULL){
				hr = E_FAIL;
			}
			else{
				wchar_t cstring[4000] = {0};
				GetPrivateProfileStructW(L"audio_wma", L"profile", cstring, sizeof(cstring)/sizeof(*cstring), AutoWide(configfile));
				hr = pProfileMgr->LoadProfileByData(cstring, ppProfile);
				if(hr != S_OK){
					hr = E_FAIL;
				}
			}
		}
		pProfileMgr->Release();
	}
	return hr;
}