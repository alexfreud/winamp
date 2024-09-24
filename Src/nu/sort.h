#pragma once

namespace nu
{
	void __cdecl qsort (
    void *base,
    size_t num,
    size_t width,
		const void *context,
    int (__fastcall *comp)(const void *, const void *, const void *));
};