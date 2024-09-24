#include "mkv_date.h"

__time64_t mkv_date_as_time_t(mkv_date_t val)
{
	__time64_t val_time = (__time64_t)val / 1000ULL /*nano->micro*/ / 1000ULL /*micro->milli*/ / 1000ULL /*milli->second*/;
	val_time+=978325200ULL;
	return val_time;
}