/****************************************************************************
*        
*   Module Title :     vfwpbdll_if.c
*
*   Description  :     Video codec playback dll interface
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>
#include "pbdll.h"
#include "vp60dversion.h"

/****************************************************************************
*  Macros
****************************************************************************/
#ifndef _MSC_VER
#define __try  
#define CommentString "\nON2.COM VERSION VP60D " VP60DVERSION "\n"
#pragma comment(exestr,CommentString)

#endif


/****************************************************************************
* Imports
****************************************************************************/ 
extern unsigned int CPUFrequency;
extern void VP6_DecodeFrameMbs(PB_INSTANCE *pbi);
extern void VP6_InitialiseConfiguration(PB_INSTANCE *pbi);
extern void InitHeaderBuffer ( FRAME_HEADER *Header, unsigned char *Buffer );
extern void SetAddNoiseMode(POSTPROC_INST , int);

#include <stdio.h>
/****************************************************************************
*  Module Statics
****************************************************************************/        
static const char vp31dVersion[] = VP60DVERSION;

/****************************************************************************
* Exports
****************************************************************************/
#ifdef PBSTATS1
static INT32  TotQ          = 0;    // TEMP diagnostic variables
static INT32  PBFrameNumber = 0;
#endif

/****************************************************************************
 * 
 *  ROUTINE       :     VP60D_GetVersionNumber
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     const char *: Pointer to decoder version string.
 *
 *  FUNCTION      :     Returns a pointer to the decoder version string.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
const char * CCONV VP60D_GetVersionNumber ( void ) 
{
    return vp31dVersion;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StartDecoder
 *
 *  INPUTS        :     PB_INSTANCE **pbi  : Pointer to pointer to decoder instance.
 *                      UINT32 ImageWidth  : Width of the image.
 *                      UINT32 ImageHeight : Height of the image.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     TRUE if succeeds, FALSE otherwise.
 *
 *  FUNCTION      :     Creates and initializes the decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
BOOL CCONV VP6_StartDecoder( PB_INSTANCE **pbi, UINT32 ImageWidth, UINT32 ImageHeight )
{ 
    __try
    {
        // set up our structure holding all formerly global information about a playback instance
        *pbi = VP6_CreatePBInstance();

        // Set Flag to indicate that a key frame is required as the first input
        (*pbi)->ScaleWidth = ImageWidth;
        (*pbi)->ScaleHeight = ImageHeight;
        (*pbi)->OutputWidth = ImageWidth;
        (*pbi)->OutputHeight = ImageHeight;
		

        // Validate the combination of height and width.
        (*pbi)->Configuration.VideoFrameWidth = ImageWidth;
        (*pbi)->Configuration.VideoFrameHeight = ImageHeight;

        (*pbi)->postproc = CreatePostProcInstance(&(*pbi)->Configuration);
        (*pbi)->quantizer = VP6_CreateQuantizer();
        (*pbi)->ProcessorFrequency = CPUFrequency;

        
        // Fills in fragment counts as well
        if ( !VP6_InitFrameDetails(*pbi) )
        {
            VP6_DeletePBInstance(pbi);
            return FALSE;
        }

        // Set last_dct_thresh to an illegal value to make sure the
        // Q tables are initialised for the new video sequence. 
        (*pbi)->quantizer->LastFrameQIndex = 0xFFFFFFFF;

        // Set up various configuration parameters.
        VP6_InitialiseConfiguration(*pbi);
        
        return TRUE;
    }
#if defined(_MSC_VER)
    __except( TRUE )
    {
        VP6_ErrorTrap( *pbi, GEN_EXCEPTIONS );
        return FALSE;
    }
#endif
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_GetPbParam
 *
 *  INPUTS        :     PB_INSTANCE **pbi       : Pointer to decoder instance.
 *                      PB_COMMAND_TYPE Command : Command action specifier.
 *                      
 *  OUTPUTS       :     UINT32 *Parameter       : Command dependent value requested.
 *
 *  RETURNS       :     void
 *  
 *  FUNCTION      :     Generalised command interface to decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void CCONV VP6_GetPbParam( PB_INSTANCE *pbi, PB_COMMAND_TYPE Command, UINT32 *Parameter )
{
    switch ( Command )
    {
#if defined(POSTPROCESS)
    case PBC_SET_POSTPROC:
        *Parameter = pbi->PostProcessingLevel;
#endif

    default:
        break;
    }
}
/****************************************************************************
 * 
 *  ROUTINE       :     VP6_PickPostProcessingLevel
 *
 *  INPUTS        :     PB_INSTANCE **pbi : Pointer to decoder instance.
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     int: Selected post-processing level.
 *  
 *  FUNCTION      :     Select the post-processing level to be used based
 *                      on how fast we're decoding.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#define CRITICALWATERMARK   (int) (31000 * pbi->CPUFree / 100)
#define DOWNWATERMARK       (int) (30000 * pbi->CPUFree / 100)
#define UPWATERMARK         (int) (28000 * pbi->CPUFree / 100)

int VP6_PickPostProcessingLevel ( PB_INSTANCE *pbi )
{
	int minimumTime = pbi->thisDecodeTime + pbi->avgBlitTime + pbi->avgPPTime[8];
	int thisTime = minimumTime + pbi->avgPPTime[pbi->PostProcessingLevel];
	int avgTime = pbi->avgDecodeTime + pbi->avgBlitTime;
	
	// estimate the times of all of our unknown postprocessors
	if(pbi->avgPPTime[6]==0)
		pbi->avgPPTime[6] = avgTime>>1;
	
	if(pbi->avgPPTime[5]==0)
		pbi->avgPPTime[5] = avgTime>>1;

	if(pbi->avgPPTime[4]==0)
		pbi->avgPPTime[4] = (avgTime ) >> 2;

	if(pbi->avgPPTime[8]==0)
		pbi->avgPPTime[8] = avgTime>>3;

	if(pbi->CPUFree == 0 )
		return pbi->PostProcessingLevel;

	// automatically select a postprocessing level based on the amount 
	// of time taken to decode blit and postprocess etc
	
	// more than 1/30 of a second no postprocessing at all (its better to show an 
	// ugly frame than none at all). We use 1/30th of a second because nothing 
	// tells us the actual framerate
	if ( thisTime > (int)(CRITICALWATERMARK) )
	{
		// this frame's taking too long try to make up time on the subsequent frames
		pbi->avgDecodeTime = pbi->thisDecodeTime; 

		// pick a post processor we can decode in less than 2/3 the time
		if(pbi->avgPPTime[6] + minimumTime < CRITICALWATERMARK )
			return 6;
		
		if(pbi->avgPPTime[5] + minimumTime < CRITICALWATERMARK )
			return 5;
		
		if(pbi->avgPPTime[4] + minimumTime < CRITICALWATERMARK )
			return 4;
		
		if(pbi->avgPPTime[8] + minimumTime < CRITICALWATERMARK )
			return 8;

		return 0;
	}

	if(thisTime < DOWNWATERMARK && thisTime > UPWATERMARK)
		return pbi->PostProcessingLevel;

	// pick a post processor we can decode in less than 2/3 the time
	if(pbi->avgPPTime[6] + avgTime < UPWATERMARK )
		return 6;

	if(pbi->avgPPTime[5] + avgTime < UPWATERMARK )
		return 5;

	if(pbi->avgPPTime[4] + avgTime < UPWATERMARK )
		return 4;

	if(pbi->avgPPTime[8] + avgTime < UPWATERMARK )
		return 8;

	return 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_GetYUVConfig
 *
 *  INPUTS        :     PB_INSTANCE **pbi            : Pointer to decoder instance.
 *                      YUV_BUFFER_CONFIG *YuvConfig : Pointer to configuration
 *                                                     data-structure.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *  
 *  FUNCTION      :     Gets details of the reconstruction buffer
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void CCONV VP6_GetYUVConfig ( PB_INSTANCE *pbi, YUV_BUFFER_CONFIG *YuvConfig )
{
    __try 
    {
#ifdef _MSC_VER
		unsigned int duration;
		unsigned int starttsc,endtsc;
		VP6_readTSC(&starttsc);
		pbi->PostProcessingLevel = VP6_PickPostProcessingLevel(pbi);
#endif
        if( pbi->PostProcessingLevel || (pbi->Configuration.Interlaced && pbi->DeInterlaceMode) )
        {
#ifdef _MSC_VER
            extern void vp6_showinfo2(PB_INSTANCE *pbi);
            extern void vp6_showinfo(PB_INSTANCE *pbi);
		
            if ( pbi->PostProcessingLevel > 200 )
            {
                PostProcess (
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel-200,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,        
                    sizeof(FRAG_INFO),
                    0x0001 );
				VP6_readTSC(&endtsc);
                vp6_showinfo(pbi);
            }
            else if ( pbi->PostProcessingLevel > 100 )
            {
				PostProcess (
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel-100,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,                
                    sizeof(FRAG_INFO),
                    0x0001 );
				VP6_readTSC(&endtsc);
				vp6_showinfo2(pbi);
            }
            else
#endif
			{
//				pbi->AvgFrameQIndex = pbi->quantizer->FrameQIndex;
				
                PostProcess (
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,                
                    sizeof(FRAG_INFO),
                    0x0001 );
#ifdef _MSC_VER
                VP6_readTSC(&endtsc);
#endif
			}
        }

        if(pbi->BlackClamp)
            ClampLevels( pbi->postproc,pbi->BlackClamp,pbi->WhiteClamp,pbi->PostProcessBuffer,	pbi->PostProcessBuffer);

        if( pbi->Configuration.VideoFrameWidth < pbi->OutputWidth &&
            pbi->Configuration.VideoFrameHeight == pbi->OutputHeight )
        {
            YuvConfig->YWidth = pbi->OutputWidth+32; 
            YuvConfig->YHeight = pbi->OutputHeight+32;
            YuvConfig->YStride = YuvConfig->YWidth;
            
            YuvConfig->UVWidth = YuvConfig->YWidth / 2;
            YuvConfig->UVHeight = YuvConfig->YHeight / 2;
            YuvConfig->UVStride = YuvConfig->YStride / 2;
			
            YuvConfig->YBuffer = (char *)pbi->ScaleBuffer;
            YuvConfig->UBuffer = (char *)pbi->ScaleBuffer+YuvConfig->YWidth*YuvConfig->YHeight;
            YuvConfig->VBuffer = (char *)pbi->ScaleBuffer+YuvConfig->YWidth*YuvConfig->YHeight+YuvConfig->UVWidth*YuvConfig->UVHeight;

			if(pbi->PostProcessingLevel)
	            ScaleOrCenter( pbi->postproc, pbi->PostProcessBuffer, YuvConfig  );
			else
	            ScaleOrCenter( pbi->postproc, pbi->LastFrameRecon, YuvConfig  );

			YuvConfig->YBuffer += 
				(YuvConfig->YHeight - pbi->OutputHeight ) / 2 * YuvConfig->YStride 
				+(YuvConfig->YWidth - pbi->OutputWidth) / 2;
            YuvConfig->YWidth = pbi->OutputWidth; 
            YuvConfig->YHeight = pbi->OutputHeight;
            
			YuvConfig->UBuffer += 
				(YuvConfig->UVHeight - pbi->OutputHeight/2 ) / 2 * YuvConfig->UVStride 
				+(YuvConfig->UVWidth - pbi->OutputWidth/2) / 2;

			YuvConfig->VBuffer += 
				(YuvConfig->UVHeight - pbi->OutputHeight/2 ) / 2 * YuvConfig->UVStride 
				+(YuvConfig->UVWidth - pbi->OutputWidth/2) / 2;

            YuvConfig->UVWidth = pbi->OutputWidth / 2; 
            YuvConfig->UVHeight = pbi->OutputHeight / 2;
        }
        else
        {
            YuvConfig->YWidth = pbi->Configuration.VideoFrameWidth;
            YuvConfig->YHeight = pbi->Configuration.VideoFrameHeight;
            YuvConfig->YStride = pbi->Configuration.YStride;
            
            YuvConfig->UVWidth = pbi->Configuration.VideoFrameWidth / 2;
            YuvConfig->UVHeight = pbi->Configuration.VideoFrameHeight / 2;
            YuvConfig->UVStride = pbi->Configuration.UVStride;

            if( pbi->PostProcessingLevel ||(pbi->Configuration.Interlaced && pbi->DeInterlaceMode))
            { 
                YuvConfig->YBuffer = (char *)&pbi->PostProcessBuffer[pbi->ReconYDataOffset+(pbi->Configuration.YStride * UMV_BORDER) + UMV_BORDER];
                YuvConfig->UBuffer = (char *)&pbi->PostProcessBuffer[pbi->ReconUDataOffset+ (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2)];
                YuvConfig->VBuffer = (char *)&pbi->PostProcessBuffer[pbi->ReconVDataOffset+ (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2)];
            }
            else
            {
                YuvConfig->YBuffer = (char *)&pbi->LastFrameRecon[pbi->ReconYDataOffset+ (pbi->Configuration.YStride * UMV_BORDER) + UMV_BORDER];
                YuvConfig->UBuffer = (char *)&pbi->LastFrameRecon[pbi->ReconUDataOffset+ (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2)];
                YuvConfig->VBuffer = (char *)&pbi->LastFrameRecon[pbi->ReconVDataOffset+ (pbi->Configuration.UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2)];
            }
        }

#if defined(_MSC_VER)   
		duration = ( endtsc - starttsc )/ pbi->ProcessorFrequency ;

		if( pbi->avgPPTime[pbi->PostProcessingLevel%10] == 0)
			pbi->avgPPTime[pbi->PostProcessingLevel%10] = duration;
		else
			pbi->avgPPTime[pbi->PostProcessingLevel%10] = ( 7 * pbi->avgPPTime[pbi->PostProcessingLevel%10] + duration ) >> 3;
#endif
    }
#if defined(_MSC_VER)   
    __except ( TRUE )
    {
        VP6_ErrorTrap( pbi, GEN_EXCEPTIONS );
    }    
#endif
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeFrameToYUV
 *
 *  INPUTS        :     PB_INSTANCE *pbi       : Pointer to decoder instance.
 *                      char *VideoBufferPtr   : Pointer to compressed data buffer.
 *                      unsigned int ByteCount : Size in bytes of compressed data buffer.
 *                      UINT32 ImageWidth      : Image width.
 *                      UINT32 ImageHeight     : Image height.
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     int: 0 for success, negative value for error.
 *
 *  FUNCTION      :     Decodes a frame into the internal YUV reconstruction buffer.
 *                      Details of this buffer can be obtained by calling GetYUVConfig().
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
int CurrentFrame = 0;
int CCONV VP6_DecodeFrameToYUV(PB_INSTANCE *pbi, char *VideoBufferPtr, unsigned int ByteCount)
{
    unsigned char *tmp;

    __try
    {
#ifdef _MSC_VER
		unsigned int duration;
		unsigned int starttsc,endtsc;
        VP6_readTSC(&starttsc);
#endif
		pbi->CurrentFrameSize = ByteCount;

		// Initialise the bit reader used to read the fixed raw part of the header
        InitHeaderBuffer ( &pbi->Header, (unsigned char*)VideoBufferPtr );

		// decode the frame header
        if ( !VP6_LoadFrame(pbi) )
            return -1;

		//  Start the second boolean decoder
		if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
		{
		    pbi->mbi.br = &pbi->br2;

			if ( pbi->UseHuffman )
			{
				// Initialise BITREADER for second bitstream partition
				pbi->br3.bitsinremainder = 0;
				pbi->br3.remainder = 0;
				pbi->br3.position = ((unsigned char*)VideoBufferPtr)+pbi->Buff2Offset;
			}
			else
				VP6_StartDecode(&pbi->br2,((unsigned char*)VideoBufferPtr)+pbi->Buff2Offset);
		}
        else
        {
        	pbi->mbi.br = &pbi->br;
        }

        // decode and reconstruct frame
        VP6_DecodeFrameMbs(pbi);

		// switch pointers so lastframe recon is this frame
        tmp = pbi->LastFrameRecon;
        pbi->LastFrameRecon = pbi->ThisFrameRecon;
        pbi->ThisFrameRecon = tmp;

        // update the border 
        UpdateUMVBorder(pbi->postproc, pbi->LastFrameRecon);

        // Update the golden frame buffer
		if( (pbi->FrameType == BASE_FRAME) || pbi->RefreshGoldenFrame )
            memcpy(pbi->GoldenFrame, pbi->LastFrameRecon, pbi->ReconYPlaneSize + 2* pbi->ReconUVPlaneSize); 

#if defined(_MSC_VER)
	    ClearSysState();
#endif

#ifdef PBSTATS1
        // Update PB stats
        TotQ += pbi->quantizer->ThisFrameQualityValue;
        PBFrameNumber += 1;
#endif

	    if(pbi->FrameType == BASE_FRAME )
			pbi->AvgFrameQIndex = pbi->quantizer->FrameQIndex;
		else
			pbi->AvgFrameQIndex = (2 + 3 * pbi->AvgFrameQIndex + pbi->quantizer->FrameQIndex) / 4 ;

#ifdef _MSC_VER
        VP6_readTSC(&endtsc);
		duration = (endtsc-starttsc)/ (pbi->ProcessorFrequency);
		pbi->thisDecodeTime = duration;

		if( pbi->avgDecodeTime == 0)
			pbi->avgDecodeTime = duration;
		else
			pbi->avgDecodeTime = (7*pbi->avgDecodeTime + duration)>>3;
#endif


#if 0
        if (pbi->br.pos>pbi->CurrentFrameSize)
        {
            FILE *f = fopen("badframes.stt","a");
            fprintf(f,"%8d %8d %8d \n", CurrentFrame,pbi->br.pos,pbi->CurrentFrameSize);
            fclose(f);
        }
#endif

        CurrentFrame++;
    }
#if defined(_MSC_VER) 
    __except ( TRUE )
    {
        VP6_ErrorTrap( pbi, GEN_EXCEPTIONS );
        return -2;
    }
#endif    
    return 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_StopDecoder
 *
 *  INPUTS        :     PB_INSTANCE **pbi : Pointer to pointer to decoder instance.
 *
 *  OUTPUTS       :     PB_INSTANCE **pbi : Pointer to pointer to decoder instance,
 *                                          set to NULL on return.
 *
 *  RETURNS       :     int: TRUE on success, FALSE otherwise.
 *
 *  FUNCTION      :     Detroys the decoder instance.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
int CCONV VP6_StopDecoder ( PB_INSTANCE **pbi )
{
    __try
    {
        if ( *pbi )
        {
            // Set flag to say that the decoder is no longer initialised
            VP6_DeleteQuantizer(&(*pbi)->quantizer);
            DeletePostProcInstance(&(*pbi)->postproc);
            VP6_DeleteFragmentInfo(*pbi);
            VP6_DeleteFrameInfo(*pbi);
            VP6_DeletePBInstance(pbi);
            return TRUE;
        }
    }

#if defined(_MSC_VER)        
    __except ( TRUE )
    {
        VP6_ErrorTrap( *pbi, GEN_EXCEPTIONS );
        return FALSE;
    }
#endif    
    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_ErrorTrap
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      int ErrorCode    : Error code to report.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Called when a fatal error is detected.
 *
 *  SPECIAL NOTES :     Currently does nothing. 
 *
 ****************************************************************************/
void VP6_ErrorTrap ( PB_INSTANCE *pbi, int ErrorCode )
{
}
