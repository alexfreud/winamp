
/*!
 ************************************************************************
 * \file  nalucommon.c
 *
 * \brief
 *    Common NALU support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger   <stewe@cs.tu-berlin.de>
 ************************************************************************
 */

#include "global.h"
#include "nalu.h"
#include "memalloc.h"
#include <bfc/platform/types.h>

/*!
 *************************************************************************************
 * \brief
 *    Allocates memory for a NALU
 *
 * \param buffersize
 *     size of NALU buffer
 *
 * \return
 *    pointer to a NALU
 *************************************************************************************
 */
NALU_t *AllocNALU(int buffersize)
{
  NALU_t *n;

  if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    return 0;

  n->max_size=buffersize;

  if ((n->buf = (uint8_t *)_aligned_malloc(buffersize, 32)) == NULL)
  {
    free (n);
    return 0;
  }
	memset(n->buf, 0, buffersize);

  return n;
}


/*!
 *************************************************************************************
 * \brief
 *    Frees a NALU
 *
 * \param n
 *    NALU to be freed
 *
 *************************************************************************************
 */
void FreeNALU(NALU_t *n)
{
  if (n != NULL)
  {
    if (n->buf != NULL)
    {
      _aligned_free(n->buf);
      n->buf=NULL;
    }
    free (n);
  }
}
