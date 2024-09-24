/******************************************************************************
  pl_types.h
  PLUSH 3D VERSION 1.2 TYPES DEFINITION HEADER
  Copyright (c) 1996-2000 Justin Frankel
******************************************************************************/

#ifndef _PL_TYPES_H_
#define _PL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
** Texture type. Read textures with plReadPCXTex(), and assign them to
** plMat.Environment or plMat.Texture.
*/
typedef struct _pl_Texture {
  pl_uChar *Data;            /* Texture data */
  pl_uChar *PaletteData;     /* Palette data (NumColors bytes) */
  pl_uChar Width, Height;    /* Log2 of dimensions */
  pl_uInt iWidth, iHeight;   /* Integer dimensions */
  pl_Float uScale, vScale;   /* Scaling (usually 2**Width, 2**Height) */
  pl_uInt NumColors;         /* Number of colors used in texture */
} pl_Texture;

/*
** Material type. Create materials with plMatCreate().
*/
typedef struct _pl_Mat {
  pl_sInt Ambient[3];          /* RGB of surface (0-255 is a good range) */
  pl_sInt Diffuse[3];          /* RGB of diffuse (0-255 is a good range) */
  pl_sInt Specular[3];         /* RGB of "specular" highlights (0-255) */
  pl_uInt Shininess;           /* Shininess of material. 1 is dullest */
  pl_Float FadeDist;           /* For distance fading, distance at
                                  which intensity is 0 */
  pl_uChar ShadeType;          /* Shade type: PL_SHADE_* */
  pl_uChar Transparent;        /* Transparency index (0 = none), 4 = alot
                                  Note: transparencies disable textures */
  pl_uChar PerspectiveCorrect; /* Correct textures every n pixels */
  pl_Texture *Texture;         /* Texture map (see pl_Texture) above */
  pl_Texture *Environment;     /* Environment map (ditto) */
  pl_Float TexScaling;         /* Texture map scaling */
  pl_Float EnvScaling;         /* Environment map scaling */
  pl_uChar TexEnvMode;         /* TexEnv combining mode (PL_TEXENV_*) */
  pl_Bool zBufferable;         /* Can this material be zbuffered? */
  pl_uInt NumGradients;        /* Desired number of gradients to be used */
                 /* The following are used mostly internally */
  pl_uInt _ColorsUsed;         /* Number of colors actually used */
  pl_uChar _st, _ft;           /* The shadetype and filltype */
  pl_uInt _tsfact;             /* Translucent shading factor */
  pl_uInt16 *_AddTable;        /* Shading/Translucent/etc table */
  pl_uChar *_ReMapTable;       /* Table to remap colors to palette */
  pl_uChar *_RequestedColors;  /* _ColorsUsed colors, desired colors */
  void (*_PutFace)();          /* Function that renders the triangle with this
                                  material */
} pl_Mat;

/*
** Vertex, used within pl_Obj
*/
typedef struct _pl_Vertex {
  pl_Float x, y, z;              /* Vertex coordinate (objectspace) */
  pl_Float xformedx, xformedy, xformedz;
                                 /* Transformed vertex
                                    coordinate (cameraspace) */
  pl_Float nx, ny, nz;           /* Unit vertex normal (objectspace) */
  pl_Float xformednx, xformedny, xformednz;
                                 /* Transformed unit vertex normal
                                    (cameraspace) */
} pl_Vertex;

/*
** Face
*/
typedef struct _pl_Face {
  pl_Vertex *Vertices[3];      /* Vertices of triangle */
  pl_Float nx, ny, nz;         /* Normal of triangle (object space) */
  pl_Mat *Material;            /* Material of triangle */
  pl_sInt32 Scrx[3], Scry[3];  /* Projected screen coordinates
                                  (12.20 fixed point) */
  pl_Float Scrz[3];            /* Projected 1/Z coordinates */
  pl_sInt32 MappingU[3], MappingV[3];
                               /* 16.16 Texture mapping coordinates */
  pl_sInt32 eMappingU[3], eMappingV[3];
                               /* 16.16 Environment map coordinates */
  pl_Float fShade;             /* Flat intensity */
  pl_Float sLighting;          /* Face static lighting. Should usually be 0.0 */
  pl_Float Shades[3];          /* Vertex intensity */
  pl_Float vsLighting[3];      /* Vertex static lighting. Should be 0.0 */
} pl_Face;

/*
** Object
*/
typedef struct _pl_Obj {
  pl_uInt32 NumVertices;              /* Number of vertices */
  pl_uInt32 NumFaces;                 /* Number of faces */
  pl_Vertex *Vertices;                /* Array of vertices */
  pl_Face *Faces;                     /* Array of faces */
  struct _pl_Obj *Children[PL_MAX_CHILDREN];
                                      /* Children */
  pl_Bool BackfaceCull;               /* Are backfacing polys drawn? */
  pl_Bool BackfaceIllumination;       /* Illuminated by lights behind them? */
  pl_Bool GenMatrix;                  /* Generate Matrix from the following
                                         if set */
  pl_Float Xp, Yp, Zp, Xa, Ya, Za;    /* Position and rotation of object:
                                         Note: rotations are around
                                         X then Y then Z. Measured in degrees */
  pl_Float Matrix[16];                /* Transformation matrix */
  pl_Float RotMatrix[16];             /* Rotation only matrix (for normals) */
} pl_Obj;

/*
** Spline type. See plSpline*().
*/
typedef struct _pl_Spline {
  pl_Float *keys;              /* Key data, keyWidth*numKeys */
  pl_sInt keyWidth;            /* Number of floats per key */
  pl_sInt numKeys;             /* Number of keys */
  pl_Float cont;               /* Continuity. Should be -1.0 -> 1.0 */
  pl_Float bias;               /* Bias. -1.0 -> 1.0 */
  pl_Float tens;               /* Tension. -1.0 -> 1.0 */
} pl_Spline;

/*
** Light type. See plLight*().
*/
typedef struct _pl_Light {
  pl_uChar Type;               /* Type of light: PL_LIGHT_* */
  pl_Float Xp, Yp, Zp;         /* If Type=PL_LIGHT_POINT*,
                                  this is Position (PL_LIGHT_POINT_*),
                                  otherwise if PL_LIGHT_VECTOR,
                                  Unit vector */
  pl_Float Intensity;           /* Intensity. 0.0 is off, 1.0 is full */
  pl_Float HalfDistSquared;     /* Distance squared at which
                                   PL_LIGHT_POINT_DISTANCE is 50% */
} pl_Light;

/*
** Camera Type.
*/
typedef struct _pl_Cam {
  pl_Float Fov;                  /* FOV in degrees valid range is 1-179 */
  pl_Float AspectRatio;          /* Aspect ratio (usually 1.0) */
  pl_sChar Sort;                 /* Sort polygons, -1 f-t-b, 1 b-t-f, 0 no */
  pl_Float ClipBack;             /* Far clipping ( < 0.0 is none) */
  pl_sInt ClipTop, ClipLeft;     /* Screen Clipping */
  pl_sInt ClipBottom, ClipRight;
  pl_uInt ScreenWidth, ScreenHeight; /* Screen dimensions */
  pl_sInt CenterX, CenterY;      /* Center of screen */
  pl_Float X, Y, Z;              /* Camera position in worldspace */
  pl_Float Pitch, Pan, Roll;     /* Camera angle in degrees in worldspace */
  pl_uChar *frameBuffer;         /* Framebuffer (ScreenWidth*ScreenHeight) */
  pl_ZBuffer *zBuffer;           /* Z Buffer (NULL if none) */
} pl_Cam;

#ifdef __cplusplus
}
#endif

#endif /* !_PL_TYPES_H_ */
