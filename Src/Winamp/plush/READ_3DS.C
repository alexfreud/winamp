/******************************************************************************
Plush Version 1.2
read_3ds.c
3DS Object Reader
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

typedef struct {
    pl_uInt16 id;
    void (*func)(FILE *f, pl_uInt32 p);
} _pl_3DSChunk;

static pl_Obj *obj;
static pl_Obj *bobj;
static pl_Obj *lobj;
static pl_sInt16 currentobj;
static pl_Mat *_m;

static pl_Float _pl3DSReadFloat(FILE *f);
static pl_uInt32 _pl3DSReadDWord(FILE *f);
static pl_uInt16 _pl3DSReadWord(FILE *f);
static void _pl3DSChunkReader(FILE *f, pl_uInt32 p);
static void _pl3DSRGBFReader(FILE *f, pl_uInt32 p);
static void _pl3DSRGBBReader(FILE *f, pl_uInt32 p);
static void _pl3DSASCIIZReader(FILE *f, pl_uInt32 p, char *as);
static void _pl3DSObjBlockReader(FILE *f, pl_uInt32 p);
static void _pl3DSTriMeshReader(FILE *f, pl_uInt32 p);
static void _pl3DSVertListReader(FILE *f, pl_uInt32 p);
static void _pl3DSFaceListReader(FILE *f, pl_uInt32 p);
static void _pl3DSFaceMatReader(FILE *f, pl_uInt32 p);
static void MapListReader(FILE *f, pl_uInt32 p);
static pl_sInt16 _pl3DSFindChunk(pl_uInt16 id);

static _pl_3DSChunk _pl3DSChunkNames[] = {
    {0x4D4D,NULL}, /* Main */
    {0x3D3D,NULL}, /* Object Mesh */
    {0x4000,_pl3DSObjBlockReader},
    {0x4100,_pl3DSTriMeshReader},
    {0x4110,_pl3DSVertListReader},
    {0x4120,_pl3DSFaceListReader},
    {0x4130,_pl3DSFaceMatReader},
    {0x4140,MapListReader},
    {0xAFFF,NULL}, /* Material */
    {0xA010,NULL}, /* Ambient */
    {0xA020,NULL}, /* Diff */
    {0xA030,NULL}, /* Specular */
    {0xA200,NULL}, /* Texture */
    {0x0010,_pl3DSRGBFReader},
    {0x0011,_pl3DSRGBBReader},
};

pl_Obj *plRead3DSObj(char *fn, pl_Mat *m) {
  FILE *f;
  pl_uInt32 p;
  _m = m;
  obj = bobj = lobj = 0;
  currentobj = 0;
  f = fopen(fn, "rb");
  if (!f) return 0;
  fseek(f, 0, 2);
  p = ftell(f);
  rewind(f);
  _pl3DSChunkReader(f, p);
  fclose(f);
  return bobj;
}

static pl_Float _pl3DSReadFloat(FILE *f) {
  pl_uInt32 *i;
  pl_IEEEFloat32 c;
  i = (pl_uInt32 *) &c;
  *i = _pl3DSReadDWord(f);
  return ((pl_Float) c);
}

static pl_uInt32 _pl3DSReadDWord(FILE *f) {
  pl_uInt32 r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  r |= fgetc(f)<<16;
  r |= fgetc(f)<<24;
  return r;
}

static pl_uInt16 _pl3DSReadWord(FILE *f) {
  pl_uInt16 r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  return r;
}

static void _pl3DSRGBFReader(FILE *f, pl_uInt32 p) {
  pl_Float c[3];
  c[0] = _pl3DSReadFloat(f);
  c[1] = _pl3DSReadFloat(f);
  c[2] = _pl3DSReadFloat(f);
}

static void _pl3DSRGBBReader(FILE *f, pl_uInt32 p) {
  unsigned char c[3];
  if (fread(&c, sizeof(c), 1, f) != 1) return;
}

static void _pl3DSASCIIZReader(FILE *f, pl_uInt32 p, char *as) {
  char c;
  if (!as) while ((c = fgetc(f)) != EOF && c != '\0');
  else {
    while ((c = fgetc(f)) != EOF && c != '\0') *as++ = c;
    *as = 0;
  }
}

static void _pl3DSObjBlockReader(FILE *f, pl_uInt32 p) {
  _pl3DSASCIIZReader(f,p,0);
  _pl3DSChunkReader(f, p);
}

static void _pl3DSTriMeshReader(FILE *f, pl_uInt32 p) {
  pl_uInt32 i;
  pl_Face *face;
  obj = plObjCreate(0,0);
  _pl3DSChunkReader(f, p);
  i = obj->NumFaces;
  face = obj->Faces;
  while (i--) {
    face->Vertices[0] = obj->Vertices + (pl_uInt32) face->Vertices[0];
    face->Vertices[1] = obj->Vertices + (pl_uInt32) face->Vertices[1];
    face->Vertices[2] = obj->Vertices + (pl_uInt32) face->Vertices[2];
    face->MappingU[0] = face->Vertices[0]->xformedx;
    face->MappingV[0] = face->Vertices[0]->xformedy;
    face->MappingU[1] = face->Vertices[1]->xformedx;
    face->MappingV[1] = face->Vertices[1]->xformedy;
    face->MappingU[2] = face->Vertices[2]->xformedx;
    face->MappingV[2] = face->Vertices[2]->xformedy;
    face++;
  }
  plObjCalcNormals(obj);
  if (currentobj == 0) {
    currentobj = 1;
    lobj = bobj = obj;
  } else {
    lobj->Children[0] = obj;
    lobj = obj;
  }
}

static void _pl3DSVertListReader(FILE *f, pl_uInt32 p) {
  pl_uInt16 nv;
  pl_Vertex *v;
  nv = _pl3DSReadWord(f);
  obj->NumVertices = nv;
  v = obj->Vertices = (pl_Vertex *) calloc(sizeof(pl_Vertex)*nv,1);
  while (nv--) {
    v->x = _pl3DSReadFloat(f);
    v->y = _pl3DSReadFloat(f);
    v->z = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v++;
  }
}

static void _pl3DSFaceListReader(FILE *f, pl_uInt32 p) {
  pl_uInt16 nv;
  pl_uInt16 c[3];
  pl_uInt16 flags;
  pl_Face *face;

  nv = _pl3DSReadWord(f);
  obj->NumFaces = nv;
  face = obj->Faces = (pl_Face *) calloc(sizeof(pl_Face)*nv,1);
  while (nv--) {
    c[0] = _pl3DSReadWord(f);
    c[1] = _pl3DSReadWord(f);
    c[2] = _pl3DSReadWord(f);
    flags = _pl3DSReadWord(f);
    if (feof(f)) return;
    face->Vertices[0] = (pl_Vertex *) (c[0]&0x0000FFFF);
    face->Vertices[1] = (pl_Vertex *) (c[1]&0x0000FFFF);
    face->Vertices[2] = (pl_Vertex *) (c[2]&0x0000FFFF);
    face->Material = _m;
    face++;
  }
  _pl3DSChunkReader(f, p);
}

static void _pl3DSFaceMatReader(FILE *f, pl_uInt32 p) {
  pl_uInt16 n, nf;

  _pl3DSASCIIZReader(f, p,0);

  n = _pl3DSReadWord(f);
  while (n--) {
    nf = _pl3DSReadWord(f);
  }
}

static void MapListReader(FILE *f, pl_uInt32 p) {
  pl_uInt16 nv;
  pl_Float c[2];
  pl_Vertex *v;
  nv = _pl3DSReadWord(f);
  v = obj->Vertices;
  if (nv == obj->NumVertices) while (nv--) {
    c[0] = _pl3DSReadFloat(f);
    c[1] = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v->xformedx = (pl_sInt32) (c[0]*65536.0);
    v->xformedy = (pl_sInt32) (c[1]*65536.0);
    v++;
  }
}

static pl_sInt16 _pl3DSFindChunk(pl_uInt16 id) {
  pl_sInt16 i;
  for (i = 0; i < sizeof(_pl3DSChunkNames)/sizeof(_pl3DSChunkNames[0]); i++)
    if (id == _pl3DSChunkNames[i].id) return i;
  return -1;
}

static void _pl3DSChunkReader(FILE *f, pl_uInt32 p) {
  pl_uInt32 hlen;
  pl_uInt16 hid;
  pl_sInt16 n;
  pl_uInt32 pc;

  while (ftell(f) < (int)p) {
    pc = ftell(f);
    hid = _pl3DSReadWord(f); if (feof(f)) return;
    hlen = _pl3DSReadDWord(f); if (feof(f)) return;
    if (hlen == 0) return;
    n = _pl3DSFindChunk(hid);
    if (n < 0) fseek(f, pc + hlen, 0);
    else {
      pc += hlen;
      if (_pl3DSChunkNames[n].func != NULL) _pl3DSChunkNames[n].func(f, pc);
      else _pl3DSChunkReader(f, pc);
      fseek(f, pc, 0);
    }
    if (ferror(f)) break;
  }
}

