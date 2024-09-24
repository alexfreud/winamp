#pragma once
#include "MP4MetadataBase.h"
#include "nx/nxuri.h"

 class MP4Metadata : public MP4MetadataBase
 {
 public:
	 MP4Metadata();
	 ~MP4Metadata();
	 int Initialize(nx_uri_t filename);

 private:
	 MP4FileHandle mp4_file;
 };