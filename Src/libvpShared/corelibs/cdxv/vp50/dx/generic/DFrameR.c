/****************************************************************************
*
*   Module Title :     DFrameR.C
*
*   Description  :     Functions to read
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.18 YWX 17/dec/02 Added DeInterlacedMode setup
*   1.17 YWX 05/08/02  Added initialization of postprocessor 's interlaced flag
*   1.16 JBB 13 Jun 01 VP4 Code Clean Out
*	1.15 AWG 08-Jun-01 Added support for DCT16
*   1.14 JBB 04 May 01 Added set of ReadTokens Function for VP5
*   1.13 JBB 04 Dec 00 Added new Center vs Scale Bits
*   1.12 JBB 30 NOV 00 Version number changes 
*   1.11 JBB 14 Oct 00 Added ifdefs around version specific code
*   1.10 PGW 06 Oct 00 QThreshTable[] made instance specific.
*					   Changes to LoadFrameHeader() to call InitQTables().
*	1.09 YWX 25 Aug 00 Added version number check
*   1.08 JBB 22 Aug 00 Ansi C conversion
*   1.07 JBB 27 Jul 00 Malloc checks
*   1.06 PGW 20/03/00  Removed InterIntra mode flag.
*	1.05 JBB 27/01/99  Globals Removed, use of PB_INSTANCE, Bit Management Functions
*   1.04 PGW 17/12/99  Changes to Synch code to reflect the fact that 0 length
*                      frames are no longer legal (simply not transmittedd)
*                      Note that this change is only relevant to the live version 
*                      of the codec
*   1.03 PGW 15/11/99  Added support for VP3 version ID.
*   1.02 PGW 30/08/99  Use bit functions to read header data.
*                      Changes to way bytes are read.
*   1.01 PGW 16/08/99  Header changes for VFW version and key frames.
*   1.00 PGW 22/06/99  pbi->Configuration baseline
*
*****************************************************************************
*/

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#include "pbdll.h"
#include "duck_mem.h" 
#include "boolhuff.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

#define START_SIZE  0
#define END_SIZE    1

#define READ_BUFFER_EMPTY_WAIT  20
 
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Module Statics
***** ************************************************************************
*/             
#ifndef MAPCA
static const UINT32 loMaskTbl_VP31[] = { 0,
	1, 3, 7, 15,
	31, 63, 127, 255,
	0x1ff, 0x3ff, 0x7ff, 0xfff,
	0x1fff, 0x3fff, 0x7fff, 0xffff,
	0x1FFFF, 0x3FFFF, 0x7FFFF, 0xfFFFF,
	0x1fFFFF, 0x3fFFFF, 0x7fFFFF, 0xffFFFF,
	0x1ffFFFF, 0x3ffFFFF, 0x7ffFFFF, 0xfffFFFF,
	0x1fffFFFF, 0x3fffFFFF, 0x7fffFFFF, 0xffffFFFF
};

static const UINT32 hiMaskTbl_VP31[] = { 0,
	0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
	0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
	0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
	0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
	0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
	0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
	0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
	0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF
};

#endif
/****************************************************************************
*  Forward References.
*****************************************************************************
*/              
static BOOL LoadFrameHeader(PB_INSTANCE *pbi);


/****************************************************************************
*  Imports
*****************************************************************************
*/              

/****************************************************************************
 * 
 *  ROUTINE       :     LoadFrame
 *
 *  INPUTS        :     None 
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     FALSE if an Error is detected or the frame is empty else TRUE.
 *
 *  FUNCTION      :     Loads a frame and decodes the fragment arrays.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
BOOL VP5_LoadFrame(PB_INSTANCE *pbi)
{ 
    BOOL RetVal = TRUE;           

    // Initialise the bit extractor.
    //ExtractInit(pbi);

    // Load the frame header (including the frame size).     
    if ( !LoadFrameHeader(pbi) )
    {
        RetVal = FALSE;
    }

    return RetVal;
}


/****************************************************************************
 * 
 *  ROUTINE       :     LoadFrameHeader
 *
 *  INPUTS        :     fptr - The file pointer for the data file.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     FALSE if and Error is detected else TRUE.
 *
 *  FUNCTION      :     Loads and interprets the frame header.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

// VFW codec version
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) ) 
static BOOL LoadFrameHeader(PB_INSTANCE *pbi)
{
	UINT8  VersionByte0;    // Must be 0 for VP30b and later
    UINT8  DctQMask;
    UINT8  SpareBits;       // Spare cfg bits
	UINT8  Unused;

    BOOL   RetVal = TRUE;

    // Is the frame and inter frame or a key frame
    pbi->FrameType = DecodeBool(&pbi->br, 128);
    
	// unused bit
    Unused = DecodeBool(&pbi->br, 128);

    // Quality (Q) index
    DctQMask = (UINT8)VP5_bitread( &pbi->br,   6 );
		

	// If the frame was a base frame then read the frame dimensions and build a bitmap structure. 
	if ( (pbi->FrameType == BASE_FRAME) )
	{
        // Read the frame dimensions bytes (0,0 indicates vp31 or later)
    	VersionByte0 = (UINT8)VP5_bitread( &pbi->br,   8 );
	    pbi->Vp3VersionNo = (UINT8)VP5_bitread( &pbi->br,   5 );

		if(pbi->Vp3VersionNo > CURRENT_DECODE_VERSION)
		{
			RetVal = FALSE;
			return RetVal;
		}
		// Initialise version specific quantiser values
		VP5_InitQTables( pbi->quantizer, pbi->Vp3VersionNo );

        // Read the type / coding method for the key frame.
        pbi->KeyFrameType = (UINT8)DecodeBool(&pbi->br, 128);

        SpareBits = (UINT8)DecodeBool(&pbi->br, 128);

		// is this keyframe section of the file interlaced
		pbi->Configuration.Interlaced = (UINT32)DecodeBool(&pbi->br, 128);		
#ifndef MAPCA
		SetPPInterlacedMode(pbi->postproc, pbi->Configuration.Interlaced);
        if(pbi->Configuration.Interlaced)
        {
            SetDeInterlaceMode(pbi->postproc, pbi->DeInterlaceMode);
        }
#endif
        // Spare config bits
         {             
             UINT32 HFragments;             
             UINT32 VFragments;             
             UINT32 HOldScaled;
             UINT32 VOldScaled;
             UINT32 HNewScaled;
             UINT32 VNewScaled;
			 UINT32 OutputHFragments;
			 UINT32 OutputVFragments;

             VFragments = 2 * ((UINT8)VP5_bitread( &pbi->br,   8 ));             
             HFragments = 2 * ((UINT8)VP5_bitread( &pbi->br,   8 ));              

             OutputVFragments = 2 * ((UINT8)VP5_bitread( &pbi->br,   8 ));             
             OutputHFragments = 2 * ((UINT8)VP5_bitread( &pbi->br,   8 ));              

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

             pbi->Configuration.ScalingMode = ((UINT32)VP5_bitread( &pbi->br, 2 ));

             // we have a new input size
             if( VFragments != pbi->VFragments ||                
                 HFragments != pbi->HFragments)             
             {
                 // Validate the combination of height and width.                 
                 pbi->Configuration.VideoFrameWidth = HFragments*8;                 
                 pbi->Configuration.VideoFrameHeight = VFragments*8;                  
				 VP5_InitFrameDetails(pbi);
             }


             // we have a new intermediate buffer clean the screen 
             if( pbi->ScaleBuffer != 0 &&
                 (HOldScaled != HNewScaled ||
                  VOldScaled != VNewScaled ) )
             {
                 // turn the screen black!!                 
                 memset(pbi->ScaleBuffer, 0x0, (pbi->OutputWidth+32) * (pbi->OutputHeight+32) );                 
                 memset(pbi->ScaleBuffer + 	(pbi->OutputWidth+32) * (pbi->OutputHeight+32),
					 0x80, (pbi->OutputWidth+32) * (pbi->OutputHeight+32) / 2 );                                   
             }
         }         
    }
	
	// Set this frame quality value from Q Index
	pbi->quantizer->FrameQIndex = DctQMask;
#ifdef MAPCA
    SetFLimit(DctQMask);
    SetSimpleDeblockFlimit(DctQMask);
#endif
    pbi->quantizer->ThisFrameQuantizerValue = pbi->quantizer->QThreshTable[DctQMask];
	VP5_UpdateQ( pbi->quantizer, pbi->Vp3VersionNo );  

    return RetVal;                    
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP5_SetFrameType
 *
 *  INPUTS        :     A Frame type.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_SetFrameType( PB_INSTANCE *pbi,UINT8 FrType )
{ 
    /* Set the appropriate frame type according to the request. */
    switch ( FrType )
    {  
    
    case BASE_FRAME:
        pbi->FrameType = FrType;
        break;
        
    default:
        pbi->FrameType = FrType;
        break;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP5_GetFrameType
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     The current frame type.
 *
 *  FUNCTION      :     Gets the current frame type.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 VP5_GetFrameType(PB_INSTANCE *pbi)
{
    return pbi->FrameType; 
}

