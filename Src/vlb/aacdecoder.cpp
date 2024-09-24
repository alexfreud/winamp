/* $Header: /cvs/root/winamp/vlb/aacdecoder.cpp,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: aacdecoder.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: decoder main object
 *
\***************************************************************************/

#include "aacdecoder.h"
#include "bitstream.h"
#include "bitsequence.h"
#include "channelinfo.h"

#include<stdio.h>
// // //

CAacDecoder::CAacDecoder (CDolbyBitStream& bs)
 : m_bs (bs), sce (bs), cpe (bs), lfe (bs), dse (bs),bIgnoreDolbyStream(false), bLookForDSEInfoStream(true), 
   bHasDSEInfoStream(false)
{
	m_BlockNumber = 0 ;
	m_SelectedProgram = 0 ;



}

CAacDecoder::~CAacDecoder ()
{

}

void CAacDecoder::SetEqualization (bool wantEQ, float Mask [])
{
  sce.SetEqualization (wantEQ, Mask) ;
  cpe.SetEqualization (wantEQ, Mask) ;
  lfe.SetEqualization (wantEQ, Mask) ;
}

void CAacDecoder::ReadFillElement (void)
{
  CVLBBitSequence count (4) ;
  if (count.Read (m_bs) == 15)
  {
    CVLBBitSequence esc_count (8) ;
    esc_count.Read (m_bs) ;

    count = esc_count + 14 ;
  }

  CVLBBitSequence fill_byte (8) ;
  for (int i = 0 ; i < count ; i++)
  {
    fill_byte.Read (m_bs) ;
  }
}
void CAacDecoder::ReadDolbyFillElement (void)
{
  CVLBBitSequence count (4) ;
  int iCountInBytes;
  int iCountInBits;

  if (count.Read (m_bs) == 15)
  {
    CVLBBitSequence esc_count (8) ;
    esc_count.Read (m_bs) ;

    count = esc_count + 14 ;
  }
  iCountInBytes=(int)count;
  iCountInBits = iCountInBytes * 8;

  CVLBBitSequence fill_byte (8) ;
  CVLBBitSequence fill_nibble(4);
  if(iCountInBytes){	  
	  fill_nibble.Read(m_bs);
	  if(bLookForDSEInfoStream && ((int)fill_nibble)==15){
		iCountInBits=spectralExtInfo(iCountInBytes,&sDSEInfo,&m_bs);

		// Debug: simulate a bitstream error. This will cause the 
		// MDCT record to be zeroed-out at higher frequencies prior to
		// the inverse transform.
//		iCountInBits = iCountInBytes*8 - 4;
//		sDSEInfo.iDolbyBitStreamWarning = 942;




		bHasDSEInfoStream=true;
	  }
	  else{

		// read out the rest of the byte; adjust bits read counter
		fill_nibble.Read(m_bs);
		iCountInBits -= 8;

	  }
  }

  // If there was an error in reading the SE bitstream, or if there are fill
  // bits left in the same fill element, then read out the rest of the Fill Element.

  while(iCountInBits > 8)
  {
	  fill_byte.Read (m_bs);
	  iCountInBits -= 8;
  }
  if (iCountInBits > 0 && iCountInBits <= 8)
  {
	  m_bs.Get(iCountInBits);
	  iCountInBits -= iCountInBits;
  }

}

void CAacDecoder::FrameInit(CStreamInfo &info)
{
  if (m_bs.IsAdifHeaderPresent ())
  {
    m_AdifHeader.Read (m_bs) ;

    info.SetBitRate (m_AdifHeader.GetBitRate ()) ;
	info.SetOriginalCopy(m_AdifHeader.GetOriginalCopy());
	info.SetHome(m_AdifHeader.GetHome());

	info.SetSamplingRateIndex(m_AdifHeader.GetProgramConfig(0).GetSamplingFrequencyIndex());
	info.SetSamplingRate(CChannelInfo::SamplingRateFromIndex(info.GetSamplingRateIndex ()));
	info.SetChannels(m_AdifHeader.GetProgramConfig(0).GetNumChannels());
	info.SetProfile(m_AdifHeader.GetProgramConfig(0).GetProfile());
  }
}


//MSV:
void CAacDecoder::InitDSEInfo(	CDolbyBitStream*poBS,
								CChannelElement *poChannelElement)
{
	// note that poChannelElement can be either poSingleChannel or poChannelPair.
	// depending on a mono or stereo vlb bitstream. We will deal with poChannelElement
	// (base class) calls in this function for either type of Channel Element, with the
	// exception of calling Get[Left|Right]Block() for ChannelPair Elements.
	int numChannels = poChannelElement->GetNumberOfChannels();
	int channelIndex;

	if (numChannels == 1 || numChannels == 2)
	{
		CBlock *poCBlock[2];
		CChannelInfo *poCChannelInfo;

		// This is garaunteed to set poCLongBlock to valid values,
		// since we've already checked the number of channels!

		// GetChannelInfo could be made a virtual function of the base class, thus
		// avoiding the need to specifically call different versions of GetChannelInfo()
		// below, but this would require moving more code to the Base Class, which is
		// unnecessarily complex. Note, however, that for CPE's, there poChannelInfo returns
		// a pointer to an array of 2 ChannelInfo objects.
		if (numChannels == 1)
		{
			poCBlock[0]=(CBlock*)( ((CSingleChannel*)poChannelElement)->GetBlock() );
			poCChannelInfo = ((CSingleChannel*)poChannelElement)->GetChannelInfo();

		} else { // num_channels == 2
			poCBlock[0]=(CBlock*)( ((CChannelPair*)poChannelElement)->GetLeftBlock()  );
			poCBlock[1]=(CBlock*)( ((CChannelPair*)poChannelElement)->GetRightBlock() );
			poCChannelInfo = ((CChannelPair*)poChannelElement)->GetChannelInfo();

		}

		//Fill Out Dolby Payload Structure:
		sDSEInfo.iChannels=numChannels;

		 // sampling rate is same for both channels; just use sr info from the left (0th) channel
		sDSEInfo.iSampleRateIndex=poCChannelInfo[0].GetSamplingIndex();
		sDSEInfo.iSampleRate=poCChannelInfo[0].GetSamplingFrequency();

		// all other information must be read for left and right channels
		for (channelIndex=0;channelIndex<numChannels;channelIndex++)
		{
			sDSEInfo.aiMaxSFB[channelIndex]=poCChannelInfo[channelIndex].GetScaleFactorBandsTransmitted();
			sDSEInfo.aiTotalSFB[channelIndex]=poCChannelInfo[channelIndex].GetScaleFactorBandsTotal();
			sDSEInfo.asDNSInfoStruct[channelIndex].psSectionInfoStruct=poCBlock[channelIndex]->GetSectionInfo();
			sDSEInfo.asDNSInfoStruct[channelIndex].iWindowSequence=poCChannelInfo[channelIndex].GetWindowSequence();
			sDSEInfo.asDNSInfoStruct[channelIndex].iGroupCount=poCChannelInfo[channelIndex].GetWindowGroups();
			sDSEInfo.asDNSInfoStruct[channelIndex].iLastBin=poCChannelInfo[channelIndex].GetLastBin();
			sDSEInfo.asDNSInfoStruct[channelIndex].iMaxSFB=poCChannelInfo[channelIndex].GetScaleFactorBandsTransmitted();
			sDSEInfo.asDNSInfoStruct[channelIndex].piBandOffsets=poCChannelInfo[channelIndex].GetScaleFactorBandOffsets();
			sDSEInfo.aiCopyStop[channelIndex]=poCChannelInfo[channelIndex].GetLastBin();

			sDSEInfo.iGroupCount[channelIndex]=poCChannelInfo[channelIndex].GetWindowGroups();
			for (int i=0;i<sDSEInfo.asDNSInfoStruct[channelIndex].iGroupCount;i++)
			{
				sDSEInfo.iGroupLength[channelIndex][i] = poCChannelInfo[channelIndex].GetWindowGroupLength(i);
				sDSEInfo.asDNSInfoStruct[channelIndex].iGroupLength[i] = poCChannelInfo[channelIndex].GetWindowGroupLength(i);
			}
		}
	}
	else{
		bLookForDSEInfoStream=false;
	}
	if(bIgnoreDolbyStream) bLookForDSEInfoStream=false;
}

#define ELEMENT_TYPE_SCE		0
#define ELEMENT_TYPE_CPE		1
#define ELEMENT_TYPE_IGNORE		3

void CAacDecoder::DecodeFrame (AudioIOControl *poAudioIO, CStreamInfo &info)
{
	int iRepeatCount = m_bs.GetNRDB() + 1;
	
	for(int n = 0; n < iRepeatCount; n++)
	{
		bool bHasElement;
		int  iElementType;
		
		m_SelectedProgram	  = 0;
		bLookForDSEInfoStream = true;
		bHasDSEInfoStream	  = false;
		
		bHasElement  = false;
		iElementType = ELEMENT_TYPE_IGNORE;
		// support Audio_Data_Interchange_Format header, if present

		CProgramConfig &pce = m_AdifHeader.GetProgramConfig(m_SelectedProgram);

		if ((m_BlockNumber == 0) && m_bs.IsAdifHeaderPresent())
		{
			info.SetSamplingRateIndex (pce.GetSamplingFrequencyIndex());
			info.SetProfile (pce.GetProfile());
		}


		info.SetChannels(0);
		info.SetSamplingRate (CChannelInfo::SamplingRateFromIndex(info.GetSamplingRateIndex ()));

		// // //

		CVLBBitSequence type (3), tag (4);

		m_bs.ByteAlign();

		while (type != CAacDecoder::ID_END)
		{
			type.Read(m_bs);
			switch (type)
			{
				case CAacDecoder::ID_SCE:
					if(bHasElement)
					{
						
						throw EUnimplemented();
					}

					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStart);

					tag.Read (m_bs);
					sce.Read (info);

					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStop);

					if(!bHasElement)
					{
						iElementType = ELEMENT_TYPE_SCE;
					}
					else
					{
						throw EUnimplemented();
						iElementType = ELEMENT_TYPE_IGNORE;
					}

					info.IncChannels(1);
					bHasElement = true;
				break ;
      
				case CAacDecoder::ID_CPE:
					if(bHasElement)
					{
						throw EUnimplemented();
					}

					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStart);

					tag.Read(m_bs);
					cpe.Read(info);

					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStop);

					if(!bHasElement)
					{
						iElementType = ELEMENT_TYPE_CPE;
					}
					else
					{
						throw EUnimplemented();
						iElementType = ELEMENT_TYPE_IGNORE;
					}

					info.IncChannels (2);
					bHasElement = true;
				break;
      
				case CAacDecoder::ID_CCE:
      
					throw EUnimplemented();
				break;
      
				case CAacDecoder::ID_LFE:
        
					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStart);

					tag.Read(m_bs);
					lfe.Read(info);

					m_bs.SetPositionMarker(CDolbyBitStream::ChannelElementStop);

					if (pce.AddChannel(tag,false))
					{
						throw EUnimplemented();
						info.IncChannels(1);
					}

					iElementType = ELEMENT_TYPE_IGNORE;
					bHasElement  = true;
				break;
      
				case CAacDecoder::ID_DSE:
      
					tag.Read(m_bs);
					dse.Read();
					break;
      
				case CAacDecoder::ID_PCE:
    
					m_AdifHeader.GetProgramConfig(tag.Read(m_bs)).Read(m_bs);
					break;
      
				case CAacDecoder::ID_FIL:
				
					switch(iElementType)
					{
						case ELEMENT_TYPE_SCE:
							InitDSEInfo(&m_bs,&sce);
						break;

						case ELEMENT_TYPE_CPE:
							InitDSEInfo(&m_bs,&cpe);
						break;
					}

					ReadDolbyFillElement();
				break;
      
				case CAacDecoder::ID_END:
     
				break;
			}
		}


// Check for main profile.  abort is bitstream is main profile

#ifndef MAIN_PROFILE
		if (info.GetProfile() == 0)
		{
			throw EIllegalProfile();
		}
#endif

		if(poAudioIO) //Only perform actual decode if we have a valid output buffer object pointer
		{
			switch(iElementType)
			{
				case ELEMENT_TYPE_SCE:
					if (pce.AddChannel(tag,false))
					{
						if(bHasDSEInfoStream)
						{
							sce.DecodeDolby(poAudioIO,&sDSEInfo,info);
						}
						else
						{
							sce.Decode(poAudioIO,info);
						}

					}
				break;

				case ELEMENT_TYPE_CPE:
					if (pce.AddChannel (tag,true))
					{

						if(bHasDSEInfoStream)
						{
							cpe.DecodeDolby(poAudioIO,&sDSEInfo,info);
						}
						else
						{
							cpe.Decode(poAudioIO,info,2);
						}

					}
				break;
			}
		}

		info.SetNumberOfFrontChannels (info.GetChannels ());
		info.SetChannelMask (Speaker_FrontLeft + Speaker_FrontRight);

		m_BlockNumber++;

		if(n && poAudioIO) m_bs.DecrementBlocks();//NRDB == N-1!
		
		m_bs.ByteAlign();
	}

	if (poAudioIO == NULL)
	{
		m_bs.SetFrameReadButNotDecoded();
	}
	else
	{
		m_bs.ClearFrameReadButNotDecoded();
	}
}