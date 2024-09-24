/******************************************************************************
  pl_defs.h
  PLUSH 3D VERSION 1.2 CONSTANTS DEFINITION HEADER
  Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#ifndef _PL_DEFS_H_
#define _PL_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* pi! */
#define PL_PI 3.14159265359

/* Utility min() and max() functions */
#define plMin(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define plMax(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))

/*
** Shade modes. Used with plMat.ShadeType
** Note that (PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE) and
** (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
#define PL_SHADE_NONE (1)
#define PL_SHADE_FLAT (2)
#define PL_SHADE_FLAT_DISTANCE (4)
#define PL_SHADE_GOURAUD (8)
#define PL_SHADE_GOURAUD_DISTANCE (16)

/*
** Light modes. Used with plLight.Type or plLightSet().
** Note that PL_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, PL_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see plLightSet() for setting it), PL_LIGHT_POINT does both.
*/
#define PL_LIGHT_NONE (0x0)
#define PL_LIGHT_VECTOR (0x1)
#define PL_LIGHT_POINT (0x2|0x4)
#define PL_LIGHT_POINT_DISTANCE (0x2)
#define PL_LIGHT_POINT_ANGLE (0x4)

/* Used internally; PL_FILL_* are stored in plMat._st. */
#define PL_FILL_SOLID (0x0)
#define PL_FILL_TEXTURE (0x1)
#define PL_FILL_ENVIRONMENT (0x2)
#define PL_FILL_TRANSPARENT (0x4)

#define PL_TEXENV_ADD (0)
#define PL_TEXENV_MUL (1)
#define PL_TEXENV_AVG (2)
#define PL_TEXENV_TEXMINUSENV (3)
#define PL_TEXENV_ENVMINUSTEX (4)
#define PL_TEXENV_MIN (5)
#define PL_TEXENV_MAX (6)

#ifdef __cplusplus
}
#endif

#endif /* !_PL_DEFS_H_ */
