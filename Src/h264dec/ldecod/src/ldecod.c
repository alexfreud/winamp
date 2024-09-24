
/*!
 ***********************************************************************
 *  \mainpage
 *     This is the H.264/AVC decoder reference software. For detailed documentation
 *     see the comments in each file.
 *
 *     The JM software web site is located at:
 *     http://iphome.hhi.de/suehring/tml
 *
 *     For bug reporting and known issues see:
 *     https://ipbt.hhi.de
 *
 *  \author
 *     The main contributors are listed in contributors.h
 *
 *  \version
 *     JM 16.1 (FRExt)
 *
 *  \note
 *     tags are used for document system "doxygen"
 *     available at http://www.doxygen.org
 */
/*!
 *  \file
 *     ldecod.c
 *  \brief
 *     H.264/AVC reference decoder project main()
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Inge Lille-Langøy       <inge.lille-langoy@telenor.com>
 *     - Rickard Sjoberg         <rickard.sjoberg@era.ericsson.se>
 *     - Stephan Wenger          <stewe@cs.tu-berlin.de>
 *     - Jani Lainema            <jani.lainema@nokia.com>
 *     - Sebastian Purreiter     <sebastian.purreiter@mch.siemens.de>
 *     - Byeong-Moon Jeon        <jeonbm@lge.com>
 *     - Gabi Blaettermann
 *     - Ye-Kui Wang             <wyk@ieee.org>
 *     - Valeri George           <george@hhi.de>
 *     - Karsten Suehring        <suehring@hhi.de>
 *
 ***********************************************************************
 */

#include "contributors.h"

#include <sys/stat.h>

#include "global.h"
#include "image.h"
#include "memalloc.h"
#include "mc_prediction.h"
#include "mbuffer.h"
#include "leaky_bucket.h"
#include "fmo.h"
#include "output.h"
#include "cabac.h"
#include "parset.h"
#include "sei.h"
#include "erc_api.h"
#include "quant.h"
#include "block.h"
#include "nalu.h"
#include "meminput.h"
#define LOGFILE     "log.dec"
#define DATADECFILE "dataDec.txt"
#define TRACEFILE   "trace_dec.txt"

// Decoder definition. This should be the only global variable in the entire
// software. Global variables should be avoided.
char errortext[ET_SIZE]; //!< buffer for error message for exit with error()

#ifdef TRACE
FILE *p_trace=0;
int bitcounter=0;
#endif

// Prototypes of static functions
void init        (VideoParameters *p_Vid);
void malloc_slice(InputParameters *p_Inp, VideoParameters *p_Vid);
void free_slice  (Slice *currSlice);

void init_frext(VideoParameters *p_Vid);

/*!
 ************************************************************************
 * \brief
 *    Error handling procedure. Print error message to stderr and exit
 *    with supplied code.
 * \param text
 *    Error message
 * \param code
 *    Exit code
 ************************************************************************
 */
void error(char *text, int code)
{
	RaiseException(code, 0, 1, (ULONG_PTR *)text);
  //fprintf(stderr, "%s\n", text); 
	//flush_dpb(p_Dec->p_Vid);
  //exit(code);
}

/*static */void Configure(VideoParameters *p_Vid, InputParameters *p_Inp)
{
  p_Vid->p_Inp = p_Inp;

  p_Inp->intra_profile_deblocking = 0;

#ifdef _LEAKYBUCKET_
  p_Inp->R_decoder=500000;          //! Decoder rate
  p_Inp->B_decoder=104000;          //! Decoder buffer size
  p_Inp->F_decoder=73000;           //! Decoder initial delay
  strcpy(p_Inp->LeakyBucketParamFile,"leakybucketparam.cfg");    // file where Leaky Bucket parameters (computed by encoder) are stored
#endif

}

/*!
 ***********************************************************************
 * \brief
 *    Allocate the Image structure
 * \par  Output:
 *    Image Parameters VideoParameters *p_Vid
 ***********************************************************************
 */
static void alloc_img( VideoParameters **p_Vid)
{
  if ((*p_Vid   =  (VideoParameters *) calloc(1, sizeof(VideoParameters)))==NULL) 
    no_mem_exit("alloc_img: p_Vid");

  if (((*p_Vid)->old_slice = (OldSliceParams *) calloc(1, sizeof(OldSliceParams)))==NULL) 
    no_mem_exit("alloc_img: p_Vid->old_slice");

  if (((*p_Vid)->p_Dpb =  (DecodedPictureBuffer*)calloc(1, sizeof(DecodedPictureBuffer)))==NULL) 
    no_mem_exit("alloc_img: p_Vid->p_Dpb");  

  (*p_Vid)->p_Dpb->init_done = 0;
  
  (*p_Vid)->global_init_done = 0;

#if (ENABLE_OUTPUT_TONEMAPPING)  
  if (((*p_Vid)->seiToneMapping =  (ToneMappingSEI*)calloc(1, sizeof(ToneMappingSEI)))==NULL) 
    no_mem_exit("alloc_img: (*p_Vid)->seiToneMapping");  
#endif

}


/*!
 ***********************************************************************
 * \brief
 *    Allocate the Input structure
 * \par  Output:
 *    Input Parameters InputParameters *p_Vid
 ***********************************************************************
 */
static void alloc_params( InputParameters **p_Inp )
{
  if ((*p_Inp = (InputParameters *) calloc(1, sizeof(InputParameters)))==NULL) 
    no_mem_exit("alloc_params: p_Inp");
}

  /*!
 ***********************************************************************
 * \brief
 *    Allocate the Decoder Structure
 * \par  Output:
 *    Decoder Parameters
 ***********************************************************************
 */
DecoderParams *alloc_decoder()
{
	DecoderParams *decoder = (DecoderParams *) calloc(1, sizeof(DecoderParams));
	if (decoder)

	{
		alloc_img(&(decoder->p_Vid));
		alloc_params(&(decoder->p_Inp));
#ifdef TRACE
		p_trace = 0;
		bitcounter = 0;
#endif
	}
	return decoder;
}

/*!
 ***********************************************************************
 * \brief
 *    Free the Image structure
 * \par  Input:
 *    Image Parameters VideoParameters *p_Vid
 ***********************************************************************
 */
void free_img( VideoParameters *p_Vid)
{
  if (p_Vid != NULL)
  {
		free_mem_input(p_Vid);
#if (ENABLE_OUTPUT_TONEMAPPING)  
    if (p_Vid->seiToneMapping != NULL)
    {
      free (p_Vid->seiToneMapping);
      p_Vid->seiToneMapping = NULL;
    }
#endif

    if (p_Vid->p_Dpb != NULL)
    {
      free (p_Vid->p_Dpb);
      p_Vid->p_Dpb = NULL;
    }
    if (p_Vid->old_slice != NULL)
    {
      free (p_Vid->old_slice);
      p_Vid->old_slice = NULL;
    }

    free (p_Vid);
    p_Vid = NULL;
  }
}
/*!
 ***********************************************************************
 * \brief
 *    main function for TML decoder
 ***********************************************************************
 */
#if 0
int main(int argc, char **argv)
{  
  DecoderParams  *p_Dec = alloc_decoder();
  if (!p_Dec)
	  return 1;

  Configure(p_Dec->p_Vid, p_Dec->p_Inp, argc, argv);

  initBitsFile(p_Dec->p_Vid, p_Dec->p_Inp->FileFormat);

  p_Dec->p_Vid->bitsfile->OpenBitsFile(p_Dec->p_Vid, p_Dec->p_Inp->infile);
  
  // Allocate Slice data struct
  malloc_slice(p_Dec->p_Inp, p_Dec->p_Vid);
  init_old_slice(p_Dec->p_Vid->old_slice);

  init(p_Dec->p_Vid);
 
  init_out_buffer(p_Dec->p_Vid);  

  while (decode_one_frame(p_Dec->p_Vid) != EOS)
    ;

  free_slice(p_Dec->p_Vid->currentSlice);
  FmoFinit(p_Dec->p_Vid);

  free_global_buffers(p_Dec->p_Vid);
  flush_dpb(p_Dec->p_Vid);

#if (PAIR_FIELDS_IN_OUTPUT)
  flush_pending_output(p_Dec->p_Vid, p_Dec->p_Vid->p_out);
#endif

  p_Dec->p_Vid->bitsfile->CloseBitsFile(p_Dec->p_Vid);

  close(p_Dec->p_Vid->p_out);

  if (p_Dec->p_Vid->p_ref != -1)
    close(p_Dec->p_Vid->p_ref);

#if TRACE
  fclose(p_trace);
#endif

  ercClose(p_Dec->p_Vid, p_Dec->p_Vid->erc_errorVar);

  CleanUpPPS(p_Dec->p_Vid);
  free_dpb(p_Dec->p_Vid);
  uninit_out_buffer(p_Dec->p_Vid);

  free (p_Dec->p_Inp);
  free_img (p_Dec->p_Vid);
  free(p_Dec);

  return 0;
}
#endif

/*!
 ***********************************************************************
 * \brief
 *    Initilize some arrays
 ***********************************************************************
 */
void init(VideoParameters *p_Vid)  //!< image parameters
{
  int i;
  InputParameters *p_Inp = p_Vid->p_Inp;
  p_Vid->oldFrameSizeInMbs = -1;

  p_Vid->recovery_point = 0;
  p_Vid->recovery_point_found = 0;
  p_Vid->recovery_poc = 0x7fffffff; /* set to a max value */

  p_Vid->number = 0;
  p_Vid->type = I_SLICE;

  p_Vid->dec_ref_pic_marking_buffer = NULL;

  p_Vid->dec_picture = NULL;
  // reference flag initialization
  for(i=0;i<17;++i)
  {
    p_Vid->ref_flag[i] = 1;
  }

  p_Vid->MbToSliceGroupMap = NULL;
  p_Vid->MapUnitToSliceGroupMap = NULL;

  p_Vid->LastAccessUnitExists  = 0;
  p_Vid->NALUCount = 0;


  p_Vid->out_buffer = NULL;
  p_Vid->pending_output = NULL;
  p_Vid->pending_output_state = FRAME;
  p_Vid->recovery_flag = 0;


#if (ENABLE_OUTPUT_TONEMAPPING)
  init_tone_mapping_sei(p_Vid->seiToneMapping);
#endif

}

/*!
 ***********************************************************************
 * \brief
 *    Initialize FREXT variables
 ***********************************************************************
 */
void init_frext(VideoParameters *p_Vid)  //!< image parameters
{
  //pel bitdepth init
  p_Vid->bitdepth_luma_qp_scale   = 6 * (p_Vid->bitdepth_luma - 8);

  p_Vid->dc_pred_value_comp[0]    = 1<<(p_Vid->bitdepth_luma - 1);
  p_Vid->max_pel_value_comp[0] = (1<<p_Vid->bitdepth_luma) - 1;
  p_Vid->mb_size[IS_LUMA][0] = p_Vid->mb_size[IS_LUMA][1] = MB_BLOCK_SIZE;

  if (p_Vid->active_sps->chroma_format_idc != YUV400)
  {
    //for chrominance part
    p_Vid->bitdepth_chroma_qp_scale = 6 * (p_Vid->bitdepth_chroma - 8);
    p_Vid->dc_pred_value_comp[1]    = (1 << (p_Vid->bitdepth_chroma - 1));
    p_Vid->dc_pred_value_comp[2]    = p_Vid->dc_pred_value_comp[1];
    p_Vid->max_pel_value_comp[1] = (1 << p_Vid->bitdepth_chroma) - 1;
    p_Vid->max_pel_value_comp[2] = (1 << p_Vid->bitdepth_chroma) - 1;
    p_Vid->num_blk8x8_uv = (1 << p_Vid->active_sps->chroma_format_idc) & (~(0x1));
    p_Vid->num_uv_blocks = (p_Vid->num_blk8x8_uv >> 1);
    p_Vid->num_cdc_coeff = (p_Vid->num_blk8x8_uv << 1);
    p_Vid->mb_size[IS_CHROMA][0] = p_Vid->mb_size[2][0] = p_Vid->mb_cr_size_x  = (p_Vid->active_sps->chroma_format_idc==YUV420 || p_Vid->active_sps->chroma_format_idc==YUV422)?  8 : 16;
    p_Vid->mb_size[IS_CHROMA][1] = p_Vid->mb_size[2][1] = p_Vid->mb_cr_size_y  = (p_Vid->active_sps->chroma_format_idc==YUV444 || p_Vid->active_sps->chroma_format_idc==YUV422)? 16 :  8;

		p_Vid->subpel_x    = p_Vid->mb_cr_size_x == 8 ? 7 : 3;
		p_Vid->subpel_y    = p_Vid->mb_cr_size_y == 8 ? 7 : 3;
		p_Vid->shiftpel_x  = p_Vid->mb_cr_size_x == 8 ? 3 : 2;
		p_Vid->shiftpel_y  = p_Vid->mb_cr_size_y == 8 ? 3 : 2;
  }
  else
  {
    p_Vid->bitdepth_chroma_qp_scale = 0;
    p_Vid->max_pel_value_comp[1] = 0;
    p_Vid->max_pel_value_comp[2] = 0;
    p_Vid->num_blk8x8_uv = 0;
    p_Vid->num_uv_blocks = 0;
    p_Vid->num_cdc_coeff = 0;
    p_Vid->mb_size[IS_CHROMA][0] = p_Vid->mb_size[2][0] = p_Vid->mb_cr_size_x  = 0;
    p_Vid->mb_size[IS_CHROMA][1] = p_Vid->mb_size[2][1] = p_Vid->mb_cr_size_y  = 0;

		p_Vid->subpel_x      = 0;
		p_Vid->subpel_y      = 0;
		p_Vid->shiftpel_x    = 0;
		p_Vid->shiftpel_y    = 0;
  }
  p_Vid->mb_size_blk[0][0] = p_Vid->mb_size_blk[0][1] = p_Vid->mb_size[0][0] >> 2;
  p_Vid->mb_size_blk[1][0] = p_Vid->mb_size_blk[2][0] = p_Vid->mb_size[1][0] >> 2;
  p_Vid->mb_size_blk[1][1] = p_Vid->mb_size_blk[2][1] = p_Vid->mb_size[1][1] >> 2;

  p_Vid->mb_size_shift[0][0] = p_Vid->mb_size_shift[0][1] = CeilLog2_sf (p_Vid->mb_size[0][0]);
  p_Vid->mb_size_shift[1][0] = p_Vid->mb_size_shift[2][0] = CeilLog2_sf (p_Vid->mb_size[1][0]);
  p_Vid->mb_size_shift[1][1] = p_Vid->mb_size_shift[2][1] = CeilLog2_sf (p_Vid->mb_size[1][1]);
}

/*!
 ************************************************************************
 * \brief
 *    Allocates a stand-alone partition structure.  Structure should
 *    be freed by FreePartition();
 *    data structures
 *
 * \par Input:
 *    n: number of partitions in the array
 * \par return
 *    pointer to DataPartition Structure, zero-initialized
 ************************************************************************
 */

DataPartition *AllocPartition(int n)
{
  DataPartition *partArr, *dataPart;
  int i;

  partArr = (DataPartition *) calloc(n, sizeof(DataPartition));
  if (partArr == NULL)
  {
    snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Data Partition failed");
    error(errortext, 100);
  }

  for (i=0; i<n; ++i) // loop over all data partitions
  {
    dataPart = &(partArr[i]);
    dataPart->bitstream = (Bitstream *) calloc(1, sizeof(Bitstream));
    if (dataPart->bitstream == NULL)
    {
      snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Bitstream failed");
      error(errortext, 100);
    }
		dataPart->bitstream->streamBuffer = 0;
  }
  return partArr;
}




/*!
 ************************************************************************
 * \brief
 *    Frees a partition structure (array).
 *
 * \par Input:
 *    Partition to be freed, size of partition Array (Number of Partitions)
 *
 * \par return
 *    None
 *
 * \note
 *    n must be the same as for the corresponding call of AllocPartition
 ************************************************************************
 */


void FreePartition (DataPartition *dp, int n)
{
  int i;

  assert (dp != NULL);
  assert (dp->bitstream != NULL);
  //assert (dp->bitstream->streamBuffer != NULL);
  for (i=0; i<n; ++i)
  {
    //free (dp[i].bitstream->streamBuffer);
    free (dp[i].bitstream);
  }
  free (dp);
}


/*!
 ************************************************************************
 * \brief
 *    Allocates the slice structure along with its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters InputParameters *p_Inp,  VideoParameters *p_Vid
 ************************************************************************
 */
void malloc_slice(InputParameters *p_Inp, VideoParameters *p_Vid)
{
  int memory_size = 0;
  Slice *currSlice;

  p_Vid->currentSlice = (Slice *) _aligned_malloc(sizeof(Slice), 32);
  if ( (currSlice = p_Vid->currentSlice) == NULL)
  {
    error("Memory allocation for Slice datastruct failed",100);
  }
	memset(p_Vid->currentSlice, 0, sizeof(Slice));
  //  p_Vid->currentSlice->rmpni_buffer=NULL;
  //! you don't know whether we do CABAC here, hence initialize CABAC anyway
  // if (p_Inp->symbol_mode == CABAC)

  // create all context models
  currSlice->mot_ctx = create_contexts_MotionInfo();
  currSlice->tex_ctx = create_contexts_TextureInfo();


  currSlice->max_part_nr = 3;  //! assume data partitioning (worst case) for the following mallocs()
  currSlice->partArr = AllocPartition(currSlice->max_part_nr);
  currSlice->p_colocated = NULL;

	currSlice->coeff_ctr = -1;
  currSlice->pos       =  0;
}


/*!
 ************************************************************************
 * \brief
 *    Memory frees of the Slice structure and of its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters InputParameters *p_Inp,  VideoParameters *p_Vid
 ************************************************************************
 */
void free_slice(Slice *currSlice)
{
  FreePartition (currSlice->partArr, 3);
  
  if (1)
  {
    // delete all context models
    delete_contexts_MotionInfo(currSlice->mot_ctx);
    delete_contexts_TextureInfo(currSlice->tex_ctx);
  }
  _aligned_free(currSlice);

  currSlice = NULL;
}

/*!
 ************************************************************************
 * \brief
 *    Dynamic memory allocation of frame size related global buffers
 *    buffers are defined in global.h, allocated memory must be freed in
 *    void free_global_buffers()
 *
 *  \par Input:
 *    Input Parameters InputParameters *p_Inp, Image Parameters VideoParameters *p_Vid
 *
 *  \par Output:
 *     Number of allocated bytes
 ***********************************************************************
 */
int init_global_buffers(VideoParameters *p_Vid)
{
  int memory_size=0;
  int i;

  if (p_Vid->global_init_done)
  {
    free_global_buffers(p_Vid);
  }

  // allocate memory in structure p_Vid
  if( IS_INDEPENDENT(p_Vid) )
  {
    for( i=0; i<MAX_PLANE; ++i )
    {
      if(((p_Vid->mb_data_JV[i]) = (Macroblock *) calloc(p_Vid->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
        no_mem_exit("init_global_buffers: p_Vid->mb_data");
    }
    p_Vid->mb_data = NULL;
  }
  else
  {
    if(((p_Vid->mb_data) = (Macroblock *) calloc(p_Vid->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
      no_mem_exit("init_global_buffers: p_Vid->mb_data");
  }

  if(((p_Vid->intra_block) = (int*)calloc(p_Vid->FrameSizeInMbs, sizeof(int))) == NULL)
    no_mem_exit("init_global_buffers: p_Vid->intra_block");

	p_Vid->PicPos = (h264_pic_position *)calloc(p_Vid->FrameSizeInMbs + 1, sizeof(h264_pic_position)); //! Helper array to access macroblock positions. We add 1 to also consider last MB.

  for (i = 0; i < (int) p_Vid->FrameSizeInMbs + 1;++i)
  {
    p_Vid->PicPos[i][0] = (i % p_Vid->PicWidthInMbs);
    p_Vid->PicPos[i][1] = (i / p_Vid->PicWidthInMbs);
  }

  memory_size += get_mem2D(&(p_Vid->ipredmode), 4*p_Vid->FrameHeightInMbs, 4*p_Vid->PicWidthInMbs);

  // CAVLC mem
	p_Vid->nz_coeff = (h264_nz_coefficient *)_aligned_malloc(p_Vid->FrameSizeInMbs*sizeof(h264_nz_coefficient), 32);
	memset(p_Vid->nz_coeff, 0, p_Vid->FrameSizeInMbs*sizeof(h264_nz_coefficient));
  //memory_size += get_mem4D(&(p_Vid->nz_coeff), p_Vid->FrameSizeInMbs, 3, BLOCK_SIZE, BLOCK_SIZE);

  memory_size += get_mem2Dint(&(p_Vid->siblock), p_Vid->FrameHeightInMbs, p_Vid->PicWidthInMbs);

  init_qp_process(p_Vid);

  p_Vid->global_init_done = 1;

  p_Vid->oldFrameSizeInMbs = p_Vid->FrameSizeInMbs;

  return (memory_size);
}

/*!
 ************************************************************************
 * \brief
 *    Free allocated memory of frame size related global buffers
 *    buffers are defined in global.h, allocated memory is allocated in
 *    int init_global_buffers()
 *
 * \par Input:
 *    Input Parameters InputParameters *p_Inp, Image Parameters VideoParameters *p_Vid
 *
 * \par Output:
 *    none
 *
 ************************************************************************
 */
void free_global_buffers(VideoParameters *p_Vid)
{  
  // CAVLC free mem
  _aligned_free(p_Vid->nz_coeff);

  free_mem2Dint(p_Vid->siblock);

  // free mem, allocated for structure p_Vid
  if (p_Vid->mb_data != NULL)
    free(p_Vid->mb_data);

  free(p_Vid->PicPos);

  free (p_Vid->intra_block);
  free_mem2D(p_Vid->ipredmode);

  free_qp_matrices(p_Vid);

  p_Vid->global_init_done = 0;

}
