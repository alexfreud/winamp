/* $Header: /cvs/root/winamp/vlb/bitbuffer.cpp,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: bitbuffer.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: memory input class with transport format
 *
\***************************************************************************/

#include "bitbuffer.h"
#include "streaminfo.h"

static const unsigned int Adts_Value_SyncWord        = 0xFFF ;
static const unsigned int Adts_Value_SyncMSByte      = 0xFF ;
static const unsigned int Adts_Value_MinHeaderLength = 56 ;

static const unsigned int Adts_Length_SyncWord = 12 ;
static const unsigned int Adts_Length_Id = 1 ;
static const unsigned int Adts_Length_Layer = 2 ;
static const unsigned int Adts_Length_ProtectionAbsent = 1 ;
static const unsigned int Adts_Length_Profile = 2 ;
static const unsigned int Adts_Length_SamplingFrequencyIndex = 4 ;
static const unsigned int Adts_Length_PrivateBit = 1 ;
static const unsigned int Adts_Length_ChannelConfiguration = 3 ;
static const unsigned int Adts_Length_OriginalCopy = 1 ;
static const unsigned int Adts_Length_Home = 1 ;

static const unsigned int Adts_Length_CopyrightIdentificationBit = 1 ;
static const unsigned int Adts_Length_CopyrightIdentificationStart = 1 ;
static const unsigned int Adts_Length_FrameLength = 13 ;
static const unsigned int Adts_Length_BufferFullness = 11 ;
static const unsigned int Adts_Length_NumberOfRawDataBlocksInFrame = 2 ;

static const unsigned int Adts_Length_CrcCheck = 16 ;

static  char copyright_id[COPYRIGHT_SIZE];
static  char copyright_id_temp[COPYRIGHT_SIZE];
static	unsigned int cid_bitcount;
static	unsigned int cid_bytecount;

DECLARE_EXCEPTION(ECRCError,		 AAC_CRCERROR,			 "CRC calculation failed");
DECLARE_EXCEPTION(EInputBufferError, AAC_INPUT_BUFFER_EMPTY, "No sync word found!");
DECLARE_EXCEPTION(ESyncError,		 AAC_SYNCERROR,			 "Synchronization Error!") ;

CDolbyBitBuffer::CDolbyBitBuffer ()
 : CDolbyBitStream ()
{
  m_IsADTSCompliant=true;

  m_bFrameReadButNotDecoded = false;

  Initialize () ;
}

CDolbyBitBuffer::~CDolbyBitBuffer()
{
}

void CDolbyBitBuffer::Initialize (void)
{
  m_ValidBits  = 0 ;
  m_ReadOffset = 0 ;
  m_BitCnt     = 0 ;
  m_BitNdx     = 0 ;

  m_EOF        = false ;

  for (int i = 0 ; i < BufferSize ; i++)
  {
    m_Buffer [i] = 0 ;
  }

  m_BlocksLeftInFrame = 0 ;
  m_Markers = 0 ;

  m_FrameCrcValue = InvalidCrcValue ;

  cid_bitcount = BYTE_SIZE;				// init copyright id bit counter
  cid_bytecount = COPYRIGHT_SIZE;		// inti copyright id byte counter

  buffer_fullness = 0;
  last_buffer_fullness = 0;
  last_frame_length = 0;
  bytes_skipped = 0;
  hold_bytes_skipped = false;
  number_of_raw_data_blocks_in_frame = 0;
}

long CDolbyBitBuffer::Get (int nBits)
{
  unsigned short tmp, tmp1 ;

  int nWordNdx   = (m_BitNdx >> 4) << 1 ;
  int nBitsAvail = 16 - (m_BitNdx & 15) ;
  
  tmp   = m_Buffer [nWordNdx] ;
  tmp <<= 8 ;
  tmp  |= m_Buffer [nWordNdx + 1] ;
  tmp <<= (m_BitNdx & 15) ;

  if (nBits > nBitsAvail)
  {
    nWordNdx  = (nWordNdx + 2) & (BufferSize - 1) ;
    tmp1      = m_Buffer [nWordNdx] ;
    tmp1    <<= 8 ;
    tmp1     |= m_Buffer [nWordNdx + 1] ;

    tmp1    >>= (16 - (m_BitNdx & 15)) ;
    tmp      |= tmp1 ;
  }

  tmp >>= (16 - nBits) ;

  m_BitNdx     = (m_BitNdx+nBits) & (BufferBits - 1) ;
  m_BitCnt    += nBits ;
  m_ValidBits -= nBits ;

  return tmp ;
}

void CDolbyBitBuffer::PushBack (int n)
{
  m_BitCnt    -= n ;
  m_ValidBits += n ;
  m_BitNdx     = (m_BitNdx - n) & (BufferBits - 1) ;
}

void CDolbyBitBuffer::ByteAlign (void)
{
  long alignment = m_BitCnt % 8 ;

  if (alignment)
  {
    Get (8 - alignment) ;
  }

  m_BitCnt = 0 ;
}

bool CDolbyBitBuffer::IsDecodableFrame (CStreamInfo &info)
{
  ByteAlign ();

  if (FrameReadButNotDecoded())
  {
	  ClearFrameReadButNotDecoded();
	  return true;
  }

tryagain :
  
  SetPositionMarker (AncillaryElementStart) ;
  bytes_skipped += GetBitCount() >> 3;
 
  if (m_ValidBits < Adts_Value_MinHeaderLength)
  {
	  if (m_EOF) throw EEndOfFile () ;
	  else return false ;
  }


  /* Since the 1-bit ID must be 1 and the 2-bit layer must be 0, we can 
   * look for 15 bits of sync rather than just 12.  We'll get 16 bits from the
   * buffer and mask the top 15 bits for ease of readability.
   */
  if ((Get(Adts_Length_SyncWord + 4) & 0xFFFE) != 0xFFF8) 
  {
	  PushBack (Adts_Length_SyncWord + 4) ;
	  
	  while (m_ValidBits >> 3)
	  {
		  if (Get (8) == Adts_Value_SyncMSByte)
		  {
			  PushBack (8) ;
			  
			  if ((Get(Adts_Length_SyncWord + 4) & 0xFFFE) == 0xFFF8)
			  {
				  PushBack (Adts_Length_SyncWord + 4) ;
				  goto tryagain ;
			  }
			  else
			  {
				  /* Push back 8 of the 16 bits we read. This will advance us by a byte. */
				  PushBack (Adts_Length_SyncWord - 4);
			  }
		  }
	  }
	  
      // syncword not found in BitBuffer so accumulate 
      // bytes skipped before throwing an error
      bytes_skipped += GetBitCount() >> 3;

      // eaten up input
	  if (m_EOF)
      {
		  throw EEndOfFile();
      }
	  else 
      {
		  throw EInputBufferError();
      }
  }

  /* Push back the 4 extra bits we read beyond the 12-bit sync word. */
  PushBack(4);
  
  // adts_fixed_header

  id = Get (Adts_Length_Id) ;
  layer = Get (Adts_Length_Layer) ;
  protection_absent = Get (Adts_Length_ProtectionAbsent) ;
  profile = Get (Adts_Length_Profile) ;
  sampling_frequency_index = Get (Adts_Length_SamplingFrequencyIndex) ;
  private_bit = Get (Adts_Length_PrivateBit) ;
  channel_configuration = Get (Adts_Length_ChannelConfiguration) ;
  original_copy = Get (Adts_Length_OriginalCopy) ;
  home = Get (Adts_Length_Home) ;

  // adts_variable_header

  copyright_identification_bit = Get (Adts_Length_CopyrightIdentificationBit) ;
  copyright_identification_start = Get (Adts_Length_CopyrightIdentificationStart) ;
  frame_length = Get (Adts_Length_FrameLength) ;
  buffer_fullness = Get (Adts_Length_BufferFullness) ;
  number_of_raw_data_blocks_in_frame = Get (Adts_Length_NumberOfRawDataBlocksInFrame) ;

  unsigned int i;

  if ((m_ValidBits >> 3) < (frame_length - (Adts_Value_MinHeaderLength / 8)))
  {
    if (m_EOF) 
		throw EEndOfFile () ;

    PushBack (Adts_Value_MinHeaderLength) ;
    return false ;
  }

  SetPositionMarker (AncillaryElementStop) ;

  // adts_error_check

  if (!protection_absent)
  {
    m_FrameCrcValue = Get (Adts_Length_CrcCheck) ;
  }

  if (layer != 0)
  {
    return false ;
  }

  // // //

  ByteAlign () ;

  // // //  Copy data from ADTS header to info structure
  info.SetProtectionAbsent(protection_absent);
  info.SetOriginalCopy(original_copy);
  info.SetHome(home);
  info.SetFrameLength(frame_length);

  // Assemble copyright ID
  if (copyright_identification_start)
  {
	cid_bitcount = 0;
	cid_bytecount = 0;
	for (i=0; i<COPYRIGHT_SIZE; i++)
		copyright_id[i] = copyright_id_temp[i];
	copyright_id_temp[cid_bytecount++] = copyright_identification_bit;
	cid_bitcount++;
  }
  else
  {
	  if (cid_bitcount++ < BYTE_SIZE && cid_bytecount < COPYRIGHT_SIZE)
	  {
		  copyright_id_temp[cid_bytecount] <<=  1 | (copyright_identification_bit & 1);
		  if (cid_bitcount >= BYTE_SIZE)
		  {
			  cid_bitcount = 0;
			  cid_bytecount++;
		  }
	  }
	  else
	  {
		  // here, if counter overflow: delete copyright info
		  for (i=0; i<COPYRIGHT_SIZE; i++)
			  copyright_id[i] = 0;
	  }
  }
  
  for (i = 0; i < COPYRIGHT_SIZE; i++)
	info.SetCopyrightID(copyright_id[i], i);		// write copyright ID to info structure


  unsigned int nextsync = m_BitNdx >> 3 ;
  unsigned int header_length = 7 + (protection_absent ? 0 : 2) ;
  nextsync += frame_length - header_length ;
  nextsync %= BufferSize ;

  // this is the only place where we assume the bitstream to be
  // aligned on byte boundaries (which is true for software),
  // maybe someday we might want to remove this restriction.

 /* If a 'RIFF' header is detected (due to concatenated RIFF+ADTS files), look beyond it
  * for the next ADTS sync.
  */
  if (m_Buffer[nextsync]					== 'R' &&
	  m_Buffer[(nextsync + 1) % BufferSize] == 'I' &&
	  m_Buffer[(nextsync + 2) % BufferSize] == 'F' &&
	  m_Buffer[(nextsync + 3) % BufferSize] == 'F'
	 )
  {
	  bool		   bFoundDataChunk = true;
	  unsigned int uiMarker		   = nextsync;
	  
	  while (m_Buffer[nextsync]					   != 'd' ||
			 m_Buffer[(nextsync + 1) % BufferSize] != 'a' ||
			 m_Buffer[(nextsync + 2) % BufferSize] != 't' ||
			 m_Buffer[(nextsync + 3) % BufferSize] != 'a'
			)
	  {		  
		  nextsync = (nextsync + 1) % BufferSize;
		  
		  if (nextsync == uiMarker)
		  {
			  /* We've gone all the way around our circular buffer without ever finding a 'data' chunk. */
			  bFoundDataChunk = false;
			  break;
		  }
	  }
	  
	  if (bFoundDataChunk)
	  {
		  /* Skip over data chunk ID and length.  The next bytes should be our ADTS sync word. */
		  nextsync += 8;
		  nextsync %= BufferSize;
	  }
  }

  if (m_Buffer[nextsync] == Adts_Value_SyncMSByte && (m_Buffer[(nextsync + 1) % BufferSize] & 0xFE) == 0xF8)
  {
    if (info.GetRawSamplingRate ())
    {
      float flen = (float) (info.GetSamplesPerFrame () * (number_of_raw_data_blocks_in_frame + 1)) ;
      float bitrate = (float) (8 * last_frame_length + 32 * (buffer_fullness - last_buffer_fullness)) ;

      m_ActualBitrate += (unsigned int) (bitrate / (flen / info.GetRawSamplingRate ())) ;

      last_buffer_fullness = buffer_fullness ;
      last_frame_length = frame_length ;

      info.SetBitRate (m_ActualBitrate) ;
    }

    info.SetSamplingRateIndex (sampling_frequency_index) ;
    info.SetProfile (profile) ;
    info.SetChannelConfig (channel_configuration) ;

    m_BlocksLeftInFrame = number_of_raw_data_blocks_in_frame ;

    return true ;
  }
  else
  {
	  if (!m_EOF)
      {
         // syncword not found in next frame, but 
         // current frame's header has already been 
         // read, so add it to the # of bytes skipped
         bytes_skipped += header_length;

         // hold on to the bytes skipped so that we can
         // accumulate them across multiple calls to  
         // the function IsDecodableFrame
         HoldBytesSkipped();

		 throw ESyncError();
      }
	  else
      {
		 return true;
      }
  }
}


void CDolbyBitBuffer::ClearBytesSkipped(void)
{
    if (!hold_bytes_skipped)
    {
        bytes_skipped = 0;
    }
    else
    {
        // clear flag for next time around
        hold_bytes_skipped = false;
    }
}


void CDolbyBitBuffer::Feed (unsigned char pBuf [], unsigned int cbSize, unsigned int &cbValid)
{
  pBuf = &pBuf [cbSize - cbValid] ;

  unsigned int bTotal = 0 ;

  unsigned int bToRead   = (BufferBits - m_ValidBits) >> 3 ;
  unsigned int noOfBytes = (bToRead < cbValid) ? bToRead : cbValid ;

  while (noOfBytes > 0)
  {
    // split read to buffer size

    bToRead = BufferSize - m_ReadOffset ;
    bToRead = (bToRead < noOfBytes) ? bToRead : noOfBytes ;

    // copy 'bToRead' bytes from 'ptr' to inputbuffer

    for (unsigned int i = 0 ; i < bToRead ; i++)
    {
      m_Buffer [m_ReadOffset + i] = pBuf [i] ;
    }

    // add noOfBits to number of valid bits in buffer

    m_ValidBits  += bToRead * 8 ;
    bTotal       += bToRead ;
    pBuf         += bToRead ;

    m_ReadOffset  = (m_ReadOffset + bToRead) & (BufferSize - 1) ;
    noOfBytes    -= bToRead ;
  }

  cbValid -= bTotal ;

  // invalidate crc markers

  m_FrameCrcValue = InvalidCrcValue ;
  m_Markers = 0 ;
}

// crc stuff

void CDolbyBitBuffer::SetPositionMarker (MarkerPosition position)
{
  switch (position)
  {
    case SecondIndividualChannelStart :

      m_Markers++ ;

    case ChannelElementStart :
    case AncillaryElementStart :

      m_MarkerList [m_Markers] = CMarker (position, m_ValidBits, m_BitCnt, m_BitNdx) ;

      break ;

    case ChannelElementStop :
    case AncillaryElementStop :

      for (int i = 0 ; i < ((m_MarkerList [m_Markers].what == SecondIndividualChannelStart) ? 2 : 1) ; i++)
      {
        int delimiter = m_BitNdx + ((m_BitNdx > m_MarkerList [m_Markers - i].m_BitNdx) ? 0 : BufferBits) ;
        m_MarkerList [m_Markers - i].m_elementBits = delimiter - m_MarkerList [m_Markers - i].m_BitNdx ;
      }

      m_Markers++ ;
      break ;
  }
}

void CDolbyBitBuffer::IsCrcConsistent (unsigned int *crcValue)
{
  if (m_FrameCrcValue == InvalidCrcValue)
    return;

  CMarker current (CurrentPosition, m_ValidBits, m_BitCnt, m_BitNdx) ;

  *crcValue = 0xFFFF ;

  for (unsigned int i = 0 ; i < m_Markers ; i++)
  {
    RestoreMarker (m_MarkerList [i]) ;

    switch (m_MarkerList [i].what)
    {
      case ChannelElementStart :

        UpdateCrc (*crcValue, m_MarkerList [i].m_elementBits, 192) ;
        break ;

      case SecondIndividualChannelStart :

        UpdateCrc (*crcValue, m_MarkerList [i].m_elementBits, 128) ;
        break ;

      case AncillaryElementStart :

        UpdateCrc (*crcValue, m_MarkerList [i].m_elementBits) ;
        break ;
    }
  }

  RestoreMarker (current) ;

  if (*crcValue != m_FrameCrcValue)
	  throw ECRCError();

  return;
}

void CDolbyBitBuffer::UpdateCrc (unsigned int &crcValue, int numberOfBits, int minimumBits)
{
  static const unsigned int HIGHBIT_MASK = 0x8000 ;
  static const unsigned int CRC_POLY = 0x8005 ;

  static const unsigned int crcTable [] = 
  {
    0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011, 
    0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022, 
    0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072, 
    0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041, 
    0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2, 
    0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1, 
    0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1, 
    0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082, 
    0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192, 
    0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1, 
    0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1, 
    0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2, 
    0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151, 
    0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162, 
    0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132, 
    0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101, 
    0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312, 
    0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321, 
    0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371, 
    0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342, 
    0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1, 
    0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2, 
    0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2, 
    0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381, 
    0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291, 
    0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2, 
    0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2, 
    0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1, 
    0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252, 
    0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261, 
    0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231, 
    0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
  } ;

  if (minimumBits && (numberOfBits > minimumBits))
    numberOfBits = minimumBits ;

  for (int bytes = 0 ; bytes < (numberOfBits >> 3) ; bytes++)
  {
    crcValue = ((crcValue << 8) & 0xFF00) ^ crcTable [((crcValue >> 8) ^ Get (8)) & 0x00FF] ;
  }

  for (int bits = 0 ; bits < (numberOfBits & 0x07) ; bits++)
  {
    int flag = ((crcValue & HIGHBIT_MASK) ? 1 : 0) ^ Get (1) ;

    crcValue <<= 1 ;
    crcValue  &= 0xFFFF ;

    if (flag) crcValue ^= CRC_POLY ;
  }

  if (minimumBits && (minimumBits > numberOfBits))
  {
    numberOfBits = minimumBits - numberOfBits ;

    for (int bytes = 0 ; bytes < (numberOfBits >> 3) ; bytes++)
    {
      crcValue = ((crcValue << 8) & 0xFF00) ^ crcTable [(crcValue >> 8) & 0x00FF] ;
    }

    for (int bits = 0 ; bits < (numberOfBits & 0x07) ; bits++)
    {
      int flag = (crcValue & HIGHBIT_MASK) ? 1 : 0 ;

      crcValue <<= 1 ;
      crcValue  &= 0xFFFF ;

      if (flag) crcValue ^= CRC_POLY ;
    }
  }
}
