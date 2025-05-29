#ifndef __HELPERS_H__
#define __HELPERS_H__
#include "stdbool.h"

#define TO_GLOBAL_PTR(obj, segmentedPtr) ((void *)((uintptr_t) + SEGMENT_OFFSET(segmentedPtr)))

bool isSegmentedPtr(void *p);

#endif
