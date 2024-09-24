#include <precomp.h>

#include "drawpoly.h"
#include <bfc/parse/pathparse.h>

#define MAXPOINTS 32

static ARGB32 *bits, color;
static int w, h, across;
struct Point2d {
  int X, Y;
};
typedef struct Point2d Point2d; // bleh
static Point2d points[MAXPOINTS];
static int npoints;

void Draw::beginPolygon(ARGB32 *bits, int w, int h, ARGB32 color) {
  ::bits = bits;
  ::w = w;
  ::h = h;
  ::color = color;
  ::across = w;
  npoints = 0;
}

void Draw::addPoint(int x, int y) {
  if (npoints >= MAXPOINTS) return;
  points[npoints].X = x;
  points[npoints].Y = y;
  npoints++;
}

static void premultiply(ARGB32 *m_pBits, int nwords) {
  for (; nwords > 0; nwords--, m_pBits++) {
    unsigned __int8 *pixel = (unsigned __int8 *)m_pBits;
    unsigned int alpha = pixel[3];
    if (alpha == 255) continue;
    pixel[0] = (pixel[0] * alpha) >> 8;	// blue
    pixel[1] = (pixel[1] * alpha) >> 8;	// green
    pixel[2] = (pixel[2] * alpha) >> 8;	// red
  }
}

void Draw::drawPointList(ARGB32 *bits, int w, int h, const wchar_t *pointlist) {
  if (pointlist == NULL || *pointlist == '\0') return;
  PathParserW outer(pointlist, L"|");
  const wchar_t *pl;
  for (int i = 0; (pl = outer.enumString(i)) != NULL; i++) 
	{
    PathParserW inner(pl, L"=");
		ARGB32 color = WASABI_API_SKIN->parse(inner.enumStringSafe(1, L"255,255,255,255"), L"coloralpha");
    int a = color & 0xff000000;
    color = _byteswap_ulong(color<<8) | a;
    premultiply(&color, 1);
    beginPolygon(bits, w, h, color);
    PathParserW eener(inner.enumStringSafe(0, L"0,0"), L";");
    const wchar_t *cc;
    for (int j = 0; (cc = eener.enumString(j)) != NULL; j++) {
      PathParserW com(cc, L",");
      const wchar_t *xs = com.enumStringSafe(0, L"0");
      int x = wcschr(xs, '.') ? (int)floor(WTOF(xs) * w + .5f) : WTOI(xs);
      const wchar_t *ys = com.enumStringSafe(1, L"0");
      int y = wcschr(ys, '.') ? (int)floor(WTOF(ys) * h + .5f) : WTOI(ys);
      addPoint(x, y);
    }
    endPolygon();
  }
}

#define PIXEL ARGB32

// this originally came from Michael Abrash's Zen of Graphics Programming
// been modified a bit
/* DRAWPOLY.H: Header file for polygon-filling code */

/* Describes a single point (used for a single vertex) */

//struct Point2d {
//   int X;   /* X coordinate */
//   int Y;   /* Y coordinate */
//};

//typedef struct Point2d Point2d;

typedef struct {
  int X, Y;
} Point2dC;

/* Describes a series of points (used to store a list of vertices that
   describe a polygon; each vertex is assumed to connect to the two
   adjacent vertices, and the last vertex is assumed to connect to the
   first) */
struct Point2dListHeader {
   int Length;                /* # of points */
   struct Point2d *Point2dPtr;   /* pointer to list of points */
};

typedef struct Point2dListHeader Point2dListHeader;

/* Describes the beginning and ending X coordinates of a single
   horizontal line */
struct HLine {
   int XStart; /* X coordinate of leftmost pixel in line */
   int XEnd;   /* X coordinate of rightmost pixel in line */
};

typedef struct {
  int XStart, XEnd;
} HLineColor;

/* Describes a Length-long series of horizontal lines, all assumed to
   be on contiguous scan lines starting at YStart and proceeding
   downward (used to describe a scan-converted polygon to the
   low-level hardware-dependent drawing code) */
struct HLineList {
   int Length;                /* # of horizontal lines */
   int YStart;                /* Y coordinate of topmost line */
   struct HLine * HLinePtr;   /* pointer to list of horz lines */
};

static void DrawHorizontalLineList(struct HLineList * HLineListPtr, PIXEL *dest,
                                   PIXEL Color) {
   struct HLine *HLinePtr, *ptr;
   int Length, Width, c;
   PIXEL *ScreenPtr;

   /* Point to the start of the first scan line on which to draw */
   ScreenPtr = dest + HLineListPtr->YStart * across;
   Length = HLineListPtr->Length;
   /* Point to the XStart/XEnd descriptor for the first (top)
      horizontal line */
   HLinePtr = HLineListPtr->HLinePtr;

   /* clip left/right */
   for (ptr = HLinePtr, c = Length; c; c--) {
     if (ptr->XStart < 0) ptr->XStart = 0;
     if (ptr->XEnd >= w) ptr->XEnd = w - 1;
     ptr++;
   }
   /* clip top */
   if (HLineListPtr->YStart < 0) {
     int skip = -HLineListPtr->YStart;
     HLineListPtr->YStart = 0;
     ScreenPtr += across * skip;
     Length -= skip;
     HLinePtr += skip;
   }
   /* clip bottom */
   if (HLineListPtr->YStart + Length > h) {
     Length -= (HLineListPtr->YStart + Length) - h;
   }

   /* Draw each horizontal line in turn, starting with the top one and
      advancing one line each time */
   while (Length-- > 0) {
      /* Draw the whole horizontal line if it has a positive width */
      if ((Width = HLinePtr->XEnd - HLinePtr->XStart + 1) > 0)
//         bmemsetw(ScreenPtr+HLinePtr->XStart, Color, Width);
        MEMFILL<PIXEL>(ScreenPtr+HLinePtr->XStart, Color, Width);
      HLinePtr++;                /* point to next scan line X info */
      ScreenPtr += across; /* point to next scan line start */
   }
}

/* Scan converts an edge from (X1,Y1) to (X2,Y2), not including the
   point at (X2,Y2). If SkipFirst == 1, the point at (X1,Y1) isn't
   drawn; if SkipFirst == 0, it is. For each scan line, the pixel
   closest to the scanned edge without being to the left of the
   scanned edge is chosen. Uses an all-integer approach for speed and
   precision

   Link with L21-1.C, L21-3.C, and L22-1.C in Compact model.
   Tested with Borland C++ 4.02 by Jim Mischel 12/16/94.
*/

static void ScanEdge(int X1, int Y1, int X2, int Y2, int SetXStart,
                              int SkipFirst, struct HLine **EdgePoint2dPtr) {
   int DeltaX, Height, Width, AdvanceAmt, ErrorTerm, i;
   int ErrorTermAdvance, XMajorAdvanceAmt;
   struct HLine *WorkingEdgePoint2dPtr;

   WorkingEdgePoint2dPtr = *EdgePoint2dPtr; /* avoid double dereference */
   AdvanceAmt = ((DeltaX = X2 - X1) > 0) ? 1 : -1;
                            /* direction in which X moves (Y2 is
                               always > Y1, so Y always counts up) */

   if ((Height = Y2 - Y1) <= 0)  /* Y length of the edge */
      return;     /* guard against 0-length and horizontal edges */

   /* Figure out whether the edge is vertical, diagonal, X-major
      (mostly horizontal), or Y-major (mostly vertical) and handle
      appropriately */
   if ((Width = abs(DeltaX)) == 0) {
      /* The edge is vertical; special-case by just storing the same
         X coordinate for every scan line */
      /* Scan the edge for each scan line in turn */
      for (i = Height - SkipFirst; i-- > 0; WorkingEdgePoint2dPtr++) {
         /* Store the X coordinate in the appropriate edge list */
         if (SetXStart == 1)
            WorkingEdgePoint2dPtr->XStart = X1;
         else
            WorkingEdgePoint2dPtr->XEnd = X1;
      }
   } else if (Width == Height) {
      /* The edge is diagonal; special-case by advancing the X
         coordinate 1 pixel for each scan line */
      if (SkipFirst) /* skip the first point if so indicated */
         X1 += AdvanceAmt; /* move 1 pixel to the left or right */
      /* Scan the edge for each scan line in turn */
      for (i = Height - SkipFirst; i-- > 0; WorkingEdgePoint2dPtr++) {
         /* Store the X coordinate in the appropriate edge list */
         if (SetXStart == 1)
            WorkingEdgePoint2dPtr->XStart = X1;
         else
            WorkingEdgePoint2dPtr->XEnd = X1;
         X1 += AdvanceAmt; /* move 1 pixel to the left or right */
      }
   } else if (Height > Width) {
      /* Edge is closer to vertical than horizontal (Y-major) */
      if (DeltaX >= 0)
         ErrorTerm = 0; /* initial error term going left->right */
      else
         ErrorTerm = -Height + 1;   /* going right->left */
      if (SkipFirst) {   /* skip the first point if so indicated */
         /* Determine whether it's time for the X coord to advance */
         if ((ErrorTerm += Width) > 0) {
            X1 += AdvanceAmt; /* move 1 pixel to the left or right */
            ErrorTerm -= Height; /* advance ErrorTerm to next point */
         }
      }
      /* Scan the edge for each scan line in turn */
      for (i = Height - SkipFirst; i-- > 0; WorkingEdgePoint2dPtr++) {
         /* Store the X coordinate in the appropriate edge list */
         if (SetXStart == 1)
            WorkingEdgePoint2dPtr->XStart = X1;
         else
            WorkingEdgePoint2dPtr->XEnd = X1;
         /* Determine whether it's time for the X coord to advance */
         if ((ErrorTerm += Width) > 0) {
            X1 += AdvanceAmt; /* move 1 pixel to the left or right */
            ErrorTerm -= Height; /* advance ErrorTerm to correspond */
         }
      }
   } else {
      /* Edge is closer to horizontal than vertical (X-major) */
      /* Minimum distance to advance X each time */
      XMajorAdvanceAmt = (Width / Height) * AdvanceAmt;
      /* Error term advance for deciding when to advance X 1 extra */
      ErrorTermAdvance = Width % Height;
      if (DeltaX >= 0)
         ErrorTerm = 0; /* initial error term going left->right */
      else
         ErrorTerm = -Height + 1;   /* going right->left */
      if (SkipFirst) {   /* skip the first point if so indicated */
         X1 += XMajorAdvanceAmt;    /* move X minimum distance */
         /* Determine whether it's time for X to advance one extra */
         if ((ErrorTerm += ErrorTermAdvance) > 0) {
            X1 += AdvanceAmt;       /* move X one more */
            ErrorTerm -= Height; /* advance ErrorTerm to correspond */
         }
      }
      /* Scan the edge for each scan line in turn */
      for (i = Height - SkipFirst; i-- > 0; WorkingEdgePoint2dPtr++) {
         /* Store the X coordinate in the appropriate edge list */
         if (SetXStart == 1)
            WorkingEdgePoint2dPtr->XStart = X1;
         else
            WorkingEdgePoint2dPtr->XEnd = X1;
         X1 += XMajorAdvanceAmt;    /* move X minimum distance */
         /* Determine whether it's time for X to advance one extra */
         if ((ErrorTerm += ErrorTermAdvance) > 0) {
            X1 += AdvanceAmt;       /* move X one more */
            ErrorTerm -= Height; /* advance ErrorTerm to correspond */
         }
      }
   }

   *EdgePoint2dPtr = WorkingEdgePoint2dPtr;   /* advance caller's ptr */
}

/* Color-fills a convex polygon. All vertices are offset by (XOffset,
   YOffset). "Convex" means that every horizontal line drawn through
   the polygon at any point would cross exactly two active edges
   (neither horizontal lines nor zero-length edges count as active
   edges; both are acceptable anywhere in the polygon), and that the
   right & left edges never cross. (It's OK for them to touch, though,
   so long as the right edge never crosses over to the left of the
   left edge.) Nonconvex polygons won't be drawn properly. Returns 1
   for success, 0 if memory allocation failed.

   Compiled with Borland C++ 4.02.  Link with L21-3.C.
   Checked by Jim Mischel 11/30/94.
 */

/* Advances the index by one vertex forward through the vertex list,
   wrapping at the end of the list */
#define INDEX_FORWARD(Index) \
   Index = (Index + 1) % VertexList->Length;

/* Advances the index by one vertex backward through the vertex list,
   wrapping at the start of the list */
#define INDEX_BACKWARD(Index) \
   Index = (Index - 1 + VertexList->Length) % VertexList->Length;

/* Advances the index by one vertex either forward or backward through
   the vertex list, wrapping at either end of the list */
#define INDEX_MOVE(Index,Direction)                                  \
   if (Direction > 0)                                                \
      Index = (Index + 1) % VertexList->Length;                      \
   else                                                              \
      Index = (Index - 1 + VertexList->Length) % VertexList->Length;

int FillConvexPolygon(struct Point2dListHeader *VertexList, PIXEL *dest,
           PIXEL Color) {
   int i, MinIndexL, MaxIndex, MinIndexR, SkipFirst, Temp;
   int MinPoint2d_Y, MaxPoint2d_Y, TopIsFlat, LeftEdgeDir;
   int NextIndex, CurrentIndex, PreviousIndex;
   int DeltaXN, DeltaYN, DeltaXP, DeltaYP;
   struct HLineList WorkingHLineList;
   struct HLine *EdgePoint2dPtr;
   struct Point2d *VertexPtr;

   /* Point to the vertex list */
   VertexPtr = VertexList->Point2dPtr;

   /* Scan the list to find the top and bottom of the polygon */
   if (VertexList->Length == 0)
      return(1);  /* reject null polygons */
   MaxPoint2d_Y = MinPoint2d_Y = VertexPtr[MinIndexL = MaxIndex = 0].Y;
   for (i = 1; i < VertexList->Length; i++) {
      if (VertexPtr[i].Y < MinPoint2d_Y)
         MinPoint2d_Y = VertexPtr[MinIndexL = i].Y; /* new top */
      else if (VertexPtr[i].Y > MaxPoint2d_Y)
         MaxPoint2d_Y = VertexPtr[MaxIndex = i].Y; /* new bottom */
   }
   if (MinPoint2d_Y == MaxPoint2d_Y)
      return(1);  /* polygon is 0-height; avoid infinite loop below */

   /* Scan in ascending order to find the last top-edge point */
   MinIndexR = MinIndexL;
   while (VertexPtr[MinIndexR].Y == MinPoint2d_Y)
      INDEX_FORWARD(MinIndexR);
   INDEX_BACKWARD(MinIndexR); /* back up to last top-edge point */

   /* Now scan in descending order to find the first top-edge point */
   while (VertexPtr[MinIndexL].Y == MinPoint2d_Y)
      INDEX_BACKWARD(MinIndexL);
   INDEX_FORWARD(MinIndexL); /* back up to first top-edge point */

   /* Figure out which direction through the vertex list from the top
      vertex is the left edge and which is the right */
   LeftEdgeDir = -1; /* assume left edge runs down thru vertex list */
   if ((TopIsFlat = (VertexPtr[MinIndexL].X !=
         VertexPtr[MinIndexR].X) ? 1 : 0) == 1) {
      /* If the top is flat, just see which of the ends is leftmost */
      if (VertexPtr[MinIndexL].X > VertexPtr[MinIndexR].X) {
         LeftEdgeDir = 1;  /* left edge runs up through vertex list */
         Temp = MinIndexL;       /* swap the indices so MinIndexL   */
         MinIndexL = MinIndexR;  /* points to the start of the left */
         MinIndexR = Temp;       /* edge, similarly for MinIndexR   */
      }
   } else {
      /* Point to the downward end of the first line of each of the
         two edges down from the top */
      NextIndex = MinIndexR;
      INDEX_FORWARD(NextIndex);
      PreviousIndex = MinIndexL;
      INDEX_BACKWARD(PreviousIndex);
      /* Calculate X and Y lengths from the top vertex to the end of
         the first line down each edge; use those to compare slopes
         and see which line is leftmost */
      DeltaXN = VertexPtr[NextIndex].X - VertexPtr[MinIndexL].X;
      DeltaYN = VertexPtr[NextIndex].Y - VertexPtr[MinIndexL].Y;
      DeltaXP = VertexPtr[PreviousIndex].X - VertexPtr[MinIndexL].X;
      DeltaYP = VertexPtr[PreviousIndex].Y - VertexPtr[MinIndexL].Y;
      if (((long)DeltaXN * DeltaYP - (long)DeltaYN * DeltaXP) < 0L) {
         LeftEdgeDir = 1;  /* left edge runs up through vertex list */
         Temp = MinIndexL;       /* swap the indices so MinIndexL   */
         MinIndexL = MinIndexR;  /* points to the start of the left */
         MinIndexR = Temp;       /* edge, similarly for MinIndexR   */
      }
   }

   /* Set the # of scan lines in the polygon, skipping the bottom edge
      and also skipping the top vertex if the top isn't flat because
      in that case the top vertex has a right edge component, and set
      the top scan line to draw, which is likewise the second line of
      the polygon unless the top is flat */
   if ((WorkingHLineList.Length =
         MaxPoint2d_Y - MinPoint2d_Y - 1 + TopIsFlat) <= 0)
      return(1);  /* there's nothing to draw, so we're done */
   //WorkingHLineList.YStart = YOffset + MinPoint2d_Y + 1 - TopIsFlat;
   WorkingHLineList.YStart = MinPoint2d_Y + 1 - TopIsFlat;

   /* Get memory in which to store the line list we generate */
   if ((WorkingHLineList.HLinePtr =
         (struct HLine *) (malloc(sizeof(struct HLine) *
         WorkingHLineList.Length))) == NULL)
      return(0);  /* couldn't get memory for the line list */

   /* Scan the left edge and store the boundary points in the list */
   /* Initial pointer for storing scan converted left-edge coords */
   EdgePoint2dPtr = WorkingHLineList.HLinePtr;
   /* Start from the top of the left edge */
   PreviousIndex = CurrentIndex = MinIndexL;
   /* Skip the first point of the first line unless the top is flat;
      if the top isn't flat, the top vertex is exactly on a right
      edge and isn't drawn */
   SkipFirst = TopIsFlat ? 0 : 1;
   /* Scan convert each line in the left edge from top to bottom */
   do {
      INDEX_MOVE(CurrentIndex,LeftEdgeDir);
      ScanEdge(VertexPtr[PreviousIndex].X,
            VertexPtr[PreviousIndex].Y,
            VertexPtr[CurrentIndex].X,
            VertexPtr[CurrentIndex].Y, 1, SkipFirst, &EdgePoint2dPtr);
      PreviousIndex = CurrentIndex;
      SkipFirst = 0; /* scan convert the first point from now on */
   } while (CurrentIndex != MaxIndex);

   /* Scan the right edge and store the boundary points in the list */
   EdgePoint2dPtr = WorkingHLineList.HLinePtr;
   PreviousIndex = CurrentIndex = MinIndexR;
   SkipFirst = TopIsFlat ? 0 : 1;
   /* Scan convert the right edge, top to bottom. X coordinates are
      adjusted 1 to the left, effectively causing scan conversion of
      the nearest points to the left of but not exactly on the edge */
   do {
      INDEX_MOVE(CurrentIndex,-LeftEdgeDir);
      //ScanEdge(VertexPtr[PreviousIndex].X + XOffset - 1,
      ScanEdge(VertexPtr[PreviousIndex].X - 1,
            VertexPtr[PreviousIndex].Y,
	    //VertexPtr[CurrentIndex].X + XOffset - 1,
            VertexPtr[CurrentIndex].X - 1,
            VertexPtr[CurrentIndex].Y, 0, SkipFirst, &EdgePoint2dPtr);
      PreviousIndex = CurrentIndex;
      SkipFirst = 0; /* scan convert the first point from now on */
   } while (CurrentIndex != MaxIndex);

   /* Draw the line list representing the scan converted polygon */
   //CUT (*drawfn)(&WorkingHLineList, dest, Color, vc);
   DrawHorizontalLineList(&WorkingHLineList, dest, Color);

   /* Release the line list's memory and we're successfully done */
   free(WorkingHLineList.HLinePtr);
   return(1);
}

// done with abrashitude

void Draw::endPolygon() {
  if (npoints == 0) return;

  struct Point2dListHeader head;
  head.Length = npoints;
  head.Point2dPtr = &points[0];
  FillConvexPolygon(&head, bits, color);
}
