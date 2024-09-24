/****************************************************************************
*
*   Module Title :     DFrameR.C
*
*   Description  :     Functions to read from the input bitstream.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Frames
****************************************************************************/
#include "pbdll.h"
#include "postproc_if.h"


/****************************************************************************
 * 
 *  ROUTINE       :     VP6_bitread
 *
 *  INPUTS        :     BOOL_CODER *br : Pointer to a Bool Decoder instance.
 *                      int bits       : Number of bits to be read from input stream.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: The requested bits.
 *
 *  FUNCTION      :     Decodes the requested number of bits from the encoded data buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 VP6_bitread ( BOOL_CODER *br, int bits )
{
	UINT32 z = 0;
	int bit;
	for ( bit=bits-1; bit>=0; bit-- )
	{
		z |= (VP6_DecodeBool128(br)<<bit);
	}
	return z;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_bitread1
 *
 *  INPUTS        :     BOOL_CODER *br : Pointer to a Bool Decoder instance.
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     UINT32: The next decoded bit (0 or 1).
 *
 *  FUNCTION      :     Decodes the next bit from the encoded data buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
INLINE 
UINT32 VP6_bitread1 ( BOOL_CODER *br ) 
{
	return (VP6_DecodeBool128(br));
}

/****************************************************************************
 * 
 *  ROUTINE       :     InitHeaderBuffer
 *
 *  INPUTS        :     FRAME_HEADER *Header  : Pointer to FRAME_HEADER data structure.
 *                      unsigned char *Buffer : Pointer to buffer containing bitstream header.
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     void
 *
 *
 *  FUNCTION      :     Initialises extraction of bits from header buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void InitHeaderBuffer ( FRAME_HEADER *Header, unsigned char *Buffer )
{
    Header->buffer = Buffer;
    Header->value  = (Buffer[0]<<24)+(Buffer[1]<<16)+(Buffer[2]<<8)+Buffer[3];
    Header->bits_available = 32;
	Header->pos = 4;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ReadHeaderBits
 *
 *  INPUTS        :     FRAME_HEADER *Header : Pointer to FRAME_HEADER data structure.
 *                      UINT32 BitsRequired  : Number of bits to extract.
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     UINT32: Bits requested
 *
 *  FUNCTION      :     Extracts requested number of bits from header buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 ReadHeaderBits ( FRAME_HEADER *Header, UINT32 BitsRequired )
{
    UINT32 pos       = Header->pos;
    UINT32 available = Header->bits_available;
    UINT32 value     = Header->value;
    UINT8 *Buffer    = &Header->buffer[pos];
    UINT32 RetVal    = 0;

    if ( available < BitsRequired )
    {
        // Need more bits from input buffer...
        RetVal = value >> (32-available);
        BitsRequired -= available;
        RetVal <<= BitsRequired;

        value  = (Buffer[0]<<24)+(Buffer[1]<<16)+(Buffer[2]<<8)+(Buffer[3]);
        pos += 4;
        available = 32;
    }

    RetVal |= value >> (32-BitsRequired);
    
    // Update data struucture
    Header->value          = value<<BitsRequired;
    Header->bits_available = available-BitsRequired;
    Header->pos = pos;

    return RetVal;
}

/****************************************************************************
 * 
 *  ROUTINE       :     LoadFrameHeader
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     BOOL: FALSE in case of error, TRUE otherwise.
 *
 *  FUNCTION      :     Loads a frame header & carries out some initialization.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) ) 

static BOOL LoadFrameHeader ( PB_INSTANCE *pbi )
{
    UINT8  DctQMask;
    FRAME_HEADER *Header = &pbi->Header;
    BOOL   RetVal = TRUE;

    // Is the frame and inter frame or a key frame
    pbi->FrameType = (UINT8)ReadHeaderBits(Header, 1);

    // Quality (Q) index
    DctQMask = (UINT8)ReadHeaderBits(Header, 6);

	// Are we using two BOOL coder data streams/partitions
    pbi->MultiStream = (UINT8)ReadHeaderBits(Header, 1);

	// If the frame was a base frame then read the frame dimensions and build a bitmap structure. 
	if ( (pbi->FrameType == BASE_FRAME) )
	{
        // Read the frame dimensions bytes (0,0 indicates vp31 or later)
	    pbi->Vp3VersionNo = (UINT8)ReadHeaderBits(Header,   5 );
	    pbi->VpProfile = (UINT8)ReadHeaderBits(Header,   2 );

		if(pbi->Vp3VersionNo > CURRENT_DECODE_VERSION)
		{
			RetVal = FALSE;
			return RetVal;
		}

		// Initialise version specific quantiser values
		VP6_InitQTables( pbi->quantizer, pbi->Vp3VersionNo );

		// is this keyframe section of the file interlaced
		pbi->Configuration.Interlaced = (UINT8)ReadHeaderBits(Header, 1);	

		// Start the first bool decoder (modes, mv, probs and some flags)
		// The offset depends on whether or not we are using multiple bool code streams
		if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
		{
			VP6_StartDecode(&pbi->br, ((unsigned char*)(Header->buffer + 4)));

			// Read the buffer offset for the second bool decoder buffer if it is being used
		    pbi->Buff2Offset = (UINT32)ReadHeaderBits(Header, 16);
		}
		else
			VP6_StartDecode(&pbi->br, ((unsigned char*)(Header->buffer + 2)));

		// SCALING related stuff
		SetPPInterlacedMode(pbi->postproc, pbi->Configuration.Interlaced);
        if(pbi->Configuration.Interlaced)
        {
            SetDeInterlaceMode(pbi->postproc, pbi->DeInterlaceMode);
        }

        {             
             UINT32 HFragments;             
             UINT32 VFragments;             
             UINT32 HOldScaled;
             UINT32 VOldScaled;
             UINT32 HNewScaled;
             UINT32 VNewScaled;
			 UINT32 OutputHFragments;
			 UINT32 OutputVFragments;

             VFragments = 2 * ((UINT8)VP6_bitread( &pbi->br,   8 ));             
             HFragments = 2 * ((UINT8)VP6_bitread( &pbi->br,   8 ));              

             OutputVFragments = 2 * ((UINT8)VP6_bitread( &pbi->br,   8 ));             
             OutputHFragments = 2 * ((UINT8)VP6_bitread( &pbi->br,   8 ));              

             if(pbi->Configuration.HRatio == 0)
                 pbi->Configuration.HRatio = 1;

             if(pbi->Configuration.VRatio == 0)
                 pbi->Configuration.VRatio = 1;

             HOldScaled = pbi->Configuration.HScale * pbi->HFragments * 8 / pbi->Configuration.HRatio;
             VOldScaled = pbi->Configuration.VScale * pbi->VFragments * 8 / pbi->Configuration.VRatio;

			 pbi->Configuration.ExpandedFrameWidth = OutputHFragments * 8;
			 pbi->Configuration.ExpandedFrameHeight = OutputVFragments * 8;

			 if(VFragments >= OutputVFragments)
			 {
	             pbi->Configuration.VScale = 1;
		         pbi->Configuration.VRatio = 1;
			 }
			 else if (5*VFragments >= 4*OutputVFragments)
			 {
	             pbi->Configuration.VScale = 5;
		         pbi->Configuration.VRatio = 4;
			 }
			 else if (5*VFragments >= 3*OutputVFragments)
			 {
	             pbi->Configuration.VScale = 5;
		         pbi->Configuration.VRatio = 3;
			 }
			 else
			 {
	             pbi->Configuration.VScale = 2;
		         pbi->Configuration.VRatio = 1;
			 }

			 if(HFragments >= OutputHFragments)
			 {
	             pbi->Configuration.HScale = 1;
		         pbi->Configuration.HRatio = 1;
			 }
			 else if (5*HFragments >= 4*OutputHFragments)
			 {
	             pbi->Configuration.HScale = 5;
		         pbi->Configuration.HRatio = 4;
			 }
			 else if (5*HFragments >= 3*OutputHFragments)
			 {
	             pbi->Configuration.HScale = 5;
		         pbi->Configuration.HRatio = 3;
			 }
			 else
			 {
	             pbi->Configuration.HScale = 2;
		         pbi->Configuration.HRatio = 1;
			 }

             HNewScaled = pbi->Configuration.HScale * HFragments * 8 / pbi->Configuration.HRatio;
             VNewScaled = pbi->Configuration.VScale * VFragments * 8 / pbi->Configuration.VRatio;

			 pbi->ScaleWidth = HNewScaled;
			 pbi->ScaleHeight = VNewScaled; 

             pbi->Configuration.ScalingMode = ((UINT32)VP6_bitread( &pbi->br, 2 ));

             // we have a new input size
             if( VFragments != pbi->VFragments || HFragments != pbi->HFragments )
             {
                 // Validate the combination of height and width.                 
                 pbi->Configuration.VideoFrameWidth = HFragments*8;                 
                 pbi->Configuration.VideoFrameHeight = VFragments*8;                  
				 VP6_InitFrameDetails(pbi);
             }

             // we have a new intermediate buffer clean the screen 
             if( pbi->ScaleBuffer != 0 &&
                 (HOldScaled != HNewScaled || VOldScaled != VNewScaled) )
             {
                 // turn the screen black!!                 
                 memset(pbi->ScaleBuffer, 0x0, (pbi->OutputWidth+32) * (pbi->OutputHeight+32) );                 
                 memset(pbi->ScaleBuffer + 	(pbi->OutputWidth+32) * (pbi->OutputHeight+32),
					 0x80, (pbi->OutputWidth+32) * (pbi->OutputHeight+32) / 2 );                                   
             }
		}         

		// Unless in SIMPLE_PROFILE read the the filter strategy for fractional pels
		if ( pbi->VpProfile != SIMPLE_PROFILE )
		{
			// Find out if selective bicubic filtering should be used for motion prediction.
			if ( (BOOL)VP6_DecodeBool(&pbi->br, 128) )
			{
				pbi->PredictionFilterMode = AUTO_SELECT_PM;

				// Read in the variance threshold to be used
				pbi->PredictionFilterVarThresh = ((UINT32)VP6_bitread( &pbi->br, 5) << ((pbi->Vp3VersionNo > 7) ? 0 : 5) );

				// Read the bicubic vector length limit (0 actually means ignore vector length)
				pbi->PredictionFilterMvSizeThresh = (UINT8)VP6_bitread( &pbi->br, 3);
			}
			else
			{
				if ( (BOOL)VP6_DecodeBool(&pbi->br, 128) )
					pbi->PredictionFilterMode = BICUBIC_ONLY_PM;
				else
					pbi->PredictionFilterMode = BILINEAR_ONLY_PM;
			}

			if ( pbi->Vp3VersionNo > 7 )
				pbi->PredictionFilterAlpha = VP6_bitread( &pbi->br, 4);
			else
				pbi->PredictionFilterAlpha = 16;	// VP61 backwards compatibility
		}
    }
	// Non key frame sopecific stuff
	else
	{
		// Start the first bool decoder (modes, mv, probs and some flags)
		// The offset depends on whether or not we are using multiple bool code streams
		if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
		{
			VP6_StartDecode(&pbi->br, ((unsigned char*)(Header->buffer + 3)));
			
			// Read the buffer offset for the second bool decoder buffer if it is being used
		    pbi->Buff2Offset = (UINT32)ReadHeaderBits(Header, 16);
		}
		else
			VP6_StartDecode(&pbi->br, ((unsigned char*)(Header->buffer + 1)));

		// Find out if the golden frame should be refreshed this frame - use bool decoder
		pbi->RefreshGoldenFrame = (BOOL)VP6_DecodeBool(&pbi->br, 128);

		if ( pbi->VpProfile != SIMPLE_PROFILE )
		{
			// Determine if loop filtering is on and if so what type should be used
			pbi->UseLoopFilter = VP6_DecodeBool(&pbi->br, 128);
			if ( pbi->UseLoopFilter )
			{
				pbi->UseLoopFilter = (pbi->UseLoopFilter << 1) | VP6_DecodeBool(&pbi->br, 128);
			}

			if ( pbi->Vp3VersionNo > 7 )
			{
				// Are the prediction characteristics being updated this frame
				if ( VP6_DecodeBool(&pbi->br, 128) )
				{
					// Find out if selective bicubic filtering should be used for motion prediction.
					if ( (BOOL)VP6_DecodeBool(&pbi->br, 128) )
					{
						pbi->PredictionFilterMode = AUTO_SELECT_PM;

						// Read in the variance threshold to be used
						pbi->PredictionFilterVarThresh = (UINT32)VP6_bitread( &pbi->br, 5);

						// Read the bicubic vector length limit (0 actually means ignore vector length)
						pbi->PredictionFilterMvSizeThresh = (UINT8)VP6_bitread( &pbi->br, 3);
					}
					else
					{
						if ( (BOOL)VP6_DecodeBool(&pbi->br, 128) )
							pbi->PredictionFilterMode = BICUBIC_ONLY_PM;
						else
							pbi->PredictionFilterMode = BILINEAR_ONLY_PM;
					}

					pbi->PredictionFilterAlpha = VP6_bitread( &pbi->br, 4 );
				}
			}
			else
				pbi->PredictionFilterAlpha = 16;	// VP61 backwards compatibility
		}
	}

	// All frames (Key & Inter frames)
	if(pbi->Vp3VersionNo < 3 )
		RetVal = FALSE;

	// Should this frame use huffman for the dct data
	pbi->UseHuffman = (BOOL)VP6_DecodeBool(&pbi->br, 128);

	// Set this frame quality value from Q Index
	pbi->quantizer->FrameQIndex = DctQMask;
	VP6_UpdateQ( pbi->quantizer, pbi->Vp3VersionNo );  

    return RetVal;                    
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_LoadFrame
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     BOOL: FALSE on error or frame empty, TRUE otherwise.
 *
 *  FUNCTION      :     Loads the next frame from the encoded data buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
BOOL VP6_LoadFrame ( PB_INSTANCE *pbi )
{ 
    BOOL RetVal = TRUE;           

    // Load the frame header (including the frame size).     
    if ( !LoadFrameHeader(pbi) )
        RetVal = FALSE;
    return RetVal;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_SetFrameType
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      UINT8 FrType     : Type of the frame.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Sets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_SetFrameType ( PB_INSTANCE *pbi, UINT8 FrType )
{ 
    /* Set the appropriate frame type according to the request */
    pbi->FrameType = FrType;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_GetFrameType
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT8: The current frame type.
 *
 *  FUNCTION      :     Gets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT8 VP6_GetFrameType ( PB_INSTANCE *pbi )
{
    return pbi->FrameType; 
}
