/******************************************************************************
Plush Version 1.2
mat.c
Material Control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

static void _plGenerateSinglePalette(pl_Mat *);
static void _plGeneratePhongPalette(pl_Mat *);
static void _plGenerateTextureEnvPalette(pl_Mat *);
static void _plGenerateTexturePalette(pl_Mat *, pl_Texture *);
static void _plGeneratePhongTexturePalette(pl_Mat *, pl_Texture *);
static void _plGeneratePhongTransparentPalette(pl_Mat *m);
static void  _plGenerateTransparentPalette(pl_Mat *);
static void _plSetMaterialPutFace(pl_Mat *m);
static void _plMatSetupTransparent(pl_Mat *m, pl_uChar *pal);

pl_Mat *plMatCreate() {
  pl_Mat *m;
  m = (pl_Mat *) malloc(sizeof(pl_Mat));
  if (!m) return 0;
  memset(m,0,sizeof(pl_Mat));
  m->EnvScaling = 1.0f;
  m->TexScaling = 1.0f;
  m->Ambient[0] = m->Ambient[1] = m->Ambient[2] = 0;
  m->Diffuse[0] = m->Diffuse[1] = m->Diffuse[2] = 128;
  m->Specular[0] = m->Specular[1] = m->Specular[2] = 128;
  m->Shininess = 4;
  m->NumGradients = 32;
  m->FadeDist = 1000.0;
  m->zBufferable = 1;
  return m;
}

void plMatDelete(pl_Mat *m) {
  if (m) {
    if (m->_ReMapTable) free(m->_ReMapTable);
    if (m->_RequestedColors) free(m->_RequestedColors);
    if (m->_AddTable) free(m->_AddTable);
    free(m);
  }
}

void plMatInit(pl_Mat *m) {
  if (m->Shininess < 1) m->Shininess = 1;
  m->_ft = ((m->Environment ? PL_FILL_ENVIRONMENT : 0) |
           (m->Texture ? PL_FILL_TEXTURE : 0));
  m->_st = m->ShadeType;

  if (m->Transparent) m->_ft = PL_FILL_TRANSPARENT;

  if (m->_ft == (PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT))
    m->_st = PL_SHADE_NONE;

  if (m->_ft == PL_FILL_SOLID) {
    if (m->_st == PL_SHADE_NONE) _plGenerateSinglePalette(m);
    else _plGeneratePhongPalette(m);
  } else if (m->_ft == PL_FILL_TEXTURE) {
    if (m->_st == PL_SHADE_NONE)
      _plGenerateTexturePalette(m,m->Texture);
    else _plGeneratePhongTexturePalette(m,m->Texture);
  } else if (m->_ft == PL_FILL_ENVIRONMENT) {
    if (m->_st == PL_SHADE_NONE)
      _plGenerateTexturePalette(m,m->Environment);
    else _plGeneratePhongTexturePalette(m,m->Environment);
  } else if (m->_ft == (PL_FILL_ENVIRONMENT|PL_FILL_TEXTURE))
    _plGenerateTextureEnvPalette(m);
  else if (m->_ft == PL_FILL_TRANSPARENT) {
    if (m->_st == PL_SHADE_NONE) _plGenerateTransparentPalette(m);
    else _plGeneratePhongTransparentPalette(m);
  }
  _plSetMaterialPutFace(m);
}

static void _plMatSetupTransparent(pl_Mat *m, pl_uChar *pal) {
  pl_uInt x, intensity;
  if (m->Transparent)
  {
    if (m->_AddTable) free(m->_AddTable);
    m->_AddTable = (pl_uInt16 *) malloc(256*sizeof(pl_uInt16));
    for (x = 0; x < 256; x ++) {
      intensity = *pal++;
      intensity += *pal++;
      intensity += *pal++;
      m->_AddTable[x] = ((intensity*(m->_ColorsUsed-m->_tsfact))/768);
    }
  }
}

void plMatMapToPal(pl_Mat *m, pl_uChar *pal, pl_sInt pstart, pl_sInt pend) {
  pl_sInt32 j, r, g, b, bestdiff, r2, g2, b2;
  pl_sInt bestpos,k;
  pl_uInt32 i;
  pl_uChar *p;
  if (!m->_RequestedColors) plMatInit(m);
  if (!m->_RequestedColors) return;
  if (m->_ReMapTable) free(m->_ReMapTable);
  m->_ReMapTable = (pl_uChar *) malloc(m->_ColorsUsed);
  for (i = 0; i < m->_ColorsUsed; i ++) {
    bestdiff = 1000000000;
    bestpos = pstart;
    r = m->_RequestedColors[i*3];
    g = m->_RequestedColors[i*3+1];
    b = m->_RequestedColors[i*3+2];
    p = pal + pstart*3;
    for (k = pstart; k <= (pl_sInt)pend; k ++) {
      r2 = p[0] - r;
      g2 = p[1] - g;
      b2 = p[2] - b;
      p += 3;
      j = r2*r2+g2*g2+b2*b2;
      if (j < bestdiff) {
        bestdiff = j;
        bestpos = k;
      }
    }
    m->_ReMapTable[i] = bestpos;
  }
  _plMatSetupTransparent(m,pal);
}

static void _plGenerateSinglePalette(pl_Mat *m) {
  m->_ColorsUsed = 1;
  if (m->_RequestedColors) free(m->_RequestedColors);
  m->_RequestedColors = (pl_uChar *) malloc(3);
  m->_RequestedColors[0] = plMin(plMax(m->Ambient[0],0),255);
  m->_RequestedColors[1] = plMin(plMax(m->Ambient[1],0),255);
  m->_RequestedColors[2] = plMin(plMax(m->Ambient[2],0),255);
}

static void _plGeneratePhongPalette(pl_Mat *m) {
  pl_uInt i = m->NumGradients, x;
  pl_sInt c;
  pl_uChar *pal;
  double a, da, ca, cb;
  m->_ColorsUsed = m->NumGradients;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal =  m->_RequestedColors = (pl_uChar *) malloc(m->_ColorsUsed*3);
  a = PL_PI/2.0;

  if (m->NumGradients > 1) da = -PL_PI/((m->NumGradients-1)<<1);
  else da=0.0;

  do {
    if (m->NumGradients == 1) ca = 1;
    else {
      ca = cos((double) a);
      a += da;
    }
    cb = pow((double) ca, (double) m->Shininess);
    for (x = 0; x < 3; x ++) {
      c = (pl_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x]);
      *(pal++) = plMax(0,plMin(c,255));
    }
  } while (--i);
}

static void _plGenerateTextureEnvPalette(pl_Mat *m) {
  pl_sInt c;
  pl_uInt whichlevel,whichindex;
  pl_uChar *texpal, *envpal, *pal;
  m->_ColorsUsed = m->Texture->NumColors*m->Environment->NumColors;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (pl_uChar *) malloc(m->_ColorsUsed*3);
  envpal = m->Environment->PaletteData;
  if (m->_AddTable) free(m->_AddTable);
  m->_AddTable = (pl_uInt16 *) malloc(m->Environment->NumColors*sizeof(pl_uInt16));
  for (whichlevel = 0; whichlevel < m->Environment->NumColors; whichlevel++) {
    texpal = m->Texture->PaletteData;
    switch (m->TexEnvMode)
    {
      case PL_TEXENV_MUL: // multiply
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) * (pl_sInt) envpal[0])>>8);
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) * (pl_sInt) envpal[1])>>8);
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) * (pl_sInt) envpal[2])>>8);
        }
      break;
      case PL_TEXENV_AVG: // average
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) + (pl_sInt) envpal[0])>>1);
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) + (pl_sInt) envpal[1])>>1);
          *pal++ = (pl_uChar) (((pl_sInt) (*texpal++) + (pl_sInt) envpal[2])>>1);
        }
      break;
      case PL_TEXENV_TEXMINUSENV: // tex-env
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          c = (pl_sInt) (*texpal++) - (pl_sInt) envpal[0]; *pal++ = plMax(0,plMin(255,c));
          c = (pl_sInt) (*texpal++) - (pl_sInt) envpal[1]; *pal++ = plMax(0,plMin(255,c));
          c = (pl_sInt) (*texpal++) - (pl_sInt) envpal[2]; *pal++ = plMax(0,plMin(255,c));
        }
      break;
      case PL_TEXENV_ENVMINUSTEX: // env-tex
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          c = -(pl_sInt) (*texpal++) - (pl_sInt) envpal[0]; *pal++ = plMax(0,plMin(255,c));
          c = -(pl_sInt) (*texpal++) - (pl_sInt) envpal[1]; *pal++ = plMax(0,plMin(255,c));
          c = -(pl_sInt) (*texpal++) - (pl_sInt) envpal[2]; *pal++ = plMax(0,plMin(255,c));
        }
      break;
      case PL_TEXENV_MIN:
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          *pal++ = plMin(texpal[0],envpal[0]);
          *pal++ = plMin(texpal[1],envpal[1]);
          *pal++ = plMin(texpal[2],envpal[2]);
          texpal+=3;
        }
      break;
      case PL_TEXENV_MAX:
      break;
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          *pal++ = plMax(texpal[0],envpal[0]);
          *pal++ = plMax(texpal[1],envpal[1]);
          *pal++ = plMax(texpal[2],envpal[2]);
          texpal+=3;
        }
      default: // add
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          c = (pl_sInt) (*texpal++) + (pl_sInt) envpal[0]; *pal++ = plMax(0,plMin(255,c));
          c = (pl_sInt) (*texpal++) + (pl_sInt) envpal[1]; *pal++ = plMax(0,plMin(255,c));
          c = (pl_sInt) (*texpal++) + (pl_sInt) envpal[2]; *pal++ = plMax(0,plMin(255,c));
        }
      break;
    }
    envpal += 3;
    m->_AddTable[whichlevel] = whichlevel*m->Texture->NumColors;
  }
}

static void _plGenerateTexturePalette(pl_Mat *m, pl_Texture *t) {
  pl_uChar *ppal, *pal;
  pl_sInt c, i, x;
  m->_ColorsUsed = t->NumColors;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (pl_uChar *) malloc(m->_ColorsUsed*3);
  ppal = t->PaletteData;
  i = t->NumColors;
  do {
    for (x = 0; x < 3; x ++) {
      c = m->Ambient[x] + *ppal++;
      *(pal++) = plMax(0,plMin(c,255));
    }
  } while (--i);
}

static void _plGeneratePhongTexturePalette(pl_Mat *m, pl_Texture *t) {
  double a, ca, da, cb;
  pl_uInt16 *addtable;
  pl_uChar *ppal, *pal;
  pl_sInt c, i, i2, x;
  pl_uInt num_shades;

  if (t->NumColors) num_shades = (m->NumGradients / t->NumColors);
  else num_shades=1;

  if (!num_shades) num_shades = 1;
  m->_ColorsUsed = num_shades*t->NumColors;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (pl_uChar *) malloc(m->_ColorsUsed*3);
  a = PL_PI/2.0;
  if (num_shades>1) da = (-PL_PI/2.0)/(num_shades-1);
  else da=0.0;
  i2 = num_shades;
  do {
    ppal = t->PaletteData;
    ca = cos((double) a);
    a += da;
    cb = pow(ca, (double) m->Shininess);
    i = t->NumColors;
    do {
      for (x = 0; x < 3; x ++) {
        c = (pl_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x] + *ppal++);
        *(pal++) = plMax(0,plMin(c,255));
      }
    } while (--i);
  } while (--i2);
  ca = 0;
  if (m->_AddTable) free(m->_AddTable);
  m->_AddTable = (pl_uInt16 *) malloc(256*sizeof(pl_uInt16));
  addtable = m->_AddTable;
  i = 256;
  do {
    a = sin(ca) * num_shades;
    ca += PL_PI/512.0;
    *addtable++ = ((pl_sInt) a)*t->NumColors;
  } while (--i);
}

static void _plGeneratePhongTransparentPalette(pl_Mat *m) {
  m->_tsfact = (pl_sInt) (m->NumGradients*(1.0/(1+m->Transparent)));
  _plGeneratePhongPalette(m);
}

static void  _plGenerateTransparentPalette(pl_Mat *m) {
  m->_tsfact = 0;
  _plGeneratePhongPalette(m);
}

static void _plSetMaterialPutFace(pl_Mat *m) {
  m->_PutFace = 0;
  switch (m->_ft) {
    case PL_FILL_TRANSPARENT: switch(m->_st) {
      case PL_SHADE_NONE: case PL_SHADE_FLAT:
      case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT:
        m->_PutFace = plPF_TransF;
      break;
      case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE:
      case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE:
        m->_PutFace = plPF_TransG;
      break;
    }
    break;
    case PL_FILL_SOLID: switch(m->_st) {
      case PL_SHADE_NONE: case PL_SHADE_FLAT:
      case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT:
        m->_PutFace = plPF_SolidF;
      break;
      case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE:
      case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE:
        m->_PutFace = plPF_SolidG;
      break;
    }
    break;
    case PL_FILL_ENVIRONMENT:
    case PL_FILL_TEXTURE:
      if (m->PerspectiveCorrect) switch (m->_st) {
        case PL_SHADE_NONE: case PL_SHADE_FLAT:
        case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT:
          m->_PutFace = plPF_PTexF;
        break;
        case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE:
        case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE:
          m->_PutFace = plPF_PTexG;
        break;
      }
      else switch (m->_st) {
        case PL_SHADE_NONE: case PL_SHADE_FLAT:
        case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT:
          m->_PutFace = plPF_TexF;
        break;
        case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE:
        case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE:
          m->_PutFace = plPF_TexG;
        break;
      }
    break;
    case PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT:
      m->_PutFace = plPF_TexEnv;
    break;
  }
}

typedef struct __ct {
  pl_uChar r,g,b;
  pl_Bool visited;
  struct __ct *next;
} _ct;

static int mdist(_ct *a, _ct *b) {
  return ((a->r-b->r)*(a->r-b->r)+(a->g-b->g)*(a->g-b->g)+(a->b-b->b)*(a->b-b->b));
}

void plMatMakeOptPal(pl_uChar *p, pl_sInt pstart,
                     pl_sInt pend, pl_Mat **materials, pl_sInt nmats) {
  pl_uChar *allColors = 0;
  pl_sInt numColors = 0, nc, x;
  pl_sInt len = pend + 1 - pstart;
  pl_sInt32 current, newnext, bestdist, thisdist;
  _ct *colorBlock, *best, *cp;

  for (x = 0; x < nmats; x ++) {
    if (materials[x]) {
      if (!materials[x]->_RequestedColors) plMatInit(materials[x]);
      if (materials[x]->_RequestedColors) numColors+=materials[x]->_ColorsUsed;
    }
  }
  if (!numColors) return;

  allColors=(pl_uChar*)malloc(numColors*3);
  numColors=0;

  for (x = 0; x < nmats; x ++) {
    if (materials[x]) {
      if (materials[x]->_RequestedColors)
        memcpy(allColors + (numColors*3), materials[x]->_RequestedColors,
             materials[x]->_ColorsUsed*3);
      numColors += materials[x]->_ColorsUsed;
    }
  }

  if (numColors <= len) {
    memcpy(p+pstart*3,allColors,numColors*3);
    free(allColors);
    return;
  }

  colorBlock = (_ct *) malloc(sizeof(_ct)*numColors);
  for (x = 0; x < numColors; x++) {
    colorBlock[x].r = allColors[x*3];
    colorBlock[x].g = allColors[x*3+1];
    colorBlock[x].b = allColors[x*3+2];
    colorBlock[x].visited = 0;
    colorBlock[x].next = 0;
  }
  free(allColors);

  /* Build a list, starting at color 0 */
  current = 0;
  nc = numColors;
  do {
    newnext = -1;
    bestdist = 300000000;
    colorBlock[current].visited = 1;
    for (x = 0; x < nc; x ++) {
      if (!colorBlock[x].visited) {
        thisdist = mdist(colorBlock + x, colorBlock + current);
        if (thisdist < 5) { colorBlock[x].visited = 1; numColors--; }
        else if (thisdist < bestdist) { bestdist = thisdist; newnext = x; }
      }
    }
    if (newnext != -1) {
      colorBlock[current].next = colorBlock + newnext;
      current = newnext;
    }
  } while (newnext != -1);
  colorBlock[current].next = 0; /* terminate the list */

  /* we now have a linked list starting at colorBlock, which is each one and
     it's closest neighbor */

  while (numColors > len) {
    bestdist = mdist(colorBlock,colorBlock->next);
    for (best = cp = colorBlock; cp->next; cp = cp->next) {
      if (bestdist > (thisdist = mdist(cp,cp->next))) {
        best = cp;
        bestdist = thisdist;
      }
    }
    best->r = ((int) best->r + (int) best->next->r)>>1;
    best->g = ((int) best->g + (int) best->next->g)>>1;
    best->b = ((int) best->b + (int) best->next->b)>>1;
    best->next = best->next->next;
    numColors--;
  }
  x = pstart*3;
  for (cp = colorBlock; cp; cp = cp->next) {
    p[x++] = cp->r;
    p[x++] = cp->g;
    p[x++] = cp->b;
  }
  free(colorBlock);
}
