
/*!
 ************************************************************************
 * \file  memalloc.c
 *
 * \brief
 *    Memory allocation and free helper functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *
 ************************************************************************
 */

#include "global.h"
#include "memalloc.h"
#include "mbuffer.h"

#define ROUNDUP16(size) (((size)+15) & ~15)

#if !defined(USEMMX)
 /*!
 ************************************************************************
 * \brief
 *    Initialize 2-dimensional top and bottom field to point to the proper
 *    lines in frame
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int init_top_bot_planes(imgpel **imgFrame, int dim0, imgpel ***imgTopField, imgpel ***imgBotField)
{
  int i;

  if((*imgTopField   = (imgpel**) malloc((dim0>>1) * sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgTopField");

  if((*imgBotField   = (imgpel**) malloc((dim0>>1) * sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgBotField");

  for(i = 0; i < (dim0>>1); i++)
  {
    (*imgTopField)[i] =  imgFrame[2 * i    ];
    (*imgBotField)[i] =  imgFrame[2 * i + 1];
  }

  return dim0 * sizeof(imgpel*);
}

 /*!
 ************************************************************************
 * \brief
 *    free 2-dimensional top and bottom fields without freeing target memory
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
void free_top_bot_planes(imgpel **imgTopField, imgpel **imgBotField)
{
  free (imgTopField);
  free (imgBotField);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 1D memory array -> imgpel array1D[dim0
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem1Dpel(imgpel **array1D, int dim0)
{
  if((*array1D    = (imgpel*)calloc(dim0,       sizeof(imgpel))) == NULL)
    no_mem_exit("get_mem1Dpel: arra12D");

  return (sizeof(imgpel*) + dim0 * sizeof(imgpel));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> imgpel array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (imgpel**)malloc(dim0 * sizeof(imgpel*))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");
  if((*(*array2D) = (imgpel* )calloc(dim0 * dim1,sizeof(imgpel ))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(imgpel*) + dim1 * sizeof(imgpel));
}

VideoImage *get_memImage(int width, int height)
{
	int i, stride;
	VideoImage *image = (VideoImage *)calloc(1, sizeof(VideoImage));

#ifdef H264_IPP

	IppiSize roi = {width, height};
	if (!image)
		return 0;
	if((image->img   = (imgpel**)malloc(height * sizeof(imgpel*))) == NULL)
		return 0;

	image->base_address = (imgpel* )ippiMalloc_8u_C1(width, height+1, &stride); // height+1 so we can deal with overreading
	if (!image->base_address)
		return 0;

	image->stride=stride;

	for(i = 0 ; i < height; i++)
		image->img[i] = image->base_address + stride*i;

	image->next = 0;

	return image;
#else
	if (!image)
		return 0;
	stride = ROUNDUP16(width);
	image->stride = stride;

  if((image->img   = (imgpel**)malloc(height * sizeof(imgpel*))) == NULL)
    return 0;
  if((image->base_address = (imgpel* )malloc(stride * height* sizeof(imgpel))) == NULL)
    return 0;
	memset(image->base_address, 0, stride * height* sizeof(imgpel));

  for(i = 0 ; i < height; i++)
    image->img[i] = image->base_address + stride*i;

  return image; 
#endif
}

void free_memImage(VideoImage *image)
{
	free(image->img);
	#ifdef H264_IPP
	ippiFree(image->base_address);
	#else
	free(image->base_address);
	#endif
	free(image);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> imgpel array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(imgpel**);

  if(((*array3D) = (imgpel***)malloc(dim0 * sizeof(imgpel**))) == NULL)
    no_mem_exit("get_mem3Dpel: array3D");

  mem_size += get_mem2Dpel(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}
/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> imgpel array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3)
{  
  int  i, mem_size = dim0 * sizeof(imgpel***);

  if(((*array4D) = (imgpel****)malloc(dim0 * sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  mem_size += get_mem3Dpel(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}




/*!
 ************************************************************************
 * \brief
 *    free 1D memory array
 *    which was allocated with get_mem1Dpel()
 ************************************************************************
 */
void free_mem1Dpel(imgpel *array1D)
{
  if (array1D)
  {
    free (array1D);
  } 
  else
  {
    error ("free_mem1Dpel: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dpel()
 ************************************************************************
 */
void free_mem2Dpel(imgpel **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dpel: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dpel: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dpel()
 ************************************************************************
 */
void free_mem3Dpel(imgpel ***array3D)
{
  if (array3D)
  {
    free_mem2Dpel(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dpel: trying to free unused memory",100);
  }
}
/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dpel()
 ************************************************************************
 */
void free_mem4Dpel(imgpel ****array4D)
{
  if (array4D)
  {
    free_mem3Dpel(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dpel: trying to free unused memory",100);
  }
}
/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dpel()
 ************************************************************************
 */
void free_mem5Dpel(imgpel *****array5D)
{
  if (array5D)
  {
    free_mem4Dpel(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dpel: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> unsigned char array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2D(byte ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (byte**)malloc(dim0 *      sizeof(byte*))) == NULL)
    no_mem_exit("get_mem2D: array2D");
  if((*(*array2D) = (byte* )calloc(dim0 * dim1,sizeof(byte ))) == NULL)
    no_mem_exit("get_mem2D: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(byte*) + dim1 * sizeof(byte));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dint(int ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int**)malloc(dim0 *       sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if((*(*array2D) = (int* )calloc(dim0 * dim1, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(int*) + dim1 * sizeof(int));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int64 array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
static int get_mem2Dref(h264_ref_t ***array2D, int dim0, int dim1)
{
  int i;
	size_t malloc_size = ROUNDUP16(dim0 * dim1*sizeof(h264_ref_t));
  if((*array2D    = (h264_ref_t**)malloc(dim0 *      sizeof(h264_ref_t*))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
  if((*(*array2D) = (h264_ref_t* )_aligned_malloc(malloc_size, 32)) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
	memset((*array2D)[0], 0, malloc_size);

  for(i = 1; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(h264_ref_t*) + dim1 * sizeof(h264_ref_t));
}

int get_mem2DPicMotion(PicMotion ***array2D, int dim0, int dim1)
{
	// we allocate with one extra position in the first dimension
	// so the motion_cache can use it as a next pointer
  int i;
	size_t malloc_size = ROUNDUP16(dim0 * dim1*sizeof(PicMotion));
  if((*array2D    = (PicMotion**)malloc((dim0+1) *      sizeof(PicMotion*))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
  if((*(*array2D) = (PicMotion* )_aligned_malloc(malloc_size, 32)) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
	memset((*array2D)[0], 0, malloc_size);

  for(i = 1; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;
	(*array2D)[dim0] = 0;

  return dim0 * (sizeof(PicMotion*) + dim1 * sizeof(PicMotion));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> unsigned char array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3D(byte ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(byte**);

  if(((*array3D) = (byte***)malloc(dim0 * sizeof(byte**))) == NULL)
    no_mem_exit("get_mem3D: array3D");

  mem_size += get_mem2D(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> unsigned char array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4D(byte *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(byte***);

  if(((*array4D) = (byte****)malloc(dim0 * sizeof(byte***))) == NULL)
    no_mem_exit("get_mem4D: array4D");

  mem_size += get_mem3D(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(int**);

  if(((*array3D) = (int***)malloc(dim0 * sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Dint: array3D");

  mem_size += get_mem2Dint(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int64 array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dref(h264_ref_t ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(h264_ref_t**);

  if(((*array3D) = (h264_ref_t***)malloc(dim0 * sizeof(h264_ref_t**))) == NULL)
    no_mem_exit("get_mem3Dint64: array3D");

  mem_size += get_mem2Dref(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> int array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dint(int *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(int***);

  if(((*array4D) = (int****)malloc(dim0 * sizeof(int***))) == NULL)
    no_mem_exit("get_mem4Dint: array4D");

  mem_size += get_mem3Dint(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}




/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2D()
 ************************************************************************
 */
void free_mem2D(byte **array2D)
{
  if (array2D)
  {
    free (*array2D);
    free (array2D);
  } 
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint()
 ************************************************************************
 */
void free_mem2Dint(int **array2D)
{
  if (array2D)
  {
    free (*array2D);
    free (array2D);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint64()
 ************************************************************************
 */
void free_mem2Dref(h264_ref_t **array2D)
{
	if (array2D)
	{
		_aligned_free (*array2D);
		free (array2D);
	} 
}

void free_mem2DPicMotion(PicMotion **array2D)
{
	if (array2D)
	{
		_aligned_free (*array2D);
		free (array2D);
	} 
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3D()
 ************************************************************************
 */
void free_mem3D(byte ***array3D)
{
  if (array3D)
  {
   free_mem2D(*array3D);
   free (array3D);
  } 
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem3D()
 ************************************************************************
 */
void free_mem4D(byte ****array4D)
{
  if (array4D)
  {
   free_mem3D(*array4D);
   free (array4D);
  } 
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Dint(int ***array3D)
{
  if (array3D)
  {
   free_mem2Dint(*array3D);
   free (array3D);
  } 
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint64()
 ************************************************************************
 */
void free_mem3Dref(h264_ref_t ***array3D)
{
  if (array3D)
  {
   free_mem2Dref(*array3D);
   free (array3D);
  } 
}

void free_mem3DPicMotion(PicMotion ***array3D)
{
  if (array3D)
  {
   free_mem2DPicMotion(*array3D);
   free (array3D);
  } 
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dint()
 ************************************************************************
 */
void free_mem4Dint(int ****array4D)
{
  if (array4D)
  {
    free_mem3Dint( *array4D);
    free (array4D);
  } 
}


/*!
 ************************************************************************
 * \brief
 *    Exit program if memory allocation failed (using error())
 * \param where
 *    string indicating which memory allocation failed
 ************************************************************************
 */
void no_mem_exit(char *where)
{
   snprintf(errortext, ET_SIZE, "Could not allocate memory: %s",where);
   error (errortext, 100);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D uint16 memory array -> uint16 array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Duint16(uint16 ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (uint16**)malloc(dim0 *      sizeof(uint16*))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");

  if((*(*array2D) = (uint16* )calloc(dim0 * dim1,sizeof(uint16 ))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(uint16*) + dim1 * sizeof(uint16));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D short memory array -> short array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dshort(short ***array2D, int dim0, int dim1)
{
  int i;
	size_t malloc_size = ROUNDUP16(dim0 * dim1*sizeof(short));
  if((  *array2D  = (short**)malloc(dim0 *      sizeof(short*))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");
  if((*(*array2D) = (short* )_aligned_malloc(malloc_size, 32)) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");
	memset((*array2D)[0], 0, malloc_size);

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(short*) + dim1 * sizeof(short));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory short array -> short array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dshort(short ****array3D,int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(short**);

  if(((*array3D) = (short***)malloc(dim0 * sizeof(short**))) == NULL)
    no_mem_exit("get_mem3Dshort: array3D");

  mem_size += get_mem2Dshort(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory short array -> short array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
static MotionVector **get_mem2DMotionVector(int dim0, int dim1)
{
	MotionVector **array2D;
  int i;
	size_t malloc_size = ROUNDUP16(dim0 * dim1)*sizeof(MotionVector);
  if((array2D  = (MotionVector**)malloc(dim0 *      sizeof(MotionVector*))) == NULL)
    return 0;
	
  if((array2D[0] = (MotionVector* )_aligned_malloc(malloc_size, 32)) == NULL)
	{
		free(array2D);
    return 0;
	}
	memset(array2D[0], 0, malloc_size);

  for(i = 1; i < dim0; i++)
    array2D[i] = array2D[i-1] + dim1;

  return array2D;
}

MotionVector ***get_mem3DMotionVector(int dim0, int dim1, int dim2)
{
	MotionVector ***array3D;
  int  i;

  if((array3D = (MotionVector***)malloc(dim0 * sizeof(MotionVector **))) == NULL)
    return 0;

	array3D[0] = get_mem2DMotionVector(dim0 * dim1, dim2);
	if (!array3D[0])
	{
		free(array3D);
		return 0;
	}

  for(i = 1; i < dim0; i++)
    array3D[i] =  array3D[i-1] + dim1;

  return array3D;
}



/*!
 ************************************************************************
 * \brief
 *    free 2D uint16 memory array
 *    which was allocated with get_mem2Duint16()
 ************************************************************************
 */
void free_mem2Duint16(uint16 **array2D)
{
  if (array2D)
  {
    free (*array2D);
    free (array2D);
  } 

}

/*!
 ************************************************************************
 * \brief
 *    free 2D short memory array
 *    which was allocated with get_mem2Dshort()
 ************************************************************************
 */
void free_mem2Dshort(short **array2D)
{
  if (array2D)
  {
    _aligned_free (*array2D);
    free (array2D);
  } 
}

/*!
 ************************************************************************
 * \brief
 *    free 4D short memory array
 *    which was allocated with get_mem4Dshort()
 ************************************************************************
 */

static void free_mem2DMotionVector(MotionVector **array2D)
{
  if (array2D)
  {
    _aligned_free(*array2D);
    free (array2D);
  } 

}

void free_mem3DMotionVector(MotionVector ***array3D)
{  
  if (array3D)
  {
    free_mem2DMotionVector( *array3D);
    free (array3D);
  } 
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Ddouble(double ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D      = (double**)malloc(dim0 * sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");
  
  if(((*array2D)[0] = (double* )calloc(dim0 * dim1,sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(double*) + dim1 * sizeof(double));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dodouble(double ***array2D, int dim0, int dim1, int offset)
{
  int i;

  if((*array2D      = (double**)malloc(dim0 *       sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");
  if(((*array2D)[0] = (double* )calloc(dim0 * dim1, sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(double*) + dim1 * sizeof(double));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory double array -> double array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dodouble(double ****array3D, int dim0, int dim1, int dim2, int offset)
{
  int  i,j;

  if(((*array3D) = (double***)malloc(dim0 * sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  if(((*array3D)[0] = (double** )calloc(dim0 * dim1, sizeof(double*))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1  ;

  for (i = 0; i < dim0; i++)
    for (j = -offset; j < dim1 - offset; j++)
      if(((*array3D)[i][j] = (double* )calloc(dim2, sizeof(double))) == NULL)
        no_mem_exit("get_mem3Dodouble: array3D");

  return dim0*( sizeof(double**) + dim1 * ( sizeof(double*) + dim2 * sizeof(double)));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_offset_mem2Dshort(short ***array2D, int dim0, int dim1, int offset_y, int offset_x)
{
  int i;

  if((*array2D      = (short**)malloc(dim0 * sizeof(short*))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");

  if(((*array2D)[0] = (short* )calloc(dim0 * dim1, sizeof(short))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");
  (*array2D)[0] += offset_x + offset_y * dim1;

  for(i=-1 ; i > -offset_y - 1; i--)
  {
    (*array2D)[i] =  (*array2D)[i+1] - dim1;
  }

  for(i=1 ; i < dim1 - offset_y; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(short*) + dim1 * sizeof(short));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory int array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Doint(int ****array3D, int dim0, int dim1, int dim2, int offset)
{
  int  i,j;

  if(((*array3D) = (int***)malloc(dim0 * sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  if(((*array3D)[0] = (int** )calloc(dim0 * dim1, sizeof(int*))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1  ;

  for (i = 0; i < dim0; i++)
    for (j = -offset; j < dim1 - offset; j++)
      if(((*array3D)[i][j] = (int* )calloc(dim2, sizeof(int))) == NULL)
        no_mem_exit("get_mem3Doint: array3D");

  return dim0 * (sizeof(int**) + dim1 * (sizeof(int*) + dim2 * sizeof(int)));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Doint(int ***array2D, int dim0, int dim1, int offset)
{
  int i;

  if((*array2D      = (int**)malloc(dim0 * sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if(((*array2D)[0] = (int* )calloc(dim0 * dim1, sizeof(int))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(int*) + dim1 * sizeof(int));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
// same change as in get_mem3Dint
int get_mem3Ddouble(double ****array3D, int dim0, int dim1, int dim2)
{
	int  j, mem_size = dim0 * sizeof(double**);

  double **array2D;

  if(((*array3D) = (double***)malloc(dim0 * sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Ddouble: array3D");

  mem_size += get_mem2Ddouble(&array2D, dim0 * dim1, dim2);

  for(j = 0; j < dim0; j++)
  {    
    (*array3D)[j] = &array2D[j * dim1];
  }

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D double memory array
 *    which was allocated with get_mem2Ddouble()
 ************************************************************************
 */
void free_mem2Ddouble(double **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Ddouble: trying to free unused memory",100);

    free (array2D);

  }
  else
  {
    error ("free_mem2Ddouble: trying to free unused memory",100);
  }
}


/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Dodouble(double **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else error ("free_mem2Dodouble: trying to free unused memory",100);

    free (array2D);

  } else
  {
    error ("free_mem2Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Dodouble(double ***array3D, int dim0, int dim1, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < dim0; i++)
    {
      for (j = -offset; j < dim1 - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Dodouble: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Dodouble: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Doint(int ***array3D, int dim0, int dim1, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < dim0; i++)
    {
      for (j = -offset; j < dim1 - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Doint: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Doint: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Doint: trying to free unused memory",100);
  }
}


/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Doint(int **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_mem2Doint: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_mem2Doint: trying to free unused memory",100);
  }
}

/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_offset_mem2Dshort(short **array2D, int dim1, int offset_y, int offset_x)
{
  if (array2D)
  {
    array2D[0] -= offset_x + offset_y * dim1;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_offset_mem2Dshort: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_offset_mem2Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was alocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Ddouble(double ***array3D)
{
  if (array3D)
  {
    free_mem2Ddouble(*array3D);
    free (array3D);
  } 
  else
  {
    error ("free_mem3D: trying to free unused memory",100);
  }
}


#endif
