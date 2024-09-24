/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "../Agave/DecodeFile/api_decodefile.h"

#include "./BurnManager.h"

BurnManager::BurnManager(void) : decodeFile(NULL), context(NULL)
{
}

BurnManager::~BurnManager()
{
}

void BurnManager::SetDecodeAPI(api_decodefile *decoderAPI) 
{
	decodeFile = decoderAPI;
}

api_decodefile *BurnManager::GetDecodeAPI(void)
{
	return decodeFile;
}
void BurnManager::SetFiles(size_t numFiles, const wchar_t **filenames, BurnManagerCallback *callback)
{
	WRESULT *results = new WRESULT[numFiles];
	memset(results, 0, numFiles * sizeof(WRESULT));
	callback->OnLicenseCallback(numFiles, results);
	delete [] results;
}
	
ifc_audiostream* BurnManager::CreateDecoder(const wchar_t *filename)
{
	AudioParameters parameters;
	parameters.bitsPerSample = 16;
	parameters.channels = 2;
	parameters.sampleRate = 44100;

	ifc_audiostream *decoder = decodeFile->OpenAudio(filename, &parameters);
	if (decoder && (parameters.bitsPerSample != 16 || parameters.channels != 2 || parameters.sampleRate != 44100))
	{
		parameters.errorCode = API_DECODEFILE_BAD_RESAMPLE;
		decodeFile->CloseAudio(decoder);
		decoder=0;
	}
	return decoder;
}

void BurnManager::CloseDecoder(ifc_audiostream *decoder)
{
	decodeFile->CloseAudio(decoder);
}

void BurnManager::CancelBurn()
{
}

void BurnManager::BurnFinished()
{
}

