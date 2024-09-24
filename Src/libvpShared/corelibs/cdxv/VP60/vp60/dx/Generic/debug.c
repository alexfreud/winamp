#include "pbdll.h"
#include "misc_common.h"
#include "xprintf.h"

/****************************************************************************
 * Debugging Aid Only
 ****************************************************************************
*/

#ifdef _MSC_VER

#include <stdio.h>

void vp6_writeframe(PB_INSTANCE *pbi, char * address,int x)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"y%04d.raw",x);
	yframe=fopen(filename,"wb");
	fwrite(address,pbi->ReconYPlaneSize+2*pbi->ReconUVPlaneSize,1,yframe);
	fclose(yframe);
}

void vp6_writeframe2(PB_INSTANCE *pbi, char * address,int x)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"y%d.raw",x);
	yframe=fopen(filename,"wb");
	fwrite(address,pbi->YPlaneSize,1,yframe);
	fclose(yframe);
}

void vp6_draw(unsigned char *prefix, int frame, char * address,int size)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"%s%04d.raw",prefix,frame);
	yframe=fopen(filename,"wb");
	fwrite(address,size,1,yframe);
	fclose(yframe);
}

void vp6_drawb(unsigned char *prefix, int frame, char * address,int pitch,int width,int height)
{
	// write the frame
	FILE *yframe;
	int i;
	char filename[255];
	sprintf(filename,"%s%04d.raw",prefix,frame);
	yframe=fopen(filename,"wb");
	for(i=0;i<height;i++)
	{
		fwrite(address,width,1,yframe);
		address+=pitch;
	}
	fclose(yframe);
}

void vp6_drawc(char *filename, char * address,int pitch,int width,int height)
{
	// write the frame
	FILE *yframe;
	int i;
	yframe=fopen(filename,"ab");
	for(i=0;i<height;i++)
	{
		fwrite(address,width,1,yframe);
		address+=pitch;
	}
	fclose(yframe);
}

void vp6_showinfo2(PB_INSTANCE *pbi)
{
	vp6_xprintf(pbi, 
			pbi->Configuration.YStride * UMV_BORDER + UMV_BORDER, 
			"F:%d G:%d Q:%d S:%d B: %d W:%d H:%d V:%d Decode:%8d, Blit:%8d, PP:%8d, P:%d",
			pbi->FrameType,
            pbi->RefreshGoldenFrame,
			pbi->quantizer->FrameQIndex,
			pbi->CurrentFrameSize,
            pbi->br.pos,
			pbi->HFragments,
			pbi->VFragments,
			pbi->Vp3VersionNo,
			pbi->avgDecodeTime,
			pbi->avgBlitTime,
			pbi->avgPPTime[8],
			pbi->PostProcessingLevel);
}

void vp6_appendframe(PB_INSTANCE *pbi)
{
	// write the frame
	FILE *yframe;
	yframe=fopen("test.raw","ab");
	fwrite(pbi->LastFrameRecon,pbi->ReconYPlaneSize+2*pbi->ReconUVPlaneSize,1,yframe);
	fclose(yframe);
}

void vp6_showinfo(PB_INSTANCE *pbi)
{
	UINT32 MBrow, MBcol;
	UINT32 MBRows = pbi->MBRows; 
	UINT32 MBCols = pbi->MBCols;

	// for each row of macroblocks 
	for ( MBrow=0; MBrow<MBRows; MBrow++ )
	{
		// for each macroblock within a row of macroblocks
		for ( MBcol=0; MBcol<MBCols; MBcol++)
		{
			vp6_xprintf(pbi, 
				((MBrow)* 16+5) * pbi->Configuration.YStride  + (MBcol)*16+5, 
				"%d",
				pbi->predictionMode[MBOffset(MBrow,MBcol)]);
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     PredictBlockToPostProcessBuffer
 *
 *  INPUTS        :     
 *						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Codes a DCT block
 *
 *                      Motion vectors and modes asumed to be defined at the MB level.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP6_PredictBlockToPostProcessBuffer ( PB_INSTANCE *pbi, BLOCK_POSITION bp )
{
/*
we need a VP6_PredictMacroBlockToPostProcessBuffer

	memset(pbi->ReconDataBuffer,0,64*sizeof(short));

	// Action depends on decode mode.
	if ( pbi->mbi.Mode == CODE_INTER_NO_MV )       // Inter with no motion vector
	{
		ReconInter( pbi->TmpDataBuffer, (UINT8 *)&pbi->PostProcessBuffer[pbi->mbi.Recon], 
			(UINT8 *)&pbi->LastFrameRecon[pbi->mbi.Recon], 
			pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride);
	}
	else if ( VP6_ModeUsesMC[pbi->mbi.Mode] )          // The mode uses a motion vector.
	{
		// For the compressor we did this already ( possible optimization).
		VP6_PredictFilteredBlock( pbi, pbi->TmpDataBuffer,bp);

		ReconBlock( 
			pbi->TmpDataBuffer,
			pbi->ReconDataBuffer,
			(UINT8 *)&pbi->PostProcessBuffer[pbi->mbi.Recon],
			pbi->mbi.CurrentReconStride );
	}
	else if ( pbi->mbi.Mode == CODE_USING_GOLDEN )     // Golden frame with motion vector
	{
		// Reconstruct the pixel data using the golden frame reconstruction and change data
		ReconInter( pbi->TmpDataBuffer, (UINT8 *)&pbi->PostProcessBuffer[pbi->mbi.Recon], 
			(UINT8 *)&pbi->GoldenFrame[ pbi->mbi.Recon ], 
			pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride );
	}
	else                                            // Simple Intra coding
	{
		// Get the pixel index for the first pixel in the fragment.
		ReconIntra( pbi->TmpDataBuffer, (UINT8 *)&pbi->PostProcessBuffer[pbi->mbi.Recon], (UINT16 *)pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride );
	} 
*/
}

void VP6_printmodes(PB_INSTANCE *pbi)
{
    static int nFrame = 0;  //  PB_INSTANCE doesn't provide a frame number, does it?
    FILE *f=fopen("modes.txt","a");
    unsigned int i,j;

    fprintf(f, "Frame %d\n\n", nFrame);

    for(i=BORDER_MBS;i<pbi->MBRows-BORDER_MBS;i++)
    {
		if(pbi->Configuration.Interlaced == 1)
		{
			for(j=BORDER_MBS;j<pbi->MBCols-BORDER_MBS;j++)
			{
				fprintf(f,"%d",pbi->MBInterlaced[MBOffset(i,j)]);
			}
			fprintf(f,"   ");
		}
		for(j=BORDER_MBS;j<pbi->MBCols-BORDER_MBS;j++)        
		{
            fprintf(f,"%d",pbi->predictionMode[MBOffset(i,j)]);
        }
        fprintf(f,"   ");
		for(j=BORDER_MBS;j<pbi->MBCols-BORDER_MBS;j++)        
        {
            fprintf(f,"%3d:%-3d",pbi->MBMotionVector[MBOffset(i,j)].x,pbi->MBMotionVector[MBOffset(i,j)].y);
        }
        fprintf(f,"\n");
	}

    fprintf(f,"\n");
    fprintf(f,"\n");
    fclose(f);

    ++nFrame;

    return;
}

#endif
