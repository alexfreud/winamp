#pragma once
#include "MP4MetadataBase.h"
#include "nx/nxuri.h"
#include "nx/nxfile.h"

 class MP4MetadataFile : public MP4MetadataBase
 {
 public:
	 MP4MetadataFile();
	 ~MP4MetadataFile();
	 int Initialize(nx_uri_t filename, nx_file_t file);

 private:
	 MP4FileHandle mp4_file;
 };