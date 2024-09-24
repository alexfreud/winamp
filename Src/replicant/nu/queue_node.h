#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef SLIST_ENTRY queue_node_t;
#ifdef __cplusplus
}
#endif

#else

#ifdef __cplusplus
extern "C" {
#endif

typedef struct queue_node_struct_t
{
	struct queue_node_struct_t *Next;
} queue_node_t;

#ifdef __cplusplus
}
#endif

#endif
