#pragma once
/* this parser is meant for retrieving metadata */
#include "read.h"
#include "avi_header.h"
#include "avi_reader.h"
#include "info.h"
#include "ParserBase.h"

namespace nsavi
{
	class Metadata : public ParserBase
	{
	public:	
		Metadata(nsavi::avi_reader *_reader);
		int GetDuration(int *time_ms);
		int GetHeaderList(HeaderList *header_list);
		int GetInfo(Info **info);

	private:
		/* INFO */
		Info *info;
		ParseState info_found;
	};
};
