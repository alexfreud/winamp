/******************************************************************************
Plush Version 1.2
obj.c
Object control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

pl_Obj *plObjScale(pl_Obj *o, pl_Float s) {
  pl_uInt32 i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x *= s; v->y *= s; v->z *= s; v++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (o->Children[i]) plObjScale(o->Children[i],s);
  return o;
}

pl_Obj *plObjStretch(pl_Obj *o, pl_Float x, pl_Float y, pl_Float z) {
  pl_uInt32 i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x *= x; v->y *= y; v->z *= z; v++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (o->Children[i]) plObjStretch(o->Children[i],x,y,z);
  return o;
}

pl_Obj *plObjTranslate(pl_Obj *o, pl_Float x, pl_Float y, pl_Float z) {
  pl_uInt32 i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x += x; v->y += y; v->z += z; v++;
  }
  return o;
}

pl_Obj *plObjFlipNormals(pl_Obj *o) {
  pl_uInt32 i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  pl_Face *f = o->Faces;
  while (i--) {
    v->nx = - v->nx; v->ny = - v->ny; v->nz = - v->nz; v++;
  }
  i = o->NumFaces;
  while (i--) {
    f->nx = - f->nx; f->ny = - f->ny; f->nz = - f->nz;
    f++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (o->Children[i]) plObjFlipNormals(o->Children[i]);
  return o;
}

void plObjDelete(pl_Obj *o) {
  pl_uInt i;
  if (o) {
    for (i = 0; i < PL_MAX_CHILDREN; i ++)
      if (o->Children[i]) plObjDelete(o->Children[i]);
    if (o->Vertices) free(o->Vertices);
    if (o->Faces) free(o->Faces);
    free(o);
  }
}

pl_Obj *plObjCreate(pl_uInt32 nv, pl_uInt32 nf) {
  pl_Obj *o;
  if (!(o = (pl_Obj *) malloc(sizeof(pl_Obj)))) return 0;
  memset(o,0,sizeof(pl_Obj));
  o->GenMatrix = 1;
  o->BackfaceCull = 1;
  o->NumVertices = nv;
  o->NumFaces = nf;
  if (nv && !(o->Vertices=(pl_Vertex *) malloc(sizeof(pl_Vertex)*nv))) {
    free(o);
    return 0;
  }
  if (nf && !(o->Faces = (pl_Face *) malloc(sizeof(pl_Face)*nf))) {
    free(o->Vertices);
    free(o);
    return 0;
  }
  memset(o->Vertices,0,sizeof(pl_Vertex)*nv);
  memset(o->Faces,0,sizeof(pl_Face)*nf);
  return o;
}

pl_Obj *plObjClone(pl_Obj *o) {
  pl_Face *iff, *of;
  pl_uInt32 i;
  pl_Obj *out;
  if (!(out = plObjCreate(o->NumVertices,o->NumFaces))) return 0;
  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (o->Children[i]) out->Children[i] = plObjClone(o->Children[i]);
  out->Xa = o->Xa; out->Ya = o->Ya; out->Za = o->Za;
  out->Xp = o->Xp; out->Yp = o->Yp; out->Zp = o->Zp;
  out->BackfaceCull = o->BackfaceCull;
  out->BackfaceIllumination = o->BackfaceIllumination;
  out->GenMatrix = o->GenMatrix;
  memcpy(out->Vertices, o->Vertices, sizeof(pl_Vertex) * o->NumVertices);
  iff = o->Faces;
  of = out->Faces;
  i = out->NumFaces;
  while (i--) {
    of->Vertices[0] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[0] - o->Vertices);
    of->Vertices[1] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[1] - o->Vertices);
    of->Vertices[2] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[2] - o->Vertices);
    of->MappingU[0] = iff->MappingU[0];
    of->MappingV[0] = iff->MappingV[0];
    of->MappingU[1] = iff->MappingU[1];
    of->MappingV[1] = iff->MappingV[1];
    of->MappingU[2] = iff->MappingU[2];
    of->MappingV[2] = iff->MappingV[2];
    of->nx = iff->nx;
    of->ny = iff->ny;
    of->nz = iff->nz;
    of->Material = iff->Material;
    of++;
    iff++;
  }
  return out;
}

void plObjSetMat(pl_Obj *o, pl_Mat *m, pl_Bool th) {
  pl_sInt32 i = o->NumFaces;
  pl_Face *f = o->Faces;
  while (i--) (f++)->Material = m;
  if (th) for (i = 0; i < PL_MAX_CHILDREN; i++)
    if (o->Children[i]) plObjSetMat(o->Children[i],m,th);
}

void plObjCalcNormals(pl_Obj *obj) {
  pl_uInt32 i;
  pl_Vertex *v = obj->Vertices;
  pl_Face *f = obj->Faces;
  double x1, x2, y1, y2, z1, z2;
  i = obj->NumVertices;
  while (i--) {
    v->nx = 0.0; v->ny = 0.0; v->nz = 0.0;
    v++;
  }
  i = obj->NumFaces;
  while (i--) {
    x1 = f->Vertices[0]->x-f->Vertices[1]->x;
    x2 = f->Vertices[0]->x-f->Vertices[2]->x;
    y1 = f->Vertices[0]->y-f->Vertices[1]->y;
    y2 = f->Vertices[0]->y-f->Vertices[2]->y;
    z1 = f->Vertices[0]->z-f->Vertices[1]->z;
    z2 = f->Vertices[0]->z-f->Vertices[2]->z;
    f->nx = (pl_Float) (y1*z2 - z1*y2);
    f->ny = (pl_Float) (z1*x2 - x1*z2);
    f->nz = (pl_Float) (x1*y2 - y1*x2);
    plNormalizeVector(&f->nx, &f->ny, &f->nz);
    f->Vertices[0]->nx += f->nx;
    f->Vertices[0]->ny += f->ny;
    f->Vertices[0]->nz += f->nz;
    f->Vertices[1]->nx += f->nx;
    f->Vertices[1]->ny += f->ny;
    f->Vertices[1]->nz += f->nz;
    f->Vertices[2]->nx += f->nx;
    f->Vertices[2]->ny += f->ny;
    f->Vertices[2]->nz += f->nz;
    f++;
  }
  v = obj->Vertices;
  i = obj->NumVertices;
  do {
    plNormalizeVector(&v->nx, &v->ny, &v->nz);
    v++;
  } while (--i);
  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (obj->Children[i]) plObjCalcNormals(obj->Children[i]);
}
