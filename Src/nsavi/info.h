#pragma once
#include "avi_reader.h"
#include <map>

namespace nsavi
{
	typedef std::map<uint32_t, const char*> InfoMap;
	class Info : public InfoMap
	{
	public:
		Info();
		~Info();
		int Read(avi_reader* reader, uint32_t data_len);
		const char* GetMetadata(uint32_t id);

		void Set(uint32_t chunk_id, const char* data);
	};
};