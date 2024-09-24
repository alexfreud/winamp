#include "annexb.h"
#include <bfc/platform/types.h>

enum 
{
	InitialUnit = 0,
	NewUnit = 1, // start finding start code during AddData
	MidUnit = 2, // need to find the next start code from next AddData call to form a complete unit
	UnitReady = 3, // a new unit is ready and we are waiting for a GetUnit call
};

typedef struct annex_b_demuxer
{
	size_t buffer_position;
	size_t number_of_zero_words; // number of zero words as identified from the first unit
	size_t current_zero_words; // current zero word count, saved in case NALU crosses two AddData calls
	int end_of_stream; // set to 1 when there's no more data (so we know not to look for the next start code)
	int state;
	size_t buffer_size;
	uint8_t buffer[1]; // make sure this is last
} AnnexBDemuxer;

int AddData(const uint8_t **data, size_t *data_len); // data and length remaining are updated on exit.  if data_len>0 on exit, call again after calling GetUnit
void EndOfStream();


h264_annexb_demuxer_t AnnexB_Create(int size)
{
	AnnexBDemuxer *demuxer = (AnnexBDemuxer *)malloc(sizeof(AnnexBDemuxer) + size);
	demuxer->buffer_size = size; // MAX_CODED_FRAME_SIZE;
	demuxer->state = InitialUnit;
	demuxer->buffer_position = 0;
	demuxer->number_of_zero_words = 0;
	demuxer->current_zero_words = 0;
	demuxer->end_of_stream = 0;
	return (h264_annexb_demuxer_t)demuxer;
}

static int AnnexB_GetByte(const uint8_t **data, size_t *data_len, uint8_t *data_byte)
{
	if (*data_len)
	{
		*data_byte = **data;
		*data = *data + 1;
		*data_len = *data_len - 1;;
		return 1;
	}
	else
		return 0;
}

int AnnexB_AddData(h264_annexb_demuxer_t d, const void **_data, size_t *data_len)
{
	AnnexBDemuxer *demuxer = (AnnexBDemuxer *)d;
	if (demuxer)
	{
		const uint8_t **data = (const uint8_t **)_data; // cast to something easier to do pointer math with
		if (demuxer->state == InitialUnit)
		{
			// find start code with unknown number of initial zero bytes
			while(demuxer->number_of_zero_words == 0)
			{
				uint8_t data_byte;
				if (AnnexB_GetByte(data, data_len, &data_byte))
				{
					if (data_byte == 0)
					{
						demuxer->current_zero_words++;
					}
					else if (data_byte == 1 && demuxer->current_zero_words >= 2)
					{
						demuxer->number_of_zero_words = demuxer->current_zero_words;
						demuxer->current_zero_words = 0;
						demuxer->state = MidUnit;
					}
					else
					{
						// re-sync
						demuxer->current_zero_words = 0;
					}
				}
				else
				{
					return AnnexB_NeedMoreData;
				}
			}
		}
		else if (demuxer->state == NewUnit)
		{
			// find start code with known number of initial zero b ytes
			while (demuxer->state == NewUnit)
			{
				uint8_t data_byte;
				if (AnnexB_GetByte(data, data_len, &data_byte))
				{
					if (data_byte == 0)
					{
						demuxer->current_zero_words++;
					}
					else if (data_byte == 1 && demuxer->current_zero_words >= 2) // we might get more start words than required
					{
						demuxer->current_zero_words = 0;
						demuxer->state = MidUnit;
					}
					else
					{
						// re-sync
						demuxer->current_zero_words = 0;
					}
				}
				else
				{
					return AnnexB_NeedMoreData;
				}
			}
		}

		if (demuxer->state == MidUnit) // no else because we fall through during the start code scanning)
		{
			uint8_t data_byte;
			while (AnnexB_GetByte(data, data_len, &data_byte))
			{
				if (data_byte == 0)
				{
					demuxer->current_zero_words++; // might be the next start word
			/*		if (demuxer->current_zero_words == 3)  // 00 00 00 is also a valid sequence for end-of-nal detection.
					{
						demuxer->state = UnitReady;
						return AnnexB_UnitAvailable;
					}*/
				}
				else if (data_byte == 1 && demuxer->current_zero_words >= 2)
				{
					while (demuxer->current_zero_words > demuxer->number_of_zero_words)
					{
						// write trailing zero bytes to stream
						if (demuxer->buffer_position >= demuxer->buffer_size)
							return AnnexB_BufferFull;
						demuxer->buffer[demuxer->buffer_position++] = 0;
						demuxer->current_zero_words--;
					}
					demuxer->current_zero_words = 0;
					demuxer->state = UnitReady;
					return AnnexB_UnitAvailable;
				}
				else
				{
					while (demuxer->current_zero_words)
					{
						// write any zero bytes that we read to the stream
						if (demuxer->buffer_position >= demuxer->buffer_size)
							return AnnexB_BufferFull;
						demuxer->buffer[demuxer->buffer_position++] = 0;
						demuxer->current_zero_words--;
					}
					if (demuxer->buffer_position >= demuxer->buffer_size)
						return AnnexB_BufferFull;
					demuxer->buffer[demuxer->buffer_position++] = data_byte;
				}
			}

			if (demuxer->end_of_stream)
			{
				demuxer->state = UnitReady;
			}
			else
			{
				return AnnexB_NeedMoreData;
			}
		}

		if (demuxer->state == UnitReady)
			return AnnexB_UnitAvailable;

		return AnnexB_NeedMoreData; // dunno how we'd get here
	}
	else
		return AnnexB_Error;
}

void AnnexB_EndOfStream(h264_annexb_demuxer_t d)
{
	AnnexBDemuxer *demuxer = (AnnexBDemuxer *)d;
	if (demuxer)
		demuxer->end_of_stream = 1;
}

int AnnexB_GetUnit(h264_annexb_demuxer_t d, const void **data, size_t *data_len)
{
	AnnexBDemuxer *demuxer = (AnnexBDemuxer *)d;
	if (demuxer)
	{
		if (demuxer->state == UnitReady)
		{
			*data = demuxer->buffer;
			*data_len = demuxer->buffer_position;
			demuxer->buffer_position = 0;

			// if we've found the next start code, go to MidUnit state
			if (demuxer->current_zero_words == 0)
			{
				demuxer->state = MidUnit;
			}
			else // no start code, need to find it
			{
				demuxer->state = NewUnit;
			}
			return AnnexB_UnitAvailable;
		}
		else
		{
			return AnnexB_NeedMoreData;
		}
	}

	return AnnexB_Error;
}

void AnnexB_Destroy(h264_annexb_demuxer_t d)
{
	AnnexBDemuxer *demuxer = (AnnexBDemuxer *)d;
	if (demuxer)
		free(demuxer);
}