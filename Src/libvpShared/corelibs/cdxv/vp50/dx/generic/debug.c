#include "pbdll.h"
#include "misc_common.h"


//#define     OVERLAY_MOTION_VECTORS
#include "xprintf.h"
#if defined OVERLAY_MOTION_VECTORS
/****************************************************************************
 * 
 *  ROUTINE       :     DrawVector
 *
 *  INPUTS        :     PB_INSTANCE *pbi
 *						UINT8 *BlockPtr
 *						INT32 x
 *						INT32 y
 *						UINT8 VectorColour
 *						UINT8 DotColour
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None .
 *
 *  FUNCTION      :     Draws motion vector into reconstruction buffer
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DrawVector( PB_INSTANCE *pbi, UINT8 *BlockPtr, INT32 x, INT32 y, UINT8 VectorColour, UINT8 DotColour )
{
	UINT8 *PixelPtr;
	double Xpos, Ypos;
	double Xdelta, Ydelta;
	INT32 x0, x1, y0, y1;

	if ( abs(x) > abs(y) )
	{
		// Step along x axis
		if ( x < 0 )
		{
			x0 = x;
			x1 = 0;
			Ypos = (double)y;
		}
		else
		{
			x0 = 0;
			x1 = x;
			Ypos = 0.0;
		}

		Ydelta = (double)y / (double)x;

		for ( x=x0; x<=x1; x++ )
		{
			y = (UINT32)( Ypos<0.0 ? (Ypos-0.5) : (Ypos+0.5) );
			PixelPtr = BlockPtr + y*pbi->Configuration.YStride + x;
			*PixelPtr = VectorColour;
			Ypos += Ydelta;
		}
	}
	else if ( abs(y) > abs(x) )
	{
		// Step along y axis
		if ( y < 0 )
		{
			y0 = y;
			y1 = 0;
			Xpos = (double)x;
		}
		else
		{
			y0 = 0;
			y1 = y;
			Xpos = 0.0;
		}

		Xdelta = (double)x / (double)y;

		for ( y=y0; y<=y1; y++ )
		{
			x = (UINT32)( Xpos<0.0 ? (Xpos-0.5) : (Xpos+0.5) );
			PixelPtr = BlockPtr + y*pbi->Configuration.YStride + x;
			*PixelPtr = VectorColour;
			Xpos += Xdelta;
		}
	}

	// Indicate current position in specified colour
	*BlockPtr = DotColour;
}


/****************************************************************************
 * 
 *  ROUTINE       :     DisplayMotionVectors
 *
 *  INPUTS        :     PB_INSTANCE *pbi
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None .
 *
 *  FUNCTION      :     Overlays colour coded motion vectors into reconstruction buffer
 *
 *  SPECIAL NOTES :     This routine will only display motion vectors when Post-processing
 *						is enabled since it draws into the PostProcessBuffer.
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DisplayMotionVectors ( PB_INSTANCE *pbi )
{
	INT32	FragIndex;			// Fragment number
	UINT32	MB, B;		   		// Macro-Block, Block indices
    UINT32  CodingMethod;       // Temp Storage for coding mode.
	INT32	x, y;
	UINT32	Blocks;
	UINT32	BlockOffset[4] = {0, 1, pbi->HFragments, pbi->HFragments + 1};
	UINT8	*BlockPtr;
	UINT8	DotColour;
	UINT8	VectorColour;

	// Nothing to display if keyframe
    if ( VP5_GetFrameType(pbi) == BASE_FRAME )
    {
        return;
    }

    // Traverse the quad-tree
	for ( MB=0; MB<pbi->YMacroBlocks; MB++ )
	{
		// Is the Macro-Block coded:
//		if ( pbi->MBCodedFlags[MB] )
		{
			CodingMethod = pbi->FragInfo[FragIndex].FragCodingMode;

			if ( VP5_ModeUsesMC[CodingMethod] )
			{
				// Indicate previous/golden frame predictor
				if ( CodingMethod == CODE_GOLDEN_MV )
				{
					DotColour    = 0x00;	// Black dot
					VectorColour = 0x7F;	// Mid-Grey Vector
				}
				else if( (CodingMethod == CODE_INTER_LAST_MV) || (CodingMethod == CODE_INTER_PRIOR_LAST) )
				{
					DotColour    = 0xFF;	// White dot
					VectorColour = 0x00;	// Black Vector	
				}
				else
				{
					DotColour    = 0x00;	// Black dot
					VectorColour = 0xFF;	// White Vector
				}

				if ( CodingMethod == CODE_INTER_FOURMV )
					Blocks = 4;
				else
					Blocks = 1;

				for ( B=0; B<Blocks; B++ )
				{
					// Pointer to top LH-corner of block
					BlockPtr = pbi->PostProcessBuffer ;// sorry adrian I'll fix it soon (removing getfragindex)
						//+ ReconGetFragIndex(pbi->recon_pixel_index_table, FragIndex+BlockOffset[B]);

					// Motion vector ( oops motion vectors only remembered at the macroblock level now!!
					/*
					x = pbi->FragInfo[FragIndex + BlockOffset[B]].MVectorX;
					y = pbi->FragInfo[FragIndex + BlockOffset[B]].MVectorY;
					*/
					DrawVector( pbi, BlockPtr, x/2, y/2, VectorColour, DotColour );
				}
			}
		}
	}
}
#endif
/****************************************************************************
 Debugging Aid Only
*/

/****************************************************************************
 Debugging Aid Only
*/
#ifdef _MSC_VER
#include <stdio.h>
void vp5_writeframe(PB_INSTANCE *pbi, char * address,int x)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"y%04d.raw",x);
	yframe=fopen(filename,"wb");
	fwrite(address,pbi->ReconYPlaneSize+2*pbi->ReconUVPlaneSize,1,yframe);
	fclose(yframe);
}
void vp5_writeframe2(PB_INSTANCE *pbi, char * address,int x)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"y%d.raw",x);
	yframe=fopen(filename,"wb");
	fwrite(address,pbi->YPlaneSize,1,yframe);
	fclose(yframe);
}
void vp5_draw(unsigned char *prefix, int frame, char * address,int size)
{
	// write the frame
	FILE *yframe;
	char filename[255];
	sprintf(filename,"%s%04d.raw",prefix,frame);
	yframe=fopen(filename,"wb");
	fwrite(address,size,1,yframe);
	fclose(yframe);
}
void vp5_drawb(unsigned char *prefix, int frame, char * address,int pitch,int width,int height)
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
void vp5_drawc(char *filename, char * address,int pitch,int width,int height)
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

void vp5_showinfo2(PB_INSTANCE *pbi)
{
//	int i;
//	for (i=0;i<pbi->PostProcessingLevel;i++)
//		pbi->PostProcessBuffer[pbi->Configuration.YStride * 32 + 32 + +4 +4*i] = 255;

	vp5_xprintf(pbi, 
			pbi->Configuration.YStride * 32 + 32, 
			"F:%d Q:%d S:%d W:%d H:%d V:%d Decode:%8d, Blit:%8d, PP:%8d, P:%d",
			pbi->FrameType,
			pbi->quantizer->ThisFrameQuantizerValue,
			pbi->CurrentFrameSize,
			pbi->HFragments,
			pbi->VFragments,
			pbi->Vp3VersionNo,
			pbi->avgDecodeTime,
			pbi->avgBlitTime,
			pbi->avgPPTime[8],
			pbi->PostProcessingLevel);

}
void vp5_appendframe(PB_INSTANCE *pbi)
{
	// write the frame
	FILE *yframe;
	yframe=fopen("test.raw","ab");
	fwrite(pbi->LastFrameRecon,pbi->ReconYPlaneSize+2*pbi->ReconUVPlaneSize,1,yframe);
	fclose(yframe);
}

void vp5_showinfo(PB_INSTANCE *pbi)
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
			vp5_xprintf(pbi, 
				((MBrow+1)* 16+5) * pbi->Configuration.YStride  + (MBcol+1)*16+5, 
				"%d",
				pbi->predictionMode[MBOffset(MBrow,MBcol)]);

		} // mb col


	} // mbrow

	{
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
void PredictBlockToPostProcessBuffer
( 
	PB_INSTANCE *pbi, 
	BLOCK_POSITION bp
)
{
	
	memset(pbi->ReconDataBuffer,0,64*sizeof(short));

	// Action depends on decode mode.
	if ( pbi->mbi.Mode == CODE_INTER_NO_MV )       // Inter with no motion vector
	{
		ReconInter( pbi->TmpDataBuffer, (UINT8 *)&pbi->PostProcessBuffer[pbi->mbi.Recon], 
			(UINT8 *)&pbi->LastFrameRecon[pbi->mbi.Recon], 
			pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride);
		
	}
	else if ( VP5_ModeUsesMC[pbi->mbi.Mode] )          // The mode uses a motion vector.
	{
		// For the compressor we did this already ( possible optimization).
		PredictFilteredBlock( pbi, pbi->TmpDataBuffer,bp);

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
}

 
void printmodes(PB_INSTANCE *pbi)
{
    static int nFrame = 0;  //  PB_INSTANCE doesn't provide a frame number, does it?
    FILE *f=fopen("modes.txt","a");
    unsigned int i,j;

    fprintf(f, "Frame %d\n\n", nFrame);

    for(i=2;i<pbi->MBRows-2;i++)
    {
		if(pbi->Configuration.Interlaced == 1)
		{
			for(j=2;j<pbi->MBCols-2;j++)
			{
				fprintf(f,"%d",pbi->MBInterlaced[MBOffset(i,j)]);
			}
			fprintf(f,"   ");
		}
		for(j=2;j<pbi->MBCols-2;j++)        
		{
            fprintf(f,"%d",pbi->predictionMode[MBOffset(i,j)]);
        }
        fprintf(f,"   ");
		for(j=2;j<pbi->MBCols-2;j++)        
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
