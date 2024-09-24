#pragma once
#include <bfc/platform/types.h>
#include <bfc/platform/export.h>
#include "nsutil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NSUTIL_EXPORTS
#define NSUTIL_EXPORT __declspec(dllexport)
#else
#define NSUTIL_EXPORT __declspec(dllimport)
#endif

enum 
{
    nsutil_resize_nearest_neighbour = 1,
    nsutil_resize_linear = 2,
    nsutil_resize_cubic  = 4,
    nsutil_resize_super_sampling   = 8,
    nsutil_resize_edge_subpixel = 0x40000000UL,
    nsutil_resize_edge_smooth   = 0x80000000UL,
};
typedef void *nsutil_resize_t;
//NSUTIL_EXPORT int nsutil_resize_Init_RGB(nsutil_resize_t *_context, const nsutil_rect *destination_rect, const nsutil_rect *source_rect, int resize_algorithm);

#ifdef __cplusplus
}
#endif