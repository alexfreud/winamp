/******************************************************************************
Plush Version 1.2
clip.c
3D Frustum Clipping
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

#define NUM_CLIP_PLANES 5

typedef struct
{
  pl_Vertex newVertices[8];
  double Shades[8];
  double MappingU[8];
  double MappingV[8];
  double eMappingU[8];
  double eMappingV[8];
} _clipInfo;


static _clipInfo m_cl[2];


static double m_clipPlanes[NUM_CLIP_PLANES][4];
static pl_Cam *m_cam;
static pl_sInt32 m_cx, m_cy;
static double m_fov;
static double m_adj_asp;

static void _FindNormal(double x2, double x3,
                        double y2, double y3,
                        double zv,
                        double *res);

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static pl_uInt _ClipToPlane(pl_uInt numVerts, double *plane);

void plClipSetFrustum(pl_Cam *cam) {
  m_adj_asp = 1.0 / cam->AspectRatio;
  m_fov = plMin(plMax(cam->Fov,1.0),179.0);
  m_fov = (1.0/tan(m_fov*(PL_PI/360.0)))*(double) (cam->ClipRight-cam->ClipLeft);
  m_cx = cam->CenterX<<20;
  m_cy = cam->CenterY<<20;
  m_cam = cam;
  memset(m_clipPlanes,0,sizeof(m_clipPlanes));

  /* Back */
  m_clipPlanes[0][2] = -1.0;
  m_clipPlanes[0][3] = -cam->ClipBack;

  /* Left */
  m_clipPlanes[1][3] = 0.00000001;
  if (cam->ClipLeft == cam->CenterX) {
    m_clipPlanes[1][0] = 1.0;
  }
  else _FindNormal(-100,-100,
                100, -100,
                m_fov*-100.0/(cam->ClipLeft-cam->CenterX),
                m_clipPlanes[1]);
  if (cam->ClipLeft > cam->CenterX) {
    m_clipPlanes[1][0] = -m_clipPlanes[1][0];
    m_clipPlanes[1][1] = -m_clipPlanes[1][1];
    m_clipPlanes[1][2] = -m_clipPlanes[1][2];
  }

  /* Right */
  m_clipPlanes[2][3] = 0.00000001;
  if (cam->ClipRight == cam->CenterX) {
    m_clipPlanes[2][0] = -1.0;
  }
  else _FindNormal(100,100,
                -100, 100,
                m_fov*100.0/(cam->ClipRight-cam->CenterX),
                m_clipPlanes[2]);
  if (cam->ClipRight < cam->CenterX) {
    m_clipPlanes[2][0] = -m_clipPlanes[2][0];
    m_clipPlanes[2][1] = -m_clipPlanes[2][1];
    m_clipPlanes[2][2] = -m_clipPlanes[2][2];
  }
  /* Top */
  m_clipPlanes[3][3] = 0.00000001;
  if (cam->ClipTop == cam->CenterY) {
    m_clipPlanes[3][1] = -1.0;
  } else _FindNormal(100, -100,
                100, 100,
                m_fov*m_adj_asp*100.0/(cam->CenterY-cam->ClipTop),
                m_clipPlanes[3]);
  if (cam->ClipTop > cam->CenterY) {
    m_clipPlanes[3][0] = -m_clipPlanes[3][0];
    m_clipPlanes[3][1] = -m_clipPlanes[3][1];
    m_clipPlanes[3][2] = -m_clipPlanes[3][2];
  }

  /* Bottom */
  m_clipPlanes[4][3] = 0.00000001;
  if (cam->ClipBottom == cam->CenterY) {
    m_clipPlanes[4][1] = 1.0;
  } else _FindNormal(-100, 100,
                -100, -100,
                m_fov*m_adj_asp*-100.0/(cam->CenterY-cam->ClipBottom),
                m_clipPlanes[4]);
  if (cam->ClipBottom < cam->CenterY) {
    m_clipPlanes[4][0] = -m_clipPlanes[4][0];
    m_clipPlanes[4][1] = -m_clipPlanes[4][1];
    m_clipPlanes[4][2] = -m_clipPlanes[4][2];
  }
}


void plClipRenderFace(pl_Face *face) {
  pl_uInt k, a, w, numVerts, q;
  double tmp, tmp2;
  pl_Face newface;

  for (a = 0; a < 3; a ++) {
    m_cl[0].newVertices[a] = *(face->Vertices[a]);
    m_cl[0].Shades[a] = face->Shades[a];
    m_cl[0].MappingU[a] = face->MappingU[a];
    m_cl[0].MappingV[a] = face->MappingV[a];
    m_cl[0].eMappingU[a] = face->eMappingU[a];
    m_cl[0].eMappingV[a] = face->eMappingV[a];
  }

  numVerts = 3;
  q = 0;
  a = (m_clipPlanes[0][3] < 0.0 ? 0 : 1);
  while (a < NUM_CLIP_PLANES && numVerts > 2)
  {
    numVerts = _ClipToPlane(numVerts, m_clipPlanes[a]);
    memcpy(&m_cl[0],&m_cl[1],sizeof(m_cl)/2);
    a++;
  }
  if (numVerts > 2) {
    memcpy(&newface,face,sizeof(pl_Face));
    for (k = 2; k < numVerts; k ++) {
      newface.fShade = plMax(0,plMin(face->fShade,1));
      for (a = 0; a < 3; a ++) {
        if (a == 0) w = 0;
        else w = a+(k-2);
        newface.Vertices[a] = m_cl[0].newVertices+w;
        newface.Shades[a] = (pl_Float) m_cl[0].Shades[w];
        newface.MappingU[a] = (pl_sInt32)m_cl[0].MappingU[w];
        newface.MappingV[a] = (pl_sInt32)m_cl[0].MappingV[w];
        newface.eMappingU[a] = (pl_sInt32)m_cl[0].eMappingU[w];
        newface.eMappingV[a] = (pl_sInt32)m_cl[0].eMappingV[w];
        newface.Scrz[a] = 1.0f/newface.Vertices[a]->xformedz;
        tmp2 = m_fov * newface.Scrz[a];
        tmp = tmp2*newface.Vertices[a]->xformedx;
        tmp2 *= newface.Vertices[a]->xformedy;
        newface.Scrx[a] = m_cx + ((pl_sInt32)((tmp*(float) (1<<20))));
        newface.Scry[a] = m_cy - ((pl_sInt32)((tmp2*m_adj_asp*(float) (1<<20))));
      }
      newface.Material->_PutFace(m_cam,&newface);
      plRender_TriStats[3] ++;
    }
    plRender_TriStats[2] ++;
  }
}

pl_sInt plClipNeeded(pl_Face *face) {
  double dr,dl,db,dt;
  double f;
  dr = (m_cam->ClipRight-m_cam->CenterX);
  dl = (m_cam->ClipLeft-m_cam->CenterX);
  db = (m_cam->ClipBottom-m_cam->CenterY);
  dt = (m_cam->ClipTop-m_cam->CenterY);
  f = m_fov*m_adj_asp;
  return ((m_cam->ClipBack <= 0.0 ||
           face->Vertices[0]->xformedz <= m_cam->ClipBack ||
           face->Vertices[1]->xformedz <= m_cam->ClipBack ||
           face->Vertices[2]->xformedz <= m_cam->ClipBack) &&
          (face->Vertices[0]->xformedz >= 0 ||
           face->Vertices[1]->xformedz >= 0 ||
           face->Vertices[2]->xformedz >= 0) &&
          (face->Vertices[0]->xformedx*m_fov<=dr*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*m_fov<=dr*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*m_fov<=dr*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedx*m_fov>=dl*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*m_fov>=dl*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*m_fov>=dl*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f<=db*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f<=db*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f<=db*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f>=dt*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f>=dt*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f>=dt*face->Vertices[2]->xformedz));
}



static void _FindNormal(double x2, double x3,double y2, double y3,
                        double zv, double *res) {
  res[0] = zv*(y2-y3);
  res[1] = zv*(x3-x2);
  res[2] = x2*y3 - y2*x3;
}

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static pl_uInt _ClipToPlane(pl_uInt numVerts, double *plane)
{
  pl_uInt i, nextvert, curin, nextin;
  double curdot, nextdot, scale;
  pl_uInt invert, outvert;
  invert = 0;
  outvert = 0;
  curdot = m_cl[0].newVertices[0].xformedx*plane[0] +
           m_cl[0].newVertices[0].xformedy*plane[1] +
           m_cl[0].newVertices[0].xformedz*plane[2];
  curin = (curdot >= plane[3]);

  for (i=0 ; i < numVerts; i++) {
    nextvert = (i + 1) % numVerts;
    if (curin) {
      m_cl[1].Shades[outvert] = m_cl[0].Shades[invert];
      m_cl[1].MappingU[outvert] = m_cl[0].MappingU[invert];
      m_cl[1].MappingV[outvert] = m_cl[0].MappingV[invert];
      m_cl[1].eMappingU[outvert] = m_cl[0].eMappingU[invert];
      m_cl[1].eMappingV[outvert] = m_cl[0].eMappingV[invert];
      m_cl[1].newVertices[outvert++] = m_cl[0].newVertices[invert];
    }
    nextdot = m_cl[0].newVertices[nextvert].xformedx*plane[0] +
              m_cl[0].newVertices[nextvert].xformedy*plane[1] +
              m_cl[0].newVertices[nextvert].xformedz*plane[2];
    nextin = (nextdot >= plane[3]);
    if (curin != nextin) {
      scale = (plane[3] - curdot) / (nextdot - curdot);
      m_cl[1].newVertices[outvert].xformedx = (pl_Float) (m_cl[0].newVertices[invert].xformedx +
           (m_cl[0].newVertices[nextvert].xformedx - m_cl[0].newVertices[invert].xformedx)
             * scale);
      m_cl[1].newVertices[outvert].xformedy = (pl_Float) (m_cl[0].newVertices[invert].xformedy +
           (m_cl[0].newVertices[nextvert].xformedy - m_cl[0].newVertices[invert].xformedy)
             * scale);
      m_cl[1].newVertices[outvert].xformedz = (pl_Float) (m_cl[0].newVertices[invert].xformedz +
           (m_cl[0].newVertices[nextvert].xformedz - m_cl[0].newVertices[invert].xformedz)
             * scale);
      m_cl[1].Shades[outvert] = m_cl[0].Shades[invert] +
                        (m_cl[0].Shades[nextvert] - m_cl[0].Shades[invert]) * scale;
      m_cl[1].MappingU[outvert] = m_cl[0].MappingU[invert] +
           (m_cl[0].MappingU[nextvert] - m_cl[0].MappingU[invert]) * scale;
      m_cl[1].MappingV[outvert] = m_cl[0].MappingV[invert] +
           (m_cl[0].MappingV[nextvert] - m_cl[0].MappingV[invert]) * scale;
      m_cl[1].eMappingU[outvert] = m_cl[0].eMappingU[invert] +
           (m_cl[0].eMappingU[nextvert] - m_cl[0].eMappingU[invert]) * scale;
      m_cl[1].eMappingV[outvert] = m_cl[0].eMappingV[invert] +
           (m_cl[0].eMappingV[nextvert] - m_cl[0].eMappingV[invert]) * scale;
      outvert++;
    }
    curdot = nextdot;
    curin = nextin;
    invert++;
  }
  return outvert;
}
