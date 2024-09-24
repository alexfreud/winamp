#pragma once 

#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "../Agave/DecodeFile/api_decodefile.h"

enum
{
    BURN_OK = 0,  // OK to burn
		BURN_GENERAL_FAILURE = 1, // can't burn, not 100% sure why
    BURN_FILE_NOT_FOUND = 2,  // file doesn't exist
    BURN_DRM_NO_LICENSE = 3,  // user doesn't have a license to open this DRM protected track
    BURN_DRM_NOT_ALLOWED = 4,  // DRM license disallows burning
		BURN_DRM_BURN_COUNT_EXCEEDED= 5,  // user has done too many burns already
		BURN_NO_DECODER=6, // no decoder was found to decompress this file
};
typedef unsigned __int32 WRESULT;

class BurnManagerCallback
{
public:
	virtual void OnLicenseCallback(size_t numFiles, WRESULT *results) = 0;
};

class BurnManager
{
public:
	BurnManager();
	~BurnManager();

public:
	void SetDecodeAPI(api_decodefile *decoderAPI);
	api_decodefile *GetDecodeAPI(void);
	void SetFiles(size_t numFiles, const wchar_t **filenames, BurnManagerCallback *callback);
	ifc_audiostream *CreateDecoder(const wchar_t *filename);
	void CloseDecoder(ifc_audiostream *decoder);
	void CancelBurn();
	void BurnFinished();

private:
	api_decodefile *decodeFile;
	void *context; // pImpl (pointer to implementation)
};