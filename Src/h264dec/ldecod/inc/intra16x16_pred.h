/*!
 *************************************************************************************
 * \file intra16x16_pred.h
 *
 * \brief
 *    definitions for intra 16x16 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#ifndef _INTRA16x16_PRED_H_
#define _INTRA16x16_PRED_H_

#include "global.h"
#include "mbuffer.h"

extern int intrapred16x16(Macroblock *currMB, ColorPlane pl, int b8);

#endif

