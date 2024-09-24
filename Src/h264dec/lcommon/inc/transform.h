
/*!
 ***************************************************************************
 *
 * \file transform.h
 *
 * \brief
 *    prototypes of transform functions
 *
 * \date
 *    10 July 2007
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    Alexis Michael Tourapis
 **************************************************************************/

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "global.h"

extern void forward4x4   (int **block , int **tblock, int pos_y, int pos_x);
extern void ihadamard4x4 (int block[4][4]);
extern void ihadamard2x2 (int block[4], int tblock[4]);

#endif //_TRANSFORM_H_
