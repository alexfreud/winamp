#include "vint.h"
#include "header.h"
#include "ebml_float.h"
#include "segment.h"
#include "ebml_unsigned.h"
#include "ebml_signed.h"
#include <stdio.h>
#include <assert.h>
#include "SeekTable.h"
#include <time.h>
#include "parser.h"

#include "SegmentInfo.h"
#include "global_elements.h"
#include "Tracks.h"
#include "Cluster.h"
#include "Cues.h"
#include "Chapters.h"
#include "Tags.h"
#include "read.h"
#include "Attachments.h"

using namespace nsmkv;
static nsmkv::SeekTable seekTable;
static nsmkv::Header header;
static SegmentInfo segment_info;
static Tracks tracks;
static Cues cues;
static Attachments attachments;
static Tags tags;

uint64_t max_id_length = header.ebml_max_id_length;
uint64_t max_size_length = header.ebml_max_size_length;

uint32_t num_seekhead_elements_found = 0;
uint32_t num_seek_elements_found = 0;

bool ebml_segment_found = false;

uint64_t segment_data_offset = 0;

// returns bytes read.  0 means EOF
uint64_t read_vsint(FILE *f, int64_t *val)
{
	uint8_t data[9];
	size_t bytes_read = fread(data, 1, 1, f);
	if (bytes_read != 1)
		return 0;
	uint8_t length = vint_get_number_bytes(data[0]);
	bytes_read = fread(data+1, 1, length,  f);
	if (bytes_read != length)
		return 0;

	*val = vsint_read_ptr_len(length+1, data);
	return bytes_read+1;
}




char *read_utf8(FILE *f, size_t size)
{
	char *doctype = (char *)malloc(size + 1);
	if (doctype)
	{
		doctype[size]=0;
		if (fread(doctype, 1, size, f) == size)
			return doctype;

	}
	free(doctype);
	return 0;
}

// returns bytes read.  0 means EOF
uint64_t ReadSegment(FILE *f, uint64_t size)
{
	uint64_t total_bytes_read=0;

	// store the segment element data offset for later use
	segment_data_offset = ftell(f);
#ifdef WA_VALIDATE
		printf("[%I64u] Segment element data offset\n", segment_data_offset);
#endif

	while (size)
	{
		uint64_t this_position = ftell(f);
#ifdef WA_VALIDATE
		printf("[%I64u] ", this_position);
#endif
		ebml_node node;
		uint64_t bytes_read = read_ebml_node((nsmkv::MKVReader*)f, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_segment_attachments:
			{
				printf("  Attachments\n");
				ReadAttachment((nsmkv::MKVReader*)f, node.size, attachments);
			}
			break;
		case mkv_metaseek_seekhead:
			{
				printf("  SeekHead\n");
				ReadSeekHead((nsmkv::MKVReader*)f, node.size, seekTable);
			}
			break;
		case mkv_segment_segmentinfo:
			{
				printf("  SegmentInfo\n");
				ReadSegmentInfo((nsmkv::MKVReader*)f, node.size, segment_info);
			}
			break;
		case mkv_segment_tracks:
			{
				printf("  Tracks\n");
				ReadTracks((nsmkv::MKVReader*)f, node.size, tracks);
			}
			break;
		case mkv_segment_cues:
			{
				printf("  Cues\n");
				ReadCues((nsmkv::MKVReader*)f, node.size, cues);
			}
			break;
		case mkv_segment_cluster:
			{
				printf("  Clusters\n");
				nsmkv::Cluster cluster;
				ReadCluster((nsmkv::MKVReader*)f, node.size, cluster);
			}
			break;
		case mkv_segment_chapters:
			{
				printf("  Chapters\n");
			}
			break;
		case mkv_segment_tags:
			{
				printf("  Tags\n");
				nsmkv::ReadTags(f, node.size, tags);
			}
			break;
		default:
			ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}
	return total_bytes_read;
}

int main()
{
//	char *file_in = "\\\\o2d2\\ftp\\usr\\nullsoft\\test media\\mkv\\Ratatouille.2007.nHD.720.x264.NhaNc3.mkv";
	//char *file_in = "\\\\o2d2\\ftp\\usr\\nullsoft\\test media\\mkv\\cham_mp4v_aac.mkv";
	char *file_in = "c:/users/benski/desktop/metadata.mkv";

	FILE *f = fopen(file_in, "rb");
	if (f == NULL)
	{
		printf("****Error attempting to open file: %s\n",file_in);
		return -1;
	}
	else
	{
		printf("Starting Processing of File: %s\n",file_in);
	}

	ebml_node node;

	while (read_ebml_node((nsmkv::MKVReader*)f, &node))
	{
		switch(node.id)
		{
		case mkv_header:
			if (header.ebml_header_found == false)
			{
#ifdef WA_VALIDATE
				printf("MKV header found, processing...\n");
#endif
				header.ebml_header_found = true;
				nsmkv::ReadHeader((nsmkv::MKVReader*)f, node.size, header);
			}
			else
			{
#ifdef WA_VALIDATE
				printf("Extra MKV header found, ignoring...\n");
#endif
				nsmkv::Header extraHeader;
				nsmkv::ReadHeader((nsmkv::MKVReader*)f, node.size, extraHeader);
			}
			break;
		case mkv_segment:
			printf("MKV Segment element found, processing\n");
#ifdef WA_VALIDATE
			ebml_segment_found = true;
#endif
			ReadSegment(f, node.size);
			break;
		default:
			ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}

//	seekTable.Dump();

	fclose(f);

	printf("Number of SeekHead elements found: %I32u\n",num_seekhead_elements_found);
	printf("Number of Seek elements found: %I32u\n",num_seek_elements_found);

}
