#include <precomp.h>
#include "profiler.h"

PtrListInsertSorted<__ProfilerEntry, __ProfilerEntrySort> __profiler_entries;

int __profiler_indent;
