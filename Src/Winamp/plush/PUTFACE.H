/******************************************************************************
Plush Version 1.2
putface.h
Triangle Vertex Sorting Code for pf_*.c
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#ifndef _PUTFACE_H_
#define _PUTFACE_H_

#define PUTFACE_SORT() \
  i0 = 0; i1 = 1; i2 = 2; \
  if (TriFace->Scry[0] > TriFace->Scry[1]) { \
     i0 = 1; i1 = 0; \
  } \
  if (TriFace->Scry[i0] > TriFace->Scry[2]) { \
     i2 ^= i0; i0 ^= i2; i2 ^= i0; \
  } \
  if (TriFace->Scry[i1] > TriFace->Scry[i2]) { \
     i2 ^= i1; i1 ^= i2; i2 ^= i1; \
  }


#define PUTFACE_SORT_ENV() \
  PUTFACE_SORT(); \
  MappingU1=TriFace->eMappingU[i0]*Texture->uScale*\
            TriFace->Material->EnvScaling;\
  MappingV1=TriFace->eMappingV[i0]*Texture->vScale*\
            TriFace->Material->EnvScaling;\
  MappingU2=TriFace->eMappingU[i1]*Texture->uScale*\
            TriFace->Material->EnvScaling;\
  MappingV2=TriFace->eMappingV[i1]*Texture->vScale*\
            TriFace->Material->EnvScaling;\
  MappingU3=TriFace->eMappingU[i2]*Texture->uScale*\
            TriFace->Material->EnvScaling;\
  MappingV3=TriFace->eMappingV[i2]*Texture->vScale*\
            TriFace->Material->EnvScaling;

#define PUTFACE_SORT_TEX() \
  PUTFACE_SORT(); \
  MappingU1=TriFace->MappingU[i0]*Texture->uScale*\
            TriFace->Material->TexScaling;\
  MappingV1=TriFace->MappingV[i0]*Texture->vScale*\
            TriFace->Material->TexScaling;\
  MappingU2=TriFace->MappingU[i1]*Texture->uScale*\
            TriFace->Material->TexScaling;\
  MappingV2=TriFace->MappingV[i1]*Texture->vScale*\
            TriFace->Material->TexScaling;\
  MappingU3=TriFace->MappingU[i2]*Texture->uScale*\
            TriFace->Material->TexScaling;\
  MappingV3=TriFace->MappingV[i2]*Texture->vScale*\
            TriFace->Material->TexScaling;

#endif /* !_PUTFACE_H_ */
