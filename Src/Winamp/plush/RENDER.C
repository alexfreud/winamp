/******************************************************************************
Plush Version 1.2
render.c
Rendering code: this includes transformation, lighting, etc
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

typedef struct {
  pl_Float zd;
  pl_Face *face;
} _faceInfo;

typedef struct {
  pl_Light *light;
  pl_Float l[3];
} _lightInfo;

#define MACRO_plMatrixApply(m,x,y,z,outx,outy,outz) \
      ( outx ) = ( x )*( m )[0] + ( y )*( m )[1] + ( z )*( m )[2] + ( m )[3];\
      ( outy ) = ( x )*( m )[4] + ( y )*( m )[5] + ( z )*( m )[6] + ( m )[7];\
      ( outz ) = ( x )*( m )[8] + ( y )*( m )[9] + ( z )*( m )[10] + ( m )[11]

#define MACRO_plDotProduct(x1,y1,z1,x2,y2,z2) \
      ((( x1 )*( x2 ))+(( y1 )*( y2 ))+(( z1 )*( z2 )))

#define MACRO_plNormalizeVector(x,y,z) { \
  register double length; \
  length = ( x )*( x )+( y )*( y )+( z )*( z ); \
  if (length > 0.0000000001) { \
    pl_Float l = (pl_Float) sqrt(length); \
    ( x ) /= l; \
    ( y ) /= l; \
    ( z ) /= l; \
  } \
}

pl_uInt32 plRender_TriStats[4];

static pl_uInt32 _numfaces;
static _faceInfo _faces[PL_MAX_TRIANGLES];

static pl_Float _cMatrix[16];
static pl_uInt32 _numlights;
static _lightInfo _lights[PL_MAX_LIGHTS];
static pl_Cam *_cam;
static void _RenderObj(pl_Obj *, pl_Float *, pl_Float *);
static void _sift_down(int L, int U, int dir);
static void _hsort(_faceInfo *base, int nel, int dir);

void plRenderBegin(pl_Cam *Camera) {
  pl_Float tempMatrix[16];
  memset(plRender_TriStats,0,sizeof(plRender_TriStats));
  _cam = Camera;
  _numlights = 0;
  _numfaces = 0;
  plMatrixRotate(_cMatrix,2,-Camera->Pan);
  plMatrixRotate(tempMatrix,1,-Camera->Pitch);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plMatrixRotate(tempMatrix,3,-Camera->Roll);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plClipSetFrustum(_cam);
}

void plRenderLight(pl_Light *light) {
  pl_Float *pl, xp, yp, zp;
  if (light->Type == PL_LIGHT_NONE || _numlights >= PL_MAX_LIGHTS) return;
  pl = _lights[_numlights].l;
  if (light->Type == PL_LIGHT_VECTOR) {
    xp = light->Xp;
    yp = light->Yp;
    zp = light->Zp;
    MACRO_plMatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  } else if (light->Type & PL_LIGHT_POINT) {
    xp = light->Xp-_cam->X;
    yp = light->Yp-_cam->Y;
    zp = light->Zp-_cam->Z;
    MACRO_plMatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  }
  _lights[_numlights++].light = light;
}

static void _RenderObj(pl_Obj *obj, pl_Float *bmatrix, pl_Float *bnmatrix) {
  pl_uInt32 i, x, facepos;
  pl_Float nx = 0.0, ny = 0.0, nz = 0.0;
  double tmp, tmp2;
  pl_Float oMatrix[16], nMatrix[16], tempMatrix[16];

  pl_Vertex *vertex;
  pl_Face *face;
  pl_Light *light;

  if (obj->GenMatrix) {
    plMatrixRotate(nMatrix,1,obj->Xa);
    plMatrixRotate(tempMatrix,2,obj->Ya);
    plMatrixMultiply(nMatrix,tempMatrix);
    plMatrixRotate(tempMatrix,3,obj->Za);
    plMatrixMultiply(nMatrix,tempMatrix);
    memcpy(oMatrix,nMatrix,sizeof(pl_Float)*16);
  } else memcpy(nMatrix,obj->RotMatrix,sizeof(pl_Float)*16);

  if (bnmatrix) plMatrixMultiply(nMatrix,bnmatrix);

  if (obj->GenMatrix) {
    plMatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
    plMatrixMultiply(oMatrix,tempMatrix);
  } else memcpy(oMatrix,obj->Matrix,sizeof(pl_Float)*16);
  if (bmatrix) plMatrixMultiply(oMatrix,bmatrix);

  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (obj->Children[i]) _RenderObj(obj->Children[i],oMatrix,nMatrix);
  if (!obj->NumFaces || !obj->NumVertices) return;

  plMatrixTranslate(tempMatrix, -_cam->X, -_cam->Y, -_cam->Z);
  plMatrixMultiply(oMatrix,tempMatrix);
  plMatrixMultiply(oMatrix,_cMatrix);
  plMatrixMultiply(nMatrix,_cMatrix);

  x = obj->NumVertices;
  vertex = obj->Vertices;

  do {
    MACRO_plMatrixApply(oMatrix,vertex->x,vertex->y,vertex->z,
                  vertex->xformedx, vertex->xformedy, vertex->xformedz);
    MACRO_plMatrixApply(nMatrix,vertex->nx,vertex->ny,vertex->nz,
                  vertex->xformednx,vertex->xformedny,vertex->xformednz);
    vertex++;
  } while (--x);

  face = obj->Faces;
  facepos = _numfaces;

  if (_numfaces + obj->NumFaces >= PL_MAX_TRIANGLES) // exceeded maximum face coutn
  {
    return;
  }

  plRender_TriStats[0] += obj->NumFaces;
  _numfaces += obj->NumFaces;
  x = obj->NumFaces;

  do {
    if (obj->BackfaceCull || face->Material->_st & PL_SHADE_FLAT)
    {
      MACRO_plMatrixApply(nMatrix,face->nx,face->ny,face->nz,nx,ny,nz);
    }
    if (!obj->BackfaceCull || (MACRO_plDotProduct(nx,ny,nz,
        face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,
        face->Vertices[0]->xformedz) < 0.0000001)) {
      if (plClipNeeded(face)) {
        if (face->Material->_st & (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE)) {
          tmp = face->sLighting;
          if (face->Material->_st & PL_SHADE_FLAT) {
            for (i = 0; i < _numlights; i ++) {
              tmp2 = 0.0;
              light = _lights[i].light;
              if (light->Type & PL_LIGHT_POINT_ANGLE) {
                double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
                double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
                double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
                MACRO_plNormalizeVector(nx2,ny2,nz2);
                tmp2 = MACRO_plDotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
              }
              if (light->Type & PL_LIGHT_POINT_DISTANCE) {
                double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
                double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
                double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
                if (light->Type & PL_LIGHT_POINT_ANGLE) {
                   nx2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/
                           light->HalfDistSquared));
                  tmp2 *= plMax(0,plMin(1.0,nx2))*light->Intensity;
                } else {
                  tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/
                    light->HalfDistSquared));
                  tmp2 = plMax(0,plMin(1.0,tmp2))*light->Intensity;
                }
              }
              if (light->Type == PL_LIGHT_VECTOR)
                tmp2 = MACRO_plDotProduct(nx,ny,nz,_lights[i].l[0],_lights[i].l[1],_lights[i].l[2])
                  * light->Intensity;
              if (tmp2 > 0.0) tmp += tmp2;
              else if (obj->BackfaceIllumination) tmp -= tmp2;
            } /* End of light loop */
          } /* End of flat shading if */
          if (face->Material->_st & PL_SHADE_FLAT_DISTANCE)
            tmp += 1.0-(face->Vertices[0]->xformedz+face->Vertices[1]->xformedz+
                        face->Vertices[2]->xformedz) /
                       (face->Material->FadeDist*3.0);
          face->fShade = (pl_Float) tmp;
        } else face->fShade = 0.0; /* End of flatmask lighting if */
        if (face->Material->_ft & PL_FILL_ENVIRONMENT) {
          face->eMappingU[0] = 32768 + (pl_sInt32) (face->Vertices[0]->xformednx*32768.0);
          face->eMappingV[0] = 32768 - (pl_sInt32) (face->Vertices[0]->xformedny*32768.0);
          face->eMappingU[1] = 32768 + (pl_sInt32) (face->Vertices[1]->xformednx*32768.0);
          face->eMappingV[1] = 32768 - (pl_sInt32) (face->Vertices[1]->xformedny*32768.0);
          face->eMappingU[2] = 32768 + (pl_sInt32) (face->Vertices[2]->xformednx*32768.0);
          face->eMappingV[2] = 32768 - (pl_sInt32) (face->Vertices[2]->xformedny*32768.0);
        }
        if (face->Material->_st &(PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE)) {
          register pl_uChar a;
          for (a = 0; a < 3; a ++) {
            tmp = face->vsLighting[a];
            if (face->Material->_st & PL_SHADE_GOURAUD) {
              for (i = 0; i < _numlights ; i++) {
                tmp2 = 0.0;
                light = _lights[i].light;
                if (light->Type & PL_LIGHT_POINT_ANGLE) {
                  nx = _lights[i].l[0] - face->Vertices[a]->xformedx;
                  ny = _lights[i].l[1] - face->Vertices[a]->xformedy;
                  nz = _lights[i].l[2] - face->Vertices[a]->xformedz;
                  MACRO_plNormalizeVector(nx,ny,nz);
                  tmp2 = MACRO_plDotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      nx,ny,nz) * light->Intensity;
                }
                if (light->Type & PL_LIGHT_POINT_DISTANCE) {
                  double nx2 = _lights[i].l[0] - face->Vertices[a]->xformedx;
                  double ny2 = _lights[i].l[1] - face->Vertices[a]->xformedy;
                  double nz2 = _lights[i].l[2] - face->Vertices[a]->xformedz;
                  if (light->Type & PL_LIGHT_POINT_ANGLE) {
                     double t= (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
                     tmp2 *= plMax(0,plMin(1.0,t))*light->Intensity;
                  } else {
                    tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
                    tmp2 = plMax(0,plMin(1.0,tmp2))*light->Intensity;
                  }
                }
                if (light->Type == PL_LIGHT_VECTOR)
                  tmp2 = MACRO_plDotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      _lights[i].l[0],_lights[i].l[1],_lights[i].l[2])
                                        * light->Intensity;
                if (tmp2 > 0.0) tmp += tmp2;
                else if (obj->BackfaceIllumination) tmp -= tmp2;
              } /* End of light loop */
            } /* End of gouraud shading if */
            if (face->Material->_st & PL_SHADE_GOURAUD_DISTANCE)
              tmp += 1.0-face->Vertices[a]->xformedz/face->Material->FadeDist;
            face->Shades[a] = (pl_Float) tmp;
          } /* End of vertex loop for */
        } /* End of gouraud shading mask if */
        _faces[facepos].zd = face->Vertices[0]->xformedz+
        face->Vertices[1]->xformedz+face->Vertices[2]->xformedz;
        _faces[facepos++].face = face;
        plRender_TriStats[1] ++;
      } /* Is it in our area Check */
    } /* Backface Check */
    _numfaces = facepos;
    face++;
  } while (--x); /* Face loop */
}

void plRenderObj(pl_Obj *obj) {
  _RenderObj(obj,0,0);
}

void plRenderEnd() {
  _faceInfo *f;
  if (_cam->Sort > 0) _hsort(_faces,_numfaces,0);
  else if (_cam->Sort < 0) _hsort(_faces,_numfaces,1);
  f = _faces;
  while (_numfaces--) {
    if (f->face->Material && f->face->Material->_PutFace)
    {
      plClipRenderFace(f->face);
    }
    f++;
  }
  _numfaces=0;
  _numlights = 0;
}

static _faceInfo *Base, tmp;

static void _hsort(_faceInfo *base, int nel, int dir) {
  static int i;
  Base=base-1;
  for (i=nel/2; i>0; i--) _sift_down(i,nel,dir);
  for (i=nel; i>1; ) {
    tmp = base[0]; base[0] = Base[i]; Base[i] = tmp;
    _sift_down(1,i-=1,dir);
  }
}

#define Comp(x,y) (( x ).zd < ( y ).zd ? 1 : 0)

static void _sift_down(int L, int U, int dir) {
  static int c;
  while (1) {
    c=L+L;
    if (c>U) break;
    if ( (c < U) && dir^Comp(Base[c+1],Base[c])) c++;
    if (dir^Comp(Base[L],Base[c])) return;
    tmp = Base[L]; Base[L] = Base[c]; Base[c] = tmp;
    L=c;
  }
}
#undef Comp
