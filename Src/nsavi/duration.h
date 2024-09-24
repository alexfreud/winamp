#pragma once
/* this parser is meant for retrieving duration */
#include "read.h"
#include "avi_header.h"
#include "avi_reader.h"
#include "ParserBase.h"

namespace nsavi
{
	class Duration : public ParserBase
	{
	public:	
		Duration(nsavi::avi_reader *_reader);
		int GetDuration(int *time_ms);
int GetHeaderList(HeaderList *header_list);
			
	};
};
