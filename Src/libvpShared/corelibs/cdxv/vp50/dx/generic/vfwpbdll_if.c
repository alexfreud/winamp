/****************************************************************************
*        
*   Module Title :     vfwpbdll_if.c
*
*   Description  :     Video codec demo playback dll interface
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.29 YWX 17/dec/02 Added support of deinterlace
*   1.28 YWX 05/08/02  Changed postprocess level setup for interlaced material
*   1.27 AWG 20 Jun 01 Added code to overlay Motion Vectors onto display
*   1.26 JBB 13 Jun 01 VP4 Code Clean Out
*   1.25 YWX 26-Apr-01 Removed call of SetPbParam() in StartDecoder()
*                      And set CPUFree as 70 when PostProcessingLevel=9 
*   1.24 JBB 25-apr-01 clear sysstate added at end of frame blit
*   1.23 JBB 06-Apr-01 CPU Free variable respond
*   1.22 SJL 22-Mar-01 Fixed MAC compile errors
*   1.21 JBX 22-Mar-01 Merged with new vp4-mapca bitstream  
*   1.20 SJL 01 Dec 00 Fixed MAC compile errors
*   1.19 JBB 30 Nov 00 Version number changes 
*   1.18 JBB 14 Nov 00 Added version information function and pragma and cleaned
*                      out unused code
*   1.17 JBB 17-oct-00 Ifdefs around version information
*   1.16 SJL 25 Aug 00 Fixed Mac compile error
*   1.15 JBB 25 Aug 00 Better versioning
*   1.14 JBB 22 Aug 00 Ansi C conversion
*   1.13 SJL 14 Aug00  Moved SetPbParam into another file for the MAC 
*   1.12 YWX 2 Aug00   Changed Postprocessing level initialization 
*   1.11 JBB 31Jul00   Changed requirements for postprocessing due to new 
*                      optimiztions
*   1.10 JBB 27Jul00   Added malloc checks 
*   1.09 YWX 15/05/00  Check Processor and Frame size to enable/disable 
*                      postprocessor
*   1.08 YWX 08/05/00  Added #if defined directives for postprocess
*   1.07 JBB 05/05/00  Added PostProcessing Parameter
*   1.06 JBB 27/01/99  Globals Removed, use of PB_INSTANCE, must be created
*   1.05 PGW 05/11/99  Changes to support AC range entropy tables and to output
*                      the appropriate stats to tune them.
*   1.04 PGW 01/09/99  Modified to simulate Tim's DxReference interface.
*   1.03 PGW 30/07/99  Added exception handlers and some code to try and insure
*                      decoder is initialised before any frames are decoded.
*   1.02 PGW 09/07/99  Added code to support profile timing
*   1.01 PGW 29/06/99  Changes in DecodeFrame() to handle inversion of DIB when 
*                      requested plus offsets into and pitch of the output image 
*                      buffer.
*   1.00 PGW 28/06/99  New Configuration baseline.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/
  
#define STRICT              /* Strict type checking. */
#include <stdio.h> 

#ifndef _MSC_VER

#define __try  

#endif

#include "huffman.h"
#include "pbdll.h"
#include <math.h>
#include "vp50dversion.h"
#include "decodemode.h"
#include "postproc_if.h"

#ifndef MAPCA
    #define CommentString "\nON2.COM VERSION VP50D " VP50DVERSION "\n"
    #pragma comment(exestr,CommentString)
#endif
/****************************************************************************
 *  Explicit Imports
 *****************************************************************************
 */ 

extern void DecodeFrameMbs(PB_INSTANCE *pbi);
extern unsigned int CPUFrequency;

/****************************************************************************
*  Module statics.
*****************************************************************************
*/        


#ifdef PBSTATS1
INT32  TotQ = 0;
INT32  PBFrameNumber = 0;
#endif
static const char vp31dVersion[] = VP50DVERSION;


/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
#if defined(_MSC_VER) 
#if defined(POSTPROCESS)
static const unsigned long PP_MACHINE_LOWLIMIT = 350; //Lowest CPU (MHz) to enable PostProcess
static const unsigned long PP_MACHINE_MIDLIMIT = 400; //Lowest CPU (MHz) to enable PostProcess
static const unsigned long PP_MACHINE_TOPLIMIT = 590; //Lowest CPU (MHz) to enable PostProcess
#endif
#endif

extern void VP5_InitialiseConfiguration(PB_INSTANCE *pbi);
#ifdef PBSTATS1
// TEMP diagnostic variables
INT32  TotBlocksCoded;
#endif


/****************************************************************************
*  Foreward references
*****************************************************************************
*/
/****************************************************************************
 * 
 *  ROUTINE       :     VP31D_GetVersionNumber
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None .
 *
 *  FUNCTION      :     Returns a pointer to the version string
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
const char * CCONV VP50D_GetVersionNumber(void)
{
    return vp31dVersion;
}

/****************************************************************************
 * 
 *  ROUTINE       :     StartDecoder
 *
 *  INPUTS        :     The handle of the display window.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     TRUE if succeeds else FALSE.
 *
 *  FUNCTION      :     Starts the compressor grabber
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

BOOL CCONV VP5_StartDecoder( PB_INSTANCE **pbi, UINT32 ImageWidth, UINT32 ImageHeight )
{ 
    __try
    {


        // set up our structure holding all formerly global information about a playback instance
        *pbi = VP5_CreatePBInstance();

        // Set Flag to indicate that a key frame is required as the first input
        (*pbi)->ScaleWidth = ImageWidth;
        (*pbi)->ScaleHeight = ImageHeight;
        (*pbi)->OutputWidth = ImageWidth;
        (*pbi)->OutputHeight = ImageHeight;
		(*pbi)->OutputStride = ImageWidth + 32; 
		

        // Validate the combination of height and width.
        (*pbi)->Configuration.VideoFrameWidth = ImageWidth;
        (*pbi)->Configuration.VideoFrameHeight = ImageHeight;

#ifndef MAPCA
        (*pbi)->postproc = CreatePostProcInstance(&(*pbi)->Configuration);
#endif
        //(*pbi)->postproc = CreatePostProcInstance(&(*pbi)->Configuration);
        (*pbi)->quantizer = VP5_CreateQuantizer();

        (*pbi)->ProcessorFrequency = CPUFrequency;
        (*pbi)->DeInterlaceMode = 1;
        // Fills in fragment counts as well
        if(!VP5_InitFrameDetails(*pbi) )
        {
            VP5_DeletePBInstance(pbi);
            return FALSE;
        }


        /* Set last_dct_thresh to an illegal value to make sure the
        *  Q tables are initialised for the new video sequence. 
        */
        (*pbi)->quantizer->LastQuantizerValue = -1;

        // Set up various configuration parameters.
        VP5_InitialiseConfiguration(*pbi);

        #ifdef MAPCA
        InitDMAWriteReconDS(*pbi);
        InitDMAReadReferenceDS(*pbi);
        #endif
        
        return TRUE;
    }

#if defined(_MSC_VER)
    __except( TRUE )
    {
        VP5_ErrorTrap( *pbi, GEN_EXCEPTIONS );
        return FALSE;
    }
#endif

}
/****************************************************************************
 * 
 *  ROUTINE       :     VP5_GetPbParam
 *
 *  INPUTS        :     PB_COMMAND_TYPE Command
 *                      char *          Parameter
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *  
 *  FUNCTION      :     Generalised command interface to decoder.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CCONV VP5_GetPbParam( PB_INSTANCE *pbi, PB_COMMAND_TYPE Command, UINT32 *Parameter )
{
    switch ( Command )
    {
#if defined(POSTPROCESS)
    case PBC_SET_POSTPROC:
        *Parameter =pbi->PostProcessingLevel;
#endif

    default:
        break;
    }
}


#define CRITICALWATERMARK (int) (31000 * pbi->CPUFree / 100)
#define DOWNWATERMARK (int) (30000 * pbi->CPUFree / 100)
#define UPWATERMARK   (int) (28000 * pbi->CPUFree / 100)
int PickPostProcessingLevel(PB_INSTANCE *pbi)
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
	if(thisTime > (int) (CRITICALWATERMARK))
	{
		// this frame's taking to long try to make up time on the subsequent frames
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
#ifndef MAPCA
/****************************************************************************
 * 
 *  ROUTINE       :     VP5_GetYUVConfig
 *
 *  INPUTS        :     YUV_BUFFER_CONFIG  * YuvConfig
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *  
 *  FUNCTION      :     Gets details of the reconstruction buffer
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CCONV VP5_GetYUVConfig( PB_INSTANCE (*pbi), YUV_BUFFER_CONFIG * YuvConfig )
{
    __try 
    {
#ifdef _MSC_VER
		unsigned int duration;
		unsigned int starttsc,endtsc;
		VP5_readTSC(&starttsc);
		pbi->PostProcessingLevel = PickPostProcessingLevel(pbi);
#endif
        if( pbi->PostProcessingLevel ||(pbi->Configuration.Interlaced && pbi->DeInterlaceMode))
        {
#ifdef _MSC_VER
            extern void vp5_showinfo2(PB_INSTANCE *pbi);
            extern void vp5_showinfo(PB_INSTANCE *pbi);
			
			
            if(pbi->PostProcessingLevel > 200 )
            {
                PostProcess
					(
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel-200,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,        
                    sizeof(FRAG_INFO),
                    0x0001
                    );
				VP5_readTSC(&endtsc);
                vp5_showinfo(pbi);
            }
            else if(pbi->PostProcessingLevel > 100 )
            {
				
				PostProcess
                    (
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel-100,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,                
                    sizeof(FRAG_INFO),
                    0x0001
                    );
				VP5_readTSC(&endtsc);
				vp5_showinfo2(pbi);
            }
            else
#endif
			{
				pbi->AvgFrameQIndex = pbi->quantizer->FrameQIndex;
				
                PostProcess
                    (
                    pbi->postproc,                  
                    pbi->Vp3VersionNo,          
                    pbi->FrameType,             
                    pbi->PostProcessingLevel,   
                    pbi->AvgFrameQIndex,            
                    pbi->LastFrameRecon,        
                    pbi->PostProcessBuffer,     
                    (unsigned char *) pbi->FragInfo,                
                    sizeof(FRAG_INFO),
                    0x0001
					);
#ifdef _MSC_VER
                VP5_readTSC(&endtsc);
#endif
			}
			
        }

        if(pbi->BlackClamp)
        {
            ClampLevels( pbi->postproc,pbi->BlackClamp,pbi->WhiteClamp,pbi->PostProcessBuffer,	pbi->PostProcessBuffer);
        }
        if( pbi->Configuration.VideoFrameWidth < pbi->OutputWidth ||
            pbi->Configuration.VideoFrameHeight < pbi->OutputHeight )
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
			{
	            ScaleOrCenter( pbi->postproc, pbi->PostProcessBuffer, YuvConfig  );
			}
			else
			{
	            ScaleOrCenter( pbi->postproc, pbi->LastFrameRecon, YuvConfig  );
			}

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
            //YuvConfig->UVStride = pbi->OutputWidth / 2;
        }
        else
        {
            YuvConfig->YWidth = pbi->Configuration.VideoFrameWidth;
            YuvConfig->YHeight = pbi->Configuration.VideoFrameHeight;
            YuvConfig->YStride = pbi->Configuration.YStride;
            
            YuvConfig->UVWidth = pbi->Configuration.VideoFrameWidth / 2;
            YuvConfig->UVHeight = pbi->Configuration.VideoFrameHeight / 2;
            YuvConfig->UVStride = pbi->Configuration.UVStride;

            //if(pbi->PostProcessingLevel && (pbi->quantizer->FrameQIndex < PPROC_QTHRESH))
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
		{
			pbi->avgPPTime[pbi->PostProcessingLevel%10] = duration;
		}
		else
		{
			pbi->avgPPTime[pbi->PostProcessingLevel%10] = ( 7 * pbi->avgPPTime[pbi->PostProcessingLevel%10] + duration ) >> 3;
		}
#endif
    }
#if defined(_MSC_VER)   
    __except ( TRUE )
    {
        VP5_ErrorTrap( pbi, GEN_EXCEPTIONS );
    }    
#endif
}
#endif
/****************************************************************************  
Debugging Aid Only */ 

void writeframeYX(PB_INSTANCE *pbi, char * address,int x) 
{ 	// write the frame 	
	FILE *yframe; 	
	char filename[255]; 	
#ifdef MAPCA
    sprintf(filename,"MapYF%d.raw",x); 	
#else
    sprintf(filename,"PcYF%d.raw",x); 	
#endif
	yframe=fopen(filename,"wb"); 	
	fwrite(address,pbi->ReconYPlaneSize,1,yframe); 	
	fclose(yframe); 
} 

/****************************************************************************
 * 
 *  ROUTINE       :     VP5_DecodeFrameToYUV
 *
 *  INPUTS        :     UINT8 * VideoBufferPtr
 *                              Compressed input video data
 *
 *                      UINT32  ByteCount 
 *                              Number of bytes compressed data in buffer. *  
 *
 *                      UINT32  Height and width of image to be decoded
 *
 *  OUTPUTS       :     None
 *                      None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Decodes a frame into the internal YUV reconstruction buffer.
 *                      Details of this buffer can be obtained by calling GetYUVConfig().
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
int CCONV VP5_DecodeFrameToYUV( PB_INSTANCE (*pbi), char * VideoBufferPtr, unsigned int ByteCount,
                             UINT32 ImageWidth,     UINT32 ImageHeight )
{
    unsigned char *tmp;
    (void) ImageHeight;
    (void) ImageWidth;
    __try
    {
#ifdef _MSC_VER
		unsigned int duration;
		unsigned int starttsc,endtsc;
        VP5_readTSC(&starttsc);
#endif
		pbi->CurrentFrameSize = ByteCount;

        //  start the boolean decoder
        StartDecode(&pbi->br, (unsigned char*)VideoBufferPtr);

        // decode the frame header
        if ( !VP5_LoadFrame(pbi) )
            return -1;

        
        // decode and reconstruct frame
        DecodeFrameMbs(pbi);

		// switch pointers so lastframe recon is this frame
        tmp = pbi->LastFrameRecon;
        pbi->LastFrameRecon = pbi->ThisFrameRecon;
        pbi->ThisFrameRecon = tmp;


#ifndef MAPCA
        // update the border 
        UpdateUMVBorder(pbi->postproc, pbi->LastFrameRecon);
#else
        VP5_UpdateUMVBorder(pbi, pbi->LastFrameRecon);
#endif

        
		if( pbi->FrameType == BASE_FRAME )
    	{
            memcpy(pbi->GoldenFrame, pbi->LastFrameRecon, pbi->ReconYPlaneSize + 2* pbi->ReconUVPlaneSize); 
		}

#ifdef MAPCA
		//if(debugme<1)
		{
			//EtiSysDcFlushDcache();
            //writeframeYX(pbi,pbi->LastFrameRecon,debugme);
			//debugme++;
		}
#endif
		// If appropriate clear the MMX state.
        ClearSysState();

		//temp
		//vp5_appendframe(pbi);

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
        VP5_readTSC(&endtsc);

		duration = (endtsc-starttsc)/ (pbi->ProcessorFrequency) ;

		pbi->thisDecodeTime = duration;

		if( pbi->avgDecodeTime == 0)
		{
			pbi->avgDecodeTime = duration;
		}
		else
		{
			pbi->avgDecodeTime = (7*pbi->avgDecodeTime + duration)>>3;
		}

#endif


    }
#if defined(_MSC_VER) 
    __except ( TRUE )
    {
        VP5_ErrorTrap( pbi, GEN_EXCEPTIONS );
        return -2;
    }
#endif    
    return 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP5_StopDecoder
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None .
 *
 *  FUNCTION      :     Stops the encoder and grabber
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

int CCONV VP5_StopDecoder(PB_INSTANCE **pbi)
{

#ifdef MAPCA
    CloseDMAReadReferenceDS();
    CloseDMAWriteReconDS();
#endif
    
    __try
    {
        if(*pbi)
        {
            // Set flag to say that the decoder is no longer initialised
            VP5_DeleteQuantizer(&(*pbi)->quantizer);
#ifndef MAPCA
            DeletePostProcInstance(&(*pbi)->postproc);
#endif
            VP5_DeleteFragmentInfo(*pbi);
            VP5_DeleteFrameInfo(*pbi);


            VP5_DeletePBInstance(pbi);
        
            return TRUE;
        }
    }

#if defined(_MSC_VER)        
    __except ( TRUE )
    {
        VP5_ErrorTrap( *pbi, GEN_EXCEPTIONS );
        return FALSE;
    }
#endif    
    return TRUE;
}

#ifndef MAPCA
/****************************************************************************
 * 
 *  ROUTINE       :     VP5_ErrorTrap
 *
 *  INPUTS        :     Nonex.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Called when a fatal error is detected.
 *                      Sets an error flag and loops untill the thread is
 *                      terminated.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
 static void VP5_ErrorTrap( PB_INSTANCE *pbi, int ErrorCode )
 {
 }
#endif


