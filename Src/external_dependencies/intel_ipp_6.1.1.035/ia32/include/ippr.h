/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2006-2009 Intel Corporation. All Rights Reserved.
//
//                    Intel(R) Performance Primitives
//                  Realistic Rendering Library (ippRR)
//
*/
#if !defined( __IPPR_H__ ) || defined( _OWN_BLDPCS )
#define __IPPR_H__

#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif

#ifndef __IPPDEFS_H__
  #include "ippdefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( _OWN_BLDPCS )

typedef float           IppPoint2D_32f[2];
typedef float           IppPoint3D_32f[3];
typedef float           IppVector3D_32f[4];
typedef IppPoint3D_32f  IppBox3D_32f[2];
typedef IppPoint3D_32f  IppTriangle3D_32f[3];

typedef struct TriangleAccel IpprTriangleAccel;

typedef struct KDTreeNode{
    Ipp32s  flag_k_ofs;
    union _tree_data{
        Ipp32f  split;
        Ipp32s  items;
    }tree_data;
}IpprKDTreeNode;

typedef struct IntersectContext{
    IppBox3D_32f        *pBound;    /* pointer to bounding box for a whole object */
    IpprTriangleAccel   *pAccel;    /* pointer to triangle acceleration structure */
    IpprKDTreeNode      *pRootNode; /* pointer to KD-tree root node */
}IpprIntersectContext;

/* Tree building algorithm identifiers */
typedef enum {
    ippKDTBuildSimple    = 0x499d3dc2,  /* Simple building mode */
    ippKDTBuildPureSAH   = 0x2d07705b   /* SAH building mode */
}IpprKDTreeBuildAlg;

/* Context for simple building mode */
typedef struct SimpleBuilderContext{
    IpprKDTreeBuildAlg   Alg;           /* Must be equal to ippKDTBuildSimple constant */
    Ipp32s               MaxDepth;      /* Subdivision depth (with middle point subdivision) */
}IpprSmplBldContext;

/* Context for SAH building mode */
typedef struct PSAHBuilderContext{
    IpprKDTreeBuildAlg   Alg;           /* Must be equal to ippKDTBuildPureSAH constant */
    Ipp32s               MaxDepth;      /* Maximum tree subdivision depth (minimum - 0, maximum - 51) */
    Ipp32f               QoS;           /* Termination criteria modifier */
    Ipp32s               AvailMemory;   /* Maximum available memory in Mb */
    IppBox3D_32f        *Bounds;        /* Cut-off bounding box */
}IpprPSAHBldContext;

typedef enum {
    ippNormInd  = 3,
    ippTriInd   = 4
} IpprIndexType;


typedef enum _IpprSHType{
    ipprSHNormDirect=0, /* Normalized Spherical harmonic functions, direct computation */
    ipprSHNormRecurr    /* Normalized Spherical harmonic functions, recurrent computation */
}IpprSHType;

typedef struct rSHState IpprSHState;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ipprGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippRR library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ipprGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//           Acceleration Functions
//////////////////////////////////////////////////////////////////////////////// */

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ipprTriangleAccelInit
//  Purpose:
//    Initialize a IpprtTriangleAccel for future usage in ipprIntersect...
//  Input Arguments:
//    pVertexCoord    - pointer to the array of vertex coordinate.
//    pTrnglIndex     - pointer to the triangle's indexes.
//     cntTrngl       - the number of triangles.
// Input Arguments:
//    pTrnglAccel     - pointer to the structure IpprTriangleAccel
//  Returns:
//    ippStsNoErr        No error.
*/
IPPAPI(IppStatus, ipprTriangleAccelInit,(
       IpprTriangleAccel* pTrnglAccel,const Ipp32f* pVertexCoord,const Ipp32s* pTrnglIndex,int cntTrngl ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ipprTriangleAccelGetSize
//
//  Purpose:
//    Return size of IpprtTriangleAccel
//  Parameters:
//    pTrnglAccelSize - pointer to the resulting size of the structure
//                                  IpprtTriangleAccel
//  Returns:
//    ippStsNoErr        No error.
//    ippStsNullPtrErr   Indicates an error when pointer pTrnglAccelSize is NULL.
*/
IPPAPI(IppStatus, ipprTriangleAccelGetSize,(int* pTrnglAccelSize))

/* ///////////////////////////////////////////////////////////////////////////
  Name:
    ippiKDTreeBuildAlloc
  Purpose:
    Build the k-D tree for the set of triangles using one of predefined construction
    algorithms controlled by service parameters context.
  Parameters:
    pDstKDTree     - address of a pointer to the built tree;
    pSrcVert       - pointer to the scene element vertices array;
    pSrcTriInx     - pointer to the scene element indexed triangles array;
    SrcVertSize    - size of vertices array;
    SrcTriSize     - size of triangles array;
    pDstKDTreeSize - address of the built tree size;
    QoS            - fuzzy quality control parameter. takes values from 0.0 to 1.0;
    AlgType        - type of tree construction algorithm
  Returns:
    ippStsNoErr           No error.
    ippStsNullPtrErr      Indicates an error when one of the pointers is NULL.
    ippStsSizeErr         Wrong (negative) size of one of arrays.
    ippStsOutOfRangeErr   QoS is out of [0.0, 1.0] range.
    ippStsNoMemErr        Not enough memory for the tree construction algorithm.
    ippStsBadArgErr       Unknown algorithm type.
*/
IPPAPI(IppStatus, ipprKDTreeBuildAlloc,(
       IpprKDTreeNode     **pDstKDTree,
       const Ipp32f * const pSrcVert,
       const Ipp32s * const pSrcTriInx,
       Ipp32s               SrcVertSize,
       Ipp32s               SrcTriSize,
       Ipp32s              *pDstKDTreeSize,
       const void * const   pBldContext))

/* ///////////////////////////////////////////////////////////////////////////
  Name:
    ippiKDTreeFree
  Purpose:
    Frees memory allocated for the k-D tree during ippiKDTreeBuildAlloc.
  Parameters:
    pSrcKDTree     - a pointer to the k-D tree;
  Returns:
*/
IPPAPI(void, ipprKDTreeFree,(
       IpprKDTreeNode   *pSrcKDTree))


/* /////////////////////////////////////////////////////////////////////////////
//           Ray-scene Intersection Engine
////////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Names:             ipprIntersectMO_32f
//  Purpose:   Calculates intersection points of rays with triangles,the indexes those triangles,
//             the distances from origin points to intersection points.
//
// Input Arguments:
//       pOrigin    - array of pointers to a separate coordinates(x,y,z)of planes of the origin points..
//       pDirection - array of pointers to a separate coordinates(x,y,z)of planes of the ray's directions.
//         pContext - pointer to the intersection's context.
//       blockSize  - size of rays' block.
// Input/Output Arguments:
//            pDist - pointer to the distance from origin to intersection point. Else it is input value.
//                    As input pDist[i] should be 0.f if you don't want to process this ray.
// Output Arguments:
//            pHit  - pointer to the local surface parameters( u, v )at hit point in case of intersection was found.
//          pTrngl  - pointer to the Triangle index in case of intersection was found. Else it is -1.
//  Returns:
//  ippStsNoErr      No errors
//  ippStsNoMemErr   The node stack is overfilled.
*/

IPPAPI(IppStatus,ipprIntersectMO_32f,(
       const Ipp32f* const       pOrigin[3],
       const Ipp32f* const    pDirection[3],
       Ipp32f*                        pDist,
       Ipp32f*                      pHit[2],
       Ipp32s*                       pTrngl,
       const IpprIntersectContext* pContext,
       IppiSize blockSize
       ))
/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprIntersectEyeSO_32f
//  Purpose:   Calculates intersection points of rays with triangles,the indexes those triangles,
//             the distances from origin points to intersection points.
//
// Input Arguments:
//       originEye  - origin point.All rays have a single origin.
//       pDirection - array of pointers to a separate coordinates(x,y,z)of planes of the ray's directions.
//         pContext - pointer to the intersection's context.
//       blockSize  - size of rays' block.
// Output Arguments:
//            pDist - pointer to the distance from origin to intersection point. Else it is IPP_MAXABS_32F.
//            pHit  - pointer to the local surface parameters( u, v )at hit point in case of intersection was found.
//          pTrngl  - pointer to the Triangle index in case of intersection was found. Else it is -1.
//  Returns:
//  ippStsNoErr      No errors
//  ippStsNoMemErr   The node stack is overfilled.
*/
IPPAPI(IppStatus,ipprIntersectEyeSO_32f,(
       IppPoint3D_32f             originEye,
       const Ipp32f* const    pDirection[3],
       Ipp32f*                        pDist,
       Ipp32f*                      pHit[2],
       Ipp32s*                       pTrngl,
       const IpprIntersectContext* pContext,
       IppiSize blockSize
       ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:             ipprIntersectMultipleSO_32f
//  Purpose:   Calculates intersection points of rays with triangles,the indexes those triangles,
//             the distances from origin points to intersection points.
//
//Input parameters:
//  originEye        origin point.All rays have a single origin.
//  pDirection       2D array of pointers to the vectors of directions.
//  pContext         Pointer to the intersection context.
//  blockVolume      blockVolume.width * blockVolume.height is total number of the rays.
//                   blockVolume.depth - the specified number of the scene triangles.
//Input Output parameters:
//pDist         Pointer to the 3D array of distances between the hit point and origin of  the rays.
//Output parameters:
//pHit        3D array of pointers to the local surface parameters (u, v) at the hit
//              point if the intersection is found.
//pTrngl    Pointer to the 3D array of triangle indexes if the intersection is found. If not it is set to-1.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprIntersectMultipleSO_32f,(
       IppPoint3D_32f            originEye,
       const Ipp32f* const   pDirection[3],
       Ipp32f*                   pDistance,
       Ipp32f*                     pHit[2],
       Ipp32s*                      pTrngl,
       IpprVolume              blockVolume,
       const IpprIntersectContext* pContext
       ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprIntersectAnySO_32f
//  Purpose:   performs occlusion tests for block of rays with single origin.
//
// Input Arguments:
//       originEye  - origin point.All rays have a single origin.
//       pDirection - array of pointers to a separate coordinates(x,y,z)of planes of the ray's directions.
//         pContext - pointer to the intersection's context.
//       blockSize  - size of rays' block.
// Input/Output Arguments:
//            pMask - pointer to the array of the mask. If output pMask[i][j] = 0, occlusion test for this ray is true.
//                    Else it is input value.
//                    As input pMask[i][j] should be 0 if you don't want to process this ray.Else it should be -1.
//          pTrngl  - pointer to the Triangle index in case of intersection was found. Else it is -1.
//  Returns:
//  ippStsNoErr      No errors
//  ippStsNoMemErr   The node stack is overfilled.
*/
IPPAPI(IppStatus,ipprIntersectAnySO_32f,(
       IppPoint3D_32f             originEye,
       const Ipp32f* const    pDirection[3],
       Ipp32s*                    pOccluder,
       Ipp32s*                        pMask,
       IppiSize                   blockSize,
       const IpprIntersectContext* pContext
       ))

/* /////////////////////////////////////////////////////////////////////////////
//           Shaders Support Functions
//////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:     ipprMul_32f_C1P3IM
//  Purpose: Purpose: multiplies each element of three vectors of the accumulator (pSrcDst)
//           for which the corresponding element of a vector of a mask more or is equal to zero,
//           by an element of an source vector.
//  Name:     ipprMul_32f_IM
//  Purpose: Multiplies an element of the accumulator (pSrcDst)
//           for which the corresponding element of a vector of a mask more or is equal to zero,
//           by an element of an source vector.
// Input Arguments:
//           pSrc -  pointer to the first source vector
//          pMask -  pointer to the first mask's vector
//          len                  length of the vectors
// Input/Output Arguments:
//          pSrcDst - pointer to the source/destination (accumulator) vectors.
// Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprMul_32f_C1P3IM,( const Ipp32f* pSrc, const Ipp32s* pMask,
       Ipp32f* pSrcDst[3], int len ))

IPPAPI(IppStatus,ipprMul_32f_C1IM,( const Ipp32f* pSrc, const Ipp32s* pMask,
       Ipp32f* pSrcDst, int len ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:     ipprAddMulMul_32f_AC1P3IM
//  Purpose:  multiplies elements of two triplex source vectors and adds product
//            to triplex of the accumulator vectors ???
//  Input Arguments:
//          point   - source point.
//          pSrc0   - pointer to the first source vector
//          pSrc1   - pointer to the second source vector
//          pMask   - pointer to the mask's vector
//          len     - length of the vectors
// Output Arguments:
//          pSrcDst - pointer to the source/destination (accumulator) vector.
//  Notes:
//         pSrcDst[0][n] = pSrcDst[0][n] + pSrc1[n] * pSrc2[n] * point[0], n=0,1,2,..len-1.
//         pSrcDst[1][n] = pSrcDst[1][n] + pSrc1[n] * pSrc2[n] * point[1],
//         pSrcDst[2][n] = pSrcDst[2][n] + pSrc1[n] * pSrc2[n] * point[2],
//         E.g for Lambertian cosine low.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprAddMulMul_32f_AC1P3IM,(
       IppPoint3D_32f point,
       const Ipp32f*  pSrc0,
       const Ipp32f* const pSrc1[3],
       const Ipp32s*  pMask,
       Ipp32f*   pSrcDst[3],
       int len
       ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:     ipprDiv_32f_C1IM
//  Purpose:   divides an element of the accumulator (pSrcDst) for which the corresponding
//             element of a vector of a mask more or is equal to zero, into an element of an source vector.
//  Input Arguments:
//           pSrc - pointer to the divisor source vector
//          pMask - pointer to the mask vector.
//            len - vector's length, number of items.
//  Input-Output Argument:
//        pSrcDst - pointer to the source/destination (accumulator) vector.
//
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprDiv_32f_C1IM,(const Ipp32f* pSrc, const Ipp32s* pMask, Ipp32f* pSrcDst, int len ))
/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//      ipprDot_32f_P3C1M
//  Purpose:
//      calculates dot product of the incident ray directions and normales of surface.
//  Input Arguments:
// pDirection - pointer to array of pointers to a separate coordinates(x,y,z)of planes of the ray's directions.
//      pDist - pointer to the IpprPointsOnRays_SO structure
//  pSurfNorm - pointer to the surface's normals.
//  Output arguments:
// pSurfDotIn - pointer to the dot product.
//  Returns:
//      ippStsNoErr   No errors
*/
IPPAPI(IppStatus, ipprDot_32f_P3C1M,( const Ipp32f* const pSrc0[3],
       const Ipp32f* const pSrc1[3],const Ipp32s* pMask, Ipp32f* pDot, int len ))
IPPAPI(IppStatus, ipprDotChangeNorm_32f_IM,( const Ipp32f* const pSrc[3],
       const Ipp32s* pMask, Ipp32f* pSrcDst[3],Ipp32f* pDot,int len ))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//      ipprDot_32f_M
*/
IPPAPI(IppStatus, ipprDistAttenuationSO_32f_M,( IppPoint3D_32f point, const Ipp32f* const  pSurfHit[3],
       const Ipp32s* pMask, Ipp32f* pDist, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//       Rays' casting
//////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:     ipprCastEye_32f
//  Purpose:  to calculate the primary ray's vectors.
// Input Arguments:
//     imPlaneOrg - the coordinate of origin the projection's plane.
//        dW      - a step along width  of the projection's plane.
//        dH      - a step along height of the projection's plane.
//        wB      - the number of block along width of Image.
//        hB      - the number of block along height of Image.
//        cBlock  - total number of rays in the block
//     blockSize  - total number of the rays in the current block.
// Input-Output Argument:
//     pDirection - pointer to the destination vector. It is not normalised.
//  Returns:
//  ippStsNoErr      No errors

*/

IPPAPI(IppStatus,ipprCastEye_32f,(
       IppPoint3D_32f imPlaneOrg,
       IppPoint3D_32f dW,
       IppPoint3D_32f dH,
       int wB,int hB,
       IppiSize cBlock,
       Ipp32f* pDirection[3],
       IppiSize blockSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprCastShadowSO_32f
//  Purpose:   calculates block of shadow rays.  for which the corresponding
//             element of a vector of a pMask more or is equal to zero.
// Input Arguments:
//    pOrigin       - pointer to the origin point.
//   pSurfDotIn     - pointer to the vector of dot products of incident rays and normals
//                    at intersections point.
//   pSurfNorm      - pointer  to array of pointers to a separate coordinates(x,y,z)of planes
//                    of normals at intersections point.
//    pSurfHit      - pointer to array of pointers to a separate coordinates(x,y,z)of planes of the intersection points.
//            pMask - pointer to the mask vector.
//  Output Arguments:
//     pDirection   - pointer to the destination vector. Shouldn't be normalised.
//     pDist        - . Here it is can be 0.f or 1.f
//     pDotRay      - pointer to the vector of dot products of shadow rays and normals.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprCastShadowSO_32f,(
       IppPoint3D_32f           pOrigin,
       const Ipp32f*         pSurfDotIn,
       const Ipp32f* const pSurfNorm[3],
       const Ipp32f* const  pSurfHit[3],
       Ipp32s*                    pMask,
       Ipp32f*                  pDotRay,
       Ipp32f*            pDirection[3],
       int                          len
))
/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprCastReflectionRay_32f
//  Purpose:   calculates array of reflected rays, for which the corresponding
//             element of a vector of a mask more or is equal to zero.
//
// Input Arguments:
//    pIncident   - pointer to the array of vectors of incident rays.
//          pMask - pointer to the mask vector.
//   pSurfNorm    - pointer  to array of pointers to a separate coordinates(x,y,z)of planes
//                    of normals at intersections point.
//  Output Arguments:
//       pReflect - pointer to the array of rflected vectors.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus,ipprCastReflectionRay_32f,(
       const Ipp32f* const pInc[3],
       const Ipp32s*        pMask,
       const Ipp32f* const pSurfNorm[3],
       Ipp32f*        pReflect[3],
       int len ))

/* /////////////////////////////////////////////////////////////////////////////
//       Surface properties ( pSurfHit, pSurfNorm, pSurfDotIn )
//////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:
//      ipprHitPoint3DS0_32f/ipprHitPoint3DM0_32f
//  Purpose:
//      calculates explicit intersection coordinates in world coordinate system for a block
//      of rays from single/multiple origin.
//  Input Arguments:
//       pDist - generalized distance from origin to intersection point.
//   originEye - origin point. All rays have a single origin.
//     pOrigin - pointer to array of pointers to a separate coordinates(x,y,z)of planes of the origin points.
//  pDirection - pointer to array of pointers to a separate coordinates(x,y,z)of planes of the ray's directions.
//  blockSize  - size of rays' block.
//  Output arguments:
//    pSurfHit - pointer to array of pointers to a separate coordinates(x,y,z)of planes of the intersection points.
//  Returns:
//      ippStsNoErr   No errors
*/
IPPAPI(IppStatus, ipprHitPoint3DEpsS0_32f_M,(
       const IppPoint3D_32f    originEye,
       const Ipp32f* const pDirection[3],
       const Ipp32f*               pDist,
       const Ipp32s*               pMask,
       Ipp32f*               pSurfHit[3],
       int                           len,
       Ipp32f                        eps
       ))
IPPAPI(IppStatus, ipprHitPoint3DEpsM0_32f_M,(
       const Ipp32f* const       pOrigin[3],
       const Ipp32f* const pDirection[3],
       const Ipp32f*               pDist,
       const Ipp32s*               pMask,
       Ipp32f*               pSurfHit[3],
       int                           len,
       Ipp32f                        eps
       ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprSurfTriangleNormal_32f
//  Purpose:   calculates the surface's normals from triangles' normals.
//
// Input Arguments:
//    pTrnglNorm    - pointer to the triangles' normal. Interlived
//    pTrngl        - pointer to triangles' indexes
//    pHit          - pointer to the local surface parameters( u, v )at hit point in case of intersection was found.
//    blockSize     - size of rays' block.
//  Output Arguments:
//    pSurfNorm     - pointer to the surface's normals.
//    sameTri ???
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus, ipprSurfFlatNormal_32f,(
       const Ipp32f*  pTrnglNorm,
       const Ipp32s*      pTrngl,
       Ipp32f*      pSurfNorm[3],
       int                   len
       ))
IPPAPI(IppStatus, ipprSurfSmoothNormal_32f,(
       const Ipp32f* pVertNorm,
       const Ipp32s* pIndexNorm,
       const Ipp32s* pTrngl,
       const Ipp32f* const pHit[2],
       Ipp32f* pSurfNorm[3], int len, IpprIndexType ippInd
       ))

/* /////////////////////////////////////////////////////////////////////////////
//           Helper Functions
//////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ipprSetBoundBox_32f
//  Purpose:   Calculates an axis aligned bounding box for the object.
//
// Input Arguments:
//    pVertCoor     - pointer to the coordinates of triangle's vertexes.
//    lenTri        - the number of triangles in the mesh.
// Output Arguments:
//             pBound  - pointer to the axis aligned bounding box of current object.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus, ipprSetBoundBox_32f,(
       const Ipp32f* pVertCoor,
       int             lenTri,
       IppBox3D_32f*   pBound
    ))
/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ipprTriangleNormal_32f
//  Purpose:   calculates triangles' normals from object.
//
// Input Arguments:
//    pTrnglCoor      - pointer to the coordinates of triangle's vertexes.
//    pTrnglIndex     - pointer to the triangle's indexes.
//    lenTri          - the number of triangles in the mesh.
//  Output Arguments:
//    pTrnglNorm      - pointer to the triangles' normals.
//  Returns:
//  ippStsNoErr      No errors
*/
IPPAPI(IppStatus, ipprTriangleNormal_32f,(
       const Ipp32f* pTrnglCoor,
       const int*   pTrnglIndex,
       Ipp32f*       pTrnglNorm,
       int             lenTrngl
    ))


/* /////////////////////////////////////////////////////////////////////////////
//           3D Geometric Transform Functions
//////////////////////////////////////////////////////////////////////////////// */

/*
//  Name:               ipprResizeGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    srcVOI            region of interest of source volume
//    dstVOI            region of interest of destination volume
//    nChannel          number of channels
//    interpolation     type of interpolation to perform for resizing the input volume:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  trilinear interpolation
//                        IPPI_INTER_CUBIC   tricubic polynomial interpolation
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE      B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM   Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03       special filter with parameters (1/2, 3/10)
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSize == NULL
//    ippStsSizeErr           width or height or depth of volumes is less or equal zero
//    ippStsNumChannelsErr    number of channels is not one
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_BSPLINE) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_CATMULLROM) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_B05C03)
*/

IPPAPI(IppStatus, ipprResizeGetBufSize, (
    IpprCuboid srcVOI, IpprCuboid dstVOI, int nChannel, int interpolation, int* pSize))

/*
//  Name:               ipprResize_<mode>
//  Purpose:            Performs RESIZE transform of the source volume
//                      by xFactor, yFactor, zFactor and xShift, yShift, zShift
//                            |X'|   |xFactor    0       0   |   |X|   |xShift|
//                            |Y'| = |        yFactor    0   | * |Y| + |yShift|
//                            |Z'|   |   0       0    zFactor|   |Z|   |zShift|
//  Parameters:
//    pSrc              pointer to source volume data (8u_C1V, 16u_C1V, 32f_C1V modes)
//                      or array of pointers to planes in source volume data
//    srcVolume         size of source volume
//    srcStep           step in every plane of source volume
//    srcPlaneStep      step between planes of source volume (8u_C1V, 16u_C1V, 32f_C1V modes)
//    srcVOI            volume of interest of source volume
//    pDst              pointer to destination volume data (8u_C1V and 16u_C1V modes)
//                      or array of pointers to planes in destination volume data
//    dstStep           step in every plane of destination volume
//    dstPlaneStep      step between planes of destination volume (8u_C1V, 16u_C1V, 32f_C1V modes)
//    dstVOI            volume of interest of destination volume
//    xFactor           they specify fraction of resizing in X direction
//    yFactor           they specify fraction of resizing in Y direction
//    zFactor           they specify fraction of resizing in Z direction
//    xShift            they specify shifts of resizing in X direction
//    yShift            they specify shifts of resizing in Y direction
//    zShift            they specify shifts of resizing in Z direction
//    interpolation     type of interpolation to perform for resizing the input volume:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  trilinear interpolation
//                        IPPI_INTER_CUBIC   tricubic polynomial interpolation
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE      B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM   Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03       special filter with parameters (1/2, 3/10)
//    pBuffer           pointer to work buffer
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL or pBuffer == NULL
//    ippStsSizeErr           width or height or depth of volumes is less or equal zero
//    ippStsWrongIntersectVOI VOI hasn't an intersection with the source or destination volume
//    ippStsResizeFactorErr   xFactor or yFactor or zFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_BSPLINE) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_CATMULLROM) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_B05C03)
//  Notes:
//    <mode> are 8u_C1V or 16u_C1V or 32f_C1V or 8u_C1PV or 16u_C1PV or 32f_C1PV
*/

IPPAPI(IppStatus, ipprResize_8u_C1V, (
    const Ipp8u* pSrc, IpprVolume srcVolume, int srcStep, int srcPlaneStep, IpprCuboid srcVOI,
    Ipp8u* pDst, int dstStep, int dstPlaneStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprResize_16u_C1V, (
    const Ipp16u* pSrc, IpprVolume srcVolume, int srcStep, int srcPlaneStep, IpprCuboid srcVOI,
    Ipp16u* pDst, int dstStep, int dstPlaneStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprResize_32f_C1V, (
    const Ipp32f* pSrc, IpprVolume srcVolume, int srcStep, int srcPlaneStep, IpprCuboid srcVOI,
    Ipp32f* pDst, int dstStep, int dstPlaneStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprResize_8u_C1PV, (
    const Ipp8u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp8u* const pDst[], int dstStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprResize_16u_C1PV, (
    const Ipp16u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp16u* const pDst[], int dstStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprResize_32f_C1PV, (
    const Ipp32f* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp32f* const pDst[], int dstStep, IpprCuboid dstVOI,
    double xFactor, double yFactor, double zFactor, double xShift, double yShift, double zShift,
    int interpolation, Ipp8u* pBuffer))


/*
//  Name:               ipprWarpAffineGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    srcVOI            region of interest of source volume
//    dstVOI            region of interest of destination volume
//    nChannel          number of channels
//    interpolation     type of interpolation to perform for resizing the input volume:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  trilinear interpolation
//                        IPPI_INTER_CUBIC   tricubic polynomial interpolation
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE      B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM   Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03       special filter with parameters (1/2, 3/10)
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSize == NULL
//    ippStsSizeErr           size of source or destination volumes is less or equal zero
//    ippStsNumChannelsErr    number of channels is not one
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_BSPLINE) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_CATMULLROM) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_B05C03)
*/

IPPAPI(IppStatus, ipprWarpAffineGetBufSize, (
    IpprCuboid srcVOI, IpprCuboid dstVOI, int nChannel, int interpolation, int* pSize))

/*
//  Names:              ipprWarpAffine_<mode>
//  Purpose:            Performs AFFINE transform of the source volume by matrix a[3][4]
//                            |X'|   |a00 a01 a02|   |X|   |a03|
//                            |Y'| = |a10 a11 a12| * |Y| + |a13|
//                            |Z'|   |a20 a21 a22|   |Z|   |a23|
//  Parameters:
//    pSrc              array of pointers to planes in source volume data
//    srcVolume         size of source volume
//    srcStep           step in every plane of source volume
//    srcVOI            volume of interest of source volume
//    pDst              array of pointers to planes in destination volume data
//    dstStep           step in every plane of destination volume
//    dstVOI            volume of interest of destination volume
//    coeffs            affine transform matrix
//    interpolation     type of interpolation to perform for affine transform the input volume:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  trilinear interpolation
//                        IPPI_INTER_CUBIC   tricubic polynomial interpolation
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE      B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM   Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03       special filter with parameters (1/2, 3/10)
//    pBuffer           pointer to work buffer
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL or pBuffer == NULL
//    ippStsSizeErr           width or height or depth of source volume is less or equal zero
//    ippStsWrongIntersectVOI VOI hasn't an intersection with the source or destination volume
//    ippStsCoeffErr          determinant of the transform matrix Aij is equal to zero
//    ippStsInterpolationErr  interpolation has an illegal value
//  Notes:
//    <mode> are 8u_C1PV or 16u_C1PV or 32f_C1PV
*/

IPPAPI(IppStatus, ipprWarpAffine_8u_C1PV, (
    const Ipp8u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp8u* const pDst[], int dstStep, IpprCuboid dstVOI,
    const double coeffs[3][4], int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprWarpAffine_16u_C1PV, (
    const Ipp16u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp16u* const pDst[], int dstStep, IpprCuboid dstVOI,
    const double coeffs[3][4], int interpolation, Ipp8u* pBuffer))

IPPAPI(IppStatus, ipprWarpAffine_32f_C1PV, (
    const Ipp32f* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    Ipp32f* const pDst[], int dstStep, IpprCuboid dstVOI,
    const double coeffs[3][4], int interpolation, Ipp8u* pBuffer))

/*
//  Names:              ipprRemap_<mode>
//  Purpose:            Performs REMAP TRANSFORM of the source volume by remapping
//                        dst[i,j,k] = src[xMap[i,j,k], yMap[i,j,k], zMap[i,j,k]]
//  Parameters:
//    pSrc              array of pointers to planes in source volume data
//    srcVolume         size of source volume
//    srcStep           step in every plane of source volume
//    srcVOI            volume of interest of source volume
//    pxMap             array of pointers to images with X coordinates of map
//    pyMap             array of pointers to images with Y coordinates of map
//    pzMap             array of pointers to images with Z coordinates of map
//    mapStep           step in every plane of each map volumes
//    pDst              array of pointers to planes in destination volume data
//    dstStep           step in every plane of destination volume
//    dstVolume         size of destination volume
//    interpolation     type of interpolation to perform for resizing the input volume:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  trilinear interpolation
//                        IPPI_INTER_CUBIC   tricubic polynomial interpolation
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE    B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03     special filter with parameters (1/2, 3/10)
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL or
//                            pxMap == NULL or pyMap == NULL or pzMap == NULL
//    ippStsSizeErr           width or height or depth of volumes is less or equal zero
//    ippStsInterpolationErr  interpolation has an illegal value
//    ippStsWrongIntersectVOI srcVOI hasn't intersection with the source volume, no operation
//  Notes:
//    <mode> are 8u_C1PV or 16u_C1PV or 32f_C1PV
*/

IPPAPI(IppStatus, ipprRemap_8u_C1PV, (
    const Ipp8u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    const Ipp32f* const pxMap[], const Ipp32f* const pyMap[], const Ipp32f* const pzMap[], int mapStep,
    Ipp8u* const pDst[], int dstStep, IpprVolume dstVolume, int interpolation))

IPPAPI(IppStatus, ipprRemap_16u_C1PV, (
    const Ipp16u* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    const Ipp32f* const pxMap[], const Ipp32f* const pyMap[], const Ipp32f* const pzMap[], int mapStep,
    Ipp16u* const pDst[], int dstStep, IpprVolume dstVolume, int interpolation))

IPPAPI(IppStatus, ipprRemap_32f_C1PV, (
    const Ipp32f* const pSrc[], IpprVolume srcVolume, int srcStep, IpprCuboid srcVOI,
    const Ipp32f* const pxMap[], const Ipp32f* const pyMap[], const Ipp32f* const pzMap[], int mapStep,
    Ipp32f* const pDst[], int dstStep, IpprVolume dstVolume, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//           3D General Linear Filters
//////////////////////////////////////////////////////////////////////////////// */

/*
//  Name:               ipprFilterGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    dstVolume         size of the volume
//    kernelVolume      size of the kernel volume
//    nChannel          number of channels
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr           no errors
//    ippStsNullPtrErr      pSize == NULL
//    ippStsSizeErr         width or height or depth of volumes is less or equal zero
//    ippStsNumChannelsErr  number of channels is not one
*/

IPPAPI(IppStatus, ipprFilterGetBufSize, (
    IpprVolume dstVolume, IpprVolume kernelVolume, int nChannel, int* pSize))

/*
//  Name:               ipprFilter_16s_C1PV
//  Purpose:            Filters a volume using a general integer cuboidal kernel
//  Parameters:
//    pSrc              array of pointers to planes in source volume data
//    srcStep           step in every plane of source volume
//    pDst              array of pointers to planes in destination volume data
//    dstStep           step in every plane of destination volume
//    dstVolume         size of the processed volume
//    pKernel           pointer to the kernel values
//    kernelVolume      size of the kernel volume
//    anchor            anchor 3d-cell specifying the cuboidal kernel alignment
//                      with respect to the position of the input voxel
//    divisor           the integer value by which the computed result is divided
//    pBuffer           pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  one of the pointers is NULL
//    ippStsSizeErr     width or height or depth of volumes is less or equal zero
//    ippStsDivisorErr  divisor value is zero, function execution is interrupted
*/

IPPAPI(IppStatus, ipprFilter_16s_C1PV, (
    const Ipp16s* const pSrc[], int srcStep,
    const Ipp16s* pDst[], int dstStep, IpprVolume dstVolume,
    const Ipp32s* pKernel, IpprVolume kernelVolume, IpprPoint anchor, int divisor,
    Ipp8u* pBuffer))



/* /////////////////////////////////////////////////////////////////////////////
//                  Spherical Harmonics lighting function 
///////////////////////////////////////////////////////////////////////////// */
/*
//  Name:     ipprSHGetSize_32f
//  Purpose:  Acquires the size of state structure used for Spherical Harmonic computations
//            Returns size of a memory buffer which is required for initialization of the state structure
//            used in various algorithms related to Spherical Harmonic functions. 
//  Parameters:
//      maxL    the maximal order for Spherical Harmonic to support after initialization
//              of SH structure by ipprSHInit function with given order and type
//     shType   the type of algorithm used for SH calculation: ippSHNormDirect or ippSHNormRecurr.
//     pSize    the size of memory in bytes required for this IppSHState instance to be initialized correctly
//
//
//  Returns:
//     ippStsNoErr      Indicates no error.
//     ippStsSizeErr    Indicates an error when maxL is greater then 15.
//     ippStsNullPtrErr Indicates an error when the pSize pointer is NULL.
//     ippStsRangeErr   Indicates an error when shType is not equal to  ippSHNormDirect or ippSHNormRecurr.
*/

IPPAPI(IppStatus, ipprSHGetSize_32f,(Ipp32u maxL, IpprSHType shType, Ipp32u *pSize))

/*
//  Name:     ipprSHInit_32f
//  Purpose:  Initializes the state structure used for Spherical Harmonic computations
//            in the buffer which must be provided of size not less than acquired by the ipprSHGetSize function. 
//  Parameters:
//    pSHState    pointer to the memory buffer used for pSHState structure initialization 
//    maxL        the maximal order of Spherical Harmonics to support after initialization
//                with using pSHState structure.
//    shType      the type of algorithm to use for SH calculation with using this state structure:
//                ippSHNormDirect or ippSHNormRecurr.
//  Returns:
//    ippStsNoErr      Indicates no error.
//    ippStsNullPtrErr Indicates an error when the pSHState pointer is NULL.
//    ippStsSizeErr    Indicates an error when L is greater then 15.
//    ippStsRangeErr   Indicates an error when shType is not equal to  ippSHNormDirect or ippSHNormRecurr.
*/

IPPAPI(IppStatus, ipprSHInit_32f,(IpprSHState *pSHState, Ipp32u maxL, IpprSHType shType))

/*
//  Name:           ipprSH_32f, ipprSHBand_32f
//  Purpose:        Compute the Spherical Harmonics
//  Parameters:
//    pX, pY, pZ    pointers to the source vectors of length N 
//                  which represents the points of a unit sphere given in Cartesians coordinates
//    N             the number of Cartesians points, i.e. the length of input vectors 
//    pDstYlm       pointer to the destination vector to store SH values computed at given points
//                  for orders up to order L, of size  (L+1)*(L+1)
//    pDstBandYlm   pointer to the destination vector to store SH values computed at given points
//                  for order L only, of size (2*L+1)
//    L             the order up to which to compute SH values, 
//                  must not be greater then maximal order used in the function ipprSHInit call.
//    pSHState      pointer to the SH state structure initialized with maximal order not less then L   
//  Returns:
//    ippStsNoErr       Indicates no error.
//    ippStsNullPtrErr  Indicates an error when the pX, pY, pZ, pDstYlm or pDstBandYlm pointer is NULL.
//    ippStsRangeErr    Indicates an error when L is greater then maximal order used for SH state structure
//                      initialization by the function ipprSHInit.
//    ippStsSizeErr     Indicates an error when N is equal to zero
*/
IPPAPI(IppStatus, ipprSH_32f,(const Ipp32f *pX, const Ipp32f *pY, const Ipp32f *pZ, Ipp32u N, 
                     Ipp32f *pDstYlm, Ipp32u L, IpprSHState *pSHState))
IPPAPI(IppStatus, ipprSHBand_32f,(const Ipp32f *pX, const Ipp32f *pY, const Ipp32f *pZ, Ipp32u N,
                         Ipp32f *pDstBandYlm, Ipp32u L))

/*
//  Name:           ipprSHTFwd_32f_C1I, ipprSHTFwd_32f_C3P3I, ipprSHTFwd_32f_P3I
//  Purpose:        The functions perform projecting of a given function defined on unit sphere into SH basis
//                  i.e. computes SH transform
//                  C1I: single value function (gray)
//                  C3I: RGB color function (x,y,z) -> (R,G,B)
//  Parameters:
//    pX, pY, pZ    pointers to the source vectors of length N 
//                  which represents the points of a unit sphere given in Cartesians coordinates
//    pDst          Pointer the input vector of values of a color function on a unit sphere
//                  Points to the source image for pixel-order data or 
//                  to an array of pointers to separate source color planes for plane-order data.
//    N             the number of Cartesians points, i.e. the length of input vectors 
//    pSrcDstClm    pointer to the destination vector or arrays of vectors 
//                  storing running values of SHT coefficients. Do not forget to zero them proir to first call.  
//                  of length (L+1)*(L+1) 
//    L             the order up to which to compute SH transform, 
//                  must not be greater then maximal order used in the function ipprSHInit call.
//    pSHState      pointer to the SH state structure initialized with maximal order not less then L   
//  Returns:
//    ippStsNoErr      Indicates no error.
//    ippStsNullPtrErr Indicates an error when the pX, pY, pZ, pSrc, pSrcDstSHT or pSHState pointer is NULL.
//    ippStsSizeErr    Indicates an error when N is equal to zero
//    ippStsRangeErr   Indicates an error when L is greater then maximal order
//                     used in SH state structure initialization by the function ipprSHInit.
//    
*/

IPPAPI(IppStatus, ipprSHTFwd_32f_C1I,(const Ipp32f *pX, const Ipp32f *pY, const Ipp32f *pZ, const Ipp32f *pSrc, 
                        Ipp32u N, Ipp32f *pSrcDstClm, Ipp32u L, IpprSHState *pSHState))
IPPAPI(IppStatus, ipprSHTFwd_32f_C3P3I,(const Ipp32f *pX, const Ipp32f *pY, const Ipp32f *pZ, const Ipp32f *pSrc, 
                        Ipp32u N, Ipp32f *pSrcDstClm[3], Ipp32u L, IpprSHState *pSHState))
IPPAPI(IppStatus, ipprSHTFwd_32f_P3I,(const Ipp32f *pX, const Ipp32f *pY, const Ipp32f *pZ, const Ipp32f *pSrc[3], 
                        Ipp32u N, Ipp32f *pSrcDstClm[3], Ipp32u L, IpprSHState *pSHState))

/*
//  Name:           ipprSHTInv_32f_C1,ipprSHTInv_32f_P3
//  Purpose:        The functions reconstruct a function defined on unit sphere by its SHT coefficients
//                  i.e. computes ISHT transform
//                  C1: single value function (gray)
//                  C3: RGB color function (x,y,z) -> (R,G,B)
//
//  Parameters:
//    pSrcClm       the input vector or arrays of vectors of the pre-computed SHT coefficients of the length (L+1)*(L+1)  
//    L             the order of SHT, must not be greater then maximal order used in the ipprSHInit call.
//    pX, pY, pZ    pointers to the source vectors which represents the points of a unit sphere 
//                  given in Cartesians coordinates 
//    pDst          Pointer the output vector of values of reconstructed color function on a unit sphere
//                  Points to the source image for pixel-order data or 
//                  to an array of pointers to separate source color planes for plane-order data.
//    N             the number of Cartesians points, i.e. the length of input pX, pY, pZ and number of dst pixels 
//    pSHState      pointer to the SH state structure initialized with maximal order not less then L   
*/
IPPAPI(IppStatus, ipprSHTInv_32f_C1,(const Ipp32f *pSrcClm, Ipp32u L, const Ipp32f *pX, const Ipp32f *pY,
                         const Ipp32f *pZ, Ipp32f *pDst, Ipp32u N, IpprSHState *pSHState))
IPPAPI(IppStatus, ipprSHTInv_32f_P3C3,(const Ipp32f *pSrcClm[3], Ipp32u L, const Ipp32f *pX, const Ipp32f *pY,
                         const Ipp32f *pZ, Ipp32f *pDst, Ipp32u N, IpprSHState *pSHState))
IPPAPI(IppStatus, ipprSHTInv_32f_P3,(const Ipp32f *pSrcClm[3], Ipp32u L, const Ipp32f *pX, const Ipp32f *pY,
                         const Ipp32f *pZ, Ipp32f *pDst[3], Ipp32u N, IpprSHState *pSHState))

#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPR_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */

