#pragma once
#include <windows.h>
#include <bfc/platform/types.h>
#include "mkv_reader.h"

struct ebml_node
{
	uint64_t id;
	uint64_t size;
};

// returns bytes read.  0 means EOF
uint64_t read_ebml_node(nsmkv::MKVReader *reader, ebml_node *node);
uint64_t read_vint(nsmkv::MKVReader *reader, uint64_t *val);
uint64_t read_utf8(nsmkv::MKVReader *reader, uint64_t size, char **utf8);
uint64_t read_unsigned(nsmkv::MKVReader *reader, uint64_t size, uint64_t *val);
uint64_t read_float(nsmkv::MKVReader *reader, uint64_t size, double *val);
uint64_t read_signed(nsmkv::MKVReader *reader, uint64_t size, int64_t *val);