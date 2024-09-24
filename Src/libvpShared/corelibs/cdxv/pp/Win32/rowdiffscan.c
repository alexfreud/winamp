/****************************************************************************
*
*   Module Title :     RowDiffScan.c
*
*   Description  :     Pre-processor row difference Scan
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.00 JBB 22 AUG 00	Configuration baseline
*
*****************************************************************************
*/						

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "type_aliases.h"
#include "preproc.h"


/****************************************************************************
*  Module constants.
*****************************************************************************

/****************************************************************************
 * 
 *  ROUTINE       :     RowDiffScan
 *
 *  INPUTS        :     UINT8  * YuvPtr1, YuvPtr2 
 *								 Pointers into current and previous frame
 *                      BOOL     EdgeRow
 *                               Is this row an edge row.
 *
 *  OUTPUTS       :		UINT16 * YUVDiffsPtr
 *								 Differnece map
 *                      UINT8  * bits_map_ptr
 *                               Pixels changed map
 *                      UINT8  * SgcPtr
 *								 Level change score.
 *                      INT8   * DispFragPtr
 *                               Block update map.
 *                      INT32  * RowDiffsPtr
 *								 Total sig changes for row
 *                      UINT8  * ChLocalsPtr
 *                               Changed locals data structure
 *
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Initial pixel differences scan
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void RowDiffScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, 
                  INT16 * YUVDiffsPtr, UINT8 * bits_map_ptr, 
                  INT8  * SgcPtr, INT8  * DispFragPtr, 
				  UINT8 * FDiffPixels, INT32 * RowDiffsPtr, 
                  UINT8 * ChLocalsPtr, BOOL EdgeRow )
{
    INT32 i;
    INT32 FragChangedPixels;

    INT16 Diff[8];

    UINT32  ZeroData[2] = { 0,0 };
    UINT8   OneData[8] = { 1,1,1,1,1,1,1,1 };
    UINT8   ChlocalsDummyData[8] = { 8,8,8,8,8,8,8,8 };

    // Cannot use kernel if at edge or if PAK disabled
    if ( (!ppi->PAKEnabled) || EdgeRow )
    {
        for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
        {
            // Reset count of pixels changed for the current fragment.
            FragChangedPixels = 0;

            // Test for break out conditions to save time. 
			if ((*DispFragPtr == CANDIDATE_BLOCK) )//|| !ppi->EarlyBreakAllowed)
			{
                
				__asm
				{
				
					movd		esi, [YuvPtr1];
					movd		ebx, [YuvPtr2];
					movd		edx, FragChangedPixels
					pxor		mm7, mm7;

					movq		mm0, [esi]			;76543210
					movq		mm1, [ebx]			;76543210

					movq		mm2, mm0			;make a copy
					movq		mm3, mm1			;make a copy

					punpcklbw	mm0, mm7			; 3 2 1 0
					punpcklbw	mm1, mm7			; 3 2 1 0

					punpckhbw	mm2, mm7			; 7 6 5 4
					punpckhbw   mm3, mm7			; 7 6 5 4

					psubw		mm0	 mm1			; Diff[3,2,1,0]
					psubw		mm2, mm3			; Diff[7,6,5,4]
					
					movq		QWORD PTR [YUVDiffsPtr], mm0
					movq		QWORD PTR [YUVDiffsPtr], mm2					

				;------------------------------------------------------
				;	mm0, mm1, mm3, mm4, m5, mm6, mm7, Free		
				;	mm2, keep the Diff[7 6 5 4]
				;------------------------------------------------------
					
					movd		eax, ppi->LevelThresh

					movd		mm1, eax			;
					movd		mm3, eax			;
					
					packsdw		mm1, mm3			;
					movq		mm4, mm1			;
					
					psllw		mm1, 16
					por			mm1, mm4			;4 ppi->LevelThresh
					

				;-------------------------------------------------------
				;	mm3, mm4, mm5, mm6, mm7 Free
				;   
				;-------------------------------------------------------

					movd		eax, ppi->SrfThresh
					
					movd		mm3, eax			;
					movd		mm4, eax			;

					packsdw		mm3, mm4			;
					movq		mm5, mm3			;

					psllw		mm3, 16
					por			mm3, mm6			;4 ppi->SrfThresh

				;--------------------------------------------------------
				;	mm0 mm2		diff[0]-diff[7]
				;	mm1			ppi->LevelThresh
				;	mm3			ppi->SrfThresh
				;	mm4-mm7		free
				;	Note,	ppi->NegLevelThresh = - ppi->LevelThresh
				;			ppi->NegSrfThresh = - ppi->SrfThresh
				;--------------------------------------------------------

					movq		mm4, mm0			; diff[0][1][2][3]
					movq		mm5, mm0			;

					psubsw		mm4, mm1			; if diff >= LevelThresh
					psraw		mm4, 15				; 00s(True) and ffs (False)
					pandn		mm4, FFFFFFFFh		; ffs(True) and 00s (False)
					psrlw		mm4, 15				; 01 (True) and 00	(False)

					pcmpgtw		mm5, mm3			; if diff > SrfThresh
													; ffs(True) and 00s (False)
					psrlw		mm5, 15				; 01 (True) and 00  (False)					
					pand		mm5, mm4			; 

					
					movq		mm7, mm0			; save a copy of diff[0][1][2][3]
					pxor		mm6, mm6			; clear MM6

					psubsw		mm6, mm1			; mm6 = NegLevelThresh
					pcmpgtw		mm0, mm6			; if diff > NegLevelThresh
													; ffs(True) and 00s (False)
					pandn		mm0, FFFFFFFFh		; if diff <= NegLevelThresh
													; ffs(True) and 00	(False)
					psrlw		mm0, 15				; 01 (True) and 00  (False)

					paddsw		mm7, mm3			; if diff < -NegSrfThresh
					psraw		mm7, 15				; ffs(True)	and 00s (False)
					
					psrlw	    mm7, 15				; 01 (True) and 00s (False)
					pand		mm7, mm0			;
					
				;----------------------------------------------------------------------------
				; mm0, mm1, mm2, mm3, mm4, mm5, mm7		 in use
				; mm6	free
				;----------------------------------------------------------------------------
					
					por			mm5, mm7			; mm7 is free now
					pxor		mm6, mm6			;
					movq		mm7, mm5			;
					punpcklwd	mm5, mm6			;
					punpckhwd	mm7, mm6			;

					paddw		mm5, mm7			;
					movq		mm7, mm5			;

					psrlq		mm7, 32				;
					paddd		mm7, mm5			;

					movd		eax, mm7			;
					
					add			eax, ebx

					


				// Calculate the diference values and copy to YUVDiffsPtr
				Diff[0] = ((INT16)YuvPtr1[0]) - ((INT16)YuvPtr2[0]);
			    Diff[1] = ((INT16)YuvPtr1[1]) - ((INT16)YuvPtr2[1]);
			    Diff[2] = ((INT16)YuvPtr1[2]) - ((INT16)YuvPtr2[2]);
			    Diff[3] = ((INT16)YuvPtr1[3]) - ((INT16)YuvPtr2[3]);
                ((INT32 *)YUVDiffsPtr)[0] = ((INT32 *)Diff)[0];
                ((INT32 *)YUVDiffsPtr)[1] = ((INT32 *)Diff)[1];

				// Test against the Level and ppi->SRF thresholds and record the results
                // Pixel 0
				if ( Diff[0] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[0] > ppi->SrfThresh )
					{          
						bits_map_ptr[0] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[0] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[0] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[0] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 1
				if ( Diff[1] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[1] > ppi->SrfThresh )
					{          
						bits_map_ptr[1] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[1] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[1] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[1] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 2
				if ( Diff[2] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[2] > ppi->SrfThresh )
					{          
						bits_map_ptr[2] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[2] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[2] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[2] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 3
				if ( Diff[3] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[3] > ppi->SrfThresh )
					{          
						bits_map_ptr[3] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[3] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[3] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[3] = 1;
						FragChangedPixels++;
					}
				}

                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];

                // Calculate the diference values and copy to YUVDiffsPtr
			    Diff[4] = ((INT16)YuvPtr1[4]) - ((INT16)YuvPtr2[4]);
			    Diff[5] = ((INT16)YuvPtr1[5]) - ((INT16)YuvPtr2[5]);
			    Diff[6] = ((INT16)YuvPtr1[6]) - ((INT16)YuvPtr2[6]);
			    Diff[7] = ((INT16)YuvPtr1[7]) - ((INT16)YuvPtr2[7]);
                ((INT32 *)YUVDiffsPtr)[2] = ((INT32 *)Diff)[2];
                ((INT32 *)YUVDiffsPtr)[3] = ((INT32 *)Diff)[3];

				// Test against the Level and ppi->SRF thresholds and record the results
                // Pixel 4
				if ( Diff[4] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[4] > ppi->SrfThresh )
					{          
						bits_map_ptr[4] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[4] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[4] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[4] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 5
				if ( Diff[5] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[5] > ppi->SrfThresh )
					{          
						bits_map_ptr[5] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[5] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[5] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[5] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 6
				if ( Diff[6] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[6] > ppi->SrfThresh )
					{          
						bits_map_ptr[6] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[6] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[6] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[6] = 1;
						FragChangedPixels++;
					}
				}
                // Pixel 7
				if ( Diff[7] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;
					if ( Diff[7] > ppi->SrfThresh )
					{          
						bits_map_ptr[7] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[7] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;
					if ( Diff[7] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[7] = 1;
						FragChangedPixels++;
					}
				}

                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
	        }
            else
            {
                // For EBO coded blocks mark all pixels as changed.
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                    ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }
                else
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
                }
            }

			*RowDiffsPtr += FragChangedPixels;
			*FDiffPixels += (UINT8)FragChangedPixels;

			YuvPtr1 += ppi->HFragPixels;
			YuvPtr2 += ppi->HFragPixels;
			bits_map_ptr += ppi->HFragPixels;
            ChLocalsPtr += ppi->HFragPixels;
			YUVDiffsPtr += ppi->HFragPixels;
			SgcPtr ++;
			FDiffPixels ++;

			// If we have a lot of changed pixels for this fragment on this row then 
			// the fragment is almost sure to be picked (e.g. through the line search) so we
			// can mark it as selected and then ignore it.
			// if ( ppi->EarlyBreakAllowed )
			{
				if (FragChangedPixels >= 7)
				{
					*DispFragPtr = BLOCK_CODED;
				}
			}
			DispFragPtr++;    
		}
    }
    else
    {
        for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
        {
            // Reset count of pixels changed for the current fragment.
            FragChangedPixels = 0;

            // Test for break out conditions to save time. 
			if ((*DispFragPtr == CANDIDATE_BLOCK) )//|| !ppi->EarlyBreakAllowed)
			{
                // Calculate the diference values and copy to YUVDiffsPtr
			    Diff[0] = ((INT16)YuvPtr1[0]) - ((INT16)YuvPtr2[0]);
			    Diff[1] = ((INT16)YuvPtr1[1]) - ((INT16)YuvPtr2[1]);
			    Diff[2] = ((INT16)YuvPtr1[2]) - ((INT16)YuvPtr2[2]);
			    Diff[3] = ((INT16)YuvPtr1[3]) - ((INT16)YuvPtr2[3]);
                ((INT32 *)YUVDiffsPtr)[0] = ((INT32 *)Diff)[0];
                ((INT32 *)YUVDiffsPtr)[1] = ((INT32 *)Diff)[1];

				// Test against the Level and ppi->SRF thresholds and record the results
                // Pixel 0
				if ( Diff[0] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[0] > ppi->SrfThresh) && (Diff[0] <= ppi->HighChange) )
						Diff[0] = (int)ApplyPakLowPass( ppi, &YuvPtr1[0] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[0] );

					if ( Diff[0] > ppi->SrfThresh )
					{          
						bits_map_ptr[0] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[0] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[0] < ppi->NegSrfThresh) && (Diff[0] >= ppi->NegHighChange) )
						Diff[0] = (int)ApplyPakLowPass( ppi, &YuvPtr1[0] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[0] );

					if ( Diff[0] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[0] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 1
				if ( Diff[1] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[1] > ppi->SrfThresh) && (Diff[1] <= ppi->HighChange) )
						Diff[1] = (int)ApplyPakLowPass( ppi, &YuvPtr1[1] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[1] );

					if ( Diff[1] > ppi->SrfThresh )
					{          
						bits_map_ptr[1] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[1] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[1] < ppi->NegSrfThresh) && (Diff[1] >= ppi->NegHighChange) )
						Diff[1] = (int)ApplyPakLowPass( ppi, &YuvPtr1[1] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[1] );

					if ( Diff[1] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[1] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 2
				if ( Diff[2] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[2] > ppi->SrfThresh) && (Diff[2] <= ppi->HighChange) )
						Diff[2] = (int)ApplyPakLowPass( ppi, &YuvPtr1[2] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[2] );

					if ( Diff[2] > ppi->SrfThresh )
					{          
						bits_map_ptr[2] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[2] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[2] < ppi->NegSrfThresh) && (Diff[2] >= ppi->NegHighChange) )
						Diff[2] = (int)ApplyPakLowPass( ppi, &YuvPtr1[2] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[2] );

					if ( Diff[2] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[2] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 3
				if ( Diff[3] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[3] > ppi->SrfThresh) && (Diff[3] <= ppi->HighChange) )
						Diff[3] = (int)ApplyPakLowPass( ppi, &YuvPtr1[3] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[3] );

					if ( Diff[3] > ppi->SrfThresh )
					{          
						bits_map_ptr[3] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[3] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[3] < ppi->NegSrfThresh) && (Diff[3] >= ppi->NegHighChange) )
						Diff[3] = (int)ApplyPakLowPass( ppi, &YuvPtr1[3] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[3] );

					if ( Diff[3] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[3] = 1;
						FragChangedPixels++;
					}
    			}

                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];

                // Calculate the diference values and copy to YUVDiffsPtr
			    Diff[4] = ((INT16)YuvPtr1[4]) - ((INT16)YuvPtr2[4]);
			    Diff[5] = ((INT16)YuvPtr1[5]) - ((INT16)YuvPtr2[5]);
			    Diff[6] = ((INT16)YuvPtr1[6]) - ((INT16)YuvPtr2[6]);
			    Diff[7] = ((INT16)YuvPtr1[7]) - ((INT16)YuvPtr2[7]);
                ((INT32 *)YUVDiffsPtr)[2] = ((INT32 *)Diff)[2];
                ((INT32 *)YUVDiffsPtr)[3] = ((INT32 *)Diff)[3];

				// Test against the Level and ppi->SRF thresholds and record the results
                // Pixel 4
				if ( Diff[4] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[4] > ppi->SrfThresh) && (Diff[4] <= ppi->HighChange) )
						Diff[4] = (int)ApplyPakLowPass( ppi, &YuvPtr1[4] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[4] );

					if ( Diff[4] > ppi->SrfThresh )
					{          
						bits_map_ptr[4] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[4] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[4] < ppi->NegSrfThresh) && (Diff[4] >= ppi->NegHighChange) )
						Diff[4] = (int)ApplyPakLowPass( ppi, &YuvPtr1[4] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[4] );

					if ( Diff[4] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[4] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 5
				if ( Diff[5] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[5] > ppi->SrfThresh) && (Diff[5] <= ppi->HighChange) )
						Diff[5] = (int)ApplyPakLowPass( ppi, &YuvPtr1[5] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[5] );

					if ( Diff[5] > ppi->SrfThresh )
					{          
						bits_map_ptr[5] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[5] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[5] < ppi->NegSrfThresh) && (Diff[5] >= ppi->NegHighChange) )
						Diff[5] = (int)ApplyPakLowPass( ppi, &YuvPtr1[5] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[5] );

					if ( Diff[5] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[5] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 6
				if ( Diff[6] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
			        if ( (Diff[6] > ppi->SrfThresh) && (Diff[6] <= ppi->HighChange) )
						Diff[6] = (int)ApplyPakLowPass( ppi, &YuvPtr1[6] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[6] );

					if ( Diff[6] > ppi->SrfThresh )
					{          
						bits_map_ptr[6] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[6] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[6] < ppi->NegSrfThresh) && (Diff[6] >= ppi->NegHighChange) )
						Diff[6] = (int)ApplyPakLowPass( ppi, &YuvPtr1[6] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[6] );

					if ( Diff[6] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[6] = 1;
						FragChangedPixels++;
					}
    			}

                // Pixel 7
				if ( Diff[7] >= ppi->LevelThresh )
				{         
					SgcPtr[0]++;

					// If the level change is still suspect then apply PAK kernel.
			        if ( (Diff[7] > ppi->SrfThresh) && (Diff[7] <= ppi->HighChange) )
						Diff[7] = (int)ApplyPakLowPass( ppi, &YuvPtr1[7] ) - 
							      (int)ApplyPakLowPass( ppi, &YuvPtr2[7] );

					if ( Diff[7] > ppi->SrfThresh )
					{          
						bits_map_ptr[7] = 1;
						FragChangedPixels++;
					}    
				}
				else if ( Diff[7] <= ppi->NegLevelThresh )
				{
					SgcPtr[0]--;

					// If the level change is still suspect then apply PAK kernel.
					if ( (Diff[7] < ppi->NegSrfThresh) && (Diff[7] >= ppi->NegHighChange) )
						Diff[7] = (int)ApplyPakLowPass( ppi, &YuvPtr1[7] ) - 
                                  (int)ApplyPakLowPass( ppi, &YuvPtr2[7] );

					if ( Diff[7] < ppi->NegSrfThresh )
					{          
						bits_map_ptr[7] = 1;
						FragChangedPixels++;
					}
    			}

                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
            }
            else
            {
                // For EBO coded blocks mark all pixels as changed.
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];

                    ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }
                else
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
                }
            }

			*RowDiffsPtr += FragChangedPixels;
			*FDiffPixels += (UINT8)FragChangedPixels;

            YuvPtr1 += ppi->HFragPixels;
            YuvPtr2 += ppi->HFragPixels;
            bits_map_ptr += ppi->HFragPixels;
            ChLocalsPtr += ppi->HFragPixels;
            YUVDiffsPtr += ppi->HFragPixels;
            SgcPtr ++;
			FDiffPixels ++;

			// If we have a lot of changed pixels for this fragment on this row then 
			// the fragment is almost sure to be picked (e.g. through the line search) so we
			// can mark it as selected and then ignore it.
//			if ( ppi->EarlyBreakAllowed )
			{
				if (FragChangedPixels >= 7)
				{
					*DispFragPtr = BLOCK_CODED;
				}
			}
			DispFragPtr++;    
        }
    }
}
