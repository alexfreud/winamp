/* $Header: /cvs/root/winamp/vlb/aacdecoderapi.cpp,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: aacdecode.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: AAC decoder API 
 *
\***************************************************************************/

#include "aacdecoderapi.h"
#include "aacdecoder.h"
#include "plainfile.h"
#include "bitbuffer.h"
#include "streaminfo.h"
#include "DataIO.h"

// sample frequency table
// converts sample frequency index to actual sample frequency
const unsigned int CAacDecoderApi::sfTab[16] = 
{
	96000,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000,
	0,		// reserved
	0,		// reserved
	0,		// reserved
	0		// reserved
};

DECLARE_EXCEPTION(ESyncError, AAC_SYNCERROR, "Synchronization Error!") ;


CAacDecoderApi::CAacDecoderApi (DataIOControl *paacInput) :
	cbValid(0),
	cbActual(0),
	decoder(NULL),
	input(NULL),
	buffer(NULL),
	info(NULL)
{	
	info = new CStreamInfo;
	
	input = new CPlainFile(paacInput);
	
	buffer = new CDolbyBitBuffer;
	
	decoder = input->IsAdifHeaderPresent () ? new CAacDecoder (*input) : new CAacDecoder (*buffer);

	decoder->FrameInit(*info);
}


CAacDecoderApi::~CAacDecoderApi ()
{
	delete decoder;
	delete input;
	delete buffer;
	delete info;
}


long CAacDecoderApi::Synchronize(AACStreamParameters *pAACStreamParameters)
{	
	long lReturn	  = ERR_SYNC_ERROR;
	int	 iResyncCount = 5;
	
	if (input->IsAdifHeaderPresent ())
	{
		/* Cannot resync to an ADIF stream! */
		lReturn = ERR_NO_ERROR;
	}
	else
	{

#if defined (_DEBUG)
//		cout << "Resynchronizing" << endl;
#endif

        buffer->ClearBytesSkipped();
		while(lReturn != ERR_NO_ERROR && lReturn != ERR_END_OF_FILE && iResyncCount)
		{

			//Reset the block count:
			buffer->ResetBlocks();

			try
			{
				if (!cbValid)
				{
					cbValid = input->Read (readbuf, cbSize);
	
					if (!cbValid) 
						buffer->SetEOF();
					else 
						cbActual = cbValid;
				}
	
				buffer->Feed(readbuf, cbActual, cbValid);
				
				if (buffer->IsDecodableFrame(*info))
				{
					lReturn = ERR_NO_ERROR;
				}
				else
				{
					throw ESyncError();
				}
	
				//Try to decode the first Raw Data Block to Get Dolby Info:		
				unsigned int uiValidBits1;
				unsigned int uiValidBits2;
				unsigned int uiBitsUsed;
	
				uiValidBits1 = buffer->GetBitState();
				
				decoder->DecodeFrame(NULL, *info);  //this is only for ADTS!
				
				uiValidBits2 = buffer->GetBitState();
				uiBitsUsed   = uiValidBits1 - uiValidBits2;
	
				buffer->PushBack((int)uiBitsUsed);
	
				if (!info->GetProtectionAbsent())
				{
					unsigned int CRCsyndrome;
					buffer->IsCrcConsistent(&CRCsyndrome);
				}
			}
			catch(CAacException& e)
			{
				/* If an exception was thrown, we have to clear this flag so that the next 
				* call to IsDecodableFrame() will not simply return 'true' without actually 
				* sync'ing and parsing the header.
				*/
				buffer->ClearFrameReadButNotDecoded();
	
				lReturn = TranslateException(e);
				iResyncCount--;
			}
			catch (...)
			{	
				/* All exceptions thrown should be derived from CAacException and therefore
				* should be caught above.  Just in case, though we'll add this catch() statement 
				* to catch things like access violations, etc.
				*/
				lReturn = ERR_UNEXPECTED_ERROR;
				break;
			}
		}
	}

	if (!lReturn)
	{
		if (FillStreamParameters(pAACStreamParameters))
		{
			lReturn = WARN_STREAM_PARAM_CHANGE;
		}

        /* frame_length is used here to pass up the # of bytes 
         * processed.  This is actually the # of bytes skipped
         * during the Synchronization() call. 
         */
        pAACStreamParameters->frame_length = buffer->GetBytesSkipped();

        /* Clear the number of bytes skipped so that bytes_skipped
         * no longer accumulates across calls to IsDecodableFrame()
         */
        buffer->ClearBytesSkipped();
	}
	return lReturn;
}

long CAacDecoderApi::DecodeFrame (AudioIOControl	  *poAudioIO, 
								  AACStreamParameters *pAACStreamParameters)
{	
	long lReturn = ERR_NO_ERROR;

    buffer->ClearBytesSkipped();
	try
	{
		if (!input->IsAdifHeaderPresent ())
		{
			if (!cbValid)
			{
				cbValid = input->Read (readbuf, cbSize);

				if (!cbValid) 
					buffer->SetEOF();
				else 
					cbActual = cbValid;
			}

			//
			// feed up to 'cbActual' bytes into decoder buffer
			//

			buffer->Feed(readbuf, cbActual, cbValid);

			//
			// sync
			//
		
			if (!buffer->IsDecodableFrame(*info))
			{
				lReturn = ERR_INVALID_BITSTREAM;
			}

			/* Parse the entire frame so that CRC can be verified *before* performing the decode. */		
			unsigned int uiValidBits1;
			unsigned int uiValidBits2;
			unsigned int uiBitsUsed;

			uiValidBits1 = buffer->GetBitState();
			
			/* (NULL == parse frame only) */
			decoder->DecodeFrame(NULL, *info);  //
			
			uiValidBits2 = buffer->GetBitState();
			uiBitsUsed   = uiValidBits1 - uiValidBits2;

			/* This function performs a final sanity check for frame synchronization.
			 * It will read in the next 15 bits looking for the 0xFFF frame
			 * sync word, id, and layer.  If it does not find a legal sync, it will throw an
			 * ESyncError() exception.
			 */
			VerifySync();

			/* Push back entire frame (this does not include the header) before calling DecodeFrame() a second
			 * time to actually decode.
			 */
			buffer->PushBack((int)uiBitsUsed);

			/* Verify CRC prior to actual decode. */
			if (!info->GetProtectionAbsent())
			{
				unsigned int CRCsyndrome;
				
				/* If the CRC is invalid, this function will throw an ECRCError() */
				buffer->IsCrcConsistent(&CRCsyndrome);
			}
		}

		/* Perform actual decode. */
		decoder->DecodeFrame(poAudioIO, *info);

		if (FillStreamParameters(pAACStreamParameters))
		{
			/* Output parameter (Fs, NumChans, etc.) has changed */
			lReturn = WARN_STREAM_PARAM_CHANGE;
		}
	}
	catch (CAacException& e)
	{
		/* Map the exception to a return code. */
		lReturn = TranslateException(e);

        /* frame_length is used here to pass up the # of bytes 
         * processed.  An error has occurred so no bytes were 
         * processed.  CAacDecoderAPI::Synchronication() will 
         * be called next to search for next syncword and return
         * the number of bytes processed (i.e. bytes skipped). 
         */
        pAACStreamParameters->frame_length = 0;
	}
	catch (...)
	{	
		/* All exceptions thrown should be derived from CAacException and therefore
		 * should be caught above.  Just in case, though we'll add this catch() statement 
		 * to catch things like access violations, etc.
		 */
		lReturn = ERR_UNEXPECTED_ERROR;

        /* frame_length is used here to pass up the # of bytes 
         * processed.  An error has occurred so no bytes were 
         * processed.  CAacDecoderAPI::Synchronication() will 
         * be called next to search for next syncword and return
         * the number of bytes processed (i.e. bytes skipped). 
         */
        pAACStreamParameters->frame_length = 0;
	}

	return lReturn;
}

/********** PRIVATE METHODS ***********/

long CAacDecoderApi::TranslateException(CAacException& e)
{
	long lReturn = ERR_SUBROUTINE_ERROR; /* Default to generic subroutine error. */

	static const int ErrorMap[][2] = 
	{
		{AAC_UNIMPLEMENTED,			ERR_SUBROUTINE_ERROR    },
		{AAC_NOTADIFHEADER,			ERR_INVALID_ADIF_HEADER },
		{AAC_DOESNOTEXIST,			ERR_INVALID_BITSTREAM   },
		{AAC_ENDOFSTREAM,			ERR_END_OF_FILE		    },
		{AAC_SYNCERROR,				ERR_SYNC_ERROR		    },
		{AAC_CRCERROR,				WARN_CRC_FAILED		    },
		{AAC_INPUT_BUFFER_EMPTY,	ERR_SYNC_ERROR			},
		{AAC_INVALIDCODEBOOK,		ERR_SUBROUTINE_ERROR    },

#ifdef MAIN_PROFILE
		{AAC_INVALIDPREDICTORRESET,	ERR_SUBROUTINE_ERROR    },
#endif

		{AAC_UNSUPPORTEDWINDOWSHAPE,ERR_SUBROUTINE_ERROR    },
		{AAC_DOLBY_NOT_SUPPORTED,	ERR_SUBROUTINE_ERROR    },
		{AAC_ILLEGAL_PROFILE,		ERR_ILLEGAL_PROFILE		}
	};

#if defined (_DEBUG)
//	cout << e.Explain() << endl;
#endif

	for (int i = 0; i < sizeof(ErrorMap) / (2 * sizeof(int)); i++)
	{
		if (e.What() == ErrorMap[i][0])
		{
			lReturn = ErrorMap[i][1];
			break;
		}
	}

	return lReturn;
}


void CAacDecoderApi::VerifySync(void)
{
	/* This function performs a final sanity check for frame synchronization.
	 * It will read in the next 16 bits looking for one of two things:
	 *
	 * 1) The 0xFFF frame sync word plus 1-bit ID and 2-bit layer.  
	 * 2) 0x5249 (RI) of a RIFF header (in the case of concatenated RIFF+ADTS files).
	 *
	 * If it does not find a legal sync, it will throw an ESyncError() exception.  
	 * Either way, all bits read will be pushed back.
	 */

	int nSyncWord = buffer->Get(16);
	
	buffer->PushBack(16);
	
	/* When looking for ADTS sync, examine only the 15 MS bits because the LSB is the 
	 * 'protection absent' flag. 
	 */
	if (!buffer->EndOf() && (nSyncWord & 0xFFFE) != 0xFFF8 && (nSyncWord != 0x5249))
	{
		throw ESyncError();
	}	
}

bool CAacDecoderApi::FillStreamParameters(AACStreamParameters *pAACStreamParameters)
{
	bool	   bParamChanged	    = false; //Assume no audio parameter change.
	static int nPreviousFs			= 0;
	static int nPreviousNumChannels	= 0;

	if (input->IsAdifHeaderPresent ())
	{
		pAACStreamParameters->stream_format = ADIF;
	}
	else if (input->IsRiffHeaderPresent ())
	{
		pAACStreamParameters->stream_format = RIFFADTS;
	}
	else
	{
		pAACStreamParameters->stream_format = ADTS;
	}

	if(decoder->HasDolbySpectralExtension() && decoder->HasDoubleLengthXForm()) 
	{
		// When UseDblLengthXfrm is set, the function info->GetSamplingRate
		// will double the sampling rate that it returns.  Therefore, 
		// pAACStreamParameters->sampling_frequency gets set to the correct
		// sampling rate in the code line below this one.
		info->SetUseDblLengthXfrm(true);
	}
	else
	{
		info->SetUseDblLengthXfrm(false);
	}

	pAACStreamParameters->sampling_frequency = info->GetSamplingRate();
	pAACStreamParameters->num_channels		 = info->GetChannels();
	pAACStreamParameters->bitrate			 = info->GetBitRate();

   /* frame_length is used here to pass up the # of bytes 
    * processed.  bytes_skipped needs to be added in here
    * to account for concatenated RIFF+ADTS files.
    */
    pAACStreamParameters->frame_length	     = info->GetFrameLength() + buffer->GetBytesSkipped();
	pAACStreamParameters->protection_absent	 = info->GetProtectionAbsent();
	pAACStreamParameters->copyright			 = info->GetOriginalCopy();	
	pAACStreamParameters->original_copy		 = info->GetHome();


	if (nPreviousFs && nPreviousNumChannels)
	{
		if (pAACStreamParameters->sampling_frequency != nPreviousFs)
		{
			bParamChanged = true;
		}

		if (pAACStreamParameters->num_channels != nPreviousNumChannels)
		{
			bParamChanged = true;

			/* Reset the number of channels to 0 so that AacDecoder::DecodeFrame() can
			 * re-add them as it parses the new bitstream.
			 */
			decoder->GetAdifHeader()->GetProgramConfig(0).ResetNonMCConfig();
		}
	}

	nPreviousFs			 = pAACStreamParameters->sampling_frequency;
	nPreviousNumChannels = pAACStreamParameters->num_channels;

	return bParamChanged;
}



