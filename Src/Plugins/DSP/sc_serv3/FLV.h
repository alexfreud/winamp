#pragma once
#ifndef _FLV_HEADER_H
#define _FLV_HEADER_H

#include <stdio.h>
#include <vector>
#include "nmrCommon/intTypes.h"
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "streamData.h"

void createFLVTag(std::vector<__uint8> &out_data, const char *buf,
				  const int amt, int &timestamp, const bool mp3,
				  const bool mono, const unsigned int samplerate,
				  const int bitrate, const __uint8 *asc_header,
				  const streamData::streamID_t sid);

void createMetadataTag(std::vector<__uint8> &out_data, const bool mp3,
					   const bool mono, const int bitrate,
					   const __uint8 flv_sr, const streamData::streamID_t sid);

#endif
