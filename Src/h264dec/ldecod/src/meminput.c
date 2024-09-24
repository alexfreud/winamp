#include "global.h"
#include "meminput.h"

void malloc_mem_input(VideoParameters *p_Vid)
{
  if ( (p_Vid->mem_input = (memory_input_t *) calloc(1, sizeof(memory_input_t))) == NULL)
  {
    snprintf(errortext, ET_SIZE, "Memory allocation for memory input failed");
    error(errortext,100);
  }  
}

void free_mem_input(VideoParameters *p_Vid)
{
  free(p_Vid->mem_input);
  p_Vid->mem_input = NULL;
}

/*!
************************************************************************
* \brief
*    returns a byte from IO buffer
************************************************************************
*/
static inline uint8_t getfbyte(memory_input_t *mem_input)
{
   return mem_input->user_buffer[mem_input->user_buffer_read++];
}


/*!
 ************************************************************************
 * \brief
 *    returns if new start code is found at byte aligned position buf.
 *    new-startcode is of form N 0x00 bytes, followed by a 0x01 byte.
 *
 *  \return
 *     1 if start-code is found or                      \n
 *     0, indicating that there is no start code
 *
 *  \param Buf
 *     pointer to byte-stream
 *  \param zeros_in_startcode
 *     indicates number of 0x00 bytes in start-code.
 ************************************************************************
 */
static inline int FindStartCode (unsigned char *Buf, int zeros_in_startcode)
{
  int i;

  for (i = 0; i < zeros_in_startcode; i++)
  {
    if(*(Buf++) != 0)
    {
      return 0;
    }
  }

  if(*Buf != 1)
    return 0;

  return 1;
}


/*!
 ************************************************************************
 * \brief
 *    Returns the size of the NALU (bits between start codes in case of
 *    Annex B.  nalu->buf and nalu->len are filled.  Other field in
 *    nalu-> remain uninitialized (will be taken care of by NALUtoRBSP.
 *
 * \return
 *     0 if there is nothing any more to read (EOF)
 *    -1 in case of any error
 *
 *  \note Side-effect: Returns length of start-code in bytes.
 *
 * \note
 *   GetAnnexbNALU expects start codes at byte aligned positions in the file
 *
 ************************************************************************
 */
int GetMemoryNALU (VideoParameters *p_Vid, NALU_t *nalu)
{
	memory_input_t *mem_input = p_Vid->mem_input;
	if (!mem_input->user_buffer)
		return 0;
	nalu->len = mem_input->user_buffer_size;
	memcpy(nalu->buf, mem_input->user_buffer, nalu->len);
	memzero16(nalu->buf+nalu->len); // add some extra 0's to the end
	nalu->forbidden_bit     = (*(nalu->buf) >> 7) & 1;
	nalu->nal_reference_idc = (NalRefIdc) ((*(nalu->buf) >> 5) & 3);
	nalu->nal_unit_type     = (NaluType) ((*(nalu->buf)) & 0x1f);
	nalu->lost_packets = 0;
	mem_input->user_buffer = 0;

	if (mem_input->skip_b_frames && nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE)
		return 0;

	if (mem_input->resetting && nalu->nal_unit_type != NALU_TYPE_IDR)
		return 0;

	mem_input->resetting = 0;

	return 1;
}


/*!
 ************************************************************************
 * \brief
 *    Opens the bit stream file named fn
 * \return
 *    none
 ************************************************************************
 */
void OpenMemory(VideoParameters *p_Vid, const char *fn)
{
  memory_input_t *mem_input = p_Vid->mem_input;
}


/*!
 ************************************************************************
 * \brief
 *    Closes the bit stream file
 ************************************************************************
 */
void CloseMemory(VideoParameters *p_Vid)
{
    memory_input_t *mem_input = p_Vid->mem_input;
}

