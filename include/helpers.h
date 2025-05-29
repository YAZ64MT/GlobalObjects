#ifndef __HELPERS_H__
#define __HELPERS_H__

#define TO_GLOBAL_PTR(obj, segmentedPtr) ((void *)((uintptr_t) + SEGMENT_OFFSET(segmentedPtr)))

#endif
