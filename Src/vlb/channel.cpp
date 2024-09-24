/* $Header: /cvs/root/winamp/vlb/channel.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: channel.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: independent channel stream object
 *
\***************************************************************************/

#include <new.h> // displacement new

#include "channel.h"
#include "bitstream.h"
#include "bitsequence.h"

// // // Base class for CSingleChannel, CChannelPair

CChannelElement::CChannelElement (CDolbyBitStream &bs)
 : m_bs (bs),
   m_GlobalGain (8)
{
  m_WantEqualizer = false ;
  m_WantSpectralData = false ;
}

CChannelElement::~CChannelElement ()
{}

CBlock *CChannelElement::ReadICS (CChannelInfo &ics_info, unsigned long *memory)
{
  // allocates and reads one individual_channel_stream 

  m_GlobalGain.Read (m_bs) ;

  if (!ics_info.IsValid ())
  {
    ics_info.Read (m_bs) ;
  }

  CBlock *block ;

  if (ics_info.IsLongBlock ())
  {
    block = new (static_cast<void *>(memory)) CLongBlock (ics_info) ;
  }
  else
  {
    block = new (static_cast<void *>(memory)) CShortBlock (ics_info) ;
  }

  block->Read (m_bs, m_GlobalGain) ;

  // 'block' points to the same location as 'memory', but keeping the
  // m_Block pointers as class members saves us a lot of typecasting
 
  return block ;
}

void CChannelElement::SetEqualization (bool wantEQ, float Mask [])
{
  m_WantEqualizer = wantEQ ;

  if (wantEQ)
  {
    for (int i = 0 ; i < CBlock::EqualizationMaskLength ; i++)
    {
      m_EqualizerMask [i] = Mask [i] ;
    }
  }
}

// // // CSingleChannel wraps single_channel_element()

CSingleChannel::CSingleChannel (CDolbyBitStream &theBitstream)
 : CChannelElement (theBitstream)
{

#ifdef MAIN_PROFILE
  m_Prediction.FullReset () ;
#endif

  poIMDCTObject=new IMDCTObject(4096,256);
  poNormalIMDCTObject=new IMDCTObject(2048,128);
  ppfData=new float*[1];
  ppshData=new short*[1];

  m_poAudioDSP = new AudioIODSP(1);
}

CSingleChannel::~CSingleChannel ()
{
	delete poIMDCTObject;
	delete poNormalIMDCTObject;
	delete[] ppfData;
	delete[] ppshData;
	delete m_poAudioDSP;
}

void CSingleChannel::Read (const CStreamInfo &si)
{
  m_IcsInfo.Reset (si) ;

  m_Block = ReadICS (m_IcsInfo, m_BlockMemory) ;
}

void CSingleChannel::Decode (AudioIOControl *poAudioIO, CStreamInfo &info, int stride /* = 1 */)
{

  if (m_IcsInfo.IsMainProfile ())
  {
#ifdef MAIN_PROFILE
    m_Prediction.Apply (m_IcsInfo, *m_Block) ;
#else
    throw EIllegalProfile();
#endif
  }


  m_Block->ApplyTools () ;

  if (m_WantEqualizer)
  {
    m_Block->ApplyEqualizationMask (m_EqualizerMask) ;
  }
  m_Block->FrequencyToTime_Fast (m_Previous) ;
  ppfData[0]=m_Block->AccessOutput();

  // ApplyLimiter is called to minimize distortion from clipping.  The limiter can only 
  // be applied to 128 samples at a time so there must be several calls to limit all 1024 
  // samples in a block.  This is due to the way the buffer allocation is done in the 
  // constructor of the AudioIODSP class.
  int iLength = 1024;
  int grpIndex = 0;
  short sThisLength = (iLength < DLYBUFSZ) ? iLength : DLYBUFSZ;

  m_poAudioDSP->SetSamplingRate(info.GetSamplingRate());
  for (int sSamp = 0; sSamp < iLength; sSamp += sThisLength)
  {
    m_poAudioDSP->ApplyLimiter(ppfData, sThisLength, grpIndex);
    sThisLength = ((iLength - sThisLength * grpIndex++) < DLYBUFSZ) ? 
 	   (iLength - sThisLength * grpIndex++) : DLYBUFSZ;
  }
  poAudioIO->IO(ppfData,1024);
  m_Block->~CBlock () ;
}

void CSingleChannel::DecodeDolby (	AudioIOControl *poAudioIO,
									DOLBY_PAYLOAD_STRUCT  *psDSEInfo,
									CStreamInfo &info,
									int stride /* = 1 */)
{
	int iWindowType;
	int iWindowShape;
	int i,j,grp_index,win_in_grp_index;
	int startingBinForThisWindow, startingBaseBandBin, halfWindowLength;
	int overall_window_number1, overall_window_number2;
	int firstWinThisGrp;
	float avgCopyEnergies[8][SE_MAX_NUM_BANDS];
	int iBandCounter;

	// previous window sequence
	// need to keep this variable around to handle legacy code which forces
	// spectral extend to rely on a special case for WindowSequence == 3

	static int previousWindowSequence = 0;

	// get pointer to spectral data
	// these calls really need to depend on the blocktype - 
	// short blocks require the window number (0-7) to be passed
	// to AccessSpectralData(); long blocks do not require this argument.

	ppfData[0]=m_Block->AccessSpectralData();

	// Apply DNS here.
	// Only apply DNS if we have valid DNS information read from the SE bitstream.
	// If there was a SE bitstream reading error of *any* kind, do not apply DNS.

	if (psDSEInfo->iDolbyBitStreamWarning == 0)
	{
		// applyDNS() applies to all window groups at the same time, so there is 
		// no need to call applyDNS() for each window group individually.
		applyDNS(ppfData[0],&psDSEInfo->asDNSInfoStruct[0],previousWindowSequence);
	}

#ifdef MAIN_PROFILE
	if (m_IcsInfo.IsMainProfile ())
	{
		m_Prediction.Apply (m_IcsInfo, *m_Block) ;
	}
#endif

	iWindowType=m_IcsInfo.GetWindowSequence();
	iWindowShape=m_IcsInfo.GetWindowShape();

	//Do TNS:
	m_Block->ApplyTools () ;

    // Spectral Extension.

	// for each window group...
	overall_window_number1 = 0;
	overall_window_number2 = 0;

	for(grp_index=0;grp_index<psDSEInfo->iGroupCount[0];grp_index++)
	{
		firstWinThisGrp = overall_window_number1;

		// for each window within the current window group...
		for(win_in_grp_index=0;
		    win_in_grp_index<psDSEInfo->iGroupLength[0][grp_index];
			win_in_grp_index++,overall_window_number1++)
		{

			// compute some parameters depedent on short/long blocktype, window group number, and transform length
			if (psDSEInfo->asDNSInfoStruct[0].iWindowSequence == 2)
			{
				// SHORT BLOCKS
				startingBinForThisWindow = overall_window_number1*256;
				startingBaseBandBin = 12;
				halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 256 : 128;
			} 
			else
			{
				// LONG, STOP, START blocks
				startingBinForThisWindow = 0;
#ifdef NEW_BUFFER_MODEL
				startingBaseBandBin = 96;
#else 
				startingBaseBandBin = 100;
#endif
				halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 2048 : 1024;
			}

			computeAvgCopyEnergies(&ppfData[0][startingBinForThisWindow],
									startingBaseBandBin,
									psDSEInfo->aiCopyStop[0],
									psDSEInfo->num_se_bands[0],
									psDSEInfo->seBands[0][grp_index],
									halfWindowLength,
									psDSEInfo->asDNSInfoStruct[0].iWindowSequence,
									avgCopyEnergies[overall_window_number1]);
		}/* win_in_grp_index */

		/* compute average energies for each group before passing into spectral extend */
		// accumulate energies for this group

		for(win_in_grp_index=1;
			win_in_grp_index<psDSEInfo->iGroupLength[0][grp_index];
			win_in_grp_index++)
		{
			for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[0];iBandCounter++)
			{
				avgCopyEnergies[firstWinThisGrp][iBandCounter] += avgCopyEnergies[firstWinThisGrp + win_in_grp_index][iBandCounter];
			}
		}

		// find the average energy for this group
		for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[0];iBandCounter++)
		{
			avgCopyEnergies[firstWinThisGrp][iBandCounter] /= psDSEInfo->iGroupLength[0][grp_index];
		}

		// copy this average energy to all values in the avgCopyEnergies array which correspond to windows in the current group
		for(win_in_grp_index=1;
			win_in_grp_index<psDSEInfo->iGroupLength[0][grp_index];
			win_in_grp_index++)
		{
			for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[0];iBandCounter++)
			{
				avgCopyEnergies[firstWinThisGrp + win_in_grp_index][iBandCounter] = avgCopyEnergies[firstWinThisGrp][iBandCounter];
			}
		}

		// for each window within the current window group...
		for (win_in_grp_index=0;
		     win_in_grp_index<psDSEInfo->iGroupLength[0][grp_index];
			 win_in_grp_index++,overall_window_number2++)
		{
			firstWinThisGrp = overall_window_number2;

			// check to see if dolby bitstream was read correctly.
			// If so, spectral extend all windows, whether we're dealing with a long block or short block
			if (psDSEInfo->iDolbyBitStreamWarning == 0) 
			{
				// compute some parameters dependent short/long blocktype, window group number, and transform length
				if (psDSEInfo->asDNSInfoStruct[0].iWindowSequence == 2) 
				{
					// SHORT blocks
					// The constant 256 is ugly here - but is equal to (2 * CShortBlock::MaximumBins).
					// This is the spacing of the mdct coefficients in the buffer ppfData[n], independent
					// of whether we're using a single or double length transform.
					startingBinForThisWindow = overall_window_number2*256;
					startingBaseBandBin = 12;
					halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 256 : 128;
				} 
				else
				{
					// LONG, STOP, START blocks
					startingBinForThisWindow = 0;
#ifdef NEW_BUFFER_MODEL
					startingBaseBandBin = 96;
#else 
					startingBaseBandBin = 100;
#endif
					halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 2048 : 1024;
				}

				spectralExtend(&ppfData[0][startingBinForThisWindow],
								startingBaseBandBin,
								psDSEInfo->aiCopyStop[0],
								psDSEInfo->sfm[0][grp_index],
								psDSEInfo->num_se_bands[0],
								psDSEInfo->seBands[0][grp_index],
								psDSEInfo->delta_power_values[0][grp_index],
								psDSEInfo->fdamp[0][grp_index],
								halfWindowLength,
								psDSEInfo->asDNSInfoStruct[0].iWindowSequence,
								psDSEInfo->iSEPowerResolution,
								previousWindowSequence,
								avgCopyEnergies[overall_window_number2]);

			} 
			else
			{
				// zero out the extension band of each window if there was a bitstream error.
				// be careful here- we may be dealing with many short blocks or a single long block

				if (psDSEInfo->asDNSInfoStruct[0].iWindowSequence == 2) 
				{
					// SHORT blocks
					for(i=0;i<8;i++) {
						for(j=psDSEInfo->aiCopyStop[0]; j<256; j++)
						{
							ppfData[0][i*256+j]=0.0f;
						}
					}
				} 
				else
				{
					// LONG, STOP, START blocks
					for (i = psDSEInfo->aiCopyStop[0]; i<2048; i++)
					{
						ppfData[0][i] = 0.0f;
					}
				}

			} // if psDSEInfo->iDolbyBitStreamWarning...

		} // for win_in_grp_index

	} // for grp_index

	/* Double all TCs if using a double length x-form so we do not lose 6dB
	 * after the imdct.
	 */

	if (psDSEInfo->iUsesDoubleLengthXForm)
	{
		for (i = 0; i < 2048; i++)
		{
			ppfData[0][i] *= 2.0f;
		}
	}

	// do the transform
	if (psDSEInfo->iUsesDoubleLengthXForm)
	{
		poIMDCTObject->Transform(ppfData[0],iWindowType,iWindowShape);

		// The limiter can only be applied to 128 samples at a time
		// so there must be several calls to limit all 2048 samples 
		// in a block.  This is due to the way the buffer allocation 
		// is done in the constructor of the AudioIODSP class
   	    int iLength = 2048;
 	    int grpIndex = 0;
	    int sThisLength = (iLength < DLYBUFSZ) ? iLength : DLYBUFSZ;

		m_poAudioDSP->SetSamplingRate(info.GetSamplingRate());
	    for (int sSamp = 0; sSamp < iLength; sSamp += sThisLength)
		{
		   // Limiter is applied to ppfData
		   m_poAudioDSP->ApplyLimiter(ppfData, sThisLength, grpIndex);
		   sThisLength = ((iLength - sThisLength * grpIndex++) < DLYBUFSZ) ? 
			   (iLength - sThisLength * grpIndex++) : DLYBUFSZ;
		}

		poAudioIO->IO(ppfData,2048);
	}
	else
	{
		poNormalIMDCTObject->Transform(ppfData[0],iWindowType,iWindowShape);

		// Apply Limiter so that distortion from clipping is minimized

		// The limiter can only be applied to 128 samples at a time
		// so there must be several calls to limit all 2048 samples 
		// in a block.  This is due to the way the buffer allocation 
		// is done in the constructor of the AudioIODSP class
   	    int iLength = 1024;
 	    int grpIndex = 0;
	    short sThisLength = (iLength < DLYBUFSZ) ? iLength : DLYBUFSZ;

        m_poAudioDSP->SetSamplingRate(info.GetSamplingRate());
	    for (int sSamp = 0; sSamp < iLength; sSamp += sThisLength)
		{
		   // Limiter is applied to ppfData
		   m_poAudioDSP->ApplyLimiter(ppfData, sThisLength, grpIndex);
		   sThisLength = ((iLength - sThisLength * grpIndex++) < DLYBUFSZ) ? 
			   (iLength - sThisLength * grpIndex++) : DLYBUFSZ;
		}

		poAudioIO->IO(ppfData,1024);
	}

	// update previousWindowSequence
	previousWindowSequence = psDSEInfo->asDNSInfoStruct[0].iWindowSequence;

	m_Block->~CBlock () ;
}

// // // CChannelPair wraps channel_pair_element()

CChannelPair::CChannelPair (CDolbyBitStream &theBitstream) 
 : CChannelElement (theBitstream),
   m_CommonWindow (1)
{

#ifdef MAIN_PROFILE
  m_Prediction [L].FullReset () ;
  m_Prediction [R].FullReset () ;
#endif

  ppoIMDCTObject = new IMDCTObject* [2];
  ppoNormalIMDCTObject = new IMDCTObject* [2];

  for (int n = 0; n < 2; n++)
  {  
	ppoIMDCTObject[n] = new IMDCTObject(4096,256);
	ppoNormalIMDCTObject[n] = new IMDCTObject(2048,128);
  }

  ppfData = new float*[2];
  ppshData = new short*[2];

  m_poAudioDSP = new AudioIODSP(2);
}

CChannelPair::~CChannelPair ()
{
    for (int n = 0; n < 2; n++)
    {
        delete ppoIMDCTObject[n];
        delete ppoNormalIMDCTObject[n];
    }
	delete[] ppoIMDCTObject;
	delete[] ppoNormalIMDCTObject;

    delete[] ppfData;
	delete[] ppshData;
	delete m_poAudioDSP;
}

void CChannelPair::Read (const CStreamInfo &si)
{
  m_IcsInfo [L].Reset (si) ;
  m_IcsInfo [R].Reset (si) ;

  if (m_CommonWindow.Read (m_bs))
  {
    m_IcsInfo [L].Read (m_bs) ;
    m_IcsInfo [R] = m_IcsInfo [L] ;

    m_JointStereo.Read (m_IcsInfo [L], m_bs) ;
  }

  m_Block [L] = ReadICS (m_IcsInfo [L], m_BlockMemory [L]) ;
  m_bs.SetPositionMarker (CDolbyBitStream::SecondIndividualChannelStart) ;
  m_Block [R] = ReadICS (m_IcsInfo [R], m_BlockMemory [R]) ;
}

void CChannelPair::Decode (AudioIOControl *poAudioIO, CStreamInfo &info, int stride /* = 1 */)
{

#ifdef MAIN_PROFILE
  // - apply prediction tool to left (coded) channel
  // - calculate right channel from intensity position
  // - apply prediction tool to right channel

  // unfortunately this breaks m_JointStereo::Apply() into two steps
#endif

  if (m_CommonWindow)
  {
    m_JointStereo.ApplyMS (m_IcsInfo [L], *m_Block [L], *m_Block [R]) ;
  }

#ifdef MAIN_PROFILE
  if (m_IcsInfo [L].IsMainProfile ())
  {
    m_Prediction [L].Apply (m_IcsInfo [L], *m_Block [L]) ;
  }
#endif

  if (m_CommonWindow)
  {
	  m_JointStereo.ApplyIS (m_IcsInfo [L], *m_Block [L], *m_Block [R]) ;
  }
  
#ifdef MAIN_PROFILE
  if (m_IcsInfo [R].IsMainProfile ())
  {
	  m_Prediction [R].Apply (m_IcsInfo [R], *m_Block [R]) ;
  }
#endif

  for (int channel = 0 ; channel < Channels ; channel++)
  {
    m_Block [channel]->ApplyTools () ;

    if (m_WantEqualizer)
    {
      m_Block [channel]->ApplyEqualizationMask (m_EqualizerMask) ;
    }
	m_Block [channel]->FrequencyToTime_Fast (m_Previous [channel]) ;
	ppfData[channel]=m_Block[channel]->AccessOutput();
  }

  // ApplyLimiter is called to minimize distortion from clipping.  The limiter can only 
  // be applied to 128 samples at a time so there must be several calls to limit all 
  // samples in a block.  This is due to the way the buffer allocation is done in the 
  // constructor of the AudioIODSP class.0
  int iLength = 1024;
  int grpIndex = 0;
  int sThisLength = (iLength < DLYBUFSZ) ? iLength : DLYBUFSZ;

  m_poAudioDSP->SetSamplingRate(info.GetSamplingRate());
  for (int sSamp = 0; sSamp < iLength; sSamp += sThisLength)
  {
	   m_poAudioDSP->ApplyLimiter(ppfData, sThisLength, grpIndex);
	   sThisLength = ((iLength - sThisLength * grpIndex++) < DLYBUFSZ) ? 
		   (iLength - sThisLength * grpIndex++) : DLYBUFSZ;
  }
  poAudioIO->IO(ppfData,1024);
  m_Block [L]->~CBlock () ;
  m_Block [R]->~CBlock () ;
}

void CChannelPair::DecodeDolby (AudioIOControl *poAudioIO,
								DOLBY_PAYLOAD_STRUCT  *psDSEInfo,
								CStreamInfo &info,
								int stride /* = 1 */)
{
	int channel, win_in_grp_index;
	int iWindowType;
	int iWindowShape;
	int i,j,grp_index;

	int startingBinForThisWindow;
	int startingBaseBandBin;
	int halfWindowLength;
	int overall_window_number1, overall_window_number2;
	float avgCopyEnergies[8][SE_MAX_NUM_BANDS];
	int iBandCounter;
	int firstWinThisGrp;

	// need to keep this variable around to handle legacy code which forces
	// spectral extend to rely on a special case for WindowSequence == 3

	static int previousWindowSequence[2] = {0,0};

  // apply DNS to each channel
  // these calls really need to depend on the blocktype - 
  // short blocks require the window number (0-7) to be passed
  // to AccessSpectralData(); long blocks do not require this argument.
  ppfData[L]=m_Block[L]->AccessSpectralData();
  ppfData[R]=m_Block[R]->AccessSpectralData();

  // Apply DNS here
  // Only apply DNS if we have valid DNS information read from the SE bitstream.
  // If there was a SE bitstream reading error of *any* kind, do not apply DNS


  if (psDSEInfo->iDolbyBitStreamWarning == 0)
  {
	// applyDNS() applies to all window groups at the same time, so there is 
	// no need to call applyDNS() for each window group individually.
		applyDNS(ppfData[L],&psDSEInfo->asDNSInfoStruct[L],previousWindowSequence[L]);
		applyDNS(ppfData[R],&psDSEInfo->asDNSInfoStruct[R],previousWindowSequence[R]);
  }


#ifdef MAIN_PROFILE
  // - apply prediction tool to left (coded) channel
  // - calculate right channel from intensity position
  // - apply prediction tool to right channel

  // unfortunately this breaks m_JointStereo::Apply() into two steps
#endif

  if (m_CommonWindow)
  {
    m_JointStereo.ApplyMS (m_IcsInfo [L], *m_Block [L], *m_Block [R]) ;
  }

#ifdef MAIN_PROFILE
  if (m_IcsInfo [L].IsMainProfile ())
  {
    m_Prediction [L].Apply (m_IcsInfo [L], *m_Block [L]) ;
  }
#endif

  if (m_CommonWindow)
  {
	m_JointStereo.ApplyIS (m_IcsInfo [L], *m_Block [L], *m_Block [R]) ;
  }

#ifdef MAIN_PROFILE
  if (m_IcsInfo [R].IsMainProfile ())
  {
    m_Prediction [R].Apply (m_IcsInfo [R], *m_Block [R]) ;
  }
#endif

  for (channel = 0 ; channel < Channels ; channel++)
  {
	iWindowType=m_IcsInfo[channel].GetWindowSequence();
	iWindowShape=m_IcsInfo[channel].GetWindowShape();
	ppfData[channel]=m_Block[channel]->AccessSpectralData();

	// Apply TNS
    m_Block [channel]->ApplyTools () ;

	// Spectral Extension for Stereo Goes Here!

	// for each window group...
	overall_window_number1 = 0;
	overall_window_number2 = 0;

	for (grp_index=0;grp_index<psDSEInfo->iGroupCount[channel];grp_index++)
	{
		firstWinThisGrp = overall_window_number1;

		// another window_in_group loop: compute average energy in this group for all bands in the current group
		for (win_in_grp_index=0;
		     win_in_grp_index<psDSEInfo->iGroupLength[channel][grp_index];
			 win_in_grp_index++,overall_window_number1++)
		{

			// compute some parameters dependent short/long blocktype, window group number, and transform length
			if (psDSEInfo->asDNSInfoStruct[channel].iWindowSequence == 2) 
			{
				// SHORT blocks
				// The constant 256 is ugly here - but is equal to (2 * CShortBlock::MaximumBins).
				// This is the spacing of the mdct coefficients in the buffer ppfData[n], independent
				// of whether we're using a single or double length transform.
				startingBinForThisWindow = overall_window_number1*256;
				startingBaseBandBin = 12;
				halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 256 : 128;
			} 
			else
			{
				// LONG, STOP, START blocks
				startingBinForThisWindow = 0;
#ifdef NEW_BUFFER_MODEL
				startingBaseBandBin = 96;
#else 
				startingBaseBandBin = 100;
#endif
				halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 2048 : 1024;
			}

			computeAvgCopyEnergies(&ppfData[channel][startingBinForThisWindow],
									startingBaseBandBin,
									psDSEInfo->aiCopyStop[channel],
									psDSEInfo->num_se_bands[channel],
									psDSEInfo->seBands[channel][grp_index],
									halfWindowLength,
									psDSEInfo->asDNSInfoStruct[channel].iWindowSequence,
									avgCopyEnergies[overall_window_number1]);

		} /* win_in_grp_index */

		/* compute average energies for each group before passing into spectral extend */
		// accumulate energies for this group

		for(win_in_grp_index=1;
			win_in_grp_index<psDSEInfo->iGroupLength[channel][grp_index];
			win_in_grp_index++)
		{
			for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[channel];iBandCounter++)
			{
				avgCopyEnergies[firstWinThisGrp][iBandCounter] += avgCopyEnergies[firstWinThisGrp + win_in_grp_index][iBandCounter];
			}
		}

		// find the average energy for this group
		for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[channel];iBandCounter++)
		{
			avgCopyEnergies[firstWinThisGrp][iBandCounter] /= psDSEInfo->iGroupLength[channel][grp_index];
		}

		// copy this average energy to all values in the avgCopyEnergies array which correspond to windows in the current group
		for(win_in_grp_index=1;
			win_in_grp_index<psDSEInfo->iGroupLength[channel][grp_index];
			win_in_grp_index++)
		{
			for(iBandCounter=0;iBandCounter<psDSEInfo->num_se_bands[channel];iBandCounter++)
			{
				avgCopyEnergies[firstWinThisGrp + win_in_grp_index][iBandCounter] = avgCopyEnergies[firstWinThisGrp][iBandCounter];
			}
		}

		// for each window within the current window group...
		for (win_in_grp_index=0;
		     win_in_grp_index<psDSEInfo->iGroupLength[channel][grp_index];
			 win_in_grp_index++,overall_window_number2++)
		{
			firstWinThisGrp = overall_window_number2;

			// check to see if dolby bitstream was read correctly.
			// If so, spectral extend all windows, whether we're dealing with a long block or short block
			if (psDSEInfo->iDolbyBitStreamWarning == 0) 
			{
				// compute some parameters dependent short/long blocktype, window group number, and transform length
				if (psDSEInfo->asDNSInfoStruct[channel].iWindowSequence == 2) 
				{
					// SHORT blocks
					// The constant 256 is ugly here - but is equal to (2 * CShortBlock::MaximumBins).
					// This is the spacing of the mdct coefficients in the buffer ppfData[n], independent
					// of whether we're using a single or double length transform.
					startingBinForThisWindow = overall_window_number2*256;
					startingBaseBandBin = 12;
					halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 256 : 128;
				} 
				else
				{
					// LONG, STOP, START blocks
					startingBinForThisWindow = 0;
#ifdef NEW_BUFFER_MODEL
					startingBaseBandBin = 96;
#else 
					startingBaseBandBin = 100;
#endif
					halfWindowLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 2048 : 1024;
				}

				spectralExtend (&ppfData[channel][startingBinForThisWindow],
								startingBaseBandBin,
								psDSEInfo->aiCopyStop[channel],
								psDSEInfo->sfm[channel][grp_index],
								psDSEInfo->num_se_bands[channel],
								psDSEInfo->seBands[channel][grp_index],
								psDSEInfo->delta_power_values[channel][grp_index],
								psDSEInfo->fdamp[channel][grp_index],
								halfWindowLength,
								psDSEInfo->asDNSInfoStruct[channel].iWindowSequence,
								psDSEInfo->iSEPowerResolution,
								previousWindowSequence[channel],
								avgCopyEnergies[overall_window_number2]);

			}
			else
			{
				// zero out the extension band of each window if there was a bitstream error.
				// be careful here- we may be dealing with many short blocks or a single long block

				if (psDSEInfo->asDNSInfoStruct[channel].iWindowSequence == 2) 
				{
					// SHORT blocks
					for(i=0;i<8;i++) {
						for(j=psDSEInfo->aiCopyStop[channel]; j<256; j++)
						{
							ppfData[channel][i*256+j]=0.0f;
						}
					}
				} 
				else
				{
					// LONG, STOP, START blocks
					for (i = psDSEInfo->aiCopyStop[channel]; i<2048; i++)
					{
						ppfData[channel][i] = 0.0f;
					}
				}

			} // if psDSEInfo->iDolbyBitStreamWarning...

		} // for win_in_grp_index...

	} // for grp_index...

	/* Double all TCs if using a double length x-form so we do not lose 6dB
	 * after the imdct.
	 */
	if (psDSEInfo->iUsesDoubleLengthXForm)
	{
		for (i = 0; i < 2048; i++)
		{
			ppfData[channel][i] *= 2.0f;
		}
	}

	// do the transform
	if (psDSEInfo->iUsesDoubleLengthXForm)
	{
		ppoIMDCTObject[channel]->Transform(ppfData[channel],iWindowType,iWindowShape);
	}
	else
	{
		ppoNormalIMDCTObject[channel]->Transform(ppfData[channel],iWindowType,iWindowShape);
	}

  } // for channel

  int iLength = (psDSEInfo->iUsesDoubleLengthXForm) ? 2048 : 1024;
  int grpIndex = 0;
  int sThisLength = (iLength < DLYBUFSZ) ? iLength : DLYBUFSZ;

  m_poAudioDSP->SetSamplingRate(info.GetSamplingRate());
  for (int sSamp = 0; sSamp < iLength; sSamp += sThisLength)
  {
	   m_poAudioDSP->ApplyLimiter(ppfData, sThisLength, grpIndex);
	   sThisLength = ((iLength - sThisLength * grpIndex++) < DLYBUFSZ) ? 
		   (iLength - sThisLength * grpIndex++) : DLYBUFSZ;
  }

  poAudioIO->IO(ppfData, iLength);

  // update previousWindowSequence
  previousWindowSequence[L] = psDSEInfo->asDNSInfoStruct[L].iWindowSequence;
  previousWindowSequence[R] = psDSEInfo->asDNSInfoStruct[R].iWindowSequence;

  m_Block [L]->~CBlock () ;
  m_Block [R]->~CBlock () ;
}