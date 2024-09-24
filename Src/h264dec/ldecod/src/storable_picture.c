#include "global.h"
#include "mbuffer.h"
#include "memalloc.h"

static void alloc_pic_motion(VideoParameters *p_Vid, PicMotionParams *motion, int size_y, int size_x)
{
	// TODO: benski> re-use memory just like for image data
	seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;  

	if (!active_sps->frame_mbs_only_flag)
	{
		get_mem3Dref(&(motion->field_references), 4, size_y, size_x);
	}
	else
	{
		motion->field_references = 0; // just in case
	}

	if (motion_cache_dimensions_match(&p_Vid->motion_cache, size_x, size_y))
	{
		motion->motion[LIST_0]=motion_cache_get(&p_Vid->motion_cache);
		motion->motion[LIST_1]=motion_cache_get(&p_Vid->motion_cache);
	}
	if (!motion->motion[LIST_0])
		get_mem2DPicMotion(&(motion->motion[LIST_0]), size_y, size_x);
	if (!motion->motion[LIST_1])
		get_mem2DPicMotion(&(motion->motion[LIST_1]), size_y, size_x);

	motion->mb_field = calloc (size_y * size_x, sizeof(byte));
	if (motion->mb_field == NULL)
		no_mem_exit("alloc_storable_picture: motion->mb_field");

	get_mem2D (&(motion->field_frame), size_y, size_x);
}

void free_pic_motion(VideoParameters *p_Vid, PicMotionParams *motion, int size_x, int size_y)
{
	if (motion->motion[LIST_0])
	{
		if (motion_cache_dimensions_match(&p_Vid->motion_cache, size_x / BLOCK_SIZE, size_y / BLOCK_SIZE))
		{
			motion_cache_add(&p_Vid->motion_cache,motion->motion[LIST_0]);
			motion_cache_add(&p_Vid->motion_cache,motion->motion[LIST_1]);
		}
		else
		{
			free_mem2DPicMotion(motion->motion[LIST_0]);
			free_mem2DPicMotion(motion->motion[LIST_1]);
		}
		motion->motion[LIST_0] = NULL;
		motion->motion[LIST_1] = NULL;
	}

	if (motion->field_references)
	{
		free_mem3Dref(motion->field_references);
		motion->field_references=0;
	}

  if (motion->mb_field)
  {
    free(motion->mb_field);
    motion->mb_field = NULL;
  }

  if (motion->field_frame)
  {
    free_mem2D (motion->field_frame);
    motion->field_frame=NULL;
  }
}


/*!
 ************************************************************************
 * \brief
 *    Free picture memory.
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param p
 *    Picture to be freed
 *
 ************************************************************************
 */
static void internal_free_storable_picture(VideoParameters *p_Vid, StorablePicture* p)
{
  int nplane;
  if (p)
  {

			free_pic_motion(p_Vid, &p->motion, p->size_x, p->size_y);

    //if( IS_INDEPENDENT(p_Vid) )
    {
      for( nplane=0; nplane<MAX_PLANE; nplane++ )
      {
        free_pic_motion(p_Vid, &p->JVmotion[nplane], p->size_x, p->size_y);
      }
    }

		if (image_cache_dimensions_match(&p_Vid->image_cache[0], p->size_x, p->size_y))
			image_cache_add(&p_Vid->image_cache[0], p->imgY);
		else
			free_memImage(p->imgY);

		if (image_cache_dimensions_match(&p_Vid->image_cache[1], p->size_x_cr, p->size_y_cr))
			image_cache_add(&p_Vid->image_cache[1], p->imgUV[0]);
		else
			free_memImage(p->imgUV[0]);

		if (image_cache_dimensions_match(&p_Vid->image_cache[1], p->size_x_cr, p->size_y_cr))
			image_cache_add(&p_Vid->image_cache[1], p->imgUV[1]);
		else
			free_memImage(p->imgUV[1]);

		if (p->slice_id)
		{
			free_mem2Dshort(p->slice_id);
			p->slice_id=NULL;
		}

		if (p->seiHasTone_mapping)
			free(p->tone_mapping_lut);

		_aligned_free(p);
		p = NULL;
  }
}

void free_storable_picture(VideoParameters *p_Vid, StorablePicture* p)
{
	if (p && --p->retain_count == 0)
	{
		internal_free_storable_picture(p_Vid, p);
	}
}


/*!
 ************************************************************************
 * \brief
 *    Allocate memory for a stored picture.
 *
 * \param p_Vid
 *      image decoding parameters for current picture
 * \param structure
 *    picture structure
 * \param size_x
 *    horizontal luma size
 * \param size_y
 *    vertical luma size
 * \param size_x_cr
 *    horizontal chroma size
 * \param size_y_cr
 *    vertical chroma size
 *
 * \return
 *    the allocated StorablePicture structure
 ************************************************************************
 */
#define ROUNDUP32(size) (((size)+31) & ~31)

StorablePicture* alloc_storable_picture(VideoParameters *p_Vid, PictureStructure structure, int size_x, int size_y, int size_x_cr, int size_y_cr)
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;  

  StorablePicture *s;
  int   nplane;

  //printf ("Allocating (%s) picture (x=%d, y=%d, x_cr=%d, y_cr=%d)\n", (type == FRAME)?"FRAME":(type == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", size_x, size_y, size_x_cr, size_y_cr);
  s = _aligned_malloc(sizeof(StorablePicture), 32);
  if (NULL==s)
    return 0;
	memset(s, 0, sizeof(StorablePicture));

	s->retain_count = 1;
	s->time_code = (uint64_t)-666;

  if (structure!=FRAME)
  {
    size_y    /= 2;
    size_y_cr /= 2;
  }

  s->PicSizeInMbs = (size_x*size_y)/256;

	if (image_cache_dimensions_match(&p_Vid->image_cache[0], size_x, size_y))
		s->imgY = image_cache_get(&p_Vid->image_cache[0]);
	if (!s->imgY)
		s->imgY = get_memImage(size_x, size_y);

	if (active_sps->chroma_format_idc != YUV400)
	{
		if (image_cache_dimensions_match(&p_Vid->image_cache[1], size_x_cr, size_y_cr))
		{
			s->imgUV[0] = image_cache_get(&p_Vid->image_cache[1]);
			s->imgUV[1] = image_cache_get(&p_Vid->image_cache[1]);
		}

		if (!s->imgUV[0])
			s->imgUV[0] = get_memImage(size_x_cr, size_y);
		if (!s->imgUV[1])
			s->imgUV[1] = get_memImage(size_x_cr, size_y);
	}
  
  get_mem2Dshort (&(s->slice_id), size_y / MB_BLOCK_SIZE, size_x / MB_BLOCK_SIZE);

  alloc_pic_motion(p_Vid, &s->motion, size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);

  if( IS_INDEPENDENT(p_Vid) )
  {
    for( nplane=0; nplane<MAX_PLANE; nplane++ )
    {
      alloc_pic_motion(p_Vid, &s->JVmotion[nplane], size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);
    }
  }

  s->structure=structure;

  s->size_x = size_x;
  s->size_y = size_y;
  s->size_x_cr = size_x_cr;
  s->size_y_cr = size_y_cr;
  s->size_x_m1 = size_x - 1;
  s->size_y_m1 = size_y - 1;
  s->size_x_cr_m1 = size_x_cr - 1;
  s->size_y_cr_m1 = size_y_cr - 1;

  s->top_field    = p_Vid->no_reference_picture;
  s->bottom_field = p_Vid->no_reference_picture;
  s->frame        = p_Vid->no_reference_picture;

  return s;
}

void out_storable_picture_add(VideoParameters *img, StorablePicture *pic)
{
	if (img->out_pictures)
	{
		// see if we're full
		if (img->size_out_pictures == img->num_out_pictures)
		{
			StorablePicture *pic=0;
			out_storable_picture_get(img, &pic);
			if (pic)
				free_storable_picture(img, pic);
		}

		img->out_pictures[img->num_out_pictures++] = pic;
		pic->retain_count++;
	}
}

void out_storable_picture_get(VideoParameters *img, StorablePicture **pic)
{
	*pic = 0;
	if (img->out_pictures && img->num_out_pictures)
	{
		*pic = img->out_pictures[0];
		img->num_out_pictures--;
		memmove(img->out_pictures, &img->out_pictures[1], img->num_out_pictures * sizeof(StorablePicture *));
	}
}

void out_storable_pictures_init(VideoParameters *img, size_t count)
{
	img->out_pictures = (StorablePicture **)calloc(sizeof(StorablePicture *), count);
	img->size_out_pictures = count;
	img->num_out_pictures = 0;
}

void out_storable_pictures_destroy(VideoParameters *img)
{
	size_t i=0;
	while (img->num_out_pictures)
	{
		StorablePicture *pic=0;
		out_storable_picture_get(img, &pic);
		if (pic)
			free_storable_picture(img, pic);
	}
	free(img->out_pictures);
	img->out_pictures = 0;
	img->size_out_pictures = 0;
}

