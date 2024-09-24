
/*!
 ***********************************************************************
 * \file errorconcealment.c
 *
 * \brief
 *    Implements error concealment scheme for H.264 decoder
 *
 * \date
 *    6.10.2000
 *
 * \version
 *    1.0
 *
 * \note
 *    This simple error concealment implemented in this decoder uses
 *    the existing dependencies of syntax elements.
 *    In case that an element is detected as false this elements and all
 *    dependend elements are marked as elements to conceal in the p_Vid->ec_flag[]
 *    array. If the decoder requests a new element by the function
 *    readSyntaxElement_xxxx() this array is checked first if an error concealment has
 *    to be applied on this element.
 *    In case that an error occured a concealed element is given to the
 *    decoding function in macroblock().
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Sebastian Purreiter   <sebastian.purreiter@mch.siemens.de>
 ***********************************************************************
 */

#include "contributors.h"
#include "global.h"
#include "elements.h"



/*!
 ***********************************************************************
 * \brief
 *    set concealment for all elements in same partition
 *    and dependend syntax elements
 * \param p_Vid
 *      image encoding parameters for current picture
 * \param se
 *      type of syntax element to conceal
 * \return
 *    EC_REQ, elements of same type or depending type need error concealment. \n
 *    EX_SYNC   sync on next header
 ***********************************************************************
 */
int set_ec_flag(VideoParameters *p_Vid, int se)
{

  /*
  if (p_Vid->ec_flag[se] == NO_EC)
    printf("Error concealment on element %s\n",SEtypes[se]);
  */
  switch (se)
  {
  case SE_HEADER :
    p_Vid->ec_flag[SE_HEADER] = EC_REQ;
  case SE_PTYPE :
    p_Vid->ec_flag[SE_PTYPE] = EC_REQ;
  case SE_MBTYPE :
    p_Vid->ec_flag[SE_MBTYPE] = EC_REQ;

  case SE_REFFRAME :
    p_Vid->ec_flag[SE_REFFRAME] = EC_REQ;
    p_Vid->ec_flag[SE_MVD] = EC_REQ; // set all motion vectors to zero length
    se = SE_CBP_INTER;      // conceal also Inter texture elements
    break;

  case SE_INTRAPREDMODE :
    p_Vid->ec_flag[SE_INTRAPREDMODE] = EC_REQ;
    se = SE_CBP_INTRA;      // conceal also Intra texture elements
    break;
  case SE_MVD :
    p_Vid->ec_flag[SE_MVD] = EC_REQ;
    se = SE_CBP_INTER;      // conceal also Inter texture elements
    break;

  default:
    break;
  }

  switch (se)
  {
  case SE_CBP_INTRA :
    p_Vid->ec_flag[SE_CBP_INTRA] = EC_REQ;
  case SE_LUM_DC_INTRA :
    p_Vid->ec_flag[SE_LUM_DC_INTRA] = EC_REQ;
  case SE_CHR_DC_INTRA :
    p_Vid->ec_flag[SE_CHR_DC_INTRA] = EC_REQ;
  case SE_LUM_AC_INTRA :
    p_Vid->ec_flag[SE_LUM_AC_INTRA] = EC_REQ;
  case SE_CHR_AC_INTRA :
    p_Vid->ec_flag[SE_CHR_AC_INTRA] = EC_REQ;
    break;

  case SE_CBP_INTER :
    p_Vid->ec_flag[SE_CBP_INTER] = EC_REQ;
  case SE_LUM_DC_INTER :
    p_Vid->ec_flag[SE_LUM_DC_INTER] = EC_REQ;
  case SE_CHR_DC_INTER :
    p_Vid->ec_flag[SE_CHR_DC_INTER] = EC_REQ;
  case SE_LUM_AC_INTER :
    p_Vid->ec_flag[SE_LUM_AC_INTER] = EC_REQ;
  case SE_CHR_AC_INTER :
    p_Vid->ec_flag[SE_CHR_AC_INTER] = EC_REQ;
    break;
  case SE_DELTA_QUANT_INTER :
    p_Vid->ec_flag[SE_DELTA_QUANT_INTER] = EC_REQ;
    break;
  case SE_DELTA_QUANT_INTRA :
    p_Vid->ec_flag[SE_DELTA_QUANT_INTRA] = EC_REQ;
    break;
  default:
    break;

  }
  return EC_REQ;
}

/*!
 ***********************************************************************
 * \brief
 *    resets EC_Flags called at the start of each slice
 *
 ***********************************************************************
 */
void reset_ec_flags(VideoParameters *p_Vid)
{
  int i;
  for (i=0; i<SE_MAX_ELEMENTS; i++)
    p_Vid->ec_flag[i] = NO_EC;
}

