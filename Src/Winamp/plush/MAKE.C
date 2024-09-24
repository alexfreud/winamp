/******************************************************************************
Plush Version 1.2
make.c
Object Primitives
Copyright (c) 1996-2000, Justin Frankel
*******************************************************************************
 Notes:
   Most of these routines are highly unoptimized.
   They could all use some work, such as more capable divisions (Box is
   most notable), etc... The mapping coordinates are all set up nicely,
   though.
******************************************************************************/

#include "plush.h"

pl_Obj *plMakeTorus(pl_Float r1, pl_Float r2, pl_uInt divrot, pl_uInt divrad,
                    pl_Mat *m) {
  pl_Obj *o;
  pl_Vertex *v;
  pl_Face *f;
  pl_uInt x, y;
  double ravg, rt, a, da, al, dal;
  pl_sInt32 U,V,dU,dV;
  if (divrot < 3) divrot = 3;
  if (divrad < 3) divrad = 3;
  ravg = (r1+r2)*0.5;
  rt = (r2-r1)*0.5;
  o = plObjCreate(divrad*divrot,divrad*divrot*2);
  if (!o) return 0;
  v = o->Vertices;
  a = 0.0;
  da = 2*PL_PI/divrot;
  for (y = 0; y < divrot; y ++) {
    al = 0.0;
    dal = 2*PL_PI/divrad;
    for (x = 0; x < divrad; x ++) {
      v->x = (pl_Float) (cos((double) a)*(ravg + cos((double) al)*rt));
      v->z = (pl_Float) (sin((double) a)*(ravg + cos((double) al)*rt));
      v->y = (pl_Float) (sin((double) al)*rt);
      v++;
      al += dal;
    }
    a += da;
  }
  v = o->Vertices;
  f = o->Faces;
  dV = 65535/divrad;
  dU = 65535/divrot;
  U = 0;
  for (y = 0; y < divrot; y ++) {
    V = -32768;
    for (x = 0; x < divrad; x ++) {
      f->Vertices[0] = v+x+y*divrad;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V;
      f->Material = m;
      f++;
      f->Vertices[0] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[0] = U+dU;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+(x+1==divrad?0:x+1)+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m;
      f++;
      V += dV;
    }
    U += dU;
  }
  plObjCalcNormals(o);
  return (o);
}

pl_Obj *plMakeSphere(pl_Float r, pl_uInt divr, pl_uInt divh, pl_Mat *m) {
  pl_Obj *o;
  pl_Vertex *v;
  pl_Face *f;
  pl_uInt x, y;
  double a, da, yp, ya, yda, yf;
  pl_sInt32 U,V,dU,dV;
  if (divh < 3) divh = 3;
  if (divr < 3) divr = 3;
  o = plObjCreate(2+(divh-2)*(divr),2*divr+(divh-3)*divr*2);
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0.0; v->y = r; v++;
  v->x = v->z = 0.0; v->y = -r; v++;
  ya = 0.0;
  yda = PL_PI/(divh-1);
  da = (PL_PI*2.0)/divr;
  for (y = 0; y < divh - 2; y ++) {
    ya += yda;
    yp = cos((double) ya)*r;
    yf = sin((double) ya)*r;
    a = 0.0;
    for (x = 0; x < divr; x ++) {
      v->y = (pl_Float) yp;
      v->x = (pl_Float) (cos((double) a)*yf);
      v->z = (pl_Float) (sin((double) a)*yf);
      v++;
      a += da;
    }
  }
  f = o->Faces;
  v = o->Vertices + 2;
  a = 0.0;
  U = 0;
  dU = 65535/divr;
  dV = V = 65535/divh;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = v + (x+1==divr ? 0 : x+1);
    f->Vertices[2] = v + x;
    f->MappingU[0] = U;
    f->MappingV[0] = 0;
    f->MappingU[1] = U+dU;
    f->MappingV[1] = V;
    f->MappingU[2] = U;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  da = 1.0/(divr+1);
  v = o->Vertices + 2;
  for (x = 0; x < (divh-3); x ++) {
    U = 0;
    for (y = 0; y < divr; y ++) {
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+divr+(y+1==divr?0:y+1);
      f->Vertices[2] = v+y+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V+dV;
      f->MappingU[2] = U;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+(y+1==divr?0:y+1);
      f->Vertices[2] = v+(y+1==divr?0:y+1)+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V;
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      U += dU;
    }
    V += dV;
    v += divr;
  }
  v = o->Vertices + o->NumVertices - divr;
  U = 0;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices + 1;
    f->Vertices[1] = v + x;
    f->Vertices[2] = v + (x+1==divr ? 0 : x+1);
    f->MappingU[0] = U;
    f->MappingV[0] = 65535;
    f->MappingU[1] = U;
    f->MappingV[1] = V;
    f->MappingU[2] = U+dU;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  plObjCalcNormals(o);
  return (o);
}

pl_Obj *plMakeCylinder(pl_Float r, pl_Float h, pl_uInt divr, pl_Bool captop,
                       pl_Bool capbottom, pl_Mat *m) {
  pl_Obj *o;
  pl_Vertex *v, *topverts, *bottomverts, *topcapvert=0, *bottomcapvert=0;
  pl_Face *f;
  pl_uInt32 i;
  double a, da;
  if (divr < 3) divr = 3;
  o = plObjCreate(divr*2+((divr==3)?0:(captop?1:0)+(capbottom?1:0)),
                  divr*2+(divr==3 ? (captop ? 1 : 0) + (capbottom ? 1 : 0) :
                  (captop ? divr : 0) + (capbottom ? divr : 0)));
  if (!o) return 0;
  a = 0.0;
  da = (2.0*PL_PI)/divr;
  v = o->Vertices;
  topverts = v;
  for (i = 0; i < divr; i ++) {
    v->y = h/2.0f;
    v->x = (pl_Float) (r*cos((double) a));
    v->z = (pl_Float)(r*sin(a));
    v->xformedx = (pl_Float) (32768.0 + (32768.0*cos((double) a))); // temp
    v->xformedy = (pl_Float) (32768.0 + (32768.0*sin((double) a))); // use xf
    v++;
    a += da;
  }
  bottomverts = v;
  a = 0.0;
  for (i = 0; i < divr; i ++) {
    v->y = -h/2.0f;
    v->x = (pl_Float) (r*cos((double) a));
    v->z = (pl_Float) (r*sin(a));
    v->xformedx = (pl_Float) (32768.0 + (32768.0*cos((double) a)));
    v->xformedy = (pl_Float) (32768.0 + (32768.0*sin((double) a)));
    v++; a += da;
  }
  if (captop && divr != 3) {
    topcapvert = v;
    v->y = h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  if (capbottom && divr != 3) {
    bottomcapvert = v;
    v->y = -h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  f = o->Faces;
  for (i = 0; i < divr; i ++) {
    f->Vertices[0] = bottomverts + i;
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[0] = f->MappingV[2] = 65535; f->MappingV[1] = 0;
    f->MappingU[0] = f->MappingU[1] = (i<<16)/divr;
    f->MappingU[2] = ((i+1)<<16)/divr;
    f->Material = m; f++;
    f->Vertices[0] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = topverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[1] = f->MappingV[2] = 0; f->MappingV[0] = 65535;
    f->MappingU[0] = f->MappingU[2] = ((i+1)<<16)/divr;
    f->MappingU[1] = (i<<16)/divr;
    f->Material = m; f++;
  }
  if (captop) {
    if (divr == 3) {
      f->Vertices[0] = topverts + 0;
      f->Vertices[1] = topverts + 2;
      f->Vertices[2] = topverts + 1;
      f->MappingU[0] = (pl_sInt32) topverts[0].xformedx;
      f->MappingV[0] = (pl_sInt32) topverts[0].xformedy;
      f->MappingU[1] = (pl_sInt32) topverts[1].xformedx;
      f->MappingV[1] = (pl_sInt32) topverts[1].xformedy;
      f->MappingU[2] = (pl_sInt32) topverts[2].xformedx;
      f->MappingV[2] = (pl_sInt32) topverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = topverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[1] = topverts + i;
        f->Vertices[2] = topcapvert;
        f->MappingU[0] = (pl_sInt32) topverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[0] = (pl_sInt32) topverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[1] = (pl_sInt32) topverts[i].xformedx;
        f->MappingV[1] = (pl_sInt32) topverts[i].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  if (capbottom) {
    if (divr == 3) {
      f->Vertices[0] = bottomverts + 0;
      f->Vertices[1] = bottomverts + 1;
      f->Vertices[2] = bottomverts + 2;
      f->MappingU[0] = (pl_sInt32) bottomverts[0].xformedx;
      f->MappingV[0] = (pl_sInt32) bottomverts[0].xformedy;
      f->MappingU[1] = (pl_sInt32) bottomverts[1].xformedx;
      f->MappingV[1] = (pl_sInt32) bottomverts[1].xformedy;
      f->MappingU[2] = (pl_sInt32) bottomverts[2].xformedx;
      f->MappingV[2] = (pl_sInt32) bottomverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = bottomverts + i;
        f->Vertices[1] = bottomverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[2] = bottomcapvert;
        f->MappingU[0] = (pl_sInt32) bottomverts[i].xformedx;
        f->MappingV[0] = (pl_sInt32) bottomverts[i].xformedy;
        f->MappingU[1] = (pl_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[1] = (pl_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  plObjCalcNormals(o);
  return (o);
}

pl_Obj *plMakeCone(pl_Float r, pl_Float h, pl_uInt div,
                   pl_Bool cap, pl_Mat *m) {
  pl_Obj *o;
  pl_Vertex *v;
  pl_Face *f;
  pl_uInt32 i;
  double a, da;
  if (div < 3) div = 3;
  o = plObjCreate(div + (div == 3 ? 1 : (cap ? 2 : 1)),
                  div + (div == 3 ? 1 : (cap ? div : 0)));
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0; v->y = h/2;
  v->xformedx = 1<<15;
  v->xformedy = 1<<15;
  v++;
  a = 0.0;
  da = (2.0*PL_PI)/div;
  for (i = 1; i <= div; i ++) {
    v->y = h/-2.0f;
    v->x = (pl_Float) (r*cos((double) a));
    v->z = (pl_Float) (r*sin((double) a));
    v->xformedx = (pl_Float) (32768.0 + (cos((double) a)*32768.0));
    v->xformedy = (pl_Float) (32768.0 + (sin((double) a)*32768.0));
    a += da;
    v++;
  }
  if (cap && div != 3) {
    v->y = h / -2.0f;
    v->x = v->z = 0.0f;
    v->xformedx = (pl_Float) (1<<15);
    v->xformedy = (pl_Float) (1<<15);
    v++;
  }
  f = o->Faces;
  for (i = 1; i <= div; i ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = o->Vertices + (i == div ? 1 : i + 1);
    f->Vertices[2] = o->Vertices + i;
    f->MappingU[0] = (pl_sInt32) o->Vertices[0].xformedx;
    f->MappingV[0] = (pl_sInt32) o->Vertices[0].xformedy;
    f->MappingU[1] = (pl_sInt32) o->Vertices[(i==div?1:i+1)].xformedx;
    f->MappingV[1] = (pl_sInt32) o->Vertices[(i==div?1:i+1)].xformedy;
    f->MappingU[2] = (pl_sInt32) o->Vertices[i].xformedx;
    f->MappingV[2] = (pl_sInt32) o->Vertices[i].xformedy;
    f->Material = m;
    f++;
  }
  if (cap) {
    if (div == 3) {
      f->Vertices[0] = o->Vertices + 1;
      f->Vertices[1] = o->Vertices + 2;
      f->Vertices[2] = o->Vertices + 3;
      f->MappingU[0] = (pl_sInt32) o->Vertices[1].xformedx;
      f->MappingV[0] = (pl_sInt32) o->Vertices[1].xformedy;
      f->MappingU[1] = (pl_sInt32) o->Vertices[2].xformedx;
      f->MappingV[1] = (pl_sInt32) o->Vertices[2].xformedy;
      f->MappingU[2] = (pl_sInt32) o->Vertices[3].xformedx;
      f->MappingV[2] = (pl_sInt32) o->Vertices[3].xformedy;
      f->Material = m;
      f++;
    } else {
      for (i = 1; i <= div; i ++) {
        f->Vertices[0] = o->Vertices + div + 1;
        f->Vertices[1] = o->Vertices + i;
        f->Vertices[2] = o->Vertices + (i==div ? 1 : i+1);
        f->MappingU[0] = (pl_sInt32) o->Vertices[div+1].xformedx;
        f->MappingV[0] = (pl_sInt32) o->Vertices[div+1].xformedy;
        f->MappingU[1] = (pl_sInt32) o->Vertices[i].xformedx;
        f->MappingV[1] = (pl_sInt32) o->Vertices[i].xformedy;
        f->MappingU[2] = (pl_sInt32) o->Vertices[i==div?1:i+1].xformedx;
        f->MappingV[2] = (pl_sInt32) o->Vertices[i==div?1:i+1].xformedy;
        f->Material = m;
        f++;
      }
    }
  }
  plObjCalcNormals(o);
  return (o);
}

static pl_uChar verts[6*6] = {
  0,4,1, 1,4,5, 0,1,2, 3,2,1, 2,3,6, 3,7,6,
  6,7,4, 4,7,5, 1,7,3, 7,1,5, 2,6,0, 4,0,6
};
static pl_uChar map[24*2*3] = {
  1,0, 1,1, 0,0, 0,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 1,1, 0,1, 1,0,
  0,0, 1,0, 0,1, 1,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 0,1, 1,0, 1,1,
  1,0, 0,1, 0,0, 0,1, 1,0, 1,1,
  1,0, 1,1, 0,0, 0,1, 0,0, 1,1
};


pl_Obj *plMakeBox(pl_Float w, pl_Float d, pl_Float h, pl_Mat *m) {
  pl_uChar *mm = map;
  pl_uChar *vv = verts;
  pl_Obj *o;
  pl_Vertex *v;
  pl_Face *f;
  pl_uInt x;
  o = plObjCreate(8,12);
  if (!o) return 0;
  v = o->Vertices;
  v->x = -w/2; v->y = h/2; v->z = d/2; v++;
  v->x = w/2; v->y = h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = -d/2; v++;
  f = o->Faces;
  for (x = 0; x < 12; x ++) {
    f->Vertices[0] = o->Vertices + *vv++;
    f->Vertices[1] = o->Vertices + *vv++;
    f->Vertices[2] = o->Vertices + *vv++;
    f->MappingU[0] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[0] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[1] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[1] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[2] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[2] = (pl_sInt32) ((double)*mm++ * 65535.0);
    f->Material = m;
    f++;
  }

  plObjCalcNormals(o);
  return (o);
}

pl_Obj *plMakePlane(pl_Float w, pl_Float d, pl_uInt res, pl_Mat *m) {
  pl_Obj *o;
  pl_Vertex *v;
  pl_Face *f;
  pl_uInt x, y;
  o = plObjCreate((res+1)*(res+1),res*res*2);
  if (!o) return 0;
  v = o->Vertices;
  for (y = 0; y <= res; y ++) {
    for (x = 0; x <= res; x ++) {
      v->y = 0;
      v->x = ((x*w)/res) - w/2;
      v->z = ((y*d)/res) - d/2;
      v++;
    }
  }
  f = o->Faces;
  for (y = 0; y < res; y ++) {
    for (x = 0; x < res; x ++) {
      f->Vertices[0] = o->Vertices + x+(y*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = (y<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[1] = (x<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
      f->Vertices[0] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = ((y+1)<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+1+((y+1)*(res+1));
      f->MappingU[1] = ((x+1)<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
    }
  }
  plObjCalcNormals(o);
  return (o);
}
