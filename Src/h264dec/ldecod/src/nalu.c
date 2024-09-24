
/*!
 ************************************************************************
 * \file  nalu.c
 *
 * \brief
 *    Decoder NALU support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger   <stewe@cs.tu-berlin.de>
 ************************************************************************
 */

#include "global.h"
#include "nalu.h"
#include "memalloc.h"
#include "meminput.h"

/*!
*************************************************************************************
* \brief
*    Initialize bitstream reading structure
*
* \param
*    p_Vid: Imageparameter information
* \param
*    filemode: 
*
*************************************************************************************
*/
void OpenMemory(VideoParameters *p_Vid, const char *fn);
void CloseMemory(VideoParameters *p_Vid);
int GetMemoryNALU (VideoParameters *p_Vid, NALU_t *nalu);

void initBitsFile (VideoParameters *p_Vid)
{
		malloc_mem_input(p_Vid);
		p_Vid->nalu = AllocNALU(MAX_CODED_FRAME_SIZE);

}

/*!
 *************************************************************************************
 * \brief
 *    Converts a NALU to an RBSP
 *
 * \param
 *    nalu: nalu structure to be filled
 *
 * \return
 *    length of the RBSP in bytes
 *************************************************************************************
 */

static int NALUtoRBSP (NALU_t *nalu)
{
  assert (nalu != NULL);

  nalu->len = EBSPtoRBSP (nalu->buf, nalu->len) ;

  return nalu->len ;
}

/*!
************************************************************************
* \brief
*    Read the next NAL unit (with error handling)
************************************************************************
*/
int read_next_nalu(VideoParameters *p_Vid, NALU_t *nalu)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  int ret;

  ret = GetMemoryNALU(p_Vid, nalu);

  if (ret < 0)
  {
    error ("Error while getting the next NALU, exit\n", 601);
  }
  if (ret == 0)
  {
    return 0;
  }

  //In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
  //whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
  CheckZeroByteNonVCL(p_Vid, nalu);

  ret = NALUtoRBSP(nalu);

  if (ret < 0)
    error ("Invalid startcode emulation prevention found.", 602);


  // Got a NALU
  if (nalu->forbidden_bit)
  {
    error ("Found NALU with forbidden_bit set, bit error?", 603);
  }

  return nalu->len;
}

void CheckZeroByteNonVCL(VideoParameters *p_Vid, NALU_t *nalu)
{
  int CheckZeroByte=0;

  //This function deals only with non-VCL NAL units
  if(nalu->nal_unit_type>=1&&nalu->nal_unit_type<=5)
    return;

  //for SPS and PPS, zero_byte shall exist
  if(nalu->nal_unit_type==NALU_TYPE_SPS || nalu->nal_unit_type==NALU_TYPE_PPS)
    CheckZeroByte=1;
  //check the possibility of the current NALU to be the start of a new access unit, according to 7.4.1.2.3
  if(nalu->nal_unit_type==NALU_TYPE_AUD  || nalu->nal_unit_type==NALU_TYPE_SPS ||
    nalu->nal_unit_type==NALU_TYPE_PPS || nalu->nal_unit_type==NALU_TYPE_SEI ||
    (nalu->nal_unit_type>=13 && nalu->nal_unit_type<=18))
  {
    if(p_Vid->LastAccessUnitExists)
    {
      p_Vid->LastAccessUnitExists=0;    //deliver the last access unit to decoder
      p_Vid->NALUCount=0;
    }
  }
  p_Vid->NALUCount++;
  //for the first NAL unit in an access unit, zero_byte shall exists
  if(p_Vid->NALUCount==1)
    CheckZeroByte=1;
  if(CheckZeroByte && nalu->startcodeprefix_len==3)
  {
   // printf("Warning: zero_byte shall exist\n");
    //because it is not a very serious problem, we do not exit here
  }
}

void CheckZeroByteVCL(VideoParameters *p_Vid, NALU_t *nalu)
{
  int CheckZeroByte=0;

  //This function deals only with VCL NAL units
  if(!(nalu->nal_unit_type>=1&&nalu->nal_unit_type<=5))
    return;

  if(p_Vid->LastAccessUnitExists)
  {
    p_Vid->NALUCount=0;
  }
  p_Vid->NALUCount++;
  //the first VCL NAL unit that is the first NAL unit after last VCL NAL unit indicates
  //the start of a new access unit and hence the first NAL unit of the new access unit.           (sounds like a tongue twister :-)
  if(p_Vid->NALUCount == 1)
    CheckZeroByte = 1;
  p_Vid->LastAccessUnitExists = 1;
  if(CheckZeroByte && nalu->startcodeprefix_len==3)
  {
    //printf("warning: zero_byte shall exist\n");
    //because it is not a very serious problem, we do not exit here
  }
}
